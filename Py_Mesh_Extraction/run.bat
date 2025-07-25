echo off
call .venv\Scripts\activate  || goto :error

set "ZARR=https://lsp-public-data.s3.amazonaws.com/biomedvis-challenge-2025/Dataset1-LSP13626-melanoma-in-situ/0"
set "CHANNEL=1"

python main.py ^
      --zarr "%ZARR%" ^
      --channel "%CHANNEL%"
goto :eof

:error
echo Virtual‑env activation failed.
exit /b 1
