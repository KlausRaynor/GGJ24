/**
 * Author: Klaus Holder
 * 1/28/24
 * Game Jam 2024
 * Theme: MAKE ME LAUGH
 * Game Design Doc:
 *
 *
*/


#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdio.h>
#include "rcamera.h"
#include <iostream>
#include <random>
#include <vector>

#define MAX_COLUMNS 20
#define MAX_PROJECTILES 20

// Frame Data for eventual animation of Clownybaras
struct AnimData
{
    Rectangle rec;
    Vector3 pos;
    int frame;
    float updateTime;
    float runningTime;
    int facing{1};
};

struct Projectile
{
    Vector3 position;
    Vector3 speed;
    bool isActive;
    float timeAlive;
};

struct HeartUI
{
    Texture2D fullTex;
    Texture2D emptyTex;
    bool isFull;
    Rectangle rec;
};
enum GameState
{
    START_SCREEN,
    PLAYING,
    GAME_OVER,
    GAME_OVER_WIN,
    GAME_OVER_CLOWNY_DEATH
};



// Function Declarations
bool checkVectorEquality(Vector3 v1, Vector3 v2);
bool checkVectorProximity(Vector3 v1, Vector3 v2);
bool isGrounded(AnimData data, int screenHeight);
void FireProjectile(std::vector<Projectile> &pies, Vector3 startPosition, Vector3 direction, float speed);

AnimData updateAnimData (AnimData data, float deltaTime, int maxFrame)
{
    data.runningTime += deltaTime;
    if (data.runningTime >= data.updateTime)
    {
        data.runningTime = 0.0;
        data.rec.x = data.frame * data.rec.width;
        data.frame++;
        if (data.frame > maxFrame)
        {
            data.frame = 0;
        }
    }
    return data;
}
GameState currentGameState = START_SCREEN;
int main()
{
    const int screenWidth{1200};
    const int screenHeight{800};
    const int cappyFrameCount{14};
    const int grumFrameCount{4};


    // [----------------- WINDOW INITILIZATION -----------------]
    InitWindow(screenWidth, screenHeight, "KlausRaynor's GGJ24 Entry - Clownybara");


    // [----------------- Define Camera-----------------]
    Camera cam{0};
    cam.position = (Vector3){0.0f, 2.0f, 4.0f};     // position
    cam.target = (Vector3){0.0f, 2.0f, 0.0f};       // point camera is looking at/hitting
    cam.up = (Vector3){0.0f, 1.0f, 0.0f};           // up Vector for Camera
    cam.fovy = 60.0f;                               // camera's FOV
    cam.projection = CAMERA_PERSPECTIVE;         // camera projection type

    int cameraMode = CAMERA_FIRST_PERSON;



    // Generate random columns in the room
    int max_col_height = 12;
    float heights[MAX_COLUMNS]{0};
    Vector3 positions[MAX_COLUMNS]{0};
    Color colors[MAX_COLUMNS]{0};

    for (int i = 0; i < MAX_COLUMNS; i++)
    {
        heights[i] = (float)GetRandomValue(1, max_col_height);
        positions[i] = (Vector3){ (float)GetRandomValue(-15, 15), heights[i]/2.0f,
            (float)GetRandomValue(-15, 15)};
        colors[i] = (Color){ (unsigned char)GetRandomValue(20, 255),
            (unsigned char)GetRandomValue(10, 55), 40, 255};
    }

    DisableCursor();

    // [----------------- Load Textures -----------------]
    // CLOWNY
    Texture2D cappy = LoadTexture("assets/art/cappy_ss.png");
    AnimData cappyData;
    cappyData.rec.width = cappy.width / cappyFrameCount;
    cappyData.rec.height = cappy.height;
    cappyData.rec.x = 0;
    cappyData.rec.y = 0;
    cappyData.pos.x = screenWidth / 2 - cappyData.rec.width / 2;
    cappyData.pos.y = screenHeight - cappyData.rec.height;
    cappyData.frame = 0;
    cappyData.updateTime = 1.0/12.0;
    cappyData.runningTime = 0.0;

    Texture2D cappyCry = LoadTexture("assets/art/cappy_cry.png");

    // GRUMULUM
    Texture2D grum = LoadTexture("assets/art/clown_idle_ss.png");
    AnimData grumData;
    grumData.rec.width = grum.width / grumFrameCount;
    grumData.rec.height = grum.height;
    grumData.rec.x = 0;
    grumData.rec.y = 0;
    grumData.pos.x = screenWidth / 2 - grumData.rec.width / 2;
    grumData.pos.y = screenHeight - grumData.rec.height;
    grumData.frame = 0;
    grumData.updateTime = 1.0 / 4.0;
    grumData.runningTime = 0.0;

    // HEART UI
    Texture2D full_heart = LoadTexture("assets/art/full_heart.png");
    Texture2D empty_heart = LoadTexture("assets/art/empty_heart.png");
    bool isAirborne{false};
    float velocity{0};
    float runSpeed{1.f};
    const float jumpHeight{1.5f};
    const float gravity{9.8f};
    float groundLevel{2.f};

    // scaling tests
    // load cappy as an image
    // [-------------- TESTING AREA -----------------------]
    float rotationAngle = 0.0f;
    Vector3 handOffset = {0.5f, -0.6f, .8f};
    bool isPlayerMoving{false};
    Ray ray{0};
    bool drawRay = false;
    bool confirmGameWindowExit{false};
    float shootTimer{0.f};
    float shootInterval = (float)GetRandomValue(2, 5);
    bool showDebugText{false};
    const unsigned int maxHealth{3};
    const unsigned int maxGrumHealth{3};
    const unsigned int maxCappyHealth{1};
    unsigned int cappyHealth = maxCappyHealth;
    unsigned int currentHealth = maxHealth;
    unsigned int grumHealth = maxGrumHealth;
    Vector2 heartUIPos{20, 5};
    Vector3 grumHeartsPos{};

    // [-------------- Initializing Pos, Hearts, and Pies -----------------------]

    Vector3 cappy3DPos = {0.0f, 1.0f, 0.0f};
    Vector3 grum3DPos = {3.0f, 1.0f, 0.0f};

    // Create array of Pies to be shot at player
    int pieNum = 1000;
    float projectileSpeed = 2.5f;
    float playerProjectileSpeed = 50.f;
    std::vector<Projectile> pies;
    std::vector<Projectile> playerProjectiles;
    pies.resize(pieNum);
    for (auto &pie : pies)
    {
        pie = {.position = grum3DPos, .speed = {0.0f, 0.0f, projectileSpeed}, .isActive = false};
    }
    // HEARTS UI
    std::vector<HeartUI> hearts;
    hearts.resize(maxHealth);
    for (auto &heart : hearts)
    {
        heart.fullTex = LoadTexture("assets/art/full_heart.png");
        heart.emptyTex = LoadTexture("assets/art/empty_heart.png");
        heart.isFull = true;
    }

    // GRUM HEARTS
    std::vector<HeartUI> grumHearts;
    hearts.resize(grumHealth);
    for (auto &heart : grumHearts)
    {
        heart.fullTex = LoadTexture("assets/art/full_heart.png");
        heart.emptyTex = LoadTexture("assets/art/empty_heart.png");
        heart.isFull = true;
    }

    // [-------------- RANDOM NUMBER GENERATOR -----------------------]
    // Random number generator for Cappy / Grum movement
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<>distr(-10, 10);
    float randX = distr(gen);
    float randZ = distr(gen);
    printf("randX and randZ: %f, %f\n", randX, randZ);
    float newCappy3DPosX = randX;
    float newCappy3DPosZ = randZ;

                /*
                ** [==============================================================]
                ** [===================== ++MAIN GAME LOOP++ =====================]
                ** [==============================================================]
                */
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        const float dT{GetFrameTime()};

        rotationAngle += dT * 180;
        unsigned int fps = GetFPS();

        // [----------------- Update Camera Vectors -----------------]
        // forward vector
        Vector3 forward = Vector3Subtract(cam.target, cam.position);
        forward.y = 0;
        Vector3Normalize(forward);

        // right vector
        Vector3 right = Vector3CrossProduct(forward, cam.up);
        Vector3Normalize(right);
        // up vector
        Vector3 up = {0.0f, 0.1f, 0.0f};
        Vector3Normalize(up);

        // update camera
        UpdateCamera(&cam, cameraMode);

        // [-------------- HELD OBJECT POSITIONING -------------]
        Vector3 handPosition = cam.position;
        handPosition = Vector3Add(handPosition, Vector3Scale(forward, handOffset.z));
        handPosition = Vector3Add(handPosition, Vector3Scale(right, handOffset.x));
        handPosition = Vector3Add(handPosition, Vector3Scale(up, handOffset.y));

        // check if player is moving and adjust hand motion accordingly
        if(isPlayerMoving)
        {
            float time = GetTime();
            handPosition.y += 0.1f * sinf(time * 4.0f);
            handPosition.x += 0.1f * cosf(time * 4.0f);
        }

        // [----------------- +PROJECTILES+ ------------------]

        shootTimer += dT;
        if (shootTimer >= shootInterval)
        {
            shootTimer = 0.0f;
            shootInterval = GetRandomValue(1.f, 2.f);


            // get camera pos
            Vector3 directionToCamera = Vector3Subtract(cam.position, grum3DPos);
            Vector3Normalize(directionToCamera);

            // FIRE PIE
            FireProjectile(pies, grum3DPos, directionToCamera, projectileSpeed);

        }


        // [----------- MOVEMENT + Action Check -----------------]
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && !isAirborne)
        {
            isAirborne = true;
            velocity = jumpHeight;
        }

        if (IsKeyDown(KEY_SPACE) && !isAirborne)
        {
            cam.position.y = 1.f;
        }
        if (IsKeyReleased(KEY_SPACE))
        {
            cam.position.y = 2.f;
        }


        if(IsKeyReleased(KEY_P))
        {
            showDebugText = !showDebugText;
        }

        if (isAirborne)
        {
            cam.position.y += velocity; // Update camera Y pos.
            velocity -= gravity * dT;
            isPlayerMoving = true;
            if (cam.position.y <= groundLevel)
            {
                cam.position.y = groundLevel;
                isAirborne = false;
                velocity = 0.f;

            }
        }
        if (IsKeyDown(KEY_W))
        {
            Vector3 vStrafe = Vector3Scale(forward, runSpeed *dT);
            cam.position = Vector3Add(cam.position, Vector3Scale(forward, runSpeed *dT));
            cam.target = Vector3Add(cam.target, vStrafe);
            isPlayerMoving = true;
        }
        if(IsKeyDown(KEY_S))
        {
            Vector3 vStrafe = Vector3Scale(forward, runSpeed * dT);
            cam.position = Vector3Subtract(cam.position, Vector3Scale(forward, runSpeed * dT));
            cam.target = Vector3Subtract(cam.target, vStrafe);
            isPlayerMoving = true;
        }

        if (IsKeyDown(KEY_A))
        {
            cappyData.pos.x -= runSpeed;
            // camera adjustments
            Vector3 strafe = Vector3Scale(right, runSpeed * dT);
            cam.position = Vector3Subtract(cam.position, Vector3Scale(right, runSpeed * dT));
            cam.target = Vector3Subtract(cam.target, strafe);
            cappyData.facing = -1;
            isPlayerMoving = true;
        }
        if (IsKeyDown(KEY_D))
        {
            Vector3 strafe = Vector3Scale(right, runSpeed * dT);
            cam.position = Vector3Add(cam.position, Vector3Scale(right, runSpeed * dT));
            cam.target = Vector3Add(cam.target, strafe);
            cappyData.pos.x += runSpeed;
            cappyData.facing = 1;
            isPlayerMoving = true;
        }
        // [------------ SPRINTING ------------------]
        if (IsKeyDown(KEY_LEFT_SHIFT))
        {
            runSpeed = 3;
        }
        if (IsKeyUp(KEY_LEFT_SHIFT))
        {
            runSpeed = 1;
        }
        if (IsKeyReleased(KEY_LEFT_SHIFT))
        {
            runSpeed = 1;
        }
        auto pressedKey = GetKeyPressed();

        if(pressedKey != 0)
        {
            isPlayerMoving = false;
        }

        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            Projectile newProjectile;
            newProjectile.position = cam.position;
            newProjectile.speed = Vector3Scale(Vector3Normalize(Vector3Subtract(cam.target, cam.position)), playerProjectileSpeed);
            newProjectile.isActive = true;
            playerProjectiles.push_back(newProjectile);
        }

        // [----------------- ANIMATE CAPPY & GRUM ------------------]

        if(!isAirborne)
        {
            cappyData = updateAnimData(cappyData, dT, 14);
            grumData = updateAnimData(grumData, dT, 4);
        }


        // [----------------- MOVE SPRITES ------------------]
        Vector3 targetCappyPos = {newCappy3DPosX, cappy3DPos.y, newCappy3DPosZ};
        Vector3 targetGrumPos = {targetCappyPos.x - 1.f, cappy3DPos.y + 1.f, targetCappyPos.z -1.f};

        // Check if cappy is close to target location
        if (!checkVectorProximity(cappy3DPos, targetCappyPos))
        {

            Vector3 direction = Vector3Subtract(targetCappyPos, cappy3DPos);
            Vector3Normalize(direction);
            direction = Vector3Scale(direction, runSpeed * dT);
            cappy3DPos = Vector3Add(cappy3DPos, direction);
        }
        else
        {
            randX = distr(gen);
            randZ = distr(gen);
            newCappy3DPosX = randX;
            newCappy3DPosZ = randZ;
        }
        // CHECK IF Grum is at his target spot
        if (!checkVectorProximity(grum3DPos, targetGrumPos))
        {
            Vector3 direction = Vector3Subtract(targetGrumPos, grum3DPos);
            Vector3Normalize(direction);
            direction = Vector3Scale(direction, (runSpeed -0.5f) * dT);
            grum3DPos = Vector3Add(grum3DPos, direction);
        }

        // Update player projectiles
        for (auto &projectile : playerProjectiles)
        {
            if (projectile.isActive)
            {
                projectile.position = Vector3Add(projectile.position, Vector3Scale(projectile.speed, dT));
                // TODO -- Collision Detection
            }
        }

        // [----------------- BEGIN DRAWING -----------------]
        BeginDrawing();

        if (currentGameState == PLAYING)
        {
            ClearBackground(WHITE);
            BeginMode3D(cam);
            BeginBlendMode(BLEND_ALPHA);

            // [---------------- DRAW ENVIRONMENT ----------------------]
            DrawPlane((Vector3){0.0f, 0.0f, 0.0f}, (Vector2){32.0f, 32.0f}, DARKBROWN); // ground plane
            DrawCube((Vector3){-16.0f, 2.5f, 0.0f}, 1.0f, 5.0f, 32.0f, BLUE);           // BLUE WALL
            DrawCube((Vector3){16.0f, 2.5f, 0.0f}, 1.0f, 5.0f, 32.0f, LIME);            // LIME WALL
            DrawCube((Vector3){0.0f, 2.5f, 16.0f}, 32.0f, 5.0f, 1.0f, GOLD);            // GOLD WALL
            DrawCube((Vector3){0.0f, 2.5f, -16.0f}, 32.0f, 5.0f, 1.0f, DARKGRAY);       // DarkGray WALL

            // [---------------- DRAW PROJECTILE ----------------------]
            // [----------- PIE PROJECTILE ---------------]
            for (auto &pie : pies)
            {
                if(pie.isActive)
                {
                    pie.position = Vector3Add(pie.position, Vector3Scale(pie.speed, dT));

                    if(Vector3Distance(pie.position, cam.position) < .5f || pie.timeAlive >= 100.f)
                    {
                        pie.isActive = false;
                    }
                    else
                    {
                        pie.timeAlive += dT;
                        DrawCubeV(pie.position, (Vector3){0.2f, 0.2f, 0.2f}, GREEN);
                    }

                    if (Vector3Distance(pie.position, cam.position) < 0.5f)
                    {
                        printf("Hit registered\n");
                        currentHealth--;
                    }
                }
            }
            // [----------------- PLAYER PROJECTILE -----------------]
            for (auto &projectile : playerProjectiles)
            {
                if (projectile.isActive)
                {
                    DrawCubeV(projectile.position, (Vector3){0.2f, 0.2f, 0.2f}, PINK);

                    if(Vector3Distance(projectile.position, grum3DPos) < 0.1f || projectile.timeAlive >= 100.f)
                    {
                        projectile.isActive = false;
                    }
                    else
                    {
                        projectile.timeAlive += dT;
                    }

                    if (Vector3Distance (projectile.position, grum3DPos) < 0.3f)
                    {
                        printf("Hit grum\n");
                        grumHealth--;
                        projectile.isActive = false;
                    }
                    if (Vector3Distance (projectile.position, cappy3DPos) < 0.3f)
                    {
                        printf("HIT CAPPY!");
                        cappyHealth--;
                        projectile.isActive = false;
                    }

                }
            }

            // [---------------- DRAW COLUMNS ----------------------]
            for (int i = 0; i < MAX_COLUMNS; i++)
            {
                DrawCube(positions[i], 2.0f, heights[i], 2.0f, colors[i]);
                DrawCubeWires(positions[i], 2.0f, heights[i], 2.0f, MAROON);
            }


            // DEBUG RECT Must be called before EndBlendMode();
            DrawRectangle(600, 5, 330, 150, Fade(SKYBLUE, 0.5f));


            // [---------------- DRAW CAPPY ----------------------]

            // small CAPPY - white background
            DrawBillboardPro(cam, cappy, cappyData.rec, cappy3DPos, {0.f, 1.f, 0.f}, {1.0f, 1.0f}, {0.f, 0.f}, 0.f, WHITE);

            // [---------------- DRAW GRUMULUM ----------------------]
            DrawBillboardPro(cam, grum, grumData.rec, grum3DPos, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f}, 0.f, WHITE);
            // Draw Hand cube
            DrawCubeV(handPosition, (Vector3){0.2f, 0.2f, 0.2f}, PINK);
            DrawCubeWiresV(handPosition, (Vector3){0.2f, 0.2f, 0.2f}, BLACK);

            EndMode3D();
            EndBlendMode();

            // [---------------- DRAW HEART UI -----------------]


            Vector2 heartUIOffset = {full_heart.width * 5.0f + 5, 0};
            int tempHealthVar = currentHealth;
            for (auto &heart : hearts)
            {
                if(tempHealthVar > 0)
                {
                    heart.isFull = true;
                    tempHealthVar--;
                }
                else
                {
                    heart.isFull = false;

                }
                Texture2D drawHearts = heart.isFull ? heart.fullTex : heart.emptyTex;
                DrawTextureEx(drawHearts, heartUIPos, 0.f, 5.f, RAYWHITE);
                heartUIPos.x += heartUIOffset.x;
            }

            heartUIPos = {20, 5};

            // [----------------- DRAW GRUM HEARTS ------------------]

            Vector3 grumHeartOffset = {full_heart.width * 5.0f + 5, 0};
            int grumTempHealth = grumHealth;
            for(auto &heart: grumHearts)
            {
                if (grumTempHealth > 0)
                {
                    heart.isFull = true;
                    grumTempHealth--;
                }
                else
                {
                    heart.isFull = false;
                }
                Texture2D grumHeartTex = heart.isFull ? heart.fullTex : heart.emptyTex;
                DrawBillboardPro(cam, grumHeartTex, {0.f, 0.f, (float)grumHeartTex.width, (float)grumHeartTex.height}, grumHeartsPos, {0.f, 1.f, 0.f}, {1.f, 1.f}, {0.f, 0.f}, 0.f, RAYWHITE);
                grumHeartsPos.z += grumHeartOffset.z;


                            // [---------------- DRAW GRUMULUM ----------------------]
            DrawBillboardPro(cam, grum, grumData.rec, grum3DPos, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f}, 0.f, WHITE);

            }

            // [---------------- DRAW DEBUG TEXT -----------------]
            if(showDebugText)
            {
                Vector2 debugBoxPos{screenWidth - 335, 5};
                unsigned int debugBoxPosX = debugBoxPos.x;
                DrawRectangle(debugBoxPos.x, debugBoxPos.y, 330, 200, Fade(SKYBLUE, 0.5f));
                DrawText(TextFormat("FPS: %i", fps), debugBoxPosX, 15, 30, BLACK);
                DrawText(TextFormat("- Position: (%06.3f, %06.3f, %06.3f)", cam.position.x, cam.position.y, cam.position.z), debugBoxPosX, 60, 10, BLACK);
                DrawText(TextFormat("- Target: (%06.3f, %06.3f, %06.3f)", cam.target.x, cam.target.y, cam.target.z), debugBoxPosX, 75, 10, BLACK);
                DrawText(TextFormat("- Up: (%06.3f, %06.3f, %06.3f)", cam.up.x, cam.up.y, cam.up.z), debugBoxPosX, 90, 10, BLACK);
                DrawText(TextFormat(" Forward Camera (%f, %f, %f)", forward.x, forward.y, forward.z), debugBoxPosX, 105, 10, BLACK);
                DrawText(TextFormat("Current Run Speed: %f", runSpeed), debugBoxPosX, 120, 10, BLACK);
                DrawText(TextFormat("Cappy Current Pos: %f, %f, %f", cappy3DPos.x, cappy3DPos.y, cappy3DPos.z), debugBoxPosX, 135, 10, BLACK);
                DrawText(TextFormat("Cappy Target Pos: %f, %f, %f", targetCappyPos.x, targetCappyPos.y, targetCappyPos.z), debugBoxPosX, 150, 10, BLACK);
            }

            if (currentHealth == 0)
            {
                currentGameState = GAME_OVER;
            }
            if (grumHealth == 0)
            {
                currentGameState = GAME_OVER_WIN;
            }
            if (cappyHealth == 0)
            {
                currentGameState = GAME_OVER_CLOWNY_DEATH;
            }
        }
        // [------------------ LOSE - GAME OVER ------------------]
        else if(currentGameState == GAME_OVER)
        {
            ClearBackground(BLACK);
            DrawText("Game Over", screenWidth / 2 - MeasureText("Game Over", 20) / 2, screenHeight / 2 - 10, 20, RED);
            DrawText("Press R to Restart", screenWidth / 2 - MeasureText("Press R to Restart", 20) / 2, screenHeight / 2 + 20, 20, WHITE);
            if(IsKeyPressed(KEY_R))
            {
                // Reset Game State
                currentGameState = START_SCREEN;
                currentHealth = maxHealth;
                cappy3DPos = {0.f, 1.f, 0.f};

            }
        }
        // [----------------- LOSE - KILLED CLOWNY ---------------]
        else if (currentGameState == GAME_OVER_CLOWNY_DEATH)
        {
            ClearBackground(BLACK);
            DrawText("Game Over - YOU KILLED THE CLOWNYBARA", screenWidth / 2 - MeasureText("Game Over - YOU KILLED THE CLOWNYBARA", 20) / 2, screenHeight / 2 - 10, 20, RED);
            DrawText("Press R to Restart", screenWidth / 2 - MeasureText("Press R to Restart", 20) / 2, screenHeight / 2 + 20, 20, WHITE);
            DrawTextureEx(cappyCry, {(float)(screenWidth / 2) - ((cappyCry.width*3) /2), (float)screenHeight - (cappyCry.height* 3)}, 0.f, 3.f, RAYWHITE);
            if(IsKeyPressed(KEY_R))
            {
                // Reset Game State
                currentGameState = START_SCREEN;
                currentHealth = maxHealth;
                grumHealth = maxGrumHealth;
                cappyHealth = maxCappyHealth;
                cappy3DPos = {0.f, 1.f, 0.f};

            }
        }
        // [------------------ WIN - GAME OVER ------------------]
        else if(currentGameState == GAME_OVER_WIN)
        {
            ClearBackground(BLACK);
            DrawText("YOU WIN! CLOWNYBARA IS SAVED :)", screenWidth / 2 - MeasureText("YOU WIN! CLOWNYBARA IS SAVED :)", 30) / 2, screenHeight / 2 - 10, 30, GREEN);
            DrawText("Press R to Restart", screenWidth / 2 - MeasureText("Press R to Restart", 20) / 2, screenHeight / 2 + 20, 20, WHITE);
            if(IsKeyPressed(KEY_R))
            {
                // Reset Game State
                currentGameState = START_SCREEN;
                currentHealth = maxHealth;
                grumHealth = maxGrumHealth;
                cappy3DPos = {0.f, 1.f, 0.f};
            }
        }

        // [------------------ START MENU ------------------]
        else if(currentGameState == START_SCREEN)
        {
            ClearBackground(LIGHTGRAY);
            DrawText("SAVE THE CLOWNYBARA FROM THE EVIL GRUMULUM", screenWidth / 2 - MeasureText("SAVE THE CLOWNYBARA FROM THE EVIL GRUMULUM", 30) / 2, screenHeight / 2 - 10, 30, BLUE);
            DrawText("Press SPACE to start", screenWidth / 2 - MeasureText("Press SPACE to start", 20) / 2, screenHeight / 2 + 20, 20, WHITE);
            if(IsKeyPressed(KEY_SPACE))
            {
                // Reset Game State
                currentGameState = PLAYING;
            }
        }

        // [----------------- END DRAWING -----------------]
        EndDrawing();
    }
     //[-----------------UNLOAD TEXTURES -----------------]
     UnloadTexture(cappy);
     UnloadTexture(grum);
     UnloadTexture(empty_heart);
     UnloadTexture(full_heart);
     CloseWindow();


    return 0;
}

bool checkVectorEquality(Vector3 v1, Vector3 v2)
{
    return (v1.x == v2.x) && (v1.y == v2.y) && (v1.z == v2.z);
}

bool checkVectorProximity(Vector3 v1, Vector3 v2)
{
    float threshold = 0.1f;
    return Vector3Distance(v1, v2) < threshold;
}

bool isGrounded(AnimData data, int screenHeight)
{
    return data.pos.y >= screenHeight - data.rec.height;
}

void FireProjectile(std::vector<Projectile> &pies, Vector3 startPosition, Vector3 direction, float speed)
{
    for (auto &pie : pies)
    {
        if (!pie.isActive)
        {
            pie.position = startPosition;
            pie.speed = Vector3Scale(direction, speed);
            pie.isActive = true;
            break;
        }
    }
}
