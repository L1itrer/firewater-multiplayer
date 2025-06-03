
extern "C" {
#include <raylib.h>
#include "net.h"
}
#include <vector>
#include <algorithm>
#include <cmath>
#include <string>
#include <cassert>
#include <unordered_map>
#include <memory>
#include "common.h"

using namespace std;

#define ACTION_LEFT 0
#define ACTION_RIGHT 1
#define ACTION_JUMP 2
#define ACTION_UNUSED 3
#define ACTION_COUNT 4

class ResourceManager {
public:
    // Tekstura wczytywana raz, potem przechowywana w mapie po kluczu (np. "player_fire")
    static Texture2D& LoadTexture(const std::string& key, const std::string& filePath) {
        if (textures.find(key) == textures.end()) {
            textures[key] = ::LoadTexture(filePath.c_str());
        }
        return textures[key];
    }

    static void UnloadAll() {
        for (auto& kv : textures) {
            ::UnloadTexture(kv.second);
        }
        textures.clear();
    }

private:
    static std::unordered_map<std::string, Texture2D> textures;
};

std::unordered_map<std::string, Texture2D> ResourceManager::textures;

struct Vec2 {
    float x, y;
    Vec2(float _x = 0, float _y = 0) : x(_x), y(_y) {}
};

struct InputState {
    bool moveLeft = false;
    bool moveRight = false;
    bool jump = false;
    // w przyslosci dodac akcje specjalne
};


class Entity {
public:
    Vec2   position;
    Vec2   velocity;
    bool   active = true;

    Entity(const Vec2& pos) : position(pos), velocity(0, 0) {}
    virtual ~Entity() = default;

    virtual void Update(float dt, const std::vector<Rectangle>& platforms) = 0;
    virtual void Draw() = 0;
};

class Player : public Entity {
public:
    enum Type { FIRE, WATER };
    int       id;               // na przyszłość – do rozróżniania graczy w sieci
    Type      type;
    float     speed = 200.0f;
    float     jumpVel = -500.0f;
    float     radius = 20.0f;   // jeżeli chcemy kolizje okrągłe – lub zastąmy prostokątem
    Color     drawColor;           // gdy brak tekstury
    Texture2D spriteTexture;       
    Rectangle      spriteSrc;    
    // int playerColor;
    InputState input;

    bool      onGround = false;

    Player(int _id, Type _type, const Vec2& startPos)
        : Entity(startPos), id(_id), type(_type)
    {
        if (type == FIRE) {
            drawColor = RED;
            spriteTexture = ResourceManager::LoadTexture("player_fire", "assets/fire.png");
            spriteSrc = { 0, 0, (float)spriteTexture.width, (float)spriteTexture.height };
        }
        else {
            drawColor = BLUE;
            spriteTexture = ResourceManager::LoadTexture("player_water", "assets/water.png");
            spriteSrc = { 0, 0, (float)spriteTexture.width, (float)spriteTexture.height };
        }
    }

    virtual void Update(float dt, const std::vector<Rectangle>& platforms) override {
        HandleMovement(dt);
        ApplyPhysics(dt, platforms);
    }

    virtual void Draw() override {
        if (spriteTexture.id != 0) {
            Vector2 origin = { spriteSrc.width / 2.0f, spriteSrc.height / 2.0f };
            DrawTexturePro(
                spriteTexture,
                spriteSrc,
                { position.x - spriteSrc.width / 2.0f, position.y - spriteSrc.height / 2.0f,
                  spriteSrc.width, spriteSrc.height },
                origin,
                0.0f,
                WHITE
            );
        }
        else {
            DrawCircle((int)position.x, (int)position.y, radius, drawColor);
        }
    }

private:
    void HandleMovement(float dt) {
        velocity.x = 0;

        if (input.moveLeft)  velocity.x = -speed;
        if (input.moveRight) velocity.x = speed;

        if (input.jump && onGround) {
            velocity.y = jumpVel;
            onGround = false;
        }
    }

