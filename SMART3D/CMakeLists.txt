cmake_minimum_required(VERSION 3.14)
project(SM.A.R.T.3D)

add_definitions(/openmp)

#wxWidgets
set(wxWidgets_CONFIGURATION mswu)
find_package(wxWidgets COMPONENTS html core base REQUIRED)
include(${wxWidgets_USE_FILE})

#VTK
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

#AdaptativeSolvers
find_path(ADAPTATIVESOLVERS_INCLUDE_DIR src/AdaptiveTreeVisualization.cpp REQUIRED)
if (ADAPTATIVESOLVERS_INCLUDE_DIR)
    include_directories("${ADAPTATIVESOLVERS_INCLUDE_DIR}/PoissonRecon/src")
    include_directories("${ADAPTATIVESOLVERS_INCLUDE_DIR}/SSDRecon/src")
    include_directories("${ADAPTATIVESOLVERS_INCLUDE_DIR}/SurfaceTrimmer/src")
endif ()
find_path(ADAPTATIVESOLVERS_BUILD_DIR PoissonRecon/Release/PoissonRecon.dll REQUIRED)

#FastQuadricSimplification
find_path(FASTQUADRICSIMP_INCLUDE_DIR FastQuadricSimplification.h REQUIRED)
if (FASTQUADRICSIMP_INCLUDE_DIR)
    include_directories(${FASTQUADRICSIMP_INCLUDE_DIR})
endif ()
find_path(FASTQUADRICSIMP_BUILD_DIR Release/FastQuadricSimplification.dll REQUIRED)

#Claudette
find_path(CLAUDETTE_INSTALL_DIR "lib/claudette.dll" REQUIRED)
if (CLAUDETTE_INSTALL_DIR)
    include_directories("${CLAUDETTE_INSTALL_DIR}/include")
endif ()

#AVTexturing
find_path(AVTEXTURING_INCLUDE_DIR AVTexturing.h)
find_path(AVTEXTURING_LIB_RELEASE_DIR AVTexturing.dll)
find_path(AVTEXTURING_LIB_DEBUG_DIR AVTexturing.dll)
if (AVTEXTURING_INCLUDE_DIR)
    include_directories(${AVTEXTURING_INCLUDE_DIR})
endif ()

#OpenGL
find_package(OpenGL)
include_directories(${OPENGL_INCLUDE_DIRS})

#M3C2
find_path(M3C2_INSTALL_DIR "lib/M3C2.lib" REQUIRED)
if (M3C2_INSTALL_DIR)
    include_directories("${M3C2_INSTALL_DIR}/include")
endif ()

