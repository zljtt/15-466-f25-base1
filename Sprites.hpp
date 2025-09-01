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
    std::vector<PPU466::Palette> palette_table;

    std::vector<PPU466::Tile> tile_table;
    std::array<PPU466::Tile, 256> to_ppu466_tile_table() const;
    std::array<PPU466::Palette, 8> to_ppu466_palette_table() const;

    uint8_t load(std::string const &filename);
};
