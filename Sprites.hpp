#pragma once
#include <string>
#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include "PPU466.hpp"
#include <unordered_map>
#include <utility>
#include <vector>

struct Sprites
{
    uint32_t ti = 0;
    uint32_t pi = 0;
    std::array<PPU466::Palette, 8> palette_table;
    std::array<PPU466::Tile, 256> tile_table;
    std::array<uint16_t, PPU466::BackgroundWidth * PPU466::BackgroundHeight> background;

    Sprites();
    uint8_t load(std::string const &filename);
    void gen_background();
    void set_background(u_int16_t x, u_int16_t y, u_int8_t tile_id, u_int8_t palette_id);

    void load_binary(std::string path);
    void save_assets(std::string path);
};
