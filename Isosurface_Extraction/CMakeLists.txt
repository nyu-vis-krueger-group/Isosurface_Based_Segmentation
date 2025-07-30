project(Isosurface_Extraction)

find_package(VTK COMPONENTS
      CommonCore
      CommonDataModel
      FiltersExtraction
      IOImage
      IOExport
      IOPLY
      IOGeometry
      ImagingGeneral
)

set(IO 
    "include/IO/ReadTIFF.h"
    "include/IO/ReadVTI.h"
    "include/IO/WriteOBJ.h"
    "include/IO/WriteCSV.h"
    "include/IO/WriteVTK.h"
    "include/IO/WritePLY.h"
    "src/IO/ReadTIFF.cpp"
    "src/IO/ReadVTI.cpp"
    "src/IO/WriteOBJ.cpp"
    "src/IO/WriteCSV.cpp"
    "src/IO/WriteVTK.cpp"
    "src/IO/WritePLY.cpp"
)

set(EXTRACTION 
    "include/IsosurfaceExtraction/FlyingEdges3D.h"
    "src/IsosurfaceExtraction/FlyingEdges3D.cpp"
)

set(PROCESSING 
    "include/Processing/CummulativeLaplacianHistogram.h"
    "src/Processing/CummulativeLaplacianHistogram.cpp"
)

set(SOURCE_DIRS
    ${IO} 
    ${EXTRACTION} 
    ${PROCESSING}
    "main.cpp"
)

set(INCLUDE_DIRS
    "include"
)

if (NOT VTK_FOUND)
  message(FATAL_ERROR "Isosurface_Extraction: Unable to find the VTK build folder.")
endif()


set(CMAKE_NINJA_FORCE_RESPONSE_FILE "ON" CACHE BOOL "Force Ninja to use response files.")

add_executable(Isosurface_Extraction MACOSX_BUNDLE ${SOURCE_DIRS})

if(UNIX AND NOT APPLE)
  target_link_libraries(Isosurface_Extraction
    PRIVATE
      ${VTK_LIBRARIES}
      -static-libstdc++
      -static-libgcc
      -Wl,-Bstatic
      -lc
      -lm
      -lpthread
      -lrt
      -ldl
      -Wl,-Bdynamic
  )
else()
  target_link_libraries(Isosurface_Extraction
    PRIVATE
      ${VTK_LIBRARIES}
  )
endif()

target_include_directories(Isosurface_Extraction PRIVATE ${INCLUDE_DIRS})

vtk_module_autoinit(
  TARGETS Isosurface_Extraction
  MODULES ${VTK_LIBRARIES}
)
