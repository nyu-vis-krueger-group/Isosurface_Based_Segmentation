project(Heirarchical_Clustering LANGUAGES CXX)

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "")

find_package(Eigen3    REQUIRED NO_MODULE)    
find_package(CGAL     REQUIRED COMPONENTS Core)  


add_executable(Heirarchical_Clustering MACOSX_BUNDLE main.cpp)

target_include_directories(Heirarchical_Clustering
  PRIVATE
    ${Eigen3_INCLUDE_DIRS}
    ${CGAL_INCLUDE_DIRS}
)

target_link_libraries(Heirarchical_Clustering
  PRIVATE
    Eigen3::Eigen
    CGAL::CGAL
)