cmake_minimum_required(VERSION 3.14)
project(PoissonRecon)

# Windows specific defines
if(WIN32)
  add_definitions(-DNOMINMAX)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  if(MSVC)
    add_definitions(/bigobj)
    add_definitions(/openmp)
  endif()
endif()

#VTK
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

# Declare library
add_library(${PROJECT_NAME} src/PoissonRecon.cpp
			    src/PoissonRecon.h)

add_definitions(-DPOISSONRECON_LIB_DLL
                -DPOISSONRECON_LIB_MAKE_DLL)

target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})


