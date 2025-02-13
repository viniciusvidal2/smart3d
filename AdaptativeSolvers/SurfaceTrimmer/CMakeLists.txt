cmake_minimum_required(VERSION 3.14)
project(SurfaceTrimmer)

# Windows specific defines
if(WIN32)
  add_definitions(-DNOMINMAX)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  if(MSVC)
    add_definitions(/openmp)
  endif()
endif()

#VTK
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

# Declare library
add_library(${PROJECT_NAME} src/SurfaceTrimmer.cpp
			    src/SurfaceTrimmer.h)

add_definitions(-DSURFACETRIMMER_LIB_DLL
                -DSURFACETRIMMER_LIB_MAKE_DLL)

target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})