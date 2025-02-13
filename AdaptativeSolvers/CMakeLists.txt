cmake_minimum_required(VERSION 3.14)
project(AdaptativeSolvers)

set_property (GLOBAL PROPERTY USE_FOLDERS ON)

option(BUILD_SHARED_LIBS "Build shared libraries (DLL)" ON)

include_directories(src)

set(FILES_TO_ADD src/*)

source_group("Headers" FILES ${FILES_TO_ADD})

# Sub-directories where more CMakeLists.txt exist
add_subdirectory (ZLIB)
add_subdirectory (JPEG)
add_subdirectory (PNG)
add_subdirectory (PoissonRecon)
add_subdirectory (SSDRecon)
add_subdirectory (SurfaceTrimmer)


target_link_libraries(PNG ZLIB)
target_link_libraries(PoissonRecon ZLIB)
target_link_libraries(PoissonRecon JPEG)
target_link_libraries(PoissonRecon PNG)
target_link_libraries(SSDRecon ZLIB)
target_link_libraries(SSDRecon JPEG)
target_link_libraries(SSDRecon PNG)