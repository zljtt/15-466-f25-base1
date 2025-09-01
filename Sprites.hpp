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

    Sprites();
    uint8_t load(std::string const &filename);
    void load_binary(std::string path);
    void save_assets(std::string path);
};
