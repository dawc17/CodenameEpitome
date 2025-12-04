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
    
    // Setup hub bounds
    InitHub();
    
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
            
        case GameState::HUB:
            // Hub is handled by UI clicks
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
                m_state = GameState::RUN_RESULTS;
            }
            
            // Check if room is cleared
            if (m_enemies->GetActiveCount() == 0 && m_dungeon->GetCurrentRoom()) {
                m_dungeon->GetCurrentRoom()->SetCleared(true);
                
                // Check if all rooms are cleared to activate portal
                bool allCleared = true;
                for (const auto& room : m_dungeon->GetAllRooms()) {
                    // Only check normal/treasure rooms, not start room
                    if (room->GetType() != RoomType::START && !room->IsCleared()) {
                        allCleared = false;
                        break;
                    }
                }
                
                if (allCleared && !m_dungeon->IsPortalActive()) {
                    m_dungeon->ActivatePortal();
                }
            }
            
            // Check portal entry
            CheckPortalEntry();
            
            // Check treasure collection
            if (m_dungeon->CheckTreasureCollision(m_player->GetPosition())) {
                // Give player currency for collecting treasure
                int treasureValue = 50 + m_currentStage * 25;
                m_player->AddRunCurrency(treasureValue);
            }
            break;
            
        case GameState::PAUSED:
            // Game paused, no updates
            break;
            
        case GameState::GAME_OVER:
            // Wait for restart
            break;
            
        case GameState::RUN_RESULTS:
            // Wait for user to continue to hub
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
            
        case GameState::HUB:
            m_ui->RenderHub(m_selectedCharacter);
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
            
        case GameState::RUN_RESULTS:
            m_ui->RenderRunResults(m_player->GetRunCurrency(), 
                                   m_currentStage,
                                   m_currentSubLevel,
                                   m_selectedCharacter);
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
    // Reset input block at start of frame
    if (m_blockInputThisFrame) {
        m_blockInputThisFrame = false;
        return;  // Skip input processing this frame
    }
    
    switch (m_state) {
        case GameState::MENU:
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                // Go to hub instead of directly to buff selection
                m_state = GameState::HUB;
            }
            break;
            
        case GameState::HUB:
            // Hub interaction handled by UI
            break;
            
        case GameState::BUFF_SELECT:
            // Buff selection handled by UI clicks
            break;
            
        case GameState::PLAYING:
            if (IsKeyPressed(KEY_ESCAPE)) {
                m_state = GameState::PAUSED;
            }
            
            // Shooting (skip if input blocked this frame)
            if (!m_blockInputThisFrame && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                m_player->Shoot();
            }
            
            // Ability
            if (!m_blockInputThisFrame && (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))) {
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
            
        case GameState::RUN_RESULTS:
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                ReturnToHub();
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
    
    // Reset level tracking
    m_currentStage = 1;
    m_currentSubLevel = 1;
    
    // Generate first level (1-1)
    unsigned int seed = static_cast<unsigned int>(time(nullptr));
    m_dungeon->Generate(seed, m_currentStage, m_currentSubLevel);
    
    // Place player at start room spawn point
    if (m_dungeon->GetCurrentRoom()) {
        m_player->SetPosition(m_dungeon->GetCurrentRoom()->GetPlayerSpawnPoint());
    }
    
    // Spawn enemies in current room
    if (m_dungeon->GetCurrentRoom()) {
        m_enemies->SpawnEnemiesInRoom(
            m_dungeon->GetCurrentRoom()->GetEnemySpawnPoints(),
            m_currentStage  // Use stage as difficulty
        );
    }
    
    m_state = GameState::PLAYING;
}

void Game::PrepareNewGame() {
    // Reset player first
    m_player->Reset();
    
    // Reset level tracking
    m_currentStage = 1;
    m_currentSubLevel = 1;
    
    // Generate first dungeon
    unsigned int seed = static_cast<unsigned int>(time(nullptr));
    m_dungeon->Generate(seed, m_currentStage, m_currentSubLevel);
    
    // Generate 3 random starting buffs
    m_startingBuffs = Player::GetRandomBuffs(3);
    
    m_state = GameState::BUFF_SELECT;
}

void Game::StartGameWithBuff(int buffIndex) {
    if (buffIndex >= 0 && buffIndex < static_cast<int>(m_startingBuffs.size())) {
        m_player->ApplyBuff(m_startingBuffs[buffIndex]);
    }
    
    // Place player at start room spawn point
    if (m_dungeon->GetCurrentRoom()) {
        m_player->SetPosition(m_dungeon->GetCurrentRoom()->GetPlayerSpawnPoint());
    }
    
    // Spawn enemies in current room (only if not already spawned)
    if (m_dungeon->GetCurrentRoom() && !m_dungeon->GetCurrentRoom()->IsCleared()) {
        m_enemies->SpawnEnemiesInRoom(
            m_dungeon->GetCurrentRoom()->GetEnemySpawnPoints(),
            m_currentStage
        );
    }
    
    m_startingBuffs.clear();
    m_blockInputThisFrame = true;  // Prevent shooting on buff click
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
            
            // Only spawn enemies if the new room hasn't been cleared yet
            if (m_dungeon->GetCurrentRoom() && !m_dungeon->GetCurrentRoom()->IsCleared()) {
                int difficulty = (m_currentStage - 1) * 5 + m_currentSubLevel;
                m_enemies->SpawnEnemiesInRoom(
                    m_dungeon->GetCurrentRoom()->GetEnemySpawnPoints(),
                    difficulty
                );
            }
        }
    }
}

void Game::NextLevel() {
    m_projectiles->Clear();
    m_enemies->Clear();
    
    // Progress to next sub-level
    m_currentSubLevel++;
    if (m_currentSubLevel > 5) {
        // Move to next stage
        m_currentStage++;
        m_currentSubLevel = 1;
    }
    
    unsigned int seed = static_cast<unsigned int>(time(nullptr));
    m_dungeon->Generate(seed, m_currentStage, m_currentSubLevel);
    
    if (m_dungeon->GetCurrentRoom()) {
        m_player->SetPosition(m_dungeon->GetCurrentRoom()->GetPlayerSpawnPoint());
        m_enemies->SpawnEnemiesInRoom(
            m_dungeon->GetCurrentRoom()->GetEnemySpawnPoints(),
            m_currentStage
        );
    }
    
    m_state = GameState::PLAYING;
}

void Game::ShowBuffSelection() {
    m_startingBuffs = Player::GetRandomBuffs(3);
    m_state = GameState::BUFF_SELECT;
}

void Game::CheckPortalEntry() {
    if (!m_dungeon->IsPortalActive()) return;
    
    if (m_dungeon->CheckPortalCollision(m_player->GetPosition())) {
        // Progress to next level with buff selection
        m_projectiles->Clear();
        m_enemies->Clear();
        
        // Increment level
        m_currentSubLevel++;
        if (m_currentSubLevel > 5) {
            m_currentStage++;
            m_currentSubLevel = 1;
        }
        
        // Generate new level
        unsigned int seed = static_cast<unsigned int>(time(nullptr));
        m_dungeon->Generate(seed, m_currentStage, m_currentSubLevel);
        
        // Show buff selection
        ShowBuffSelection();
    }
}

void Game::InitHub() {
    // Character selection boxes
    m_characterSelectBounds.clear();
    float boxWidth = 200;
    float boxHeight = 280;
    float spacing = 60;
    float startX = (SCREEN_WIDTH - (2 * boxWidth + spacing)) / 2.0f;
    float y = 180;
    
    m_characterSelectBounds.push_back({startX, y, boxWidth, boxHeight});
    m_characterSelectBounds.push_back({startX + boxWidth + spacing, y, boxWidth, boxHeight});
    
    // Portal bounds (centered at bottom)
    float portalWidth = 150;
    float portalHeight = 80;
    m_portalBounds = {
        (SCREEN_WIDTH - portalWidth) / 2.0f,
        static_cast<float>(SCREEN_HEIGHT - 150),
        portalWidth,
        portalHeight
    };
}

void Game::SelectCharacter(CharacterType type) {
    m_selectedCharacter = type;
    m_player->SetCharacter(type);
}

void Game::EnterPortal() {
    // Prepare new game with selected character
    PrepareNewGame();
}

void Game::ReturnToHub() {
    // Clear game state
    m_projectiles->Clear();
    m_enemies->Clear();
    
    // Reset player
    m_player->Reset();
    
    // Reset level tracking
    m_currentStage = 1;
    m_currentSubLevel = 1;
    
    m_state = GameState::HUB;
}
