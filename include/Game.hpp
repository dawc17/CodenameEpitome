#pragma once

#include "raylib.h"
#include "Player.hpp"
#include <memory>
#include <vector>

// Forward declarations
class Player;
class DungeonManager;
class EnemyManager;
class ProjectileManager;
class UIManager;

enum class GameState {
    MENU,
    BUFF_SELECT,  // Initial buff selection before gameplay
    PLAYING,
    PAUSED,
    GAME_OVER,
    FLOOR_CLEAR
};

class Game {
public:
    static Game& Instance();
    
    void Init();
    void Run();
    void Shutdown();
    
    void SetState(GameState state) { m_state = state; }
    GameState GetState() const { return m_state; }
    
    float GetDeltaTime() const { return m_deltaTime; }
    
    Player* GetPlayer() { return m_player.get(); }
    DungeonManager* GetDungeon() { return m_dungeon.get(); }
    EnemyManager* GetEnemies() { return m_enemies.get(); }
    ProjectileManager* GetProjectiles() { return m_projectiles.get(); }
    
    // Game flow
    void StartGameWithBuff(int buffIndex);
    
    // Screen dimensions
    static constexpr int SCREEN_WIDTH = 1280;
    static constexpr int SCREEN_HEIGHT = 720;
    static constexpr int TARGET_FPS = 60;
    
private:
    Game() = default;
    ~Game() = default;
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;
    
    void Update();
    void Render();
    void HandleInput();
    void CheckCollisions();
    void PrepareNewGame();
    void StartNewGame();
    void NextFloor();
    
    GameState m_state = GameState::MENU;
    float m_deltaTime = 0.0f;
    bool m_running = false;
    
    std::unique_ptr<Player> m_player;
    std::unique_ptr<DungeonManager> m_dungeon;
    std::unique_ptr<EnemyManager> m_enemies;
    std::unique_ptr<ProjectileManager> m_projectiles;
    std::unique_ptr<UIManager> m_ui;
    
    // Starting buff selection
    std::vector<BuffData> m_startingBuffs;
    
    // Camera for dungeon view
    Camera2D m_camera = {0};
};
