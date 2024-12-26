#pragma once

#include <rapidhash.h>

#include <functional>
#include <type_traits>

// general hash for any type that has unique object representations
template <typename T>
    requires std::has_unique_object_representations_v<T>
struct std::hash<T>
{
    std::size_t operator()(const T& obj) const noexcept { return rapidhash(&obj, sizeof(obj)); }
};
