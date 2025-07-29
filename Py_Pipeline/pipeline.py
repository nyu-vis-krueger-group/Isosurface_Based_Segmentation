#!/usr/bin/env python3
"""
Sliding‑window iso‑value detector + channel‑combination histogram.

* Tile‑first, channel‑inside.
* Channels deduped (first occurrence, skip "do not use"), or user-specified by --channels.
* Outputs:
  - Per‑tile iso‑values CSV (--out_iso).
  - One big CSV of per‑tile pattern frequencies (--out_freq).
"""
import argparse, csv, math, subprocess, tempfile
from pathlib import Path
import re

import dask.array as da
import numpy as np
import tifffile as tiff
from ome_zarr.io import parse_url
import ome_types


def load_zarr_volume(url, component="0"):
    root = parse_url(url, mode="r")
    vol = da.from_zarr(root.store, component=component)
    assert vol.ndim == 5, f"Expected 5D volume, got {vol.ndim}D"
    return vol


def fetch_channel_names(meta_url):
    import requests
    try:
        xml = requests.get(meta_url).text.replace("Â", "")
        return [c.name for c in ome_types.from_xml(xml).images[0].pixels.channels]
    except Exception as e:
        raise RuntimeError(f"Failed to fetch channel names: {e}")


def dedup_channels(names):
    seen, keep = set(), []
    for idx, n in enumerate(names):
        if "do not use" in n.lower():
            continue
        if n not in seen:
            keep.append((idx, n))
            seen.add(n)
    return keep


def tiff_path(tmp_dir, ch_idx, x0, y0):
    return tmp_dir / f"tile_x{x0}_y{y0}_ch{ch_idx}.tiff"


def write_tiff(arr_zyx, path):
    if arr_zyx.dtype not in (np.uint8, np.uint16):
        arr_zyx = np.clip(arr_zyx, 0, np.iinfo(np.uint16).max).astype(np.uint16)
    tiff.imwrite(path, arr_zyx, compression="zlib", bigtiff=True)


def sanitize(name):
    return re.sub(r"[,\n\r]", "_", name)


def decode(code, ch_list):
    return "|".join(ch_list[i][1] for i in range(len(ch_list)) if (code >> i) & 1)


def main():
    pa = argparse.ArgumentParser()
    pa.add_argument("--zarr",   required=True, help="OME-Zarr URL or path")
    pa.add_argument("--meta",   required=True, help="URL to OME-XML metadata")
    pa.add_argument("--exe",    required=True, help="Path to iso-value extractor executable")
    pa.add_argument("--width",  type=int, default=202, help="Tile width in X")
    pa.add_argument("--height", type=int, default=204, help="Tile height in Y")
    pa.add_argument("--channels", type=int, nargs='+',
                    help="Optional list of channel indices to process directly")
    pa.add_argument("--out_iso",  default="iso_values.csv", help="Per‑tile isovalues CSV")
    pa.add_argument("--out_freq", default="pattern_freq.csv", help="Per‑tile pattern frequencies CSV")
    args = pa.parse_args()

    vol = load_zarr_volume(args.zarr)
    _, _, Z, Y, X = vol.shape

    all_names = fetch_channel_names(args.meta)
    if args.channels:
        ch_list = [(idx, all_names[idx]) for idx in args.channels]
        print(f"[info] using user-specified channels: {args.channels}")
    else:
        ch_list = dedup_channels(all_names)
        print(f"[info] deduped to {len(ch_list)} channels")
    n_ch = len(ch_list)

    
    if n_ch <= 16:
        pattern_dtype, bit_limit = np.uint16, 16
    elif n_ch <= 32:
        pattern_dtype, bit_limit = np.uint32, 32
    else:
        pattern_dtype, bit_limit = np.uint64, 64
    print(f"[info] encoding patterns with {bit_limit}-bit ({pattern_dtype.__name__}), {n_ch} channels")

    exe_path = Path(args.exe).resolve()
    iso_rows = []
    tile_rows = []  

    n_tiles_x = math.ceil(X / args.width)
    n_tiles_y = math.ceil(Y / args.height)
    print(f"[info] grid: {n_tiles_x}×{n_tiles_y} tiles over {X}×{Y}")

    with tempfile.TemporaryDirectory() as tmpd:
        tmpd = Path(tmpd)
        for y0 in range(0, Y, args.height):
            y1 = min(y0 + args.height, Y)
            for x0 in range(0, X, args.width):
                x1 = min(x0 + args.width, X)
                print(f"\n[Tile x:{x0}-{x1}, y:{y0}-{y1}]")
                tile_masks = []

                for bit, (ch_idx, ch_name) in enumerate(ch_list):
                    if bit >= bit_limit:
                        raise RuntimeError(f">{bit_limit} channels; cannot encode with {pattern_dtype}")

                    print(f"  • Ch {ch_idx} ({ch_name}): reading…", end="", flush=True)
                    sub = vol[0, ch_idx, :, y0:y1, x0:x1].compute()
                    print(" done")

                    if not sub.any():
                        print("      » empty → skip ISO")
                        tile_masks.append(np.zeros_like(sub, dtype=bool))
                        continue

                    tif_f = tiff_path(tmpd, ch_idx, x0, y0)
                    write_tiff(sub, tif_f)
                    print("      » TIFF written, running exe…", end="", flush=True)
                    try:
                        iso = float(subprocess.check_output([exe_path, str(tif_f)], text=True).strip())
                        print(f" iso={iso}")
                        iso_rows.append([ch_idx, sanitize(ch_name), x0, x1, y0, y1, iso])
                        mask = sub >= iso
                    except (subprocess.CalledProcessError, ValueError) as e:
                        print(f" failed ({e})")
                        mask = np.zeros_like(sub, dtype=bool)
                    finally:
                        tif_f.unlink(missing_ok=True)

                    tile_masks.append(mask)

                if not tile_masks:
                    continue

               
                pattern = np.zeros(tile_masks[0].shape, dtype=pattern_dtype)
                for bit, m in enumerate(tile_masks):
                    pattern |= (m.astype(pattern_dtype) << bit)

               
                flat = pattern.ravel()
                counts = np.bincount(flat, minlength=2 ** n_ch).astype(np.uint64)
                for code, cnt in enumerate(counts):
                    if cnt:
                        tile_rows.append([x0, x1, y0, y1, code,
                                          decode(code, ch_list), int(cnt)])

   
    with open(args.out_iso, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["chan_idx","chan_name","x0","x1","y0","y1","iso"])
        w.writerows(iso_rows)
    print(f"[done] wrote {len(iso_rows)} rows → {args.out_iso}")

    
    with open(args.out_freq, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["x0","x1","y0","y1","pattern_code","channels","count"] )
        w.writerows(tile_rows)
    print(f"[done] wrote {len(tile_rows)} rows → {args.out_freq}")


if __name__ == "__main__":
    main()