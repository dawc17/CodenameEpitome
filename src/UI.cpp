#include "UI.hpp"
#include "Player.hpp"
#include "Weapon.hpp"
#include "Game.hpp"
#include "Dungeon.hpp"

UIManager::UIManager() {
}

void UIManager::Update(float dt) {
    m_animTimer += dt;
}

void UIManager::Render() {
    // Rendering is handled by specific methods based on game state
}

void UIManager::RenderHUD(Player* player) {
    if (!player) return;
    
    const int padding = 20;
    const int barWidth = 200;
    const int barHeight = 20;
    
    // Health bar (top-left)
    DrawHealthBar(
        {static_cast<float>(padding), static_cast<float>(padding)},
        barWidth, barHeight,
        player->GetHealth(), player->GetMaxHealth(),
        RED
    );
    DrawText("HP", padding, padding + barHeight + 2, 14, WHITE);
    
    // Energy bar (below health)
    DrawHealthBar(
        {static_cast<float>(padding), static_cast<float>(padding + barHeight + 20)},
        barWidth, barHeight,
        player->GetEnergy(), player->GetMaxEnergy(),
        BLUE
    );
    DrawText("ENERGY", padding, padding + barHeight * 2 + 22, 14, WHITE);
    
    // Weapon info (top-right)
    Weapon* weapon = player->GetWeapon();
    if (weapon) {
        const char* weaponName = weapon->GetName().c_str();
        int textWidth = MeasureText(weaponName, 20);
        DrawText(weaponName, Game::SCREEN_WIDTH - textWidth - padding, padding, 20, WHITE);
        
        // Weapon cooldown indicator
        if (!weapon->CanFire()) {
            float cooldownPercent = weapon->GetCooldownPercent();
            DrawRectangle(
                Game::SCREEN_WIDTH - padding - 100,
                padding + 25,
                static_cast<int>(100 * (1.0f - cooldownPercent)),
                5,
                YELLOW
            );
        }
    }
    
    // Ability cooldown (bottom-left)
    float abilityCooldown = player->GetAbilityCooldownPercent();
    Vector2 abilityCenter = {
        static_cast<float>(padding + 40),
        static_cast<float>(Game::SCREEN_HEIGHT - padding - 40)
    };
    
    DrawCooldownIndicator(abilityCenter, 35, abilityCooldown, PURPLE);
    DrawText("SKILL", static_cast<int>(abilityCenter.x - 20), 
             static_cast<int>(abilityCenter.y + 40), 14, WHITE);
    
    // Currency (bottom-right)
    char currencyText[32];
    snprintf(currencyText, sizeof(currencyText), "$ %d", player->GetRunCurrency());
    int currencyWidth = MeasureText(currencyText, 24);
    DrawText(currencyText, Game::SCREEN_WIDTH - currencyWidth - padding,
             Game::SCREEN_HEIGHT - padding - 24, 24, GOLD);
    
    // Minimap (top-right corner)
    DungeonManager* dungeon = Game::Instance().GetDungeon();
    if (dungeon) {
        dungeon->RenderMinimap(Game::SCREEN_WIDTH - 200, padding, 1.0f);
    }
}

void UIManager::RenderMainMenu() {
    const char* title = "Codename: Epitome";
    int titleWidth = MeasureText(title, 60);
    DrawText(title, (Game::SCREEN_WIDTH - titleWidth) / 2, 150, 60, WHITE);
    
    // Pulsing start text
    float alpha = (sinf(m_animTimer * 3.0f) + 1.0f) / 2.0f;
    Color startColor = ColorAlpha(WHITE, 0.5f + alpha * 0.5f);
    
    const char* startText = "Press ENTER or SPACE to start";
    int startWidth = MeasureText(startText, 24);
    DrawText(startText, (Game::SCREEN_WIDTH - startWidth) / 2, 400, 24, startColor);
    
    // Controls hint
    const char* controls = "WASD - Move | LMB - Shoot | RMB/SPACE - Ability";
    int controlsWidth = MeasureText(controls, 16);
    DrawText(controls, (Game::SCREEN_WIDTH - controlsWidth) / 2, 
             Game::SCREEN_HEIGHT - 50, 16, GRAY);
}

void UIManager::RenderPauseMenu() {
    // Darken background
    DrawRectangle(0, 0, Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT, 
                  ColorAlpha(BLACK, 0.7f));
    
    const char* pauseText = "PAUSED";
    int pauseWidth = MeasureText(pauseText, 48);
    DrawText(pauseText, (Game::SCREEN_WIDTH - pauseWidth) / 2, 
             Game::SCREEN_HEIGHT / 2 - 50, 48, WHITE);
    
    const char* resumeText = "Press ESC to resume";
    int resumeWidth = MeasureText(resumeText, 20);
    DrawText(resumeText, (Game::SCREEN_WIDTH - resumeWidth) / 2,
             Game::SCREEN_HEIGHT / 2 + 20, 20, GRAY);
}

