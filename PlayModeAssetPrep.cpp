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
    ret.load(data_path("background.png")); // tile 0 - 3
    ret.load(data_path("fruits1.png")); // tile 4 - 5
    ret.load(data_path("fruits2.png")); // tile 6 - 7
    ret.load(data_path("characters.png")); // tile 8 - 11
    ret.load(data_path("nums.png")); // tile 12 - 23
    ret.gen_background();
    // ret.load_binary(data_path("game.asset"));
    ret.save_assets(data_path("game.asset"));
    std::cout << "Game Asset Saved to " << data_path("game.asset") << "\n";
    return &ret; });

PlayMode::PlayMode()
{
}

PlayMode::~PlayMode()
{
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size)
{
    return false;
}

void PlayMode::update(float elapsed)
{
    // exit
    Mode::set_current(nullptr);
}

const GameObject PlayMode::get_first_fruit() const
{
    GameObject ret = EMPTY;
    uint16_t ly = PPU466::ScreenHeight;

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
        return 2.0f;
    case 2:
        return 1.0f;
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
        return 1;
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
        return 8.0f;
    default:
        return 0;
    }
}

float PlayMode::get_fruit_speed(int id) const
{
    switch (id)
    {
    case 0:
        return 100.0f;
    case 1:
        return 20.0f;
    case 2:
        return 160.0f;
    case 3:
        return 100.0f;
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

void PlayMode::switch_point_display(int point)
{
    scene.remove_game_object("score");
    uint16_t last3 = point % 1000;
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
    GameObject timer{PPU466::ScreenWidth - 8 * 4, PPU466::ScreenHeight - 16, {{12, NumsPalette, 0, 0, false}, get_number_sprite(p1, 8, 0), get_number_sprite(p2, 8 * 2, 0), get_number_sprite(p3, 8 * 3, 0)}};
    scene.add_game_object("timer", timer);
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