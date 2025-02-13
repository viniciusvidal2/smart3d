#ifdef AVTEXTURING_EXPORTS
#define AVTEXTURING_API __declspec(dllexport)
#else
#define AVTEXTURING_API __declspec(dllimport)
#endif

#ifndef __AVTEXTURING__H__
#define __AVTEXTURING__H__

#include <vector>
#include <map>

// Used to define the texturization options
struct OptionsAVTexturing
{
    std::string unwrapMethod = "Basic";         //"Basic", "ABF" (< 600k faces) or "LSCM" (< 600k faces)
    std::string outTextureFileTypeName = "png"; //"png", "jpg", "tif" or "exr"
    int multiBandDownscale = 4;
    double bestScoreThreshold = 0.1;            //< 0.0 to disable filtering based on threshold to relative best score
    double angleHardThreshold = 90.0;           //< 0.0 to disable angle hard threshold filtering
    bool forceVisibleByAllVertices = false;     //< triangle visibility is based on the union of vertices visiblity
    int visibilityRemappingMethod = 3;          // 1 - Pull 2 - Push 3 - Pullpush

    unsigned int textureSide = 8192;
    unsigned int padding = 5;
    unsigned int downscale = 2;
    bool fillHoles = false;
    bool useUDIM = true;
    bool flipNormals = false;
};

struct ViewAux
{
    unsigned int viewId;
    unsigned int poseId;
    unsigned int intrinsicId;
    std::string path;
    unsigned int width;
    unsigned int height;
};

struct IntrinsicAux
{
    unsigned int intrinsicId;
    unsigned int width;
    unsigned int height;
    double pxFocalLength;
    double principalPoint[2];
    std::vector<double> distortionParams;
};

struct PoseAux
{
    unsigned int poseId;
    double rotation[9];
    double center[3];
};

struct LandmarkAux
{
    double X[3];
    std::vector<unsigned int> observations;
};

struct CameraParameters
{
    std::map<unsigned int, ViewAux*> views;
    std::map<unsigned int, IntrinsicAux*> instrinsics;
    std::map<unsigned int, PoseAux*> poses;
    std::vector<LandmarkAux*> landmarks;
};

struct VertexAux
{
    float p[3];
    float normal[3];
    // Vertex index in the vector that contains all vertices
    unsigned int vertex_index;
};

struct FaceAux
{
    // Used to know wich vertices make this face
    std::vector<VertexAux*> vertices;
};

struct MeshAux
{
    std::vector<VertexAux*> points;
    std::vector<FaceAux*> polygons;
};


// This class is exported from the AVTexturing.dll
class AVTEXTURING_API AVTexturing
{
public:
    AVTexturing();
    /*Create the texture for a mesh
    inputMesh - a mesh with points and faces
    inputCameraParams - camera intrinsics, extrinsics, poses and landmarks
    options - texture parameters
    outputPath - filename of the .obj that will be saved
    */
    void applyTexture(MeshAux* inputMesh, CameraParameters* inputCameraParams, OptionsAVTexturing* options, std::string outputPath);

private:
};

#endif