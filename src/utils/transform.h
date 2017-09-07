#pragma once

struct transform
{
    yama::vector3    pos;
    yama::vector3    scl;
    yama::quaternion rot;

    yama::matrix as_mat() const {
        auto tr = yama::matrix::translation(pos);
        auto rt = yama::matrix::rotation_quaternion(rot);
        auto sc = yama::matrix::scaling(scl);
        return tr * rt * sc;
    }
};
inline bool operator==(const transform& lhs, const transform& rhs) {
    return lhs.pos == rhs.pos && lhs.scl == rhs.scl && lhs.rot == rhs.rot;
}
inline bool operator!=(const transform& lhs, const transform& rhs) { return !operator==(lhs, rhs); }
