/**
 * @file    parametric_scalar_field.h
 *
 * @authors Filip Vaverka <ivaverka@fit.vutbr.cz>
 *          Vojtech Mrazek <mrazek@fit.vutbr.cz>
 *
 * @brief   Class to represent scalar field defined by set of points in 3D space.
 *
 * @date    06 November 2020, 11:07
 **/

#ifndef PARAMETRIC_SCALAR_FIELD_H
#define PARAMETRIC_SCALAR_FIELD_H

#include <string>
#include <vector>
#include "vector_helpers.h"

/**
 * @brief The ParametricScalarField class
 */
class ParametricScalarField
{
public:
    /**
     * @brief Constructor
     * @param [in] filename Path to file containing field data.
     * @param [in] isoLevel Isosurface offset to be used.
     */
    ParametricScalarField(const std::string &filename, float isoLevel);

    /**
     * @brief Load field data from the input file.
     * @param [in] filename Path to file containing field data.
     */
    void loadFromFile(const std::string &filename);

    /**
     * @brief Get reference to vector containing all points of the field.
     * @return Returns reference to vector containing all points of the field.
     */
    const std::vector<Vec3_t<float> > &getPoints() const { return mPoints; }

    /**
     * @brief Allows to query range in which all surfaces occur.
     *        From (0, 0, 0) to (size.x, size.y, size.z).
     * @return Returns size of the field in which all surfacess occur.
     */
    const Vec3_t<float> &getSize() const { return mSize; }

    /**
     * @brief Get isosurface level for this field.
     * @return Returns isosurface level (value)
     */
    float getIsoLevel() const { return mIsoLevel; }

    /**
     * @brief Get path to file containing the field data.
     * @return Path to file containing the field data.
     */
    const std::string &GetFilename() const { return mFilename; }

protected:
    /**
     * @brief Normalize the field so that all surfaces fall into range (0, 0, 0) - (mSize.x, mSize.y, mSize.z).
     */
    void build();

    float mIsoLevel;                        ///< Value of the field at which surface is expected.
    std::vector<Vec3_t<float> > mPoints;    ///< Points which define the field.
    Vec3_t<float> mMin, mMax;               ///< Axis Aligned Bouding Box (AABB) around all points.
    Vec3_t<float> mSize;                    ///< Upper bound of all surfaces in the field (see "build(...)").
    std::string mFilename;                  ///< Path to file containing the field data.
};

#endif // PARAMETRIC_SCALAR_FIELD_H