add_executable(${PROJECT_NAME}	src/Resource.rc
				src/AboutDialog.cpp
				src/AboutDialog.h
				src/AlignWithAxisTool.cpp
				src/AlignWithAxisTool.h
				src/AngleTool.cpp                                        
				src/AngleTool.h                                          
				src/AxisWidget.cpp                                       
				src/AxisWidget.h
				src/Calibration.h                                        
				src/Camera.cpp                                           
				src/Camera.h                                             
				src/captionActor2D.cpp                                   
				src/captionActor2D.h                                     
				src/CheckCamTool.cpp                                     
				src/CheckCamTool.h                                       
				src/ConfigurationDialog.cpp                              
				src/ConfigurationDialog.h                                
				src/DeleteTool.cpp                                       
				src/DeleteTool.h                                         
				src/DistanceTool.cpp                                     
				src/DistanceTool.h
				src/Draw.cpp
				src/Draw.h
				src/ElevationTool.cpp                                    
				src/ElevationTool.h                                      
				src/exif.cpp                                             
				src/exif.h                                               
				src/FrmPrincipal.cpp                                     
				src/FrmPrincipal.h                                       
				src/GPSData.cpp                                          
				src/GPSData.h                                            
				src/ImplicitPlaneWidget.cpp                              
				src/ImplicitPlaneWidget.h                                
				src/InteractorStyle.cpp                                  
				src/InteractorStyle.h                                    
				src/InteractorStyleVR.cpp                                
				src/InteractorStyleVR.h                                  
				src/LineWidget.cpp                                       
				src/LineWidget.h                                         
				src/LineWidgetRepresentation.cpp                         
				src/LineWidgetRepresentation.h                                                                
				src/Mesh.cpp                                             
				src/Mesh.h                                               
				src/OpenVRDeletePointsWidget.cpp                         
				src/OpenVRDeletePointsWidget.h                           
				src/OpenVRDeletePointsWidgetRepresentation.cpp           
				src/OpenVRDeletePointsWidgetRepresentation.h             
				src/OpenVRElevationWidget.cpp                            
				src/OpenVRElevationWidget.h                              
				src/OpenVRElevationWidgetRepresentation.cpp              
				src/OpenVRElevationWidgetRepresentation.h                
				src/OutputErrorWindow.cpp                                
				src/OutputErrorWindow.h                                                                           
				src/ProjetoMeshApp.cpp                                   
				src/ProjetoMeshApp.h                                     
				src/ReconstructionDialog.cpp                             
				src/ReconstructionDialog.h                               
				src/resource.h                                           
				src/Resource.rc                                          
				src/SliderRepresentation2D.cpp                                      
				src/SliderRepresentation2D.h                                        
				src/SnapshotDialog.cpp                                   
				src/SnapshotDialog.h                                     
				src/SnapshotTool.cpp                                     
				src/SnapshotTool.h                                                                               
				src/TransformDialog.cpp                            
				src/TransformDialog.h                       
				src/Utils.cpp
				src/Utils.h                                            
				src/ViewTool.cpp                                         
				src/ViewTool.h                                           
				src/ViewToolRepresentation.cpp                           
				src/ViewToolRepresentation.h                             
				src/Volume.cpp
				src/Volume.h
				src/VRDialog.cpp
				src/VRDialog.h
				src/wxVTKRenderWindowInteractor.cpp
				src/wxVTKRenderWindowInteractor.h
				src/HelperCOLMAP.cpp
				src/HelperCOLMAP.h
				src/HelperMeshRecon.cpp
				src/HelperMeshRecon.h
				src/UpdateCameraTool.cpp
				src/UpdateCameraTool.h
				src/DontShowAgainDialog.cpp
				src/DontShowAgainDialog.h
				src/ReconstructionLog.h
				src/ReconstructionLog.cpp
				src/ImageIO.cpp
				src/ImageIO.h
				src/MeshIO.cpp
				src/MeshIO.h
				src/AlignTool.cpp
				src/AlignTool.h
				src/ICP.cpp
				src/ICP.h
				src/ICPDialog.cpp
				src/ICPDialog.h
				src/PoissonDialog.cpp
				src/PoissonDialog.h
				src/SSDDialog.cpp
				src/SSDDialog.h
				src/SettingsDialog.cpp
				src/SettingsDialog.h
				src/VolumeTool.cpp
				src/VolumeTool.h
				src/M3C2Dialog.cpp
				src/M3C2Dialog.h)


target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES})

target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES})

target_link_libraries(${PROJECT_NAME} debug "${ADAPTATIVESOLVERS_BUILD_DIR}/PoissonRecon/Debug/PoissonRecon.lib" 
                                      optimized "${ADAPTATIVESOLVERS_BUILD_DIR}/PoissonRecon/Release/PoissonRecon.lib")

target_link_libraries(${PROJECT_NAME} debug "${ADAPTATIVESOLVERS_BUILD_DIR}/SSDRecon/Debug/SSDRecon.lib" 
                                      optimized "${ADAPTATIVESOLVERS_BUILD_DIR}/SSDRecon/Release/SSDRecon.lib")

target_link_libraries(${PROJECT_NAME} debug "${ADAPTATIVESOLVERS_BUILD_DIR}/SurfaceTrimmer/Debug/SurfaceTrimmer.lib" 
                                      optimized "${ADAPTATIVESOLVERS_BUILD_DIR}/SurfaceTrimmer/Release/SurfaceTrimmer.lib")

target_link_libraries(${PROJECT_NAME} debug "${FASTQUADRICSIMP_BUILD_DIR}/Debug/FastQuadricSimplificationd.lib" 
                                      optimized "${FASTQUADRICSIMP_BUILD_DIR}/Release/FastQuadricSimplification.lib")

target_link_libraries(${PROJECT_NAME} "${CLAUDETTE_INSTALL_DIR}/lib/claudette.lib")

target_link_libraries(${PROJECT_NAME} debug "${AVTEXTURING_LIB_RELEASE_DIR}/AVTexturing.lib" 
                                      optimized "${AVTEXTURING_LIB_DEBUG_DIR}/AVTexturing.lib")

target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})

target_link_libraries(${PROJECT_NAME} debug "${M3C2_INSTALL_DIR}/lib/M3C2d.lib" 
                                      optimized "${M3C2_INSTALL_DIR}/lib/M3C2.lib")


set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

add_definitions(-DNOMINMAX
		-D_SCR_SECURE_NO_WARNINGS
		-D_CRT_SECURE_NO_WARNINGS)

#INSTALL
find_file(OPENVR_RELEASE_DLL "openvr_api64.dll" REQUIRED)

INSTALL(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
INSTALL(DIRECTORY dependencies/ DESTINATION bin)
INSTALL(DIRECTORY installers/ DESTINATION dep)
INSTALL(FILES ../HelpBlocksProject/Help.zip DESTINATION bin)
file(GLOB DLLS_VTK_RELEASE "${VTK_DIR}/bin/Release/*.dll")
INSTALL(FILES ${DLLS_VTK_RELEASE} DESTINATION bin)
INSTALL(FILES ${OPENVR_RELEASE_DLL} DESTINATION bin)
INSTALL(FILES "${ADAPTATIVESOLVERS_BUILD_DIR}/PoissonRecon/Release/PoissonRecon.dll" DESTINATION bin)
INSTALL(FILES "${ADAPTATIVESOLVERS_BUILD_DIR}/SSDRecon/Release/SSDRecon.dll" DESTINATION bin)
INSTALL(FILES "${ADAPTATIVESOLVERS_BUILD_DIR}/SurfaceTrimmer/Release/SurfaceTrimmer.dll" DESTINATION bin)
INSTALL(FILES "${FASTQUADRICSIMP_BUILD_DIR}/Release/FastQuadricSimplification.dll" DESTINATION bin)
INSTALL(FILES "${CLAUDETTE_INSTALL_DIR}/lib/claudette.dll" DESTINATION bin)
file(GLOB DLLS_AVTEXTURING_RELEASE "${AVTEXTURING_LIB_RELEASE_DIR}/*.dll")
INSTALL(FILES ${DLLS_AVTEXTURING_RELEASE} DESTINATION bin)
INSTALL(FILES programIcon.ico DESTINATION bin)

#
# CPACK macros below here
#
set (CPACK_PACKAGE_NAME "SM.A.R.T. 3D")
set (CPACK_PACKAGE_VENDOR "INESC P&D Brasil")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "SM.A.R.T. 3D - Surface/Mesh Analysis and Reconstruction Tool")
set (CPACK_PACKAGE_VERSION "1.9.0")
set (CPACK_PACKAGE_VERSION_MAJOR "1")
set (CPACK_PACKAGE_VERSION_MINOR "9")
set (CPACK_PACKAGE_VERSION_PATCH "0")
set (CPACK_PACKAGE_INSTALL_DIRECTORY "SM.A.R.T.3D")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/license.txt")

set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
    CreateShortCut \\\"$DESKTOP\\\\${PROJECT_NAME}.lnk\\\" \\\"$INSTDIR\\\\bin\\\\${PROJECT_NAME}.exe\\\"
")

set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
    Delete \\\"$DESKTOP\\\\${PROJECT_NAME}.lnk\\\"
")

list(APPEND CPACK_NSIS_EXTRA_INSTALL_COMMANDS " 
               ExecWait '$INSTDIR\\\\dep\\\\vcredist_x64_2017.exe'
               ")
 
# Must be after the last CPACK macros
include(CPack)

add_definitions(-DMART_VERSION_MAJOR=${CPACK_PACKAGE_VERSION_MAJOR}
		-DMART_VERSION_MINOR=${CPACK_PACKAGE_VERSION_MINOR}
		-DMART_VERSION_PATCH=${CPACK_PACKAGE_VERSION_PATCH})


