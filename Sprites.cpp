#include "Sprites.hpp"
#include "GL.hpp"
#include <fstream>
#include <vector>
#include "load_save_png.hpp"
#include <iostream>

std::array<PPU466::Tile, 256> Sprites::to_ppu466_tile_table() const
{
    std::array<PPU466::Tile, 256> ret{};
    for (PPU466::Tile &tile : ret)
    {
        tile.bit0.fill(0);
        tile.bit1.fill(0);
    }
    std::copy_n(tile_table.begin(), std::min(tile_table.size(), ret.size()), ret.begin());
    return ret;
}

std::array<PPU466::Palette, 8> Sprites::to_ppu466_palette_table() const
{
    std::array<PPU466::Palette, 8> ret{};
    for (PPU466::Palette &palette : ret)
    {
        for (int i = 0; i < 4; i++)
        {
            palette[i] = {0, 0, 0, 0};
        }
    }
    std::copy_n(palette_table.begin(), std::min(palette_table.size(), ret.size()), ret.begin());
    return ret;
}

uint8_t Sprites::load(std::string const &filename)
{
    const uint8_t SPRITE_SIZE = 8;
    // load from png
    glm::uvec2 size(0, 0);
    std::vector<glm::u8vec4> png_data;
    load_png(filename, &size, &png_data, LowerLeftOrigin);

    if (size.x % SPRITE_SIZE != 0 || size.y % SPRITE_SIZE != 0)
    {
        throw std::runtime_error("PNG is not a multiple of 8 pixel.");
    }
    // const uint8_t tile_count = (size.x / 8) * (size.y / 8);

    // read palette from data
    PPU466::Palette palette;
    palette[0] = glm::u8vec4(0, 0, 0, 0);
    uint8_t palette_size = 1;

    for (auto const &px_color : png_data)
    {
        bool found = false;
        for (uint8_t i = 0; i < palette_size; ++i)
        {
            if (palette[i] == px_color)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            if (palette_size >= 4)
            {
                throw std::runtime_error("PNG has too many colors.");
            }
            palette[palette_size++] = px_color;
        }
    }

    // create tiles from data and palette
    // loop through tiles in the png
    for (uint8_t ty = 0; ty < (size.y / SPRITE_SIZE); ty++)
    {
        for (uint8_t tx = 0; tx < (size.x / SPRITE_SIZE); tx++)
        {
            PPU466::Tile tile{};
            // loop through px in the tile
            for (uint8_t y = 0; y < SPRITE_SIZE; y++)
            {
                uint8_t bit0y = 0;
                uint8_t bit1y = 0;
                for (uint8_t x = 0; x < SPRITE_SIZE; x++)
                {
                    uint8_t iy = ty * SPRITE_SIZE + y;
                    uint8_t ix = tx * SPRITE_SIZE + x;
                    glm::u8vec4 px = png_data[iy * size.x + ix];
                    uint8_t color_id = 0;
                    // find color id in palette
                    for (uint8_t i = 0; i < palette.size(); i++)
                    {
                        if (palette[i] == px)
                            color_id = i;
                    }
                    // assign 2bit
                    switch (color_id)
                    {
                    default:
                    case 0:
                        bit0y |= (0b00000000 << x);
                        bit1y |= (0b00000000 << x);
                        break;
                    case 1:
                        bit0y |= (0b00000001 << x);
                        bit1y |= (0b00000000 << x);
                        break;
                    case 2:
                        bit0y |= (0b00000000 << x);
                        bit1y |= (0b00000001 << x);
                        break;
                    case 3:
                        bit0y |= (0b00000001 << x);
                        bit1y |= (0b00000001 << x);
                        break;
                    }
                }
                tile.bit0[y] = bit0y;
                tile.bit1[y] = bit1y;
            }
            tile_table.push_back(tile);
        }
    }
    palette_table.push_back(palette);
    return palette_table.size() - 1;
    // Sprites sprites;
    // std::ifstream file(filename, std::ios::binary);
    // file.read(reinterpret_cast<char *>(sprites.tile_table.data()), sizeof(sprites.tile_table));

    // return sprites;
    // std::vector<u_int32_t> rgb_data;
    // GLuint tex = 0;
    // glGenTextures(1, &tex);
    // glBindTexture(GL_TEXTURE_2D, tex);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, 128, 128, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
    // glBindTexture(GL_TEXTURE_2D, 0);
}