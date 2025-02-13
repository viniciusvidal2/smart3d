cmake_minimum_required(VERSION 3.14)
project(FastQuadricSimplification)

# Allow the user to build a static library
option(BUILD_SHARED_LIBS "Build shared libraries (DLL)" ON)
set(CMAKE_DEBUG_POSTFIX "d")

add_library(${PROJECT_NAME}    src/FastQuadricSimplification.cpp
			       src/FastQuadricSimplification.h
			       src/SymetricMatrix.cpp
			       src/SymetricMatrix.h
			       src/VecMath.h)

# Specific flags depending on platform
if(WIN32)
    add_definitions(-DNOMINMAX -DFOUG_OS_WINDOWS)
elseif(APPLE)
    add_definitions(-DFOUG_OS_MAC)
elseif(UNIX)
    add_definitions(-DFOUG_OS_UNIX)
endif()

if(BUILD_SHARED_LIBS)
    add_definitions(-DFASTQUADRICSIMPLIFICATION_LIB_DLL
                    -DFASTQUADRICSIMPLIFICATION_LIB_MAKE_DLL)
endif()

# Declare installs
install(FILES src/FastQuadricSimplification.h
        DESTINATION include)
install(TARGETS ${PROJECT_NAME}
          RUNTIME DESTINATION lib
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib)

message(STATUS "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")