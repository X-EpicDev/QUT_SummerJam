#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "Object.h"
#include "Player.h"
#include "raylib.h"

enum GameState {
    WAITING,
    PLAYING,
    FINISHED
};

void ArrowControl(Vector2 point1, Vector2 point2, Vector2 point3) {
    DrawTriangle(point1, point2, point3, GREEN);
}

int main() {
    const int windowWidth = 1080;
    const int windowHeight = 720;
    const float movementSpeed = 75.0f;
    bool debug = false;

    GameState gameState = WAITING;
    float timer = 10;
    int score = 0;

    std::vector<Object> objects;

    // Enable config flags for resizable window and vertical synchro
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(windowWidth, windowHeight, "Resize Window");
    SetWindowMinSize(320, 240);

    SetTargetFPS(60);

    RenderTexture2D target = LoadRenderTexture(windowWidth, windowHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

    // Load textures
    Texture2D sheet = LoadTexture("../assets/sheet.png");
    Texture2D officeWalls = LoadTexture("../assets/office_walls.png");
    Texture2D officeFloor = LoadTexture("../assets/office_floor.png");

    // Text
    std::string startText = "Press SPACE or ENTER to begin";
    int startTextWidth = MeasureText(startText.c_str(), 20);

    std::string gameOverText = "Game Over";
    int gameOverTextWidth = MeasureText(gameOverText.c_str(), 40);

    // Objects
    Player player(sheet, {16.0001, 0, 15.9999, 15.9999}, {32, 32, 16, 16}, {0, 0, 8, 7}, Vector2{4, 9});
    Object walls(officeWalls, {0, 0, 448, 320}, {0, 0, 448, 320}, {0, 0, 0, 0}, Vector2{0, 0});
    Object floor(officeFloor, {0, 0, 448, 320}, {0, 0, 448, 320}, {0, 0, 0, 0}, Vector2{0, 0});
    Image officeWallsImage = LoadImage("../assets/office_walls.png");
    Color* wallPixels = LoadImageColors(officeWallsImage);
    objects.emplace_back(sheet, Rectangle{0, 0, 15.9999, 15.9999}, Rectangle{16, 16, 16, 16}, Rectangle{0, 0, 16, 16}, Vector2{0, 0});

    Camera2D camera;
    camera.rotation = 0.0f;
    camera.zoom = 4.0f;

    bool arrow_appear = false;
    float tempTimer = 0.0f;

    while (!WindowShouldClose()) {
        const float deltaTime = GetFrameTime();
        //camera.rotation += (deltaTime * 3);

        if (IsKeyPressed(KEY_F3)) {
            debug = !debug;
        }

        // Update
        bool found = false;
        switch (gameState) {
            case WAITING:
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                    gameState = PLAYING;
                }
                break;
            case PLAYING:
                timer -= deltaTime;
                if (timer <= 0) {
                    timer = 0;
                    gameState = FINISHED;
                    break;
                }

                if (IsKeyPressed(KEY_E)) {
                    timer = 10;
                }

                player.input(deltaTime, movementSpeed, wallPixels, &officeWalls);

                for (Object object : objects) {
                    if (CheckCollisionRecs(object.getHitbox(), player.getHitbox())) {
                        player.currentObject = &object;
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    player.currentObject = nullptr;
                }

                break;
            case FINISHED:
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                    timer = 10;
                    score = 0;
                    player.setX(25);
                    player.setY(25);
                    gameState = PLAYING;
                }
                break;
        }

        camera.offset = (Vector2){GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
        camera.target = (Vector2){player.getX() + player.getWidth() / 2, player.getY() + player.getHeight() / 2};

        Vector2 mouse = GetMousePosition();
        Vector2 worldMouse = GetScreenToWorld2D(mouse, camera);

        // Drawing
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(camera);

        floor.draw(debug);
        walls.draw(debug);

        for (Object object : objects) {
            object.draw(debug);
        }

        DrawCircle(100, 75, 12.5, RED);
        if (CheckCollisionCircleRec((Vector2){100, 75}, 12.5, player.getHitbox()) && IsKeyPressed(KEY_E) && arrow_appear == false) {

            //insert a minigame function here
            arrow_appear = true;
            tempTimer = timer;
        }

        if(arrow_appear == true && CheckCollisionCircleRec((Vector2){100, 75}, 12.5, player.getHitbox())) {
            ArrowControl({(player.getXOffset() * 2) + player.getX(), player.getYOffset() + player.getY() - 10}, {((player.getXOffset() * 2) + 10) + player.getX(), player.getYOffset() + player.getY() - 20}, {((player.getXOffset() * 2) - 10) + player.getX(), player.getYOffset() + player.getY() - 20});

            if (timer <= tempTimer - 3) {
                arrow_appear = false;
            }
        }

        player.draw(debug);

        if (player.currentObject != nullptr) {
            DrawText("Press 'E' to interact", player.currentObject->getX() + 4, player.currentObject->getY() - 16, 1, WHITE);
        }

        EndMode2D();

        std::ostringstream stream;
        switch (gameState) {
            case WAITING:
                DrawText(startText.c_str(), GetScreenWidth() / 2 - startTextWidth / 2, GetScreenHeight() / 5 * 3.5f, 20, WHITE);
                break;
            case FINISHED:
                DrawText(gameOverText.c_str(), GetScreenWidth() / 2 - gameOverTextWidth / 2, GetScreenHeight() / 3, 40, WHITE);
                DrawText(startText.c_str(), GetScreenWidth() / 2 - startTextWidth / 2, GetScreenHeight() / 5 * 3.5f, 20, WHITE);
                 //No break so it still renders score/timer for now
            case PLAYING:
                DrawText(("Score: " + std::to_string(score)).c_str(), 0, 0, 20, WHITE);
                // Format to 1 decimal place
                stream << std::fixed << std::setprecision(1) << std::round(timer * 10) / 10.0;
                DrawText(("Time: " + stream.str()).c_str(), 0, 16, 20, WHITE);
                break;
        }

        // Debug
        if (debug) {
            DrawFPS(0, 32);
            DrawText(("Position: " + std::to_string(player.getX()) + "," + std::to_string(player.getY())).c_str(), 0, 48, 20, WHITE);
            DrawText(("DeltaTime: " + std::to_string(deltaTime)).c_str(), 0, 64, 20, WHITE);
        }

        EndDrawing();
    }

    UnloadImageColors(wallPixels);
    UnloadImage(officeWallsImage);
    UnloadTexture(officeWalls);
    UnloadRenderTexture(target);
    CloseWindow();

    return 0;
}
