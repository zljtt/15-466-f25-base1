#include "PlayMode.hpp"

// for the GL_ERRORS() macro:
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "Sprites.hpp"
#include "Scene.hpp"
#include <random>
// for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <ctime>

static const uint8_t Fruit1Palette = 1;     // 1
static const uint8_t Fruit2Palette = 2;     // 2
static const uint8_t CharactersPalette = 3; // 3
static const uint8_t NumsPalette = 4;       // 4

Load<Sprites> sprites(LoadTagEarly, []() -> Sprites const *
                      {
    static Sprites ret {};
    // ret.load(data_path("background.png")); // tile 0 - 3
    // ret.load(data_path("fruits1.png")); // tile 4 - 5
    // ret.load(data_path("fruits2.png")); // tile 6 - 7
    // ret.load(data_path("characters.png")); // tile 8 - 11
    // ret.load(data_path("nums.png")); // tile 12 - 23
    ret.load_binary(data_path("game.asset"));
    // ret.save_assets(data_path("game.asset"));
    return &ret; });

PlayMode::PlayMode()
{
    timer = COUNTDOWN;
    ppu.palette_table = sprites->palette_table;
    ppu.tile_table = sprites->tile_table;
    ppu.background = sprites->background;

    // preparing the scene
    // player
    GameObject player{PPU466::ScreenWidth / 2 - 4, PPU466::ScreenHeight - 8, {{4, Fruit1Palette, 0, 0, false}, {11, CharactersPalette, 0, -8, false}}};
    scene.add_game_object("player", player);
    // catcher
    GameObject catcher{PPU466::ScreenWidth / 2 - 4, 0, {{8, CharactersPalette, -8, 0, false}, {9, CharactersPalette, 0, 0, false}, {10, CharactersPalette, 8, 0, false}}};
    scene.add_game_object("catcher", catcher);

    // current point
    GameObject score_obj{PPU466::ScreenWidth - 8 * 4, PPU466::ScreenHeight - 8, {{12, NumsPalette, 0, 0, false}}};
    scene.add_game_object("score", score_obj);
    // 14 -> 0, 15 -> 1, 16 -> 2, 17 -> 3, 18 -> 4, 19 -> 5, 20 -> 6, 21 -> 7, 22 -> 8, 23 -> 9
    // current time
    GameObject timer_obj{PPU466::ScreenWidth - 8 * 4, PPU466::ScreenHeight - 16, {{13, NumsPalette, 0, 0, false}}};
    scene.add_game_object("timer", timer_obj);
}

PlayMode::~PlayMode()
{
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size)
{
    if (timer < 0)
        return false; // game ends
    if (evt.type == SDL_EVENT_KEY_DOWN)
    {
        if (evt.key.key == SDLK_LEFT)
        {
            left.downs += 1;
            left.pressed = true;
            return true;
        }
        else if (evt.key.key == SDLK_RIGHT)
        {
            right.downs += 1;
            right.pressed = true;
            return true;
        }
        else if (evt.key.key == SDLK_UP || evt.key.key == SDLK_SPACE)
        {
            current_fruit = ((current_fruit + 1) % 4);
            switch_fruit_display(current_fruit);
            return true;
        }
        else if (evt.key.key == SDLK_DOWN)
        {
            current_fruit = ((current_fruit + 3) % 4);

            switch_fruit_display(current_fruit);
            return true;
        }
    }
    else if (evt.type == SDL_EVENT_KEY_UP)
    {
        if (evt.key.key == SDLK_LEFT)
        {
            left.pressed = false;
            return true;
        }
        else if (evt.key.key == SDLK_RIGHT)
        {
            right.pressed = false;
            return true;
        }
    }

    return false;
}

void PlayMode::update(float elapsed)
{
    timer -= elapsed;
    if (timer < 0)
        return; // game ends

    // handle inputs
    constexpr float PlayerSpeed = 60.0f;
    GameObject &player = scene.lookup("player");
    GameObject &catcher = scene.lookup("catcher");
    if (left.pressed)
    {
        // player.move(-PlayerSpeed * elapsed, 0);
        player.move_clamped(-PlayerSpeed * elapsed, 0, 0, PPU466::ScreenWidth - 8, player.pos_y, player.pos_y);
    }
    if (right.pressed)
    {
        // player.move(PlayerSpeed * elapsed, 0);
        player.move_clamped(PlayerSpeed * elapsed, 0, 0, PPU466::ScreenWidth - 8, player.pos_y, player.pos_y);
    }
    left.downs = 0;
    right.downs = 0;

    // move catcher
    constexpr float CatcherSpeed = 45.0f;
    float bottom_fruit_x = get_first_fruit().pos_x;
    float dx = bottom_fruit_x - catcher.pos_x;
    if (dx > 0.5f || dx < -0.5f)
    {
        catcher.move_clamped((dx < 0 ? -CatcherSpeed : CatcherSpeed) * elapsed, 0, 8, PPU466::ScreenWidth - 16, catcher.pos_y, catcher.pos_y);
    }

    // drop
    drop_timer -= elapsed;
    if (drop_timer < 0)
    {
        drop_timer = get_fruit_cooldown(current_fruit);
        drop_fruit();
    }

    // fruits
    std::vector<std::string> to_remove;
    for (auto &[name, go] : scene.game_objects)
    {
        if (name.rfind("fruit", 0) == 0)
        {
            if (go.extra < 0)
                continue;

            go.velocity = {go.velocity.x, go.velocity.y - get_fruit_acceleration(go.extra) * elapsed};
            go.move(go.velocity.x * elapsed, go.velocity.y * elapsed);
            if (glm::distance(catcher.get_pos(), go.get_pos()) < 3)
            {
                // std::cout << "Catched!" << "\n";
                to_remove.push_back(name);
            }
            else if (go.pos_y < 0)
            {
                // std::cout << "Get Point!" << "\n";
                point += get_point_from_fruit(go.extra);
                to_remove.push_back(name);
            }
        }
    }
    // remove dropped fruit
    for (std::string fruit_name : to_remove)
    {
        scene.remove_game_object(fruit_name);
    }

    // modify point/timer number sprites
    switch_point_display(point);
    switch_time_display(timer);
}

const GameObject PlayMode::get_first_fruit() const
{
    GameObject ret = EMPTY;
    float ly = static_cast<float>(PPU466::ScreenHeight);

    for (const auto &[name, go] : scene.game_objects)
    {
        if (name.rfind("fruit", 0) == 0)
        {
            if (go.pos_y < ly)
            {
                ly = go.pos_y;
                ret = go;
            }
        }
    }
    return ret;
}

float PlayMode::get_fruit_cooldown(int id) const
{
    switch (id)
    {
    case 0:
        return 0.6f;
    case 1:
        return 1.8f;
    case 2:
        return 1.2f;
    case 3:
        return 1.0f;
    default:
        return 1.0f;
    }
}

uint8_t PlayMode::get_point_from_fruit(int id) const
{
    switch (id)
    {
    case 0:
        return 1;
    case 1:
        return 2;
    case 2:
        return 1;
    case 3:
        return 1;
    default:
        return 0;
    }
}

float PlayMode::get_fruit_acceleration(int id) const
{
    switch (id)
    {
    case 0:
        return 16.0f;
    case 1:
        return 40.0f;
    case 2:
        return 0.0f;
    case 3:
        return 10.0f;
    default:
        return 0;
    }
}

float PlayMode::get_fruit_speed(int id) const
{
    switch (id)
    {
    case 0:
        return 90.0f;
    case 1:
        return 20.0f;
    case 2:
        return 170.0f;
    case 3:
        return 120.0f;
    default:
        return 0;
    }
}

void PlayMode::drop_fruit()
{
    GameObject player = scene.lookup("player");
    GameObject fruit = {player.pos_x, player.pos_y, {get_fruit_sprite(current_fruit)}};
    fruit.extra = current_fruit;
    fruit.velocity = {0, -get_fruit_speed(current_fruit)};
    scene.add_game_object("fruit" + std::to_string(fruit_count++), fruit);
}

void PlayMode::draw(glm::uvec2 const &drawable_size)
{
    //--- set ppu state based on game state ---
    ppu.sprites = scene.gather_game_object_sprites();

    // background color will be some hsv-like fade:
    ppu.background_color = glm::u8vec4(0, 0, 0, 0);

    //--- actually draw ---
    ppu.draw(drawable_size);
}

GameObject::SpriteInput PlayMode::get_fruit_sprite(int id)
{
    switch (id)
    {
    case 0:
        return {4, Fruit1Palette, 0, 0, false};
    case 1:
        return {5, Fruit1Palette, 0, 0, false};
    case 2:
        return {6, Fruit2Palette, 0, 0, false};
    case 3:
        return {7, Fruit2Palette, 0, 0, false};
    default:
        return {4, Fruit1Palette, 0, 0, false};
    }
}

void PlayMode::switch_fruit_display(int id)
{
    float x = scene.lookup("player").pos_x;
    float y = scene.lookup("player").pos_y;
    scene.remove_game_object("player");
    GameObject new_player = {x, y, {get_fruit_sprite(id), {11, CharactersPalette, 0, -8, false}}};
    scene.add_game_object("player", new_player);
}

void PlayMode::switch_point_display(int p)
{
    scene.remove_game_object("score");
    uint16_t last3 = p % 1000;
    uint16_t p1 = last3 / 100;
    uint16_t p2 = last3 / 10 % 10;
    uint16_t p3 = last3 % 10;
    // std::cout << point << " " << p1 << p2 << p3 << " (point) \n";
    GameObject score{PPU466::ScreenWidth - 8 * 4, PPU466::ScreenHeight - 8, {{13, NumsPalette, 0, 0, false}, get_number_sprite(p1, 8, 0), get_number_sprite(p2, 8 * 2, 0), get_number_sprite(p3, 8 * 3, 0)}};
    scene.add_game_object("score", score);
}

void PlayMode::switch_time_display(float time)
{
    scene.remove_game_object("timer");
    uint16_t last3 = ((uint16_t)time) % 1000;
    uint16_t p1 = last3 / 100;
    uint16_t p2 = last3 / 10 % 10;
    uint16_t p3 = last3 % 10;
    // std::cout << time << " " << p1 << p2 << p3 << " (time) \n";
    GameObject timer_obj{PPU466::ScreenWidth - 8 * 4, PPU466::ScreenHeight - 16, {{12, NumsPalette, 0, 0, false}, get_number_sprite(p1, 8, 0), get_number_sprite(p2, 8 * 2, 0), get_number_sprite(p3, 8 * 3, 0)}};
    scene.add_game_object("timer", timer_obj);
}

GameObject::SpriteInput PlayMode::get_number_sprite(int number, float x_offset, float y_offset)
{
    // zero
    if (number <= 0 || number > 9)
    {
        return {14, NumsPalette, x_offset, y_offset, false};
    }
    return {uint8_t(14 + number), NumsPalette, x_offset, y_offset, false};
}