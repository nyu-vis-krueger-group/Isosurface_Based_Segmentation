@echo off
call .venv\Scripts\activate  || goto :error

set "ZARR=https://lsp-public-data.s3.amazonaws.com/biomedvis-challenge-2025/Dataset1-LSP13626-melanoma-in-situ/0"
set "META=https://lsp-public-data.s3.amazonaws.com/biomedvis-challenge-2025/Dataset1-LSP13626-melanoma-in-situ/OME/METADATA.ome.xml"
set "ISOVALUE_EXE=C:\Users\chaha\dev\bio_med_vis_2025\BioVis25\out\build\x64-Release\Isosurface_Extraction\Isosurface_Extraction.exe"

python pipeline.py ^
      --zarr "%ZARR%" ^
      --meta "%META%" ^
      --exe  "%ISOVALUE_EXE%" ^
      --width 909 ^
      --height 1377 ^
      --channels 19 42

python global_aggregate.py "pattern_freq.csv" "global_freq.csv"

goto :eof

:error
echo Virtual‑env activation failed.
exit /b 1