void UIManager::RenderGameOver(int score) {
    DrawRectangle(0, 0, Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT,
                  ColorAlpha(BLACK, 0.85f));
    
    const char* gameOverText = "GAME OVER";
    int gameOverWidth = MeasureText(gameOverText, 60);
    DrawText(gameOverText, (Game::SCREEN_WIDTH - gameOverWidth) / 2,
             Game::SCREEN_HEIGHT / 2 - 80, 60, RED);
    
    char scoreText[64];
    snprintf(scoreText, sizeof(scoreText), "Score: %d", score);
    int scoreWidth = MeasureText(scoreText, 32);
    DrawText(scoreText, (Game::SCREEN_WIDTH - scoreWidth) / 2,
             Game::SCREEN_HEIGHT / 2, 32, WHITE);
    
    float alpha = (sinf(m_animTimer * 3.0f) + 1.0f) / 2.0f;
    Color retryColor = ColorAlpha(WHITE, 0.5f + alpha * 0.5f);
    
    const char* retryText = "Press ENTER to return to menu";
    int retryWidth = MeasureText(retryText, 20);
    DrawText(retryText, (Game::SCREEN_WIDTH - retryWidth) / 2,
             Game::SCREEN_HEIGHT / 2 + 60, 20, retryColor);
}

void UIManager::RenderFloorClear(
    const std::vector<std::pair<std::string, std::function<void()>>>& buffs) {
    
    DrawRectangle(0, 0, Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT,
                  ColorAlpha(BLACK, 0.75f));
    
    const char* clearText = "FLOOR CLEARED!";
    int clearWidth = MeasureText(clearText, 48);
    DrawText(clearText, (Game::SCREEN_WIDTH - clearWidth) / 2, 100, 48, GREEN);
    
    const char* selectText = "Select a buff:";
    int selectWidth = MeasureText(selectText, 24);
    DrawText(selectText, (Game::SCREEN_WIDTH - selectWidth) / 2, 180, 24, WHITE);
    
    // Draw buff options
    int buffWidth = 250;
    int buffHeight = 80;
    int spacing = 30;
    int totalWidth = static_cast<int>(buffs.size()) * buffWidth + 
                     (static_cast<int>(buffs.size()) - 1) * spacing;
    int startX = (Game::SCREEN_WIDTH - totalWidth) / 2;
    int y = 250;
    
    for (size_t i = 0; i < buffs.size(); ++i) {
        Rectangle buffRect = {
            static_cast<float>(startX + i * (buffWidth + spacing)),
            static_cast<float>(y),
            static_cast<float>(buffWidth),
            static_cast<float>(buffHeight)
        };
        
        bool hovered = CheckCollisionPointRec(GetMousePosition(), buffRect);
        Color bgColor = hovered ? Color{80, 80, 100, 255} : Color{50, 50, 70, 255};
        
        DrawRectangleRec(buffRect, bgColor);
        DrawRectangleLinesEx(buffRect, 2, hovered ? WHITE : GRAY);
        
        // Draw buff name centered
        int textWidth = MeasureText(buffs[i].first.c_str(), 18);
        DrawText(buffs[i].first.c_str(),
                 static_cast<int>(buffRect.x + (buffWidth - textWidth) / 2),
                 static_cast<int>(buffRect.y + buffHeight / 2 - 9),
                 18, WHITE);
        
        // Handle click
        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (buffs[i].second) {
                buffs[i].second();
            }
        }
    }
}

void UIManager::RenderShop() {
    // Shop UI would go here
    DrawText("SHOP (Coming Soon)", 100, 100, 30, WHITE);
}