    void ApplyPhysics(float dt, const std::vector<Rectangle>& platforms) {
        const float GRAVITY = 800.0f;
        velocity.y += GRAVITY * dt;

        position.x += velocity.x * dt;
        position.y += velocity.y * dt;

        onGround = false;
        for (auto& plat : platforms) {
            float nearestX = fmax(plat.x, fmin(position.x, plat.x + plat.width));
            float nearestY = fmax(plat.y, fmin(position.y, plat.y + plat.height));
            float dx = position.x - nearestX;
            float dy = position.y - nearestY;

            if ((dx * dx + dy * dy) < (radius * radius)) {
                if (velocity.y >= 0 && position.y < plat.y) {
                    position.y = plat.y - radius;
                    velocity.y = 0;
                    onGround = true;
                }
            }
        }

        // Granica ekranu od dołu
        if (position.y + radius > 600) {
            position.y = 600 - radius;
            velocity.y = 0;
            onGround = true;
        }

        // Granice poziome okna (zapobiegamy wypadnięciu na boki)
        if (position.x - radius < 0) {
            position.x = radius;
            velocity.x = 0;
        }
        if (position.x + radius > 800) {
            position.x = 800 - radius;
            velocity.x = 0;
        }
    }
};

class Level {
public:
    std::vector<Rectangle> platforms;
    std::vector<std::unique_ptr<Entity>> interactiveEntities;
    std::vector<std::unique_ptr<Entity>> movingPlatforms;

    Level() {
        platforms.push_back({ 0, 580, 800, 20 });     
        platforms.push_back({ 100, 450, 200, 20 });
        platforms.push_back({ 400, 350, 200, 20 });
        platforms.push_back({ 250, 250, 200, 20 });
    }

    void Draw() {
        for (auto& plat : platforms) {
            DrawRectangleRec(plat, GRAY);
        }

        //NA PRZYSZŁOŚĆ:
        for (auto& ie : interactiveEntities) {
            ie->Draw();
        }

        for (auto& mp : movingPlatforms) {
            mp->Draw();
        }
    }
};


class NetworkManager {
public:
    NetworkManager() {

    }

    ~NetworkManager() {
    }



    void SendState(const Player& p) {

    }

};

class Game {
public:
    enum State {
        IN_QUEUE,
        CONNECTING,
        GAME_PLAYING,
        SERVER_FULL
    };
    enum Direction {
        LEFT,
        RIGHT,
        JUMP
    };

    Game(int screenW, int screenH, const std::string& title, State _state)
        : screenWidth(screenW), screenHeight(screenH), windowTitle(title), state(_state)
    {}

    ~Game() {
        ResourceManager::UnloadAll();
        CloseAudioDevice();  // jeśli w przyszłości audio
        CloseWindow();
    }

    void Init() {
        sock_t conn = server_connect("127.0.0.1", "2137");
        if (conn == INVALID_SOCKET) exit(1);
        InitWindow(screenWidth, screenHeight, windowTitle.c_str());
        //InitAudioDevice();        // jeśli audio
        SetTargetFPS(60);


        //camera.target = { players[0]->position.x, players[0]->position.y };
        //camera.offset = { screenWidth / 2.0f, screenHeight / 2.0f };
        //camera.rotation = 0.0f;
        //camera.zoom = 1.0f;
    }

    void Run() {
        while (!WindowShouldClose()) {
            float dt = GetFrameTime();

            //netMgr.PollInputs(players);
            PollInputs();
            if (state == State::GAME_PLAYING)
            {
                this->HandleLocalInput(dt);
                for (auto& p : players) {
                    p->Update(dt, level->platforms);
                }
            }




            //for (auto& p : players) {
            //    netMgr.SendState();
            //}

            //for (auto& mp : level.get()->movingPlatforms {
            //     mp->Update(dt, level->platforms);
            //}

            //for (auto& ie : interactiveEntities) {
            //     ie->Update(dt, level->platforms);
            //}

            //śledzimy pierwszego gracza (Fire)
            //camera.target = { players[0]->position.x, players[0]->position.y };

            // 6) Rysowanie
            BeginDrawing();
            ClearBackground(GetColor(0x181818));

            // rysujemy wszystko w przestrzeni świata (kamery)
            // BeginMode2D(camera);

            Draw();

            // EndMode2D();

            DrawText("Fire  &  Water - Demo", 10, 10, 20, DARKGRAY);

            EndDrawing();
        }
    }

    void Draw()
    {
        switch (state)
        {
            case GAME_PLAYING:
                level->Draw();
                for (auto& p : players) {
                    p->Draw();
                }
                break;
            case CONNECTING:
                // TODO
                break;
            case IN_QUEUE:
                DrawText("IN QUEUE", 100, 200, 56, GOLD);
                break;
            case SERVER_FULL:
                DrawText("SERVER FULL", 100, 200, 56, GOLD);
                break;
        }
    }

    

    void PollInputs() {
        if (server_is_data_available())
        {
            unsigned char buffer[128] = { 0 };
            int32_t packet_size = 0;
            MessageKind message;
            server_recv((const char*)buffer, 128);
            unpack(buffer, "ll", &packet_size, &message);
            printf("Received %d bytes\n", packet_size);
            debug_buffer_print(buffer, packet_size);
            switch (message)
            {
            case GAME_START:
                int local_player;
                unpack(buffer + 8, "l", &local_player);
                Start(local_player);
                break;
            case PLAYER_MOVING:
                int32_t direction;
                bool keydown;
                unpack(buffer + 8, "lc", &direction, &keydown);
                key_change(direction, local_player_id == 0 ? 1 : 0, keydown);
                break;
            case SERVER_FULL:
                state = State::SERVER_FULL;
                break;
            default:
                UNREACHABLE();
                break;
            }
        }
    }
    void Start(int local_player)
    {

        level = std::make_unique<Level>();

        players.push_back(std::make_unique<Player>(local_player, Player::FIRE, Vec2{ 100, 100 }));
        players.push_back(std::make_unique<Player>((local_player == 0 ? 1 : 0), Player::WATER, Vec2{ 200, 100 }));

        this->local_player_id = local_player;
        this->state = State::GAME_PLAYING;
    }
    
    State state;
private:

    int    screenWidth;
    int    screenHeight;
    int local_player_id;
    std::string windowTitle;

    std::unique_ptr<Level> level;
    std::vector<std::unique_ptr<Player>> players;
    NetworkManager netMgr;

    Camera2D camera;

    // Przykład lokalnego sterowania (tylko do testów, bez sieci)
    void HandleLocalInput(float dt) {
        // Gracz 0: WASD
        // players[local_player_id]->input.moveLeft = IsKeyDown(KEY_A);
        // players[local_player_id]->input.moveRight = IsKeyDown(KEY_D);
        // players[local_player_id]->input.jump = IsKeyPressed(KEY_W);
        poll_keyboard();
        // Gracz 1: Strzałki
        // players[1]->input.moveLeft = IsKeyDown(KEY_LEFT);
        // players[1]->input.moveRight = IsKeyDown(KEY_RIGHT);
        // players[1]->input.jump = IsKeyPressed(KEY_UP);
    }
    void poll_keyboard()
    {
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  local_key_change(ACTION_LEFT, true);
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) local_key_change(ACTION_RIGHT, true);
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_SPACE)) local_key_change(ACTION_JUMP, true);
        if (IsKeyUp(KEY_A)   && IsKeyUp(KEY_LEFT))    local_key_change(ACTION_LEFT, false);
        if (IsKeyUp(KEY_D)   && IsKeyUp(KEY_RIGHT))   local_key_change(ACTION_RIGHT, false);
        if (IsKeyUp(KEY_S)   && IsKeyUp(KEY_SPACE))   local_key_change(ACTION_JUMP, false);
    }

    void local_key_change(int key_code, bool keydown)
    {
        bool currentValue = false;
        switch (key_code)
        {
            case ACTION_LEFT:
                currentValue = players[local_player_id]->input.moveLeft;
                break;
            case ACTION_RIGHT:
                currentValue = players[local_player_id]->input.moveRight;
                break;
            case ACTION_JUMP:
                currentValue = players[local_player_id]->input.jump;
                break;
        }
        if (keydown != currentValue)
        {
            send_key_change(key_code, keydown);
        }

        key_change(key_code, 0, keydown);
    }

    void key_change(int key_code, int player_id, bool keydown)
    {
        assert(key_code < 4 && key_code >= 0 && "Invalid key code!\n");
        switch (key_code)
        {
            case ACTION_LEFT:
                players[player_id]->input.moveLeft = keydown;
                break;
            case ACTION_RIGHT:
                players[player_id]->input.moveRight = keydown;
                break;
            case ACTION_JUMP:
                players[player_id]->input.jump = keydown;
                break;

        }
    }
    void send_key_change(int key_code, bool keydown)
    {
        MessageKind msg = PLAYER_MOVING;
        int32_t dir = key_code;
        unsigned char buffer[128] = {0};
        int32_t packet_size = pack(buffer, "lllc", 0, msg, dir, keydown);
        pack(buffer, "l", packet_size);
        printf("Sending %d bytes\n", packet_size);
        debug_buffer_print(buffer, packet_size);
        server_send((const char*)buffer, packet_size);
    }
};


int main() {
    Game game(800, 600, "Fire & Water - Scalable (Raylib + C++)", Game::State::IN_QUEUE);
    game.Init();
    game.Run();
    return 0;
}
