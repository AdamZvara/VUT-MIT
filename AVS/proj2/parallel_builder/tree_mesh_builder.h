/**
 * @file    tree_mesh_builder.h
 *
 * @author  Adam Zvara <xzvara01@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP tasks + octree early elimination
 *
 * @date    27 November 2023
 **/

#ifndef TREE_MESH_BUILDER_H
#define TREE_MESH_BUILDER_H

#include "base_mesh_builder.h"

class TreeMeshBuilder : public BaseMeshBuilder
{
public:
    TreeMeshBuilder(unsigned gridEdgeSize);

protected:
    unsigned marchCubes(const ParametricScalarField &field);
    float evaluateFieldAt(const Vec3_t<float> &pos, const ParametricScalarField &field);
    void emitTriangle(const Triangle_t &triangle);
    const Triangle_t *getTrianglesArray() const { return mTriangles.data(); }

    std::vector<Triangle_t> mTriangles; ///< Temporary array of triangles

private:

    static const unsigned OCTREE_CUTOFF = 1;   ///< Cut off value for octree early elimination (representing size of cube)

    unsigned buildOctree(const ParametricScalarField &field, const Vec3_t<float> &cubeOffset, const unsigned cubeSize);
    bool evaluateMidPoint(const ParametricScalarField &field, const Vec3_t<float> &cubeOffset, const unsigned cubeSize);
};

#endif // TREE_MESH_BUILDER_H
