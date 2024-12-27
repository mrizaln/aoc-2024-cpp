#include "vis-common.hpp"
#include "day/14.hpp"

#include <SFML/Graphics.hpp>

namespace al    = aoc::aliases;
namespace day14 = aoc::day::day14;

using aoc::day::Day14;
using vis::ImageBuffer;
using vis::Pixel;

struct Day14Vis
{
    static constexpr auto empty_color = Pixel::from_hex("#2d3047");
    static constexpr auto robot_color = Pixel::from_hex("#1b998b");

    static constexpr auto end = Day14::map_size.m_x * Day14::map_size.m_y;

    static Day14Vis create(aoc::common::fs::path path)
    {
        ASSERT(aoc::common::fs::exists(path), fmt::format("File {} does not exist", path.string()));

        auto raw_input = aoc::common::parse_file(path);
        auto input     = Day14{}.parse(raw_input.m_lines, {});

        auto map = day14::Map{
            .m_width  = Day14::map_size.m_x,
            .m_height = Day14::map_size.m_y,
            .m_data   = std::vector<al::u8>(Day14::map_size.m_x * Day14::map_size.m_y, 0x00),
        };

        auto image = ImageBuffer{
            .m_width  = map.m_width,
            .m_height = map.m_height,
            .m_pixels = std::vector(map.m_width * map.m_height, empty_color),
        };

        return {
            .m_input             = input,
            .m_input_copy        = std::move(input),
            .m_map               = std::move(map),
            .m_highest_score     = std::numeric_limits<al::usize>::min(),
            .m_highest_score_idx = 0,
            .m_idx               = 0,
            .m_image_buffer      = std::move(image),
        };
    }

    bool update()
    {
        if (m_idx >= end) {
            return false;
        }

        m_map.fill(0x00);

        for (auto& robot : m_input) {
            auto pos = robot.move(1, Day14::map_size).m_pos;
            m_map.inc_no_wrap(pos);
        }

        if (auto score = m_map.score_cluster(Day14::map_size); score > m_highest_score) {
            m_highest_score     = score;
            m_highest_score_idx = m_idx;
        }

        ++m_idx;

        return true;
    }

    void update_image_buffer()
    {
        for (auto [x, y] : aoc::util::iter_2d(m_map.m_width, m_map.m_height)) {
            auto coord = day14::Coord{ static_cast<long>(x), static_cast<long>(y) };
            if (m_map[coord] == 0) {
                m_image_buffer[x, y].decay(empty_color, 0.4f);
            } else {
                m_image_buffer[x, y] = robot_color;
            }
        }
    }

    void draw_into(sf::Texture& tex, bool update)
    {
        update_image_buffer();

        auto image = sf::Image{};

        const auto& [width, height, pixels] = m_image_buffer;
        const auto* data                    = reinterpret_cast<const std::uint8_t*>(pixels.data());
        image.create(static_cast<unsigned int>(width), static_cast<unsigned int>(height), data);

        if (update) {
            tex.update(image);
        } else {
            tex.loadFromImage(image);
        }
    }

    Day14::Input m_input;
    Day14::Input m_input_copy;
    day14::Map   m_map;
    al::usize    m_highest_score;
    al::usize    m_highest_score_idx;
    al::usize    m_idx;

    ImageBuffer m_image_buffer;
};

int main()
{
    static constexpr auto title = "aoc 2024 day 14 part 2 visualization";

    auto window = sf::RenderWindow{ { 720, 720 }, title };

    window.setVerticalSyncEnabled(true);
    window.setActive(true);
    window.setKeyRepeatEnabled(false);

    auto highest = 0uz;

    auto latch       = false;
    auto vsync       = true;
    auto pause       = false;
    auto delay       = std::chrono::duration<float, std::milli>{ 10.0f };
    auto delay_timer = aoc::common::Timer{};
    auto title_timer = aoc::common::Timer{};
    auto fps_timer   = aoc::common::Timer{};

    auto vis = Day14Vis::create("data/inputs/14.txt");

    auto texture = sf::Texture{};
    auto sprite  = sf::Sprite{};

    constexpr auto font_path = "data/fonts/JetBrainsMono-Regular.ttf";

    ASSERT(aoc::common::fs::exists(font_path), fmt::format("File {} does not exist", font_path));

    auto font = sf::Font{};
    font.loadFromFile(font_path);

    auto text = sf::Text{};
    text.setFont(font);
    text.setCharacterSize(24);

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
                    vis = Day14Vis::create("data/inputs/14.txt");
                    vis.draw_into(texture, true);
                } break;
                case K::U: {
                    if (vis.update()) {
                        vis.draw_into(texture, true);
                    }
                } break;
                case K::L: latch = !latch; break;
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

        if ((not pause and delay_timer.elapsed() > delay)
            and not(latch and vis.m_highest_score_idx + 1 == highest)) {

            if (vis.update()) {
                vis.draw_into(texture, true);
            }
            delay_timer.reset();
        }

        window.clear();
        window.draw(sprite);

        text.setString(fmt::format("{:_>5}/{}", vis.m_idx, vis.end));
        text.setPosition(0.0f, 0.0f);
        window.draw(text);

        highest = std::max(vis.m_highest_score_idx + 1, highest);
        text.setString(fmt::format("highest: {}", highest));
        text.setPosition(0.0f, 24.0f);
        window.draw(text);

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
