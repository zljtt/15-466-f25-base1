#include "Sprites.hpp"
#include "GL.hpp"
#include <fstream>
#include <vector>
#include "load_save_png.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>

static const uint8_t BackgroundPalette = 0; // 0

Sprites::Sprites()
{
    for (PPU466::Tile &tile : tile_table)
    {
        tile.bit0.fill(0);
        tile.bit1.fill(0);
    }
    for (PPU466::Palette &palette : palette_table)
    {
        for (int i = 0; i < 4; i++)
        {
            palette[i] = {0, 0, 0, 0};
        }
    }
}

void Sprites::gen_background()
{
    // preparing background
    srand(time(nullptr));
    for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y)
    {
        for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x)
        {
            set_background(x, y, rand() % 4, BackgroundPalette);
        }
    }
}

void Sprites::set_background(uint16_t x, uint16_t y, uint8_t tile_id, uint8_t palette_id)
{
    background[x + PPU466::BackgroundWidth * y] = (palette_id << 8) | tile_id;
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
                    // bit merging algorism queried from ChatGPT
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
            tile_table[ti++] = tile;
        }
    }
    palette_table[pi++] = palette;
    return pi - 1;
}

void Sprites::load_binary(std::string path)
{
    std::ifstream in(path, std::ios::binary);
    // based on https://stackoverflow.com/questions/58434836/read-int-through-char-binary-data-from-a-file-with-stdifstreamread
    in.read(reinterpret_cast<char *>(palette_table.data()), sizeof(palette_table));
    in.read(reinterpret_cast<char *>(tile_table.data()), sizeof(tile_table));
    in.read(reinterpret_cast<char *>(background.data()), sizeof(background));
}

void Sprites::save_assets(std::string path)
{
    std::ofstream out(path, std::ios::binary);
    // based on https://stackoverflow.com/questions/58434836/read-int-through-char-binary-data-from-a-file-with-stdifstreamread
    out.write(reinterpret_cast<const char *>(palette_table.data()), sizeof(palette_table));
    out.write(reinterpret_cast<const char *>(tile_table.data()), sizeof(tile_table));
    out.write(reinterpret_cast<const char *>(background.data()), sizeof(background));
}