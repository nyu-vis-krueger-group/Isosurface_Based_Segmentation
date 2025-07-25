#!/usr/bin/env python3
"""
Sliding‑window iso‑value detector + channel‑combination histogram.

* Tile‑first, channel‑inside.
* Channels deduped (first occurrence, skip "do not use").
"""
import argparse, csv, math, subprocess, tempfile
from pathlib import Path
import re

import dask.array as da
import numpy as np
import tifffile as tiff
from ome_zarr.io import parse_url
import ome_types


# helpers
def load_zarr_volume(url, component="0"):
    root = parse_url(url, mode="r")
    return da.from_zarr(root.store, component=component)        


def fetch_channel_names(meta_url):
    import requests, io
    xml = requests.get(meta_url).text.replace("Â", "")
    return [c.name for c in ome_types.from_xml(xml).images[0].pixels.channels]


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


#main
def main():
    pa = argparse.ArgumentParser()
    pa.add_argument("--zarr", required=True)
    pa.add_argument("--meta", required=True)
    pa.add_argument("--exe",  required=True)
    pa.add_argument("--width",  type=int, default=202)
    pa.add_argument("--height", type=int, default=204)
    pa.add_argument("--out_iso",   default="iso_values.csv")
    pa.add_argument("--out_freq",  default="pattern_freq.csv")
    args = pa.parse_args()

    vol = load_zarr_volume(args.zarr)             
    _, _, Z, Y, X = vol.shape

    ch_list = dedup_channels(fetch_channel_names(args.meta))
    n_ch    = len(ch_list)
    print(f"[info] using {n_ch} unique channels:")
    for idx, name in ch_list:
        print(f"   {idx}: {name}")

    n_tiles_x = math.ceil(X / args.width)
    n_tiles_y = math.ceil(Y / args.height)
    print(f"[info] XY grid: {n_tiles_x} × {n_tiles_y} = {n_tiles_x*n_tiles_y} tiles")

    exe   = Path(args.exe).resolve()
    iso_rows   = []                                       
    global_counts = np.zeros(2 ** n_ch, dtype=np.uint64)   

    with tempfile.TemporaryDirectory() as tmpd:
        tmpd = Path(tmpd)

        # tile iter
        for y0 in range(0, Y, args.height):
            y1 = min(y0 + args.height, Y)
            for x0 in range(0, X, args.width):
                x1 = min(x0 + args.width, X)
                print(f"\n[Tile x:{x0}-{x1}, y:{y0}-{y1}]")

                tile_masks = []          

                # channel iter
                for ch_order, (ch_idx, ch_name) in enumerate(ch_list):
                    print(f"  • Ch {ch_idx} ({ch_name}): read …", end="", flush=True)
                    sub = vol[0, ch_idx, :, y0:y1, x0:x1].compute()
                    print(" done")

                    if not np.any(sub):          # skip if empty
                        print("      » empty → skipped exe")
                        tile_masks.append(np.zeros_like(sub, dtype=bool))
                        continue

                    # write tiff, calculate the isovalue
                    tif_f = tiff_path(tmpd, ch_idx, x0, y0)
                    print("      write TIFF …", end="", flush=True)
                    write_tiff(sub, tif_f); print(" done")

                    print("      run exe …", end="", flush=True)
                    try:
                        iso = float(subprocess.check_output(
                            [exe, str(tif_f)], text=True).strip())
                        print(f" done (iso={iso})")
                        iso_rows.append([ch_idx, sanitize(ch_name),
                                         x0, x1, y0, y1, iso])
                        mask = sub >= iso
                    except subprocess.CalledProcessError as e:
                        print(f" failed: {e}")
                        mask = np.zeros_like(sub, dtype=bool)
                    finally:
                        tif_f.unlink(missing_ok=True)

                    tile_masks.append(mask)

                # Counting frequencies of the combinations
                if not tile_masks:            
                    continue
                pattern = np.zeros(tile_masks[0].shape, dtype=np.uint64)  
                for bit, m in enumerate(tile_masks):
                    if bit >= 64:
                        raise RuntimeError(">64 channels; use something else for pattern dtype")
                    pattern |= (m.astype(np.uint64) << bit)

                counts = np.bincount(pattern.ravel(),
                                     minlength=2 ** n_ch).astype(np.uint64)
                global_counts += counts

    # save isovalue csv
    # with open(args.out_iso, "w", newline="") as f_iso:
    #     csv.writer(f_iso).writerows(
    #         [["chan_idx", "chan_name", "x0", "x1", "y0", "y1", "iso"], *iso_rows])
    # print(f"[done] {len(iso_rows)} iso rows → {args.out_iso}")

    # save the frequencies csv
    def decode(code):
        return "|".join(ch_list[i][1]           
                        for i in range(n_ch) if (code >> i) & 1)

    freq_rows = [["pattern_code", "channels", "count"]]
    for code, cnt in enumerate(global_counts):
        if cnt:
            freq_rows.append([code, decode(code), int(cnt)])

    with open(args.out_freq, "w", newline="") as f_freq:
        csv.writer(f_freq).writerows(freq_rows)
    print(f"[done] {len(freq_rows)-1} combinations → {args.out_freq}")


if __name__ == "__main__":
    main()