void UIManager::RenderBuffSelection(const std::vector<BuffData>& buffs) {
    DrawRectangle(0, 0, Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT,
                  ColorAlpha(Color{20, 20, 30, 255}, 1.0f));
    
    const char* titleText = "CHOOSE YOUR STARTING BUFF";
    int titleWidth = MeasureText(titleText, 40);
    DrawText(titleText, (Game::SCREEN_WIDTH - titleWidth) / 2, 80, 40, GOLD);
    
    const char* subtitleText = "Select one buff to begin your run";
    int subtitleWidth = MeasureText(subtitleText, 20);
    DrawText(subtitleText, (Game::SCREEN_WIDTH - subtitleWidth) / 2, 140, 20, LIGHTGRAY);
    
    // Draw buff options
    int buffWidth = 280;
    int buffHeight = 120;
    int spacing = 40;
    int totalWidth = static_cast<int>(buffs.size()) * buffWidth + 
                     (static_cast<int>(buffs.size()) - 1) * spacing;
    int startX = (Game::SCREEN_WIDTH - totalWidth) / 2;
    int y = 220;
    
    for (size_t i = 0; i < buffs.size(); ++i) {
        Rectangle buffRect = {
            static_cast<float>(startX + i * (buffWidth + spacing)),
            static_cast<float>(y),
            static_cast<float>(buffWidth),
            static_cast<float>(buffHeight)
        };
        
        bool hovered = CheckCollisionPointRec(GetMousePosition(), buffRect);
        Color bgColor = hovered ? Color{60, 80, 120, 255} : Color{40, 50, 80, 255};
        Color borderColor = hovered ? GOLD : Color{100, 100, 140, 255};
        
        DrawRectangleRec(buffRect, bgColor);
        DrawRectangleLinesEx(buffRect, hovered ? 3.0f : 2.0f, borderColor);
        
        // Draw buff name centered
        int nameWidth = MeasureText(buffs[i].name.c_str(), 22);
        DrawText(buffs[i].name.c_str(),
                 static_cast<int>(buffRect.x + (buffWidth - nameWidth) / 2),
                 static_cast<int>(buffRect.y + 25),
                 22, WHITE);
        
        // Draw buff description
        int descWidth = MeasureText(buffs[i].description.c_str(), 16);
        DrawText(buffs[i].description.c_str(),
                 static_cast<int>(buffRect.x + (buffWidth - descWidth) / 2),
                 static_cast<int>(buffRect.y + 65),
                 16, LIGHTGRAY);
        
        // Handle click
        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Game::Instance().StartGameWithBuff(static_cast<int>(i));
        }
    }
    
    // Instructions at bottom
    const char* instructText = "Click a buff to start the game";
    int instructWidth = MeasureText(instructText, 18);
    float alpha = (sinf(m_animTimer * 2.0f) + 1.0f) / 2.0f;
    DrawText(instructText, (Game::SCREEN_WIDTH - instructWidth) / 2, 
             Game::SCREEN_HEIGHT - 80, 18, ColorAlpha(WHITE, 0.5f + alpha * 0.5f));
}

void UIManager::DrawHealthBar(Vector2 pos, float width, float height,
                               int current, int max, Color fillColor) {
    // Background
    DrawRectangle(static_cast<int>(pos.x), static_cast<int>(pos.y),
                  static_cast<int>(width), static_cast<int>(height), DARKGRAY);
    
    // Fill
    float percent = static_cast<float>(current) / max;
    DrawRectangle(static_cast<int>(pos.x), static_cast<int>(pos.y),
                  static_cast<int>(width * percent), static_cast<int>(height), fillColor);
    
    // Border
    DrawRectangleLines(static_cast<int>(pos.x), static_cast<int>(pos.y),
                       static_cast<int>(width), static_cast<int>(height), WHITE);
    
    // Text
    char text[32];
    snprintf(text, sizeof(text), "%d/%d", current, max);
    int textWidth = MeasureText(text, 14);
    DrawText(text, static_cast<int>(pos.x + (width - textWidth) / 2),
             static_cast<int>(pos.y + (height - 14) / 2), 14, WHITE);
}

void UIManager::DrawCooldownIndicator(Vector2 center, float radius,
                                       float percent, Color color) {
    // Background circle
    DrawCircleV(center, radius, DARKGRAY);
    
    if (percent <= 0) {
        // Ready - full color
        DrawCircleV(center, radius - 3, color);
        DrawText("READY", static_cast<int>(center.x - 20),
                 static_cast<int>(center.y - 7), 14, WHITE);
    } else {
        // On cooldown - draw partial circle
        DrawCircleV(center, radius - 3, ColorAlpha(color, 0.3f));
        
        // Draw cooldown arc (simplified as a sector)
        float startAngle = -90;
        float endAngle = -90 + 360 * (1.0f - percent);
        DrawCircleSector(center, radius - 3, startAngle, endAngle, 36, color);
    }
    
    // Border
    DrawCircleLines(static_cast<int>(center.x), static_cast<int>(center.y),
                    radius, WHITE);
}

bool UIManager::Button(Rectangle bounds, const std::string& text, int fontSize) {
    bool hovered = CheckCollisionPointRec(GetMousePosition(), bounds);
    Color bgColor = hovered ? Color{80, 80, 100, 255} : Color{50, 50, 70, 255};
    
    DrawRectangleRec(bounds, bgColor);
    DrawRectangleLinesEx(bounds, 2, hovered ? WHITE : GRAY);
    
    int textWidth = MeasureText(text.c_str(), fontSize);
    DrawText(text.c_str(),
             static_cast<int>(bounds.x + (bounds.width - textWidth) / 2),
             static_cast<int>(bounds.y + (bounds.height - fontSize) / 2),
             fontSize, WHITE);
    
    return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}
