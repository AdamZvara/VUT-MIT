/**
 * @file    ref_mesh_builder.h
 *
 * @authors Filip Vaverka <ivaverka@fit.vutbr.cz>
 *
 * @brief   Reference (sequential) implementation of "BaseMeshBuilder".
 *
 * @date    06 November 2020, 11:07
 **/

#ifndef REF_MESH_BUILDER_H
#define REF_MESH_BUILDER_H

#include <vector>
#include "base_mesh_builder.h"

/**
 * @brief The RefMeshBuilder class
 */
class RefMeshBuilder : public BaseMeshBuilder
{
public:
    RefMeshBuilder(unsigned gridEdgeSize);

protected:
    unsigned marchCubes(const ParametricScalarField &field);
    float evaluateFieldAt(const Vec3_t<float> &pos, const ParametricScalarField &field);
    void emitTriangle(const Triangle_t &triangle);
    const Triangle_t *getTrianglesArray() const { return mTriangles.data(); }

    std::vector<Triangle_t> mTriangles; ///< Temporary array of triangles
};

#endif // REF_MESH_BUILDER_H
