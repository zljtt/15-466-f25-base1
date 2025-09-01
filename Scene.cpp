#include "Scene.hpp"
#include "GL.hpp"
#include <fstream>
#include <vector>
#include "load_save_png.hpp"
#include <iostream>
#include <algorithm>

glm::vec2 GameObject::get_pos()
{
    return {pos_x, pos_y};
}

GameObject::GameObject(float x, float y, std::vector<SpriteInput> sprite_entry) : pos_x{x}, pos_y{y}
{
    sprites.reserve(sprite_entry.size());
    for (auto const &entry : sprite_entry)
    {
        PPU466::Sprite sprite{};
        sprite.x = static_cast<uint8_t>(x + entry.x_offset);
        sprite.y = static_cast<uint8_t>(y + entry.y_offset);
        sprite.index = entry.tile_table_id;
        sprite.attributes = (entry.palette_id & 0b00000111) | (entry.behind_background ? 0b10000000 : 0b00000000);
        sprites.push_back({sprite, {entry.x_offset, entry.y_offset}});
    }
}

void GameObject::move(float x, float y)
{
    pos_x += x;
    pos_y += y;
    for (auto &[sprite, offset] : sprites)
    {
        // hide if out of screen;
        if (pos_x + offset.x < 0 || pos_x + offset.x > PPU466::ScreenWidth || pos_y + offset.y < 0 || pos_y + offset.y > PPU466::ScreenHeight)
        {
            sprite.x = 0;
            sprite.y = 240;
        }
        else
        {
            sprite.x = static_cast<uint8_t>(pos_x + offset.x);
            sprite.y = static_cast<uint8_t>(pos_y + offset.y);
        }
    }
}

void GameObject::move_clamped(float dx, float dy, float minx, float maxx, float miny, float maxy)
{
    pos_x += dx;
    pos_y += dy;
    if (pos_x < minx)
        pos_x = minx;
    else if (pos_x > maxx)
        pos_x = maxx;
    if (pos_y < miny)
        pos_y = miny;
    else if (pos_y > maxy)
        pos_y = maxy;

    for (auto &[sprite, offset] : sprites)
    {
        // hide if out of screen;
        if (pos_x + offset.x < 0 || pos_x + offset.x > PPU466::ScreenWidth || pos_y + offset.y < 0 || pos_y + offset.y > PPU466::ScreenHeight)
        {
            sprite.x = 0;
            sprite.y = 240;
        }
        else
        {
            sprite.x = static_cast<uint8_t>(pos_x + offset.x);
            sprite.y = static_cast<uint8_t>(pos_y + offset.y);
        }
    }
}

void GameObject::switch_sprites(std::vector<std::pair<PPU466::Sprite, glm::vec2>> new_sprites)
{
    sprites = new_sprites;
}

void Scene::add_game_object(std::string const &name, GameObject &game_object)
{
    game_objects.insert_or_assign(name, game_object);
}

void Scene::remove_game_object(std::string const &name)
{
    auto it = game_objects.find(name);
    if (it == game_objects.end())
    {
        throw std::out_of_range("[remove]Cannot find game object " + name);
    }
    game_objects.erase(name);
}

void Scene::set_background(u_int16_t x, u_int16_t y, u_int8_t tile_id, u_int8_t palette_id)
{
    background[x + PPU466::BackgroundWidth * y] = (palette_id << 8) | tile_id;
}

std::array<PPU466::Sprite, 64> Scene::gather_game_object_sprites() const
{
    std::array<PPU466::Sprite, 64> out{};
    for (auto &sprite : out)
    {
        sprite = PPU466::Sprite{};
    }
    std::vector<PPU466::Sprite> items;

    for (auto const &[name, game_object] : game_objects)
    {
        for (auto &sprite : game_object.sprites)
        {
            items.push_back(sprite.first);
        }
    };
    for (int i = 0; i < ((items.size() > 64) ? 64 : items.size()); i++)
    {
        out[i] = items[i];
    }
    return out;
}

GameObject &Scene::lookup(std::string const &name)
{
    auto it = game_objects.find(name);
    if (it == game_objects.end())
    {
        throw std::out_of_range("[Lookup]Cannot find game object " + name);
    }
    return it->second;
}