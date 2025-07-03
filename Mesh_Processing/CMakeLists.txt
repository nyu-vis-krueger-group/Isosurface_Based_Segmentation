# Created by the script cgal_create_CMakeLists
# This is the CMake script for compiling a set of CGAL applications.

project( Mesh_Processing )

include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

# CGAL and its components
find_package( CGAL COMPONENTS )

if ( NOT CGAL_FOUND )

  message(FATAL_ERROR "Mesh_Processing: This project requires the CGAL library, and will not be compiled.")
  return()  

endif()


# Boost and its components
find_package( Boost REQUIRED )

if ( NOT Boost_FOUND )

  message(FATAL_ERROR "Mesh_Processing: This project requires the Boost library, and will not be compiled.")

  return()  

endif()

# include for local directory
include_directories( BEFORE include )

# include for local package


# Creating entries for all C++ files with "main" routine
# ##########################################################




