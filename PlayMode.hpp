#pragma once
#include "PPU466.hpp"
#include "Mode.hpp"
#include "Load.hpp"
#include "Sprites.hpp"
#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include <cstdint>

struct PlayMode : Mode
{
    PlayMode();
    virtual ~PlayMode();

    // functions called by main loop:
    virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
    virtual void update(float elapsed) override;
    virtual void draw(glm::uvec2 const &drawable_size) override;

    //----- game state -----
    Scene scene;
    enum Fruit : uint8_t
    {
        CHERRY = 0,     // normal speed, normal acc, low cooldown
        WATERMELON = 1, // low speed, high acc, high cooldown
        ORANGE = 2,     // high speed, no acc, normal cooldown
        BANANA = 3      // normal speed, low acc, normal cooldown
    };
    uint8_t current_fruit = CHERRY;
    uint64_t fruit_count = 0;
    float drop_timer;
    static const int COUNTDOWN = 60;
    float timer;
    int point = 0;

    std::vector<GameObject> fruits;

    void drop_fruit();
    const GameObject get_first_fruit() const;
    void switch_fruit_display(int id);
    void switch_point_display(int point);
    void switch_time_display(float time);

    GameObject::SpriteInput get_fruit_sprite(int id);
    GameObject::SpriteInput get_number_sprite(int number, float x_offset, float y_offset);
    float get_fruit_acceleration(int id) const;
    float get_fruit_speed(int id) const;
    float get_fruit_cooldown(int id) const;
    uint8_t get_point_from_fruit(int id) const;

    // input tracking:
    struct Button
    {
        uint8_t downs = 0;
        uint8_t pressed = 0;
    } left, right;

    //----- drawing handled by PPU466 -----
    PPU466 ppu;
};
