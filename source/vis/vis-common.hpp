#pragma once

#include <algorithm>
#include <fmt/format.h>

#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <charconv>
#include <vector>

namespace vis
{
    struct Pixel
    {
        static consteval Pixel from_hex(std::string_view hex)
        {
            if (hex.size() < 7) {
                throw std::logic_error("string too short");
            }
            if (hex[0] != '#') {
                throw std::logic_error("hex color must start with #");
            }

            auto p = Pixel{};

            auto [_1, ec1] = std::from_chars(hex.data() + 1, hex.data() + 3, p.m_r, 16);
            auto [_2, ec2] = std::from_chars(hex.data() + 3, hex.data() + 5, p.m_g, 16);
            auto [_3, ec3] = std::from_chars(hex.data() + 5, hex.data() + 7, p.m_b, 16);

            if (ec1 != std::errc{} || ec2 != std::errc{} || ec3 != std::errc{}) {
                throw std::logic_error("failed to parse hex color, invalid hex");
            }

            p.m_a = 0xFF;
            return p;
        }

        void decay(const Pixel& into, float strength)
        {
            strength = std::clamp(strength, 0.0f, 1.0f);

            auto sub_abs = [](std::uint8_t a, std::uint8_t b) -> std::uint8_t {
                return a > b ? a - b : b - a;
            };

            auto diff = Pixel{
                .m_r = sub_abs(m_r, into.m_r),
                .m_g = sub_abs(m_g, into.m_g),
                .m_b = sub_abs(m_b, into.m_b),
                .m_a = sub_abs(m_a, into.m_a),
            };

            m_r = static_cast<std::uint8_t>(into.m_r + diff.m_r * (1.0f - strength));
            m_g = static_cast<std::uint8_t>(into.m_g + diff.m_g * (1.0f - strength));
            m_b = static_cast<std::uint8_t>(into.m_b + diff.m_b * (1.0f - strength));
        }

        std::uint8_t m_r;
        std::uint8_t m_g;
        std::uint8_t m_b;
        std::uint8_t m_a;
    };

    struct ImageBuffer
    {
        template <typename Self>
        auto&& operator[](this Self&& self, std::size_t x, std::size_t y)
        {
            return self.m_pixels[y * self.m_width + x];
        }

        std::size_t        m_width;
        std::size_t        m_height;
        std::vector<Pixel> m_pixels;
    };
}

template <>
struct fmt::formatter<vis::Pixel> : public fmt::formatter<std::string_view>
{
    template <typename FormatContext>
    auto format(vis::Pixel const& p, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "#{:02x}{:02x}{:02x}", p.m_r, p.m_g, p.m_b);
    }
};
