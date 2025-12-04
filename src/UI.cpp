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
    
    // Level display (top-center)
    DungeonManager* dungeon = Game::Instance().GetDungeon();
    if (dungeon) {
        char levelText[32];
        snprintf(levelText, sizeof(levelText), "LEVEL %d-%d", dungeon->GetStage(), dungeon->GetSubLevel());
        int levelWidth = MeasureText(levelText, 28);
        DrawText(levelText, (Game::SCREEN_WIDTH - levelWidth) / 2, padding, 28, WHITE);
        
        // Boss indicator
        if (dungeon->IsBossLevel()) {
            const char* bossText = "BOSS";
            int bossWidth = MeasureText(bossText, 20);
            DrawText(bossText, (Game::SCREEN_WIDTH - bossWidth) / 2, padding + 32, 20, RED);
        }
    }
    
    // Minimap (top-right corner)
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

void UIManager::RenderHub(CharacterType selectedCharacter) {
    // Title
    const char* title = "THE HUB";
    int titleWidth = MeasureText(title, 50);
    DrawText(title, (Game::SCREEN_WIDTH - titleWidth) / 2, 40, 50, WHITE);
    
    const char* subtitle = "Select your character, then enter the portal";
    int subtitleWidth = MeasureText(subtitle, 18);
    DrawText(subtitle, (Game::SCREEN_WIDTH - subtitleWidth) / 2, 100, 18, LIGHTGRAY);
    
    // Character selection boxes
    float boxWidth = 200;
    float boxHeight = 280;
    float spacing = 60;
    float startX = (Game::SCREEN_WIDTH - (2 * boxWidth + spacing)) / 2.0f;
    float y = 180;
    
    // Character 1: Terrorist
    CharacterData terroristData = Player::GetCharacterData(CharacterType::TERRORIST);
    Rectangle terroristBox = {startX, y, boxWidth, boxHeight};
    bool terroristHovered = CheckCollisionPointRec(GetMousePosition(), terroristBox);
    bool terroristSelected = (selectedCharacter == CharacterType::TERRORIST);
    
    Color terroristBg = terroristSelected ? Color{80, 50, 50, 255} : 
                        (terroristHovered ? Color{60, 40, 40, 255} : Color{40, 30, 30, 255});
    DrawRectangleRec(terroristBox, terroristBg);
    DrawRectangleLinesEx(terroristBox, terroristSelected ? 3.0f : 2.0f, 
                         terroristSelected ? GOLD : (terroristHovered ? WHITE : GRAY));
    
    // Character icon (circle)
    DrawCircle(static_cast<int>(terroristBox.x + boxWidth/2), 
               static_cast<int>(terroristBox.y + 60), 35, terroristData.color);
    
    // Name
    int nameWidth = MeasureText(terroristData.name.c_str(), 22);
    DrawText(terroristData.name.c_str(), 
             static_cast<int>(terroristBox.x + (boxWidth - nameWidth) / 2),
             static_cast<int>(terroristBox.y + 110), 22, WHITE);
    
    // Stats
    DrawText("HP: 100  Energy: 100", static_cast<int>(terroristBox.x + 20),
             static_cast<int>(terroristBox.y + 145), 14, LIGHTGRAY);
    DrawText("Weapon: Pistol", static_cast<int>(terroristBox.x + 20),
             static_cast<int>(terroristBox.y + 165), 14, LIGHTGRAY);
    DrawText("Skill: Explosion", static_cast<int>(terroristBox.x + 20),
             static_cast<int>(terroristBox.y + 185), 14, ORANGE);
    
    // Description
    DrawText("AoE damage around", static_cast<int>(terroristBox.x + 20),
             static_cast<int>(terroristBox.y + 220), 12, GRAY);
    DrawText("self", static_cast<int>(terroristBox.x + 20),
             static_cast<int>(terroristBox.y + 235), 12, GRAY);
    
    if (terroristHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Game::Instance().SelectCharacter(CharacterType::TERRORIST);
    }
    
    // Character 2: Counter-Terrorist
    CharacterData ctData = Player::GetCharacterData(CharacterType::COUNTER_TERRORIST);
    Rectangle ctBox = {startX + boxWidth + spacing, y, boxWidth, boxHeight};
    bool ctHovered = CheckCollisionPointRec(GetMousePosition(), ctBox);
    bool ctSelected = (selectedCharacter == CharacterType::COUNTER_TERRORIST);
    
    Color ctBg = ctSelected ? Color{50, 50, 80, 255} : 
                 (ctHovered ? Color{40, 40, 60, 255} : Color{30, 30, 40, 255});
    DrawRectangleRec(ctBox, ctBg);
    DrawRectangleLinesEx(ctBox, ctSelected ? 3.0f : 2.0f, 
                         ctSelected ? GOLD : (ctHovered ? WHITE : GRAY));
    
    // Character icon
    DrawCircle(static_cast<int>(ctBox.x + boxWidth/2), 
               static_cast<int>(ctBox.y + 60), 35, ctData.color);
    
    // Name
    nameWidth = MeasureText(ctData.name.c_str(), 22);
    DrawText(ctData.name.c_str(), 
             static_cast<int>(ctBox.x + (boxWidth - nameWidth) / 2),
             static_cast<int>(ctBox.y + 110), 22, WHITE);
    
    // Stats
    DrawText("HP: 110  Energy: 90", static_cast<int>(ctBox.x + 20),
             static_cast<int>(ctBox.y + 145), 14, LIGHTGRAY);
    DrawText("Weapon: Burst Rifle", static_cast<int>(ctBox.x + 20),
             static_cast<int>(ctBox.y + 165), 14, LIGHTGRAY);
    DrawText("Skill: Flashbang", static_cast<int>(ctBox.x + 20),
             static_cast<int>(ctBox.y + 185), 14, SKYBLUE);
    
    // Description
    DrawText("Immobilize enemies", static_cast<int>(ctBox.x + 20),
             static_cast<int>(ctBox.y + 220), 12, GRAY);
    DrawText("in radius", static_cast<int>(ctBox.x + 20),
             static_cast<int>(ctBox.y + 235), 12, GRAY);
    
    if (ctHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Game::Instance().SelectCharacter(CharacterType::COUNTER_TERRORIST);
    }
    
    // Portal
    float portalWidth = 150;
    float portalHeight = 80;
    Rectangle portalBox = {
        (Game::SCREEN_WIDTH - portalWidth) / 2.0f,
        static_cast<float>(Game::SCREEN_HEIGHT - 150),
        portalWidth,
        portalHeight
    };
    
    bool portalHovered = CheckCollisionPointRec(GetMousePosition(), portalBox);
    
    // Animated portal glow
    float glowIntensity = (sinf(m_animTimer * 3.0f) + 1.0f) / 2.0f;
    Color portalColor = ColorAlpha(PURPLE, 0.5f + glowIntensity * 0.3f);
    
    DrawRectangleRec(portalBox, portalColor);
    DrawRectangleLinesEx(portalBox, portalHovered ? 4.0f : 2.0f, 
                         portalHovered ? WHITE : VIOLET);
    
    const char* portalText = "ENTER";
    int portalTextWidth = MeasureText(portalText, 24);
    DrawText(portalText, 
             static_cast<int>(portalBox.x + (portalWidth - portalTextWidth) / 2),
             static_cast<int>(portalBox.y + (portalHeight - 24) / 2),
             24, WHITE);
    
    if (portalHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Game::Instance().EnterPortal();
    }
    
    // Hint at bottom
    const char* hintText = "Click a character to select, then click the portal to start your run";
    int hintWidth = MeasureText(hintText, 14);
    DrawText(hintText, (Game::SCREEN_WIDTH - hintWidth) / 2, 
             Game::SCREEN_HEIGHT - 40, 14, GRAY);
}

