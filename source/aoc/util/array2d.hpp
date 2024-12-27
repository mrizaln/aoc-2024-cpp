#pragma once

#include "common.hpp"
#include "iter2d.hpp"

#include <vector>

namespace aoc::util
{
    template <typename Elem>
    struct Array2D
    {
        Array2D(std::size_t width, std::size_t height, Elem default_val)
            : m_width{ width }
            , m_height{ height }
            , m_elems{ std::vector<Elem>(width * height, default_val) }
        {
        }

        template <typename Self>
        auto&& at(this Self&& self, std::size_t x, std::size_t y)
        {
            DEBUG_ASSERT(x < self.m_width and y < self.m_height, "out of bounds");
            return std::forward<Self>(self).m_elems[y * self.m_width + x];
        }

        template <typename Self>
        auto&& at(this Self&& self, util::Coordinate<std::size_t> coord)
        {
            auto [x, y] = coord;
            DEBUG_ASSERT(x < self.m_width and y < self.m_height, "out of bounds");
            return std::forward<Self>(self).m_elems[y * self.m_width + x];
        }

        template <typename Self>
        auto iter(this Self&& self)
        {
            return iter_2d(std::forward<Self>(self).m_elems, self.m_width, self.m_height);
        }

        template <typename Self>
        auto iter_enumerate(this Self&& self)
        {
            return iter_2d_enumerate(std::forward<Self>(self).m_elems, self.m_width, self.m_height);
        }

        std::size_t       m_width;
        std::size_t       m_height;
        std::vector<Elem> m_elems;
    };
}
