#pragma once

// TODO: honor MIT license - changed heavily - originally taken from here: https://github.com/iauns/cpm-glm-aabb

/// Standalone axis aligned bounding box
class HAPI AABB
{
public:
    /// Builds a null AABB.
    AABB();

    /// Builds an AABB that encompasses a sphere.
    /// \param[in]  center Center of the sphere.
    /// \param[in]  radius Radius of the sphere.
    AABB(const yama::vector3& center, float radius);

    /// Builds an AABB that contains the two points.
    AABB(const yama::vector3& p1, const yama::vector3& p2);

    AABB(const AABB& aabb);

    /// Set the AABB as NULL (not set).
    void setNull() {
        mMin = yama::vector3::uniform(1.0);
        mMax = yama::vector3::uniform(-1.0);
    }

    /// Returns true if AABB is NULL (not set).
    bool isNull() const { return mMin.x > mMax.x || mMin.y > mMax.y || mMin.z > mMax.z; }

    /// Extend the bounding box on all sides by \p val.
    void extend(float val);

    /// Expand the AABB to include point \p p.
    void extend(const yama::vector3& p);

    /// Expand the AABB to include a sphere centered at \p center and of radius \p
    /// radius.
    /// \param[in]  center Center of sphere.
    /// \param[in]  radius Radius of sphere.
    void extend(const yama::vector3& center, float radius);

    /// Expand the AABB to encompass the given \p aabb.
    void extend(const AABB& aabb);

    /// Expand the AABB to include a disk centered at \p center, with normal \p
    /// normal, and radius \p radius.
    void extendDisk(const yama::vector3& center, const yama::vector3& normal, float radius);

    /// Translates AABB by vector \p v.
    void translate(const yama::vector3& v);

    /// Scale the AABB by \p scale, centered around \p origin.
    /// \param[in]  scale  3D vector specifying scale along each axis.
    /// \param[in]  origin Origin of scaling operation. Most useful origin would
    ///                    be the center of the AABB.
    void scale(const yama::vector3& scale, const yama::vector3& origin);

    /// Retrieves the center of the AABB.
    yama::vector3 getCenter() const;

    /// Retrieves the diagonal vector (computed as mMax - mMin).
    /// If the AABB is NULL, then a vector of all zeros is returned.
    yama::vector3 getDiagonal() const;

    /// Retrieves the longest edge.
    /// If the AABB is NULL, then 0 is returned.
    float getLongestEdge() const;

    /// Retrieves the shortest edge.
    /// If the AABB is NULL, then 0 is returned.
    float getShortestEdge() const;

    /// Retrieves the AABB's minimum point.
    yama::vector3 getMin() const { return mMin; }

    /// Retrieves the AABB's maximum point.
    yama::vector3 getMax() const { return mMax; }

    /// Returns true if AABBs share a face overlap.
    bool overlaps(const AABB& bb) const;

    /// Type returned from call to intersect.
    enum INTERSECTION_TYPE
    {
        INSIDE,
        INTERSECT,
        OUTSIDE
    };
    /// Returns one of the intersection types. If either of the aabbs are invalid,
    /// then OUTSIDE is returned.
    INTERSECTION_TYPE intersect(const AABB& bb) const;

    /// Function from SCIRun. Here is a summary of SCIRun's description:
    /// Returns true if the two AABB's are similar. If diff is 1.0, the two
    /// bboxes have to have about 50% overlap each for x,y,z. If diff is 0.0,
    /// they have to have 100% overlap.
    /// If either of the two AABBs is NULL, then false is returned.
    bool isSimilarTo(const AABB& b, float diff = 0.5f) const;

private:
    yama::vector3 mMin; ///< Minimum point.
    yama::vector3 mMax; ///< Maximum point.
};