void UIManager::RenderRunResults(int score, int stage, int subLevel, CharacterType characterUsed) {
    DrawRectangle(0, 0, Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT,
                  ColorAlpha(BLACK, 0.9f));
    
    // Title
    const char* title = "RUN COMPLETE";
    int titleWidth = MeasureText(title, 50);
    DrawText(title, (Game::SCREEN_WIDTH - titleWidth) / 2, 100, 50, RED);
    
    // Character used
    CharacterData charData = Player::GetCharacterData(characterUsed);
    char charText[64];
    snprintf(charText, sizeof(charText), "Character: %s", charData.name.c_str());
    int charWidth = MeasureText(charText, 24);
    DrawText(charText, (Game::SCREEN_WIDTH - charWidth) / 2, 200, 24, LIGHTGRAY);
    
    // Stats box
    int boxWidth = 300;
    int boxHeight = 150;
    int boxX = (Game::SCREEN_WIDTH - boxWidth) / 2;
    int boxY = 260;
    
    DrawRectangle(boxX, boxY, boxWidth, boxHeight, Color{40, 40, 60, 255});
    DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, WHITE);
    
    // Level reached
    char levelText[64];
    snprintf(levelText, sizeof(levelText), "Reached: Level %d-%d", stage, subLevel);
    int levelWidth = MeasureText(levelText, 22);
    DrawText(levelText, (Game::SCREEN_WIDTH - levelWidth) / 2, boxY + 30, 22, WHITE);
    
    // Score
    char scoreText[64];
    snprintf(scoreText, sizeof(scoreText), "Currency Earned: %d", score);
    int scoreWidth = MeasureText(scoreText, 22);
    DrawText(scoreText, (Game::SCREEN_WIDTH - scoreWidth) / 2, boxY + 70, 22, GOLD);
    
    // Encouragement
    DrawText("Keep improving!", (Game::SCREEN_WIDTH - MeasureText("Keep improving!", 18)) / 2,
             boxY + 110, 18, GRAY);
    
    // Continue prompt
    float alpha = (sinf(m_animTimer * 3.0f) + 1.0f) / 2.0f;
    Color continueColor = ColorAlpha(WHITE, 0.5f + alpha * 0.5f);
    
    const char* continueText = "Press ENTER or SPACE to return to hub";
    int continueWidth = MeasureText(continueText, 20);
    DrawText(continueText, (Game::SCREEN_WIDTH - continueWidth) / 2, 
             Game::SCREEN_HEIGHT - 100, 20, continueColor);
}

