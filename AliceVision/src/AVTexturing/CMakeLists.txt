# Headers
set(AVTEXTURING_FILE_HEADER
  AVTexturing.h
)

# Sources
set(AVTEXTURING_FILE_SRC
  AVTexturing.cpp
)

alicevision_add_library(AVTexturing
  SOURCES ${AVTEXTURING_FILE_HEADER} ${AVTEXTURING_FILE_SRC}
  PUBLIC_LINKS
	aliceVision_system
        aliceVision_mvsData
        aliceVision_mvsUtils
        aliceVision_mesh
        aliceVision_sfmData
        aliceVision_sfmDataIO
        ${Boost_LIBRARIES}
)
