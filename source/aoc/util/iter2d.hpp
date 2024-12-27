#pragma once

#include "coordinate.hpp"

#include <iterator>
#include <ranges>

namespace aoc::util
{
    struct Iter2D
    {
        struct [[nodiscard]] Iterator;
        struct [[nodiscard]] EndIterator;

        Iterator    begin() const noexcept;
        EndIterator end() const noexcept;

        std::size_t m_width;
        std::size_t m_height;
    };

    template <typename Ref>
        requires std::is_lvalue_reference_v<Ref>
    struct Iter2DEnumerate
    {
        Coordinate<std::size_t> m_coord;
        Ref                     m_value;
    };

    struct Iter2D::Iterator
    {
        using iterator_category = std::input_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = Coordinate<std::size_t>;
        using pointer           = value_type*;
        using reference         = value_type&;

        Iterator(std::size_t x, std::size_t y, std::size_t width)
            : m_x(x)
            , m_y(y)
            , m_width(width)
        {
        }

        Iterator& operator++()
        {
            if (++m_x == m_width) {
                m_x = 0;
                ++m_y;
            }
            return *this;
        }

        Iterator operator++(int)
        {
            auto copy = *this;
            ++*this;
            return copy;
        }

        value_type operator*() const { return { m_x, m_y }; }

        auto operator<=>(const Iterator&) const = default;
        bool operator==(const Iterator&) const  = default;

        friend bool operator==(const Iterator&, const EndIterator&);

        std::size_t m_x;
        std::size_t m_y;
        std::size_t m_width;
    };

    struct Iter2D::EndIterator
    {
        friend bool operator==(const Iterator&, const EndIterator&);

        std::size_t m_width;
        std::size_t m_height;
    };

    inline bool operator==(const Iter2D::Iterator& lhs, const Iter2D::EndIterator& rhs)
    {
        return lhs.m_y == rhs.m_height and lhs.m_x == 0;
    }

    inline Iter2D::Iterator Iter2D::begin() const noexcept
    {
        return { 0, 0, m_width };
    }

    inline Iter2D::EndIterator Iter2D::end() const noexcept
    {
        return { m_width, m_height };
    }

    static_assert(std::input_iterator<Iter2D::Iterator>);
    static_assert(std::ranges::range<Iter2D>);
    static_assert(std::ranges::viewable_range<Iter2D>);

    inline Iter2D iter_2d(std::size_t width, std::size_t height)
    {
        return { .m_width = width, .m_height = height };
    }

    template <typename Container>
    auto iter_2d(Container&& container, std::size_t width, std::size_t height)
    {
        return std::views::transform(iter_2d(width, height), [&, width](auto&& coord) -> decltype(auto) {
            return container[coord.m_y * width + coord.m_x];
        });
    }

    template <typename Container>
    auto iter_2d_enumerate(Container&& container, std::size_t width, std::size_t height)
    {
        using Cont = std::remove_reference_t<Container>;
        using Ref  = std::conditional_t<
             std::is_const_v<Cont>,
             const typename Cont::value_type&,
             typename Cont::value_type&>;

        return std::views::transform(iter_2d(width, height), [&, width](auto&& coord) {
            return Iter2DEnumerate<Ref>{
                .m_coord = coord,
                .m_value = container[coord.m_y * width + coord.m_x],
            };
        });
    }
}
