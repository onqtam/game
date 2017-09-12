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

    // thanks to @ongamex
    transform inverse() const {
        auto inv = yama::inverse(as_mat());
        return {{inv.m03, inv.m13, inv.m23},
                {1.f / scl.x, 1.f / scl.y, 1.f / scl.z},
                yama::conjugate(rot)};

        //transform result;
        //result.pos   = {inv.m03, inv.m13, inv.m23};
        //result.rot   = yama::conjugate(rot);
        //result.scl.x = 1.f / scl.x;
        //result.scl.y = 1.f / scl.y;
        //result.scl.z = 1.f / scl.z;
        //return result;
    }

    // as seen in FTransform::Multiply()
    // https://github.com/EpicGames/UnrealEngine/blob/master/Engine/Source/Runtime/Core/Public/Math/TransformNonVectorized.h
    transform multiply(const transform& other) const {
        return {yama::rotate(yama::mul(other.scl, pos), other.rot) + other.pos,
                yama::mul(scl, other.scl), other.rot * rot};

        //transform result;
        //result.rot = other.rot * rot;
        //result.scl = yama::mul(scl, other.scl);
        //result.pos = yama::rotate(yama::mul(other.scl, pos), other.rot) + other.pos;
        //return result;
    }
};
inline bool operator==(const transform& lhs, const transform& rhs) {
    return lhs.pos == rhs.pos && lhs.scl == rhs.scl && lhs.rot == rhs.rot;
}
inline bool operator!=(const transform& lhs, const transform& rhs) { return !operator==(lhs, rhs); }