void UIManager::RenderDebugMenu() {
    // Semi-transparent background
    DrawRectangle(0, 0, Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT, 
                  ColorAlpha(BLACK, 0.85f));
    
    // Title
    const char* title = "DEBUG MENU";
    int titleWidth = MeasureText(title, 40);
    DrawText(title, (Game::SCREEN_WIDTH - titleWidth) / 2, 30, 40, RED);
    
    const char* hint = "Press F1 to close";
    int hintWidth = MeasureText(hint, 16);
    DrawText(hint, (Game::SCREEN_WIDTH - hintWidth) / 2, 75, 16, GRAY);
    
    int panelWidth = 280;
    int panelHeight = 520;
    int panelSpacing = 40;
    int totalWidth = 3 * panelWidth + 2 * panelSpacing;
    int startX = (Game::SCREEN_WIDTH - totalWidth) / 2;
    int panelY = 110;
    
    // ========== WEAPONS PANEL ==========
    int weaponPanelX = startX;
    DrawRectangle(weaponPanelX, panelY, panelWidth, panelHeight, Color{40, 40, 60, 255});
    DrawRectangleLines(weaponPanelX, panelY, panelWidth, panelHeight, PURPLE);
    
    const char* weaponsTitle = "WEAPONS";
    int weaponsTitleW = MeasureText(weaponsTitle, 24);
    DrawText(weaponsTitle, weaponPanelX + (panelWidth - weaponsTitleW) / 2, panelY + 15, 24, PURPLE);
    
    const char* weaponNames[] = {"Pistol", "Shotgun", "SMG", "Magic Wand", "Heavy Cannon", "Burst Rifle"};
    Color weaponColors[] = {YELLOW, ORANGE, YELLOW, PURPLE, RED, ORANGE};
    
    for (int i = 0; i < 6; ++i) {
        Rectangle btnRect = {
            static_cast<float>(weaponPanelX + 20),
            static_cast<float>(panelY + 55 + i * 55),
            static_cast<float>(panelWidth - 40),
            45
        };
        
        bool hovered = CheckCollisionPointRec(GetMousePosition(), btnRect);
        Color bgColor = hovered ? Color{80, 80, 120, 255} : Color{50, 50, 80, 255};
        
        DrawRectangleRec(btnRect, bgColor);
        DrawRectangleLinesEx(btnRect, 2, hovered ? weaponColors[i] : GRAY);
        
        int nameW = MeasureText(weaponNames[i], 18);
        DrawText(weaponNames[i], 
                 static_cast<int>(btnRect.x + (btnRect.width - nameW) / 2),
                 static_cast<int>(btnRect.y + 13), 18, WHITE);
        
        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Game::Instance().DebugEquipWeapon(i);
        }
    }
    
    // ========== ENEMIES PANEL ==========
    int enemyPanelX = startX + panelWidth + panelSpacing;
    DrawRectangle(enemyPanelX, panelY, panelWidth, panelHeight, Color{40, 40, 60, 255});
    DrawRectangleLines(enemyPanelX, panelY, panelWidth, panelHeight, GREEN);
    
    const char* enemiesTitle = "ENEMIES";
    int enemiesTitleW = MeasureText(enemiesTitle, 24);
    DrawText(enemiesTitle, enemyPanelX + (panelWidth - enemiesTitleW) / 2, panelY + 15, 24, GREEN);
    
    const char* enemyNames[] = {"Slime", "Skeleton", "Bat", "Goblin", "Golem (Mini Boss)"};
    
    for (int i = 0; i < 5; ++i) {
        Rectangle btnRect = {
            static_cast<float>(enemyPanelX + 20),
            static_cast<float>(panelY + 55 + i * 55),
            static_cast<float>(panelWidth - 40),
            45
        };
        
        bool hovered = CheckCollisionPointRec(GetMousePosition(), btnRect);
        Color bgColor = hovered ? Color{80, 120, 80, 255} : Color{50, 80, 50, 255};
        
        DrawRectangleRec(btnRect, bgColor);
        DrawRectangleLinesEx(btnRect, 2, hovered ? GREEN : GRAY);
        
        int nameW = MeasureText(enemyNames[i], 18);
        DrawText(enemyNames[i], 
                 static_cast<int>(btnRect.x + (btnRect.width - nameW) / 2),
                 static_cast<int>(btnRect.y + 13), 18, WHITE);
        
        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Game::Instance().DebugSpawnEnemy(i);
        }
    }
    
    // Clear all enemies button
    Rectangle clearEnemiesBtn = {
        static_cast<float>(enemyPanelX + 20),
        static_cast<float>(panelY + 55 + 5 * 55 + 20),
        static_cast<float>(panelWidth - 40),
        50
    };
    
    bool clearHovered = CheckCollisionPointRec(GetMousePosition(), clearEnemiesBtn);
    DrawRectangleRec(clearEnemiesBtn, clearHovered ? Color{120, 60, 60, 255} : Color{80, 40, 40, 255});
    DrawRectangleLinesEx(clearEnemiesBtn, 2, clearHovered ? RED : MAROON);
    
    const char* clearText = "CLEAR ALL ENEMIES";
    int clearW = MeasureText(clearText, 16);
    DrawText(clearText, 
             static_cast<int>(clearEnemiesBtn.x + (clearEnemiesBtn.width - clearW) / 2),
             static_cast<int>(clearEnemiesBtn.y + 17), 16, WHITE);
    
    if (clearHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Game::Instance().DebugClearEnemies();
    }
    
    // ========== GAME CONTROLS PANEL ==========
    int controlPanelX = startX + 2 * (panelWidth + panelSpacing);
    DrawRectangle(controlPanelX, panelY, panelWidth, panelHeight, Color{40, 40, 60, 255});
    DrawRectangleLines(controlPanelX, panelY, panelWidth, panelHeight, SKYBLUE);
    
    const char* controlsTitle = "GAME CONTROLS";
    int controlsTitleW = MeasureText(controlsTitle, 24);
    DrawText(controlsTitle, controlPanelX + (panelWidth - controlsTitleW) / 2, panelY + 15, 24, SKYBLUE);
    
    // Character selection sub-section
    DrawText("Change Character:", controlPanelX + 20, panelY + 55, 18, WHITE);
    
    // Terrorist button
    Rectangle terroristBtn = {
        static_cast<float>(controlPanelX + 20),
        static_cast<float>(panelY + 85),
        static_cast<float>(panelWidth - 40),
        45
    };
    
    bool terroristHovered = CheckCollisionPointRec(GetMousePosition(), terroristBtn);
    DrawRectangleRec(terroristBtn, terroristHovered ? Color{100, 60, 60, 255} : Color{70, 40, 40, 255});
    DrawRectangleLinesEx(terroristBtn, 2, terroristHovered ? Color{180, 80, 80, 255} : GRAY);
    
    const char* terroristText = "Terrorist";
    int terroristW = MeasureText(terroristText, 18);
    DrawText(terroristText, 
             static_cast<int>(terroristBtn.x + (terroristBtn.width - terroristW) / 2),
             static_cast<int>(terroristBtn.y + 13), 18, WHITE);
    
    if (terroristHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Game::Instance().DebugChangeCharacter(CharacterType::TERRORIST);
    }
    
    // Counter-Terrorist button
    Rectangle ctBtn = {
        static_cast<float>(controlPanelX + 20),
        static_cast<float>(panelY + 140),
        static_cast<float>(panelWidth - 40),
        45
    };
    
    bool ctHovered = CheckCollisionPointRec(GetMousePosition(), ctBtn);
    DrawRectangleRec(ctBtn, ctHovered ? Color{60, 60, 100, 255} : Color{40, 40, 70, 255});
    DrawRectangleLinesEx(ctBtn, 2, ctHovered ? Color{80, 80, 180, 255} : GRAY);
    
    const char* ctText = "Counter-Terrorist";
    int ctW = MeasureText(ctText, 18);
    DrawText(ctText, 
             static_cast<int>(ctBtn.x + (ctBtn.width - ctW) / 2),
             static_cast<int>(ctBtn.y + 13), 18, WHITE);
    
    if (ctHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Game::Instance().DebugChangeCharacter(CharacterType::COUNTER_TERRORIST);
    }
    
    // Separator
    DrawLine(controlPanelX + 20, panelY + 210, controlPanelX + panelWidth - 20, panelY + 210, GRAY);
    
    // Quick actions
    DrawText("Quick Actions:", controlPanelX + 20, panelY + 225, 18, WHITE);
    
    // Restore Health button
    Rectangle healBtn = {
        static_cast<float>(controlPanelX + 20),
        static_cast<float>(panelY + 255),
        static_cast<float>(panelWidth - 40),
        45
    };
    
    bool healHovered = CheckCollisionPointRec(GetMousePosition(), healBtn);
    DrawRectangleRec(healBtn, healHovered ? Color{60, 100, 60, 255} : Color{40, 70, 40, 255});
    DrawRectangleLinesEx(healBtn, 2, healHovered ? GREEN : GRAY);
    
    const char* healText = "Restore Full Health";
    int healW = MeasureText(healText, 16);
    DrawText(healText, 
             static_cast<int>(healBtn.x + (healBtn.width - healW) / 2),
             static_cast<int>(healBtn.y + 14), 16, WHITE);
    
    if (healHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Player* player = Game::Instance().GetPlayer();
        if (player) {
            player->Heal(player->GetMaxHealth());
        }
    }
    
    // Restore Energy button
    Rectangle energyBtn = {
        static_cast<float>(controlPanelX + 20),
        static_cast<float>(panelY + 310),
        static_cast<float>(panelWidth - 40),
        45
    };
    
    bool energyHovered = CheckCollisionPointRec(GetMousePosition(), energyBtn);
    DrawRectangleRec(energyBtn, energyHovered ? Color{60, 60, 120, 255} : Color{40, 40, 80, 255});
    DrawRectangleLinesEx(energyBtn, 2, energyHovered ? BLUE : GRAY);
    
    const char* energyText = "Restore Full Energy";
    int energyW = MeasureText(energyText, 16);
    DrawText(energyText, 
             static_cast<int>(energyBtn.x + (energyBtn.width - energyW) / 2),
             static_cast<int>(energyBtn.y + 14), 16, WHITE);
    
    if (energyHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Player* player = Game::Instance().GetPlayer();
        if (player) {
            player->RestoreFullEnergy();
        }
    }
    
    // Add Currency button
    Rectangle currencyBtn = {
        static_cast<float>(controlPanelX + 20),
        static_cast<float>(panelY + 365),
        static_cast<float>(panelWidth - 40),
        45
    };
    
    bool currencyHovered = CheckCollisionPointRec(GetMousePosition(), currencyBtn);
    DrawRectangleRec(currencyBtn, currencyHovered ? Color{100, 90, 40, 255} : Color{70, 60, 30, 255});
    DrawRectangleLinesEx(currencyBtn, 2, currencyHovered ? GOLD : GRAY);
    
    const char* currencyText = "Add 100 Currency";
    int currencyW = MeasureText(currencyText, 16);
    DrawText(currencyText, 
             static_cast<int>(currencyBtn.x + (currencyBtn.width - currencyW) / 2),
             static_cast<int>(currencyBtn.y + 14), 16, WHITE);
    
    if (currencyHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Player* player = Game::Instance().GetPlayer();
        if (player) {
            player->AddRunCurrency(100);
        }
    }
    
    // Separator
    DrawLine(controlPanelX + 20, panelY + 430, controlPanelX + panelWidth - 20, panelY + 430, GRAY);
    
    // End Game button
    Rectangle endGameBtn = {
        static_cast<float>(controlPanelX + 20),
        static_cast<float>(panelY + 450),
        static_cast<float>(panelWidth - 40),
        50
    };
    
    bool endHovered = CheckCollisionPointRec(GetMousePosition(), endGameBtn);
    DrawRectangleRec(endGameBtn, endHovered ? Color{150, 40, 40, 255} : Color{100, 30, 30, 255});
    DrawRectangleLinesEx(endGameBtn, 3, endHovered ? RED : MAROON);
    
    const char* endText = "END GAME";
    int endW = MeasureText(endText, 20);
    DrawText(endText, 
             static_cast<int>(endGameBtn.x + (endGameBtn.width - endW) / 2),
             static_cast<int>(endGameBtn.y + 15), 20, WHITE);
    
    if (endHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Game::Instance().DebugEndGame();
        Game::Instance().ToggleDebugMenu();  // Close menu after ending game
    }
}
