#include <raylib.h>

#include <array>

namespace headless {
    std::array<bool, 512> pressed{};
    std::array<bool, 512> held{};
    int width = 1280;
    int height = 720;

    void reset_keys(){
        pressed.fill(false);
        held.fill(false);
    }
}

extern "C" {
    void SetConfigFlags(unsigned int) {}
    void InitWindow(int width, int height, const char*) {
        headless::width = width;
        headless::height = height;
    }
    bool IsWindowReady(void) { return true; }
    void CloseWindow(void) {}
    bool WindowShouldClose(void) { return false; }
    void SetTargetFPS(int) {}
    int GetScreenWidth(void) { return headless::width; }
    int GetScreenHeight(void) { return headless::height; }
    float GetFrameTime(void) { return 1.0f / 60.0f; }
    void BeginDrawing(void) {}
    void EndDrawing(void) {}
    void ClearBackground(Color) {}
    void DrawRectangle(int, int, int, int, Color) {}
    void DrawRectangleRounded(Rectangle, float, int, Color) {}
    void DrawFPS(int, int) {}
    void DrawText(const char*, int, int, int, Color) {}
    bool IsKeyPressed(int key) {
        return key >= 0 && key < static_cast<int>(headless::pressed.size())
            && headless::pressed[static_cast<std::size_t>(key)];
    }
    bool IsKeyDown(int key) {
        return key >= 0 && key < static_cast<int>(headless::held.size())
            && headless::held[static_cast<std::size_t>(key)];
    }
    bool CheckCollisionRecs(Rectangle a, Rectangle b) {
        return a.x < b.x + b.width && a.x + a.width > b.x
            && a.y < b.y + b.height && a.y + a.height > b.y;
    }
}
