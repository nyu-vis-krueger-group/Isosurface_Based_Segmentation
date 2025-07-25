SET RELEASE_DIR="..\out\build\x64-release"

SET MESH_PROCESSING="%RELEASE_DIR%\Mesh_Processing\Mesh_Processing.exe"
SET ISOSURFACE_EXTRACTION="%RELEASE_DIR%\Isosurface_Extraction\Isosurface_Extraction.exe"
SET HEIRARCHICAL_CLUSTERING="%RELEASE_DIR%\Heirarchical_Clustering\Heirarchical_Clustering.exe"
SET SIMILARITY="..\py\build\exe.win-amd64-3.9"

COPY %MESH_PROCESSING% "Release"
COPY %ISOSURFACE_EXTRACTION% "Release"
COPY %HEIRARCHICAL_CLUSTERING% "Release"
XCOPY/s/e/y %SIMILARITY% "Release"

