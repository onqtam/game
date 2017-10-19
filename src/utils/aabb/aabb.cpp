#include "aabb.h"

using namespace yama;

AABB::AABB() { setNull(); }

AABB::AABB(const yama::vector3& center, float radius) {
    setNull();
    extend(center, radius);
}

AABB::AABB(const yama::vector3& p1, const yama::vector3& p2) {
    setNull();
    extend(p1);
    extend(p2);
}

AABB::AABB(const AABB& aabb) {
    setNull();
    extend(aabb);
}

void AABB::extend(float val) {
    if(!isNull()) {
        mMin -= yama::vector3::uniform(val);
        mMax += yama::vector3::uniform(val);
    }
}

void AABB::extend(const yama::vector3& p) {
    if(!isNull()) {
        mMin = min(p, mMin);
        mMax = max(p, mMax);
    } else {
        mMin = p;
        mMax = p;
    }
}

void AABB::extend(const yama::vector3& p, float radius) {
    auto r = yama::vector3::uniform(radius);
    if(!isNull()) {
        mMin = min(p - r, mMin);
        mMax = max(p + r, mMax);
    } else {
        mMin = p - r;
        mMax = p + r;
    }
}

void AABB::extend(const AABB& aabb) {
    if(!aabb.isNull()) {
        extend(aabb.mMin);
        extend(aabb.mMax);
    }
}

void AABB::extendDisk(const yama::vector3& c, const yama::vector3& n, float r) {
    if(n.length() < 1.e-12f) {
        extend(c);
        return;
    }
    yama::vector3    norm = normalize(n);
    float x    = sqrt(1 - norm.x) * r;
    float y    = sqrt(1 - norm.y) * r;
    float z    = sqrt(1 - norm.z) * r;
    extend(c + v(x, y, z));
    extend(c - v(x, y, z));
}

yama::vector3 AABB::getDiagonal() const {
    if(!isNull())
        return mMax - mMin;
    else
        return yama::vector3::zero();
}

float AABB::getLongestEdge() const {
    auto d = getDiagonal();
    return std::max(std::max(d.x, d.y), d.z);
}

float AABB::getShortestEdge() const {
    auto d = getDiagonal();
    return std::min(std::min(d.x, d.y), d.z);
}

yama::vector3 AABB::getCenter() const {
    if(!isNull()) {
        return (mMin + mMax) * 0.5f;
    } else {
        return yama::vector3::zero();
    }
}

void AABB::translate(const yama::vector3& v) {
    if(!isNull()) {
        mMin += v;
        mMax += v;
    }
}

void AABB::scale(const yama::vector3& s, const yama::vector3& o) {
    if(!isNull()) {
        mMin -= o;
        mMax -= o;

        mMin = mul(mMin, s);
        mMax = mul(mMax, s);

        mMin += o;
        mMax += o;
    }
}

bool AABB::overlaps(const AABB& bb) const {
    if(isNull() || bb.isNull())
        return false;

    if(bb.mMin.x > mMax.x || bb.mMax.x < mMin.x)
        return false;
    else if(bb.mMin.y > mMax.y || bb.mMax.y < mMin.y)
        return false;
    else if(bb.mMin.z > mMax.z || bb.mMax.z < mMin.z)
        return false;

    return true;
}

AABB::INTERSECTION_TYPE AABB::intersect(const AABB& b) const {
    if(isNull() || b.isNull())
        return OUTSIDE;

    if((mMax.x < b.mMin.x) || (mMin.x > b.mMax.x) || (mMax.y < b.mMin.y) || (mMin.y > b.mMax.y) ||
       (mMax.z < b.mMin.z) || (mMin.z > b.mMax.z)) {
        return OUTSIDE;
    }

    if((mMin.x <= b.mMin.x) && (mMax.x >= b.mMax.x) && (mMin.y <= b.mMin.y) &&
       (mMax.y >= b.mMax.y) && (mMin.z <= b.mMin.z) && (mMax.z >= b.mMax.z)) {
        return INSIDE;
    }

    return INTERSECT;
}

bool AABB::isSimilarTo(const AABB& b, float diff) const {
    if(isNull() || b.isNull())
        return false;

    yama::vector3 acceptable_diff = ((getDiagonal() + b.getDiagonal()) / float(2.0)) * diff;
    yama::vector3 min_diff(mMin - b.mMin);
    min_diff = abs(min_diff);
    if(min_diff.x > acceptable_diff.x)
        return false;
    if(min_diff.y > acceptable_diff.y)
        return false;
    if(min_diff.z > acceptable_diff.z)
        return false;
    yama::vector3 max_diff(mMax - b.mMax);
    max_diff = abs(max_diff);
    if(max_diff.x > acceptable_diff.x)
        return false;
    if(max_diff.y > acceptable_diff.y)
        return false;
    if(max_diff.z > acceptable_diff.z)
        return false;
    return true;
}
