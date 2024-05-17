/**
 * @file    tree_mesh_builder.cpp
 *
 * @author  Adam Zvara <xzvara01@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP tasks + octree early elimination
 *
 * @date    27 November 2023
 **/

#include <iostream>
#include <math.h>
#include <limits>

#include "tree_mesh_builder.h"

TreeMeshBuilder::TreeMeshBuilder(unsigned gridEdgeSize)
    : BaseMeshBuilder(gridEdgeSize, "Octree")
{

}

unsigned TreeMeshBuilder::marchCubes(const ParametricScalarField &field)
{
    unsigned totalTriangles = 0;

    // Recurively build octree by calling buildOctree() on initial cube
    #pragma omp parallel
    {
        #pragma omp master
        totalTriangles = buildOctree(field, Vec3_t<float>(0, 0, 0), mGridSize);    
    }

    return totalTriangles;
}

unsigned TreeMeshBuilder::buildOctree(const ParametricScalarField &field, const Vec3_t<float> &cubeOffset, const unsigned cubeSize)
{
    // Stop if cube is outside of field
    if (evaluateMidPoint(field, cubeOffset, cubeSize)) {
        return 0;
    }
    
    // Build cube if it is small enough and it is inside of field
    if (cubeSize <= OCTREE_CUTOFF) { 
        return buildCube(cubeOffset, field);
    }

    unsigned allTriangles = 0;
    const unsigned subCubeSize = cubeSize / 2;

    // Divide cube into 8 subcubes
    for (int i = 0; i < 8; i++) {
        // Create task for each subcube and evaluate if it is inside of field
        // Only create task if subcube is big enough
        #pragma omp task shared(allTriangles, field, cubeOffset, cubeSize, subCubeSize) if (cubeSize > OCTREE_CUTOFF * 2) 
        {
            // Calculate offset of given subcube
            Vec3_t<float> subCubeOffset = cubeOffset;
            if (i & 1) subCubeOffset.x += subCubeSize;
            if (i & 2) subCubeOffset.y += subCubeSize;
            if (i & 4) subCubeOffset.z += subCubeSize;
       
            #pragma omp atomic // atomic assignment, not calling function buildOctree
            allTriangles += buildOctree(field, subCubeOffset, subCubeSize);
        }
    }

    // Wait for all subtasks to finish
    #pragma omp taskwait
    return allTriangles;
}

bool TreeMeshBuilder::evaluateMidPoint(const ParametricScalarField &field, const Vec3_t<float> &cubeOffset, const unsigned cubeSize)
{
	const float subCubeSize = cubeSize / 2.F;
	const Vec3_t<float> midPoint(
		(cubeOffset.x + subCubeSize) * mGridResolution,
		(cubeOffset.y + subCubeSize) * mGridResolution,
		(cubeOffset.z + subCubeSize) * mGridResolution
	);

    static const float e = sqrtf(3.F) / 2.F;

	return evaluateFieldAt(midPoint, field) > mIsoLevel + cubeSize * mGridResolution * e;
}


float TreeMeshBuilder::evaluateFieldAt(const Vec3_t<float> &pos, const ParametricScalarField &field)
{
    const Vec3_t<float> *pPoints = field.getPoints().data();
    const unsigned count = unsigned(field.getPoints().size());

    float value = std::numeric_limits<float>::max();

    for(unsigned i = 0; i < count; ++i)
    {
        float distanceSquared  = (pos.x - pPoints[i].x) * (pos.x - pPoints[i].x);
        distanceSquared       += (pos.y - pPoints[i].y) * (pos.y - pPoints[i].y);
        distanceSquared       += (pos.z - pPoints[i].z) * (pos.z - pPoints[i].z);

        value = std::min(value, distanceSquared);
    }

    return sqrt(value);
}

void TreeMeshBuilder::emitTriangle(const BaseMeshBuilder::Triangle_t &triangle)
{
    #pragma omp critical (emitTriangleCritical)
    mTriangles.push_back(triangle);
}
