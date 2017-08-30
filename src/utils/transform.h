#pragma once

struct transform
{
    yama::vector3 pos;
    yama::vector3 scl;
    yama::quaternion rot;
};
inline bool operator==(const transform& lhs, const transform& rhs) {
    return lhs.pos == rhs.pos && lhs.scl == rhs.scl && lhs.rot == rhs.rot;
}
inline bool operator!=(const transform& lhs, const transform& rhs) { return !operator==(lhs, rhs); }
