project(Heirarchical_Clustering)


set(SOURCE_DIRS
	"main.cpp"
)

set(INCLUDE_DIRS
	$ENV{LIBIGL_INCLUDE_DIR}
)


set(CMAKE_NINJA_FORCE_RESPONSE_FILE "ON" CACHE BOOL "Force Ninja to use response files.")
add_executable(Heirarchical_Clustering MACOSX_BUNDLE ${SOURCE_DIRS} )

target_include_directories(Isosurface_Extraction PRIVATE ${INCLUDE_DIRS})