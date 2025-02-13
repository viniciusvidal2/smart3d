# README #

### How do I get set up? ###

Use Visual Studio 2017 or newer and CMake >= 3.14.

Libraries  
1 - wxWidgest >= 3.1.2 - You need to build it using the project wx_vcXX.sln (XX is your visual studio version) wich is inside the "wxWidgets-3.1.2\build\msw" folder.  
2 - VTK >= 8.2.0 - You need to build it adding the VR support, for this you need SDL2 >= 2.0.9 and OpenVR >= 1.5.17.  
3 - AdaptativeSolvers - Included in the repository, used to create a poisson/SSD reconstruction and the surface trimmer.  
4 - Claudette - Included in the repository, used to create point visibility.  
5 - AVTexturing (AliceVision texturing) - Included in the repository, used to texturize the mesh.  
6 - OpenVR >= 1.5.17 - Should be the same used to build VTK.  
7 - NSIS >= 3.04 - Used to create the installer.  

The compiled AVTexturing is available in the Downloads section.  

Dependencies  
1 - MeshRecon_Plus_TexRecon - Download it from the Downloads section.  
2 - installers - Download the .zip from the Downloads section and extract inside the SM.A.R.T.3D folder (SM.A.R.T.3D/installers/).  
3 - dependencies - Download the .zip from the Downloads section and extract inside the SM.A.R.T.3D folder (SM.A.R.T.3D/dependencies/).  

Help  
To create the Help you need to download HelpBlocks and go to tools and "generate wxHTML Help". After that select all the itens inside the folder and create a file Help.zip.   

### Using the SM.A.R.T3D ###

If you do not want to compile the project you just need to execute the "SM.A.R.T. 3D-x.x.x-win64" file. After this just execute the SM.A.R.T 3D.exe.  

### Creating the installer ###

After compiling the solution with CMake build the project PACKAGE.  