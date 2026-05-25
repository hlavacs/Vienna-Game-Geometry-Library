#pragma once

#include "vgeo/Handle.hpp"

namespace vgeo::internal {

struct CandidatePair {
    Handle a;
    Handle b;

    bool operator==(const CandidatePair& other) const noexcept {
        return (a == other.a && b == other.b) || (a == other.b && b == other.a);
    }
};

} // vgeo::internal

namespace std {

template <>
struct hash<vgeo::internal::CandidatePair> {
    size_t operator()(const vgeo::internal::CandidatePair& pair) const noexcept {
        return hash<uint64_t>{}(pair.a.getId()) ^ hash<uint64_t>{}(pair.b.getId());
    }
};

} // namespace std
