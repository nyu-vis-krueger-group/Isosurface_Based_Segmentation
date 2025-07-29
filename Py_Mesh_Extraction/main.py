import argparse, csv, math, subprocess, tempfile
from pathlib import Path
import os
import re

import dask.array as da
import numpy as np
import tifffile as tiff
from ome_zarr.io import parse_url
import ome_types

"""
Takes a given channel.
Iterates over it tile by tile. (909x1377=48 tiles for res 0)
Extracts mesh for each tile.
Processes it.
Combines it with the previous tile combination.
"""

ISOSURFACE_EXTRACTION_EXE = "Isosurface_Extraction.exe"
MESH_PROCESSING_EXE = "Mesh_Processing.exe"
COMBINE_MESHES_EXE = "Combine_Meshes.exe"

def main():
    global ISOSURFACE_EXTRACTION_EXE, MESH_PROCESSING_EXE, COMBINE_MESHES_EXE

    pa = argparse.ArgumentParser()
    pa.add_argument("--zarr", required=True)
    pa.add_argument("--channel", required=True)
    pa.add_argument("--iso_exe",  type=str, default=ISOSURFACE_EXTRACTION_EXE)
    pa.add_argument("--proc_exe",  type=str, default=MESH_PROCESSING_EXE)
    pa.add_argument("--combine_exe", type=str, default=COMBINE_MESHES_EXE)
    pa.add_argument("--width",  type=int, default=909)
    pa.add_argument("--height", type=int, default=1377)
    args = pa.parse_args()

    path = args.zarr
    root = parse_url(path, mode="r")
    store = root.store
    daskArray = da.from_zarr(store, component="0") # t=0, high-res

    ISOSURFACE_EXTRACTION_EXE = args.iso_exe
    MESH_PROCESSING_EXE = args.proc_exe
    COMBINE_MESHES_EXE = args.combine_exe

    channel = int(args.channel)

    full_w, full_h = daskArray.shape[-1], daskArray.shape[-2]
    tile_w, tile_h = args.width, args.height

    n_tiles_x = math.ceil(full_w  / tile_w)
    n_tiles_y = math.ceil(full_h  / tile_h)
    print(f"[info] full channel {full_w}, {full_h}: {n_tiles_x}×{n_tiles_y} tiles")

    for y0 in range(0, full_h, tile_h):
        y1 = min(y0 + tile_h, full_h)
        for x0 in range(0, full_w, tile_w):
            x1 = min(x0 + tile_w, full_w)
            process_tile(daskArray, channel, x0, x1, y0, y1)



def process_tile(daskArray, channel, x0,x1,y0,y1):
    print("Extracting volume tile.")
    volume_tile = daskArray[0, channel, :, y0:y1, x0:x1].compute()
    print("Done\n")

    filename = f"{channel}_{x0}_{y0}_{x1}_{y1}.tiff"

    tiff.imwrite(filename, volume_tile, compression="zlib", bigtiff=True)

    # isosurface extraction
    try:
        subprocess.run([ISOSURFACE_EXTRACTION_EXE, filename, "EXTRACT"], check=True)
    except subprocess.CalledProcessError as e:
        print(f"isosurface extraction failed: {e}")
    finally:
        Path(filename).unlink() # delete the volume since the mesh has been extracted.

    # mesh processing
    try:
        subprocess.run([MESH_PROCESSING_EXE, filename+"_output.ply"], check=True)
    except subprocess.CalledProcessError as e:
        print(f"mesh processing failed: {e}")
    finally:
        Path(filename+"_output.ply").unlink() # delete the original isosurface mesh.

    # mesh combination
    if not (x0 == 0 and y0 == 0): # ignore first tile
        try:
            subprocess.run([COMBINE_MESHES_EXE, "union.off", filename+"_output.ply.off", str(x0), str(-y0)], check=True)
        except subprocess.CalledProcessError as e:
            print(f"combine meshes failed: {e}")
        finally:
            Path(filename+"_output.ply.off").unlink() # delete the processed mesh since we have combined.

    else: # if this is the first tile then we just rename this file into "union.off"
        os.rename(filename+"_output.ply.off", "union.off")


if __name__ == "__main__":
    main()