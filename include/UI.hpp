#pragma once

#include "raylib.h"
#include "Player.hpp"
#include <string>
#include <vector>
#include <functional>

class Player;

class UIManager {
public:
    UIManager();
    ~UIManager() = default;
    
    void Update(float dt);
    void Render();
    
    // HUD
    void RenderHUD(Player* player);
    
    // Menus
    void RenderMainMenu();
    void RenderPauseMenu();
    void RenderGameOver(int score);
    void RenderFloorClear(const std::vector<std::pair<std::string, std::function<void()>>>& buffs);
    void RenderBuffSelection(const std::vector<BuffData>& buffs);
    
    // Shop
    void RenderShop();
    
    // Helpers
    static void DrawHealthBar(Vector2 pos, float width, float height, 
                              int current, int max, Color fillColor);
    static void DrawCooldownIndicator(Vector2 center, float radius, 
                                       float percent, Color color);
    
private:
    // Button helper
    bool Button(Rectangle bounds, const std::string& text, int fontSize = 20);
    
    // Menu state
    int m_selectedOption = 0;
    
    // Animation
    float m_animTimer = 0.0f;
};
