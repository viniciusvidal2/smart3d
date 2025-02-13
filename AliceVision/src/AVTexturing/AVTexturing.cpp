// This file is part of the AliceVision project.
// Copyright (c) 2017 AliceVision contributors.
// This Source Code Form is subject to the terms of the Mozilla Public License,
// v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#include <aliceVision/sfmData/SfMData.hpp>
#include <aliceVision/sfmDataIO/sfmDataIO.hpp>
#include <aliceVision/mesh/Mesh.hpp>
#include <aliceVision/mesh/Texturing.hpp>
#include <aliceVision/mesh/meshVisibility.hpp>
#include <aliceVision/mvsData/image.hpp>
#include <aliceVision/mvsUtils/common.hpp>
#include <aliceVision/mvsUtils/MultiViewParams.hpp>
#include <aliceVision/system/cmdline.hpp>
#include <aliceVision/system/Logger.hpp>
#include <aliceVision/system/Timer.hpp>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "AVTexturing.h"

// These constants define the current software version.
// They must be updated when the command line is changed.
#define ALICEVISION_SOFTWARE_VERSION_MAJOR 3
#define ALICEVISION_SOFTWARE_VERSION_MINOR 0

using namespace aliceVision;

namespace fs = boost::filesystem;
namespace po = boost::program_options;

bfs::path absolutePathNoExt(const bfs::path& p)
{
    return p.parent_path() / p.stem();
}

void loadMeshfromMeshAux(MeshAux* inputMesh, aliceVision::mesh::Texturing* mesh, bool flipNormals)
{
    mesh->me = new aliceVision::mesh::Mesh();
    mesh->me->pts = new StaticVector<Point3d>();
    mesh->me->tris = new StaticVector<aliceVision::mesh::Mesh::triangle>();

    mesh->normals.reserve(inputMesh->points.size());
    mesh->me->pts->reserve(inputMesh->points.size());
    mesh->me->tris->reserve(inputMesh->polygons.size());

    for(auto point : inputMesh->points)
    {
        Point3d p(point->p[0], point->p[1], point->p[2]);
        Point3d n(point->normal[0], point->normal[1], point->normal[2]);
        mesh->me->pts->push_back(p);
        mesh->normals.push_back(n);
    }
    for(auto face : inputMesh->polygons)
    {
        aliceVision::mesh::Mesh::triangle tri(face->vertices.at(0)->vertex_index, face->vertices.at(1)->vertex_index,
                                              face->vertices.at(2)->vertex_index);
        mesh->me->tris->push_back(tri);
    }

    // Handle normals flipping
    if(flipNormals)
        mesh->me->invertTriangleOrientations();

    // Fill atlases (1 atlas per material) with corresponding rectangles
    // if no material, create only one atlas with all triangles
    mesh->_atlases.resize(1);
    for(int triangleID = 0; triangleID < inputMesh->polygons.size(); triangleID++)
    {
        mesh->_atlases[0].push_back(triangleID);
    }
}

void loadSFMDataFromCameraParameters(sfmData::SfMData& sfmData, CameraParameters* inputCameraParams)
{
    // Remove the views with no pose
    for(auto pair : inputCameraParams->views)
    {
        ViewAux* view = pair.second;
        if(inputCameraParams->poses.find(view->poseId) == inputCameraParams->poses.end())
        {
            inputCameraParams->views.erase(view->viewId);
        }
    }
    // Intrinsics
    sfmData::Intrinsics& intrinsics = sfmData.getIntrinsics();
    for(auto pair : inputCameraParams->instrinsics)
    {
        IntrinsicAux* intrinsic = pair.second;
        std::shared_ptr<camera::IntrinsicBase> intrinsicBase;
        // Only Pinhole camera model supported
        std::shared_ptr<camera::Pinhole> pinholeIntrinsic = camera::createPinholeIntrinsic(
            camera::EINTRINSIC::PINHOLE_CAMERA_RADIAL3, intrinsic->width, intrinsic->height, intrinsic->pxFocalLength,
            intrinsic->principalPoint[0], intrinsic->principalPoint[1]);
        pinholeIntrinsic->setInitializationMode(camera::EIntrinsicInitMode::UNKNOWN);
        pinholeIntrinsic->setDistortionParams(intrinsic->distortionParams);
        intrinsicBase = std::static_pointer_cast<camera::IntrinsicBase>(pinholeIntrinsic);
        intrinsicBase->unlock();
        intrinsics.emplace(intrinsic->intrinsicId, intrinsicBase);
    }
    // Views
    sfmData::Views& views = sfmData.getViews();
    for(auto pair : inputCameraParams->views)
    {
        ViewAux* view = pair.second;
        sfmData::View viewBase;
        viewBase.setViewId(view->viewId);
        viewBase.setPoseId(view->poseId);
        viewBase.setIntrinsicId(view->intrinsicId);
        viewBase.setImagePath(view->path);
        viewBase.setWidth(view->width);
        viewBase.setHeight(view->height);
        views.emplace(view->viewId, std::make_shared<sfmData::View>(viewBase));
    }
    // Poses
    sfmData::Poses& poses = sfmData.getPoses();
    for(auto pair : inputCameraParams->poses)
    {
        PoseAux* pose = pair.second;
        sfmData::CameraPose poseBase;
        Mat3 rot;
        Vec3 cent;
        for(unsigned int i = 0; i < 3; i++)
        {
            for(unsigned int j = 0; j < 3; j++)
            {
                rot(i, j) = pose->rotation[i * 3 + j];
            }
            cent[i] = pose->center[i];
        }
        geometry::Pose3 pose3(rot, cent);
        poseBase.setTransform(pose3);
        poseBase.unlock();
        poses.emplace(pose->poseId, poseBase);
    }
    // Structure
    sfmData::Landmarks& structure = sfmData.structure;
    unsigned int idxLandmark = 0;
    for(auto landmark : inputCameraParams->landmarks)
    {
        sfmData::Landmark landmarkBase;
        for(unsigned int i = 0; i < 3; i++)
        {
            landmarkBase.X[i] = landmark->X[i];
        }
        for(auto observation : landmark->observations)
        {
            sfmData::Observation observationBase;
            landmarkBase.observations.emplace(observation, observationBase);
        }
        structure.emplace(idxLandmark, landmarkBase);
        idxLandmark++;
    }
}

