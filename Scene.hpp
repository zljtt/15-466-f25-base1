#pragma once
#include <string>
#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include "PPU466.hpp"
#include <unordered_map>
#include <utility>
#include <vector>

struct GameObject
{

    // sprites with offset
    std::vector<std::pair<PPU466::Sprite, glm::vec2>> sprites;
    // game position
    float pos_x;
    float pos_y;
    glm::vec2 velocity = {0, 0};
    u_int8_t extra = -1;

    struct SpriteInput
    {
        uint8_t tile_table_id;
        uint8_t palette_id;
        float x_offset;
        float y_offset;
        bool behind_background;
    };
    GameObject(float x, float y, std::vector<SpriteInput> sprite_entry);
    glm::vec2 get_pos();

    void move(float x, float y);
    void move_clamped(float x, float y, float minx, float maxx, float miny, float maxy);
    void switch_sprites(std::vector<std::pair<PPU466::Sprite, glm::vec2>> new_sprites);
};
static const GameObject EMPTY = {0, 0, {}};

struct Scene
{
    std::unordered_map<std::string, GameObject> game_objects;

    void add_game_object(std::string const &name, GameObject &game_object);
    void remove_game_object(std::string const &name);
    GameObject &lookup(std::string const &name);

    std::array<PPU466::Sprite, 64> gather_game_object_sprites() const;
};
