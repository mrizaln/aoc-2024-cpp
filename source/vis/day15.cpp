#include "common.hpp"
#include "day/15.hpp"

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <fmt/base.h>

#include <charconv>

struct Pixel
{
    static consteval Pixel from_hex(std::string_view hex)
    {
        if (hex.size() < 7) {
            throw std::runtime_error("Invalid hex color");
        }
        if (hex[0] != '#') {
            throw std::runtime_error("Invalid hex color");
        }

        auto p = Pixel{};

        auto [_1, ec1] = std::from_chars(hex.data() + 1, hex.data() + 3, p.m_r, 16);
        auto [_2, ec2] = std::from_chars(hex.data() + 3, hex.data() + 5, p.m_g, 16);
        auto [_3, ec3] = std::from_chars(hex.data() + 5, hex.data() + 7, p.m_b, 16);

        if (ec1 != std::errc{} || ec2 != std::errc{} || ec3 != std::errc{}) {
            throw std::runtime_error("Invalid hex color");
        }
        p.m_a = 0xFF;

        return p;
    }

    std::uint8_t m_r;
    std::uint8_t m_g;
    std::uint8_t m_b;
    std::uint8_t m_a;
};

struct ImageBuffer
{
    std::size_t        m_width;
    std::size_t        m_height;
    std::vector<Pixel> m_pixels;
};

struct Day15Vis
{
    using Day15 = aoc::day::Day15;

    static constexpr auto empty_color = Pixel::from_hex("#0b0033");
    static constexpr auto left_color  = Pixel::from_hex("#832232");
    static constexpr auto right_color = Pixel::from_hex("#932638");
    static constexpr auto robot_color = Pixel::from_hex("#0094c6");
    static constexpr auto wall_color  = Pixel::from_hex("#ce8964");

    static Day15Vis create(aoc::common::fs::path path)
    {
        ASSERT(aoc::common::fs::exists(path), "File does not exist: {}", path.string());

        auto day       = aoc::day::Day15{};
        auto raw_input = aoc::common::parse_file(path);
        auto input     = day.parse(raw_input.m_lines, {});

        auto warehouse = widen(input.m_warehouse);

        auto image = ImageBuffer{
            .m_width  = warehouse.m_width,
            .m_height = warehouse.m_height,
            .m_pixels = std::vector<Pixel>(warehouse.m_width * warehouse.m_height, empty_color)
        };

        return { std::move(warehouse), input.m_robot_pos, std::move(input.m_movements), std::move(image) };
    }

    bool update()
    {
        if (m_idx >= m_moves.size()) {
            return false;
        }

        auto [move, steps] = m_moves[m_idx++];
        m_robot            = m_warehouse.move(m_robot, move, steps);

        return true;
    }

    void update_image_buffer()
    {
        auto colorize = [this](std::size_t x, std::size_t y, Pixel color) {
            m_image.m_pixels[y * m_image.m_width + x] = color;
        };

        for (auto [x, y] : aoc::util::iter_2d(m_warehouse.m_width, m_warehouse.m_height)) {
            if (m_robot.m_x == x && m_robot.m_y == y) {
                colorize(x, y, robot_color);
                continue;
            }

            using T = aoc::day::day15::ThingWide;
            switch (m_warehouse[x, y]) {
            case T::Empty: colorize(x, y, empty_color); break;
            case T::BoxLeft: colorize(x, y, left_color); break;
            case T::BoxRight: colorize(x, y, right_color); break;
            case T::Wall: colorize(x, y, wall_color); break;
            }
        }
    }

    void draw_into(sf::Texture& tex, bool update)
    {
        update_image_buffer();

        auto image = sf::Image{};

        const auto& [width, height, pixels] = m_image;
        const auto* data                    = reinterpret_cast<const std::uint8_t*>(pixels.data());
        image.create(static_cast<unsigned int>(width), static_cast<unsigned int>(height), data);

        if (update) {
            tex.update(image);
        } else {
            tex.loadFromImage(image);
        }
    }

    Day15::WarehouseWide             m_warehouse;
    Day15::Coord                     m_robot;
    std::vector<Day15::MovementStep> m_moves;
    ImageBuffer                      m_image;
    std::size_t                      m_idx = 0;
};

int main()
{
    static constexpr auto title = "aoc 2024 day 15 part 2 visualization";

    auto window = sf::RenderWindow{ { 1280, 720 }, title };

    window.setVerticalSyncEnabled(true);
    window.setActive(true);

    auto vsync       = true;
    auto pause       = false;
    auto delay       = std::chrono::duration<float, std::milli>{ 10.0f };
    auto delay_timer = aoc::common::Timer{};
    auto title_timer = aoc::common::Timer{};
    auto fps_timer   = aoc::common::Timer{};

    auto texture = sf::Texture{};
    auto sprite  = sf::Sprite{};
    auto vis     = Day15Vis::create("data/inputs/15.txt");

    vis.draw_into(texture, false);
    sprite.setTexture(texture);

    while (window.isOpen()) {
        for (auto event = sf::Event{}; window.pollEvent(event); /* nothing */) {
            switch (event.type) {
            case sf::Event::Closed: window.close(); break;

            case sf::Event::KeyPressed: {
                using K = sf::Keyboard::Key;
                switch (event.key.code) {
                case K::Q: window.close(); break;
                case K::Space: pause = !pause; break;
                case K::Up: delay = delay / 1.1f; break;
                case K::Down: delay = delay * 1.1f; break;
                case K::V: window.setVerticalSyncEnabled(vsync = !vsync); break;
                case K::R: {
                    vis = Day15Vis::create("data/inputs/15.txt");
                    vis.draw_into(texture, true);
                } break;
                case K::U: {
                    if (vis.update()) {
                        vis.draw_into(texture, true);
                    }
                }
                default: /* do nothing */;
                }
            } break;

            default: /* do nothing */;
            }
        }

        sprite.setScale(
            window.getView().getSize().x / sprite.getLocalBounds().width,
            window.getView().getSize().y / sprite.getLocalBounds().height
        );

        if (not pause and delay_timer.elapsed() > delay) {
            if (vis.update()) {
                vis.draw_into(texture, true);
            }
            delay_timer.reset();
        }

        window.clear();
        window.draw(sprite);

        if (aoc::common::to_ms<float>(title_timer.elapsed()).count() > 250.0f) {
            window.setTitle(fmt::format(
                "{} - (delay: {:.2f}ms, frametime: {:.2f})",
                title,
                delay.count(),
                aoc::common::to_ms<float>(fps_timer.elapsed()).count()
            ));
            title_timer.reset();
        }
        fps_timer.reset();

        window.display();
    }
}