void AVTexturing::applyTexture(MeshAux* inputMesh, CameraParameters* inputCameraParams, OptionsAVTexturing* options,
                               std::string outputPath)
{
    system::Timer timer;

    std::string verboseLevel = system::EVerboseLevel_enumToString(system::Logger::getDefaultVerboseLevel());
    std::string outputFolder;
    std::string outputFileName;
    if(outputPath.find_last_of('/') != std::string::npos)
	{
		outputFolder = outputPath.substr(0, outputPath.find_last_of('/'));
        outputFileName = outputPath.substr(outputPath.find_last_of('/') + 1, outputPath.size());
	}
	else
	{
        outputFolder = outputPath.substr(0, outputPath.find_last_of('\\'));
        outputFileName = outputPath.substr(outputPath.find_last_of('\\') + 1, outputPath.size());
	}
	//Remove .obj
    outputFileName = outputFileName.substr(0, outputFileName.find_last_of('.'));
   

    mesh::TexturingParams texParams;
    texParams.angleHardThreshold = options->angleHardThreshold;
    texParams.bestScoreThreshold = options->bestScoreThreshold;
    texParams.downscale = options->downscale;
    texParams.fillHoles = options->fillHoles;
    texParams.multiBandDownscale = options->multiBandDownscale;
    texParams.forceVisibleByAllVertices = options->forceVisibleByAllVertices;
    texParams.padding = options->padding;
    texParams.textureSide = options->textureSide;
    texParams.useUDIM = options->useUDIM;
    texParams.visibilityRemappingMethod = (mesh::EVisibilityRemappingMethod)options->visibilityRemappingMethod;

    // set verbose level
    system::Logger::get()->setLogLevel(verboseLevel);
    // set output texture file type
    // const EImageFileType outputTextureFileType = EImageFileType_stringToEnum(options->outTextureFileTypeName);

    // read the input SfM scene
    sfmData::SfMData sfmData;
    loadSFMDataFromCameraParameters(sfmData, inputCameraParams);

    // initialization
    mvsUtils::MultiViewParams mp(sfmData);

    mesh::Texturing mesh;
    mesh.texParams = texParams;

    // load and remap mesh
    {
        mesh.clear();

        // load input obj file
        loadMeshfromMeshAux(inputMesh, &mesh, options->flipNormals);

        // load reference dense point cloud with visibilities
        mesh::Mesh refPoints;
        mesh::PointsVisibility* refVisibilities = new mesh::PointsVisibility();
        const std::size_t nbPoints = sfmData.getLandmarks().size();
        refPoints.pts = new StaticVector<Point3d>();
        refPoints.pts->reserve(nbPoints);
        refVisibilities->reserve(nbPoints);

        for(const auto& landmarkPair : sfmData.getLandmarks())
        {
            const sfmData::Landmark& landmark = landmarkPair.second;
            mesh::PointVisibility* pointVisibility = new mesh::PointVisibility();

            pointVisibility->reserve(landmark.observations.size());
            for(const auto& observationPair : landmark.observations)
                pointVisibility->push_back(mp.getIndexFromViewId(observationPair.first));

            refVisibilities->push_back(pointVisibility);
            refPoints.pts->push_back(Point3d(landmark.X(0), landmark.X(1), landmark.X(2)));
        }

        mesh.remapVisibilities(texParams.visibilityRemappingMethod, refPoints, *refVisibilities);

        // delete visibilities
        deleteArrayOfArrays(&refVisibilities);
    }

    fs::create_directory(outputFolder);

    if(!mesh.hasUVs())
    {
        ALICEVISION_LOG_INFO("Input mesh has no UV coordinates, start unwrapping (" + options->unwrapMethod + ")");
        mesh.unwrap(mp, mesh::EUnwrapMethod_stringToEnum(options->unwrapMethod));
        ALICEVISION_LOG_INFO("Unwrapping done.");
    }

    // save final obj file
    mesh.saveAsOBJ(outputFolder, outputFileName); //, outputTextureFileType);

    // generate textures
    ALICEVISION_LOG_INFO("Generate textures.");
    mesh.generateTextures(mp, outputFolder); //, outputTextureFileType);
    ALICEVISION_LOG_INFO("Task done in (s): " + std::to_string(timer.elapsed()));
}

AVTexturing::AVTexturing() {}