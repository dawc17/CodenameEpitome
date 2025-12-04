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
    HUB,              // Hub area with character selection and portal
    BUFF_SELECT,      // Initial buff selection before gameplay
    PLAYING,
    PAUSED,
    GAME_OVER,
    RUN_RESULTS,      // Show run results after death
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
    void SelectCharacter(CharacterType type);
    void EnterPortal();  // Start run from hub
    void ReturnToHub();  // Return to hub after run results
    void ApplyFloorBuff(int buffIndex); // Apply selected floor buff and continue
    
    // Debug menu
    bool IsDebugMenuOpen() const { return m_debugMenuOpen; }
    void ToggleDebugMenu() { m_debugMenuOpen = !m_debugMenuOpen; }
    void DebugEquipWeapon(int weaponIndex);
    void DebugSpawnEnemy(int enemyType);
    void DebugClearEnemies();
    void DebugChangeCharacter(CharacterType type);
    void DebugEndGame();
    
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
    void NextLevel();    // Progress to next sub-level (or next stage)
    void InitHub();      // Initialize hub state
    void CheckPortalEntry();  // Check if player enters portal
    void ShowBuffSelection(); // Show buff selection screen
    void ShowFloorBuffSelection(); // Show floor clear buff selection
    
    GameState m_state = GameState::MENU;
    float m_deltaTime = 0.0f;
    bool m_running = false;
    
    // Hub state
    CharacterType m_selectedCharacter = CharacterType::TERRORIST;
    Rectangle m_portalBounds = {0};
    std::vector<Rectangle> m_characterSelectBounds;
    
    // Level tracking
    int m_currentStage = 1;
    int m_currentSubLevel = 1;
    
    // Input blocking (to prevent shooting when clicking UI)
    bool m_blockInputThisFrame = false;
    
    // Debug menu
    bool m_debugMenuOpen = false;
    
    // Floor buff selection (true when selecting after floor clear, false for starting buffs)
    bool m_isFloorBuffSelection = false;
    
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
