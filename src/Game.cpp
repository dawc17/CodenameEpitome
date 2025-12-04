#include "Game.hpp"
#include "Player.hpp"
#include "Dungeon.hpp"
#include "Enemy.hpp"
#include "Projectile.hpp"
#include "UI.hpp"
#include "Utils.hpp"
#include <ctime>

Game& Game::Instance() {
    static Game instance;
    return instance;
}

void Game::Init() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Codename: Epitome");
    SetTargetFPS(TARGET_FPS);
    
    // Initialize subsystems
    m_player = std::make_unique<Player>();
    m_dungeon = std::make_unique<DungeonManager>();
    m_enemies = std::make_unique<EnemyManager>();
    m_projectiles = std::make_unique<ProjectileManager>();
    m_ui = std::make_unique<UIManager>();
    
    // Setup camera
    m_camera.target = m_player->GetPosition();
    m_camera.offset = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    m_camera.rotation = 0.0f;
    m_camera.zoom = 1.0f;
    
    m_running = true;
    m_state = GameState::MENU;
}

void Game::Run() {
    while (m_running && !WindowShouldClose()) {
        m_deltaTime = GetFrameTime();
        
        HandleInput();
        Update();
        Render();
    }
}

void Game::Shutdown() {
    m_player.reset();
    m_dungeon.reset();
    m_enemies.reset();
    m_projectiles.reset();
    m_ui.reset();
    
    CloseWindow();
}

void Game::Update() {
    switch (m_state) {
        case GameState::MENU:
            // Menu updates handled in UI
            break;
            
        case GameState::BUFF_SELECT:
            // Buff selection handled in UI
            break;
            
        case GameState::PLAYING:
            m_player->Update(m_deltaTime);
            m_dungeon->Update(m_deltaTime);
            m_enemies->Update(m_deltaTime);
            m_projectiles->Update(m_deltaTime);
            
            // Update camera to follow player
            m_camera.target = m_player->GetPosition();
            
            // Check for collisions
            CheckCollisions();
            
            // Check if player is dead
            if (m_player->GetHealth() <= 0) {
                m_state = GameState::GAME_OVER;
            }
            
            // Check if room is cleared
            if (m_enemies->GetActiveCount() == 0 && m_dungeon->GetCurrentRoom()) {
                m_dungeon->GetCurrentRoom()->SetCleared(true);
            }
            break;
            
        case GameState::PAUSED:
            // Game paused, no updates
            break;
            
        case GameState::GAME_OVER:
            // Wait for restart
            break;
            
        case GameState::FLOOR_CLEAR:
            // Buff selection handled in UI
            break;
    }
    
    m_ui->Update(m_deltaTime);
}

void Game::Render() {
    BeginDrawing();
    ClearBackground(Color{20, 20, 30, 255});
    
    switch (m_state) {
        case GameState::MENU:
            m_ui->RenderMainMenu();
            break;
            
        case GameState::BUFF_SELECT:
            m_ui->RenderBuffSelection(m_startingBuffs);
            break;
            
        case GameState::PLAYING:
        case GameState::PAUSED:
            BeginMode2D(m_camera);
            
            m_dungeon->Render();
            m_player->Render();
            m_enemies->Render();
            m_projectiles->Render();
            
            EndMode2D();
            
            m_ui->RenderHUD(m_player.get());
            
            if (m_state == GameState::PAUSED) {
                m_ui->RenderPauseMenu();
            }
            break;
            
        case GameState::GAME_OVER:
            m_ui->RenderGameOver(m_player->GetRunCurrency());
            break;
            
        case GameState::FLOOR_CLEAR:
            // Render game world behind buff selection
            BeginMode2D(m_camera);
            m_dungeon->Render();
            m_player->Render();
            EndMode2D();
            // Buff selection overlay handled separately
            break;
    }
    
    EndDrawing();
}

void Game::HandleInput() {
    switch (m_state) {
        case GameState::MENU:
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                // Start new game - go to buff selection first
                PrepareNewGame();
            }
            break;
            
        case GameState::BUFF_SELECT:
            // Buff selection handled by UI clicks
            break;
            
        case GameState::PLAYING:
            if (IsKeyPressed(KEY_ESCAPE)) {
                m_state = GameState::PAUSED;
            }
            
            // Shooting
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                m_player->Shoot();
            }
            
            // Ability
            if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                m_player->UseAbility();
            }
            break;
            
        case GameState::PAUSED:
            if (IsKeyPressed(KEY_ESCAPE)) {
                m_state = GameState::PLAYING;
            }
            break;
            
        case GameState::GAME_OVER:
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                m_state = GameState::MENU;
            }
            break;
            
        case GameState::FLOOR_CLEAR:
            // Handled by buff selection UI
            break;
    }
}

void Game::StartNewGame() {
    // Reset player
    m_player->Reset();
    
    // Generate first floor
    unsigned int seed = static_cast<unsigned int>(time(nullptr));
    m_dungeon->Generate(seed, 1);
    
    // Place player at start room spawn point
    if (m_dungeon->GetCurrentRoom()) {
        m_player->SetPosition(m_dungeon->GetCurrentRoom()->GetPlayerSpawnPoint());
    }
    
    // Spawn enemies in current room
    if (m_dungeon->GetCurrentRoom()) {
        m_enemies->SpawnEnemiesInRoom(
            m_dungeon->GetCurrentRoom()->GetEnemySpawnPoints(),
            m_dungeon->GetFloorNumber()
        );
    }
    
    m_state = GameState::PLAYING;
}

void Game::PrepareNewGame() {
    // Reset player first
    m_player->Reset();
    
    // Generate 3 random starting buffs
    m_startingBuffs = Player::GetRandomBuffs(3);
    
    m_state = GameState::BUFF_SELECT;
}

void Game::StartGameWithBuff(int buffIndex) {
    if (buffIndex >= 0 && buffIndex < static_cast<int>(m_startingBuffs.size())) {
        m_player->ApplyBuff(m_startingBuffs[buffIndex]);
    }
    
    // Generate first floor
    unsigned int seed = static_cast<unsigned int>(time(nullptr));
    m_dungeon->Generate(seed, 1);
    
    // Place player at start room spawn point
    if (m_dungeon->GetCurrentRoom()) {
        m_player->SetPosition(m_dungeon->GetCurrentRoom()->GetPlayerSpawnPoint());
    }
    
    // Spawn enemies in current room
    if (m_dungeon->GetCurrentRoom()) {
        m_enemies->SpawnEnemiesInRoom(
            m_dungeon->GetCurrentRoom()->GetEnemySpawnPoints(),
            m_dungeon->GetFloorNumber()
        );
    }
    
    m_startingBuffs.clear();
    m_state = GameState::PLAYING;
}

void Game::CheckCollisions() {
    auto& projectiles = m_projectiles->GetProjectiles();
    auto& enemies = m_enemies->GetEnemies();
    
    for (auto& proj : projectiles) {
        if (!proj.IsActive()) continue;
        
        if (proj.IsPlayerOwned()) {
            // Check against enemies
            for (auto& enemy : enemies) {
                if (!enemy || enemy->IsDead()) continue;
                
                if (proj.CollidesWith(*enemy)) {
                    enemy->TakeDamage(proj.GetDamage());
                    if (!proj.IsPiercing()) {
                        proj.MarkForDestroy();
                    }
                    
                    // Drop currency if enemy died
                    if (enemy->IsDead()) {
                        m_player->AddRunCurrency(enemy->GetData().currencyDrop);
                    }
                    break;
                }
            }
        } else {
            // Enemy projectile, check against player
            if (proj.CollidesWith(*m_player)) {
                m_player->TakeDamage(proj.GetDamage());
                proj.MarkForDestroy();
            }
        }
        
        // Check wall collision
        if (!m_dungeon->IsWalkable(proj.GetPosition())) {
            proj.MarkForDestroy();
        }
    }
    
    // Check enemy melee collision with player
    for (auto& enemy : enemies) {
        if (!enemy || enemy->IsDead()) continue;
        
        if (enemy->CollidesWith(*m_player)) {
            // Melee damage handled in enemy AI
        }
    }
    
    // Check door collision
    int roomId, direction;
    if (m_dungeon->CheckDoorCollision(m_player->GetPosition(), roomId, direction)) {
        if (m_dungeon->GetCurrentRoom()->IsCleared()) {
            m_dungeon->TransitionToRoom(roomId, direction);
            m_enemies->Clear();
            
            // Spawn enemies in new room
            if (m_dungeon->GetCurrentRoom()) {
                m_enemies->SpawnEnemiesInRoom(
                    m_dungeon->GetCurrentRoom()->GetEnemySpawnPoints(),
                    m_dungeon->GetFloorNumber()
                );
            }
        }
    }
}

void Game::NextFloor() {
    m_projectiles->Clear();
    m_enemies->Clear();
    
    unsigned int seed = static_cast<unsigned int>(time(nullptr));
    m_dungeon->Generate(seed, m_dungeon->GetFloorNumber() + 1);
    
    if (m_dungeon->GetCurrentRoom()) {
        m_player->SetPosition(m_dungeon->GetCurrentRoom()->GetPlayerSpawnPoint());
        m_enemies->SpawnEnemiesInRoom(
            m_dungeon->GetCurrentRoom()->GetEnemySpawnPoints(),
            m_dungeon->GetFloorNumber()
        );
    }
    
    m_state = GameState::PLAYING;
}
