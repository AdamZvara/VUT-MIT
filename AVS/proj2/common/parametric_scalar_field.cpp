/**
 * @file    parametric_scalar_field.cpp
 *
 * @authors Filip Vaverka <ivaverka@fit.vutbr.cz>
 *          Vojtech Mrazek <mrazek@fit.vutbr.cz>
 *
 * @brief   Class to represent scalar field defined by set of points in 3D space.
 *
 * @date    06 November 2020, 11:07
 **/

#include <limits>
#include <fstream>
#include "parametric_scalar_field.h"

ParametricScalarField::ParametricScalarField(const std::string &filename, float isoLevel)
    : mIsoLevel(isoLevel)
    , mMin(std::numeric_limits<float>::max())
    , mMax(std::numeric_limits<float>::min())
    , mSize(0.0f)
    , mFilename(filename)
{
    loadFromFile(filename);
}

void ParametricScalarField::loadFromFile(const std::string &filename)
{
    mFilename = filename;
    mMin = Vec3_t<float>(std::numeric_limits<float>::max());
    mMax = Vec3_t<float>(std::numeric_limits<float>::min());

    std::ifstream fieldFile(filename.c_str());

    char elementType;
    Vec3_t<float> point;
    while(fieldFile >> elementType >> point.x >> point.y >> point.z)
    {
        mMin.x = std::min(mMin.x, point.x);
        mMin.y = std::min(mMin.y, point.y);
        mMin.z = std::min(mMin.z, point.z);

        mMax.x = std::max(mMax.x, point.x);
        mMax.y = std::max(mMax.y, point.y);
        mMax.z = std::max(mMax.z, point.z);

        mPoints.push_back(point);
    }

    build();
}

void ParametricScalarField::build()
{
    Vec3_t<float> min(mMin.x - mIsoLevel, mMin.y - mIsoLevel, mMin.z - mIsoLevel);
    Vec3_t<float> max(mMax.x + mIsoLevel, mMax.y + mIsoLevel, mMax.z + mIsoLevel);

    for(unsigned i = 0; i < mPoints.size(); ++i)
    {
        mPoints[i].x -= min.x;
        mPoints[i].y -= min.y;
        mPoints[i].z -= min.z;
    }

    mSize.x = max.x - min.x;
    mSize.y = max.y - min.y;
    mSize.z = max.z - min.z;
}
