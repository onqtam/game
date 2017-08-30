#pragma once

struct transform
{
    glm::vec3 pos;
    glm::vec3 scl;
    glm::quat rot;
};
inline bool operator==(const transform& lhs, const transform& rhs) {
    return lhs.pos == rhs.pos && lhs.scl == rhs.scl && lhs.rot == rhs.rot;
}
inline bool operator!=(const transform& lhs, const transform& rhs) { return !operator==(lhs, rhs); }
