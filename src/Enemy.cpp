#include "Enemy.hpp"
#include "Game.hpp"
#include "Player.hpp"
#include "Projectile.hpp"
#include "Dungeon.hpp"
#include "Pathfinding.hpp"
#include "Utils.hpp"
#include "raymath.h"
#include <algorithm>

// Enemy implementation
Enemy::Enemy(const EnemyData& data, Vector2 pos) 
    : Entity(pos, 20.0f), m_data(data), m_health(data.maxHealth) {
    // Configure the Seeker component (similar to Unity's AIPath settings)
    m_seeker.repathRate = PATH_UPDATE_INTERVAL;  // How often to recalculate paths
    m_seeker.pickNextWaypointDist = 20.0f;       // Distance to pick next waypoint
    m_seeker.constrainInsideGraph = true;        // Keep on walkable tiles
    
    // Configure the AIPathHelper
    m_pathHelper.speed = m_data.moveSpeed;
    m_pathHelper.slowdownDistance = 30.0f;
    m_pathHelper.endReachedDistance = 10.0f;
}

void Enemy::Update(float dt) {
    if (IsDead()) return;
    
    // Update status effect timers
    if (m_immobilizeTimer > 0) {
        m_immobilizeTimer -= dt;
    }
    
    // Skip AI update if immobilized
    if (!IsImmobilized()) {
        UpdateAI(dt);
    }
    
    m_attackTimer -= dt;
    m_stateTimer -= dt;
    m_repositionTimer -= dt;
    m_searchTimer -= dt;
    m_pathUpdateTimer -= dt;
}

void Enemy::Render() {
    if (IsDead()) return;
    
    // Draw enemy body - tint blue if immobilized
    Color bodyColor = IsImmobilized() ? ColorTint(m_data.color, SKYBLUE) : m_data.color;
    DrawCircleV(m_position, m_radius, bodyColor);
    
    // Draw stun indicator if immobilized
    if (IsImmobilized()) {
        DrawCircleLinesV(m_position, m_radius + 3, SKYBLUE);
    }
    
    // Draw health bar above enemy
    float healthPercent = static_cast<float>(m_health) / m_data.maxHealth;
    float barWidth = m_radius * 2;
    float barHeight = 4;
    Vector2 barPos = { m_position.x - barWidth/2, m_position.y - m_radius - 10 };
    
    DrawRectangle(static_cast<int>(barPos.x), static_cast<int>(barPos.y), 
                  static_cast<int>(barWidth), static_cast<int>(barHeight), DARKGRAY);
    DrawRectangle(static_cast<int>(barPos.x), static_cast<int>(barPos.y), 
                  static_cast<int>(barWidth * healthPercent), static_cast<int>(barHeight), RED);
}

void Enemy::TakeDamage(int amount) {
    m_health -= amount;
    if (m_health < 0) m_health = 0;
}

void Enemy::Immobilize(float duration) {
    m_immobilizeTimer = duration;
}

bool Enemy::HasLineOfSight() const {
    Player* player = Game::Instance().GetPlayer();
    DungeonManager* dungeon = Game::Instance().GetDungeon();
    if (!player || !dungeon) return false;
    
    Vector2 playerPos = player->GetPosition();
    Vector2 toPlayer = Vector2Subtract(playerPos, m_position);
    float dist = Vector2Length(toPlayer);
    
    // Check several points along the line
    int steps = static_cast<int>(dist / 20.0f);
    if (steps < 1) steps = 1;
    
    Vector2 step = Vector2Scale(toPlayer, 1.0f / steps);
    Vector2 checkPos = m_position;
    
    for (int i = 0; i < steps; ++i) {
        checkPos = Vector2Add(checkPos, step);
        if (!dungeon->IsWalkable(checkPos)) {
            return false;
        }
    }
    
    return true;
}

Vector2 Enemy::FindRepositionTarget() const {
    DungeonManager* dungeon = Game::Instance().GetDungeon();
    Player* player = Game::Instance().GetPlayer();
    if (!dungeon || !player) return m_position;
    
    Vector2 playerPos = player->GetPosition();
    float preferredDist = GetPreferredDistance();
    
    // Try to find a position at preferred distance that has line of sight
    for (int attempt = 0; attempt < 8; ++attempt) {
        float angle = Utils::RandomFloat(0, 2 * PI);
        Vector2 offset = {cosf(angle) * preferredDist, sinf(angle) * preferredDist};
        Vector2 testPos = Vector2Add(playerPos, offset);
        
        if (dungeon->IsWalkable(testPos)) {
            // Verify we can reach it roughly
            if (dungeon->IsWalkable(Vector2Lerp(m_position, testPos, 0.5f))) {
                return testPos;
            }
        }
    }
    
    // Fallback: just move perpendicular to player
    Vector2 toPlayer = Vector2Subtract(playerPos, m_position);
    Vector2 perpendicular = {-toPlayer.y, toPlayer.x};
    perpendicular = Vector2Normalize(perpendicular);
    
    Vector2 testPos = Vector2Add(m_position, Vector2Scale(perpendicular, 50.0f));
    if (dungeon->IsWalkable(testPos)) return testPos;
    
    testPos = Vector2Add(m_position, Vector2Scale(perpendicular, -50.0f));
    if (dungeon->IsWalkable(testPos)) return testPos;
    
    return m_position;
}

float Enemy::GetPreferredDistance() const {
    switch (m_data.type) {
        case EnemyType::SKELETON:
            return 180.0f;  // Ranged - stay back
        default:
            return 30.0f;   // Melee - get close
    }
}

void Enemy::UpdatePath(Vector2 targetPos) {
    DungeonManager* dungeon = Game::Instance().GetDungeon();
    if (!dungeon) return;
    
    Room* currentRoom = dungeon->GetCurrentRoom();
    if (!currentRoom) return;
    
    // Use the new Seeker-based pathfinding
    m_seeker.StartPath(m_position, targetPos, currentRoom);
    
    // Also update legacy path for compatibility
    m_currentPath = m_seeker.GetCurrentPath().vectorPath;
    m_pathUpdateTimer = PATH_UPDATE_INTERVAL;
}

void Enemy::MoveAlongPath(float dt, float speedMultiplier) {
    DungeonManager* dungeon = Game::Instance().GetDungeon();
    if (!dungeon) return;
    
    Room* currentRoom = dungeon->GetCurrentRoom();
    if (!currentRoom) return;
    
    // Use the new Seeker-based movement if we have a seeker path
    if (m_seeker.HasPath()) {
        // Update path helper speed based on our move speed
        m_pathHelper.speed = m_data.moveSpeed;
        
        // Get target from current path destination
        Vector2 target = m_seeker.GetCurrentPath().vectorPath.empty() ? 
            m_position : m_seeker.GetCurrentPath().vectorPath.back();
        
        // Use AIPathHelper for smooth movement
        Vector2 newPos = m_pathHelper.MoveToward(m_seeker, m_position, target, 
                                                   currentRoom, dt, speedMultiplier);
        
        // Verify the new position is walkable (safety check)
        if (dungeon->IsWalkable(newPos)) {
            m_position = newPos;
        } else {
            // Path may be outdated, force recalculation
            m_seeker.ClearPath();
        }
        return;
    }
    
    // Fallback to legacy path following if no seeker path
    if (m_currentPath.empty()) return;
    
    // Remove waypoints we've already passed (using pickNextWaypointDist)
    while (!m_currentPath.empty()) {
        float distToFirst = Vector2Distance(m_position, m_currentPath[0]);
        if (distToFirst < m_seeker.pickNextWaypointDist) {
            m_currentPath.erase(m_currentPath.begin());
        } else {
            break;
        }
    }
    
    if (m_currentPath.empty()) return;
    
    // Move toward first waypoint in path
    Vector2 nextWaypoint = m_currentPath[0];
    Vector2 toWaypoint = Vector2Subtract(nextWaypoint, m_position);
    float distToWaypoint = Vector2Length(toWaypoint);
    
    if (distToWaypoint < 1.0f) return;  // Already there
    
    Vector2 moveDir = Vector2Normalize(toWaypoint);
    Vector2 newPos = Vector2Add(m_position, 
        Vector2Scale(moveDir, m_data.moveSpeed * speedMultiplier * dt));
    
    // Verify the new position is walkable (safety check)
    if (dungeon->IsWalkable(newPos)) {
        m_position = newPos;
    } else {
        // Path may be outdated, force recalculation
        m_pathUpdateTimer = 0;
        m_currentPath.clear();
    }
}

void Enemy::MoveWithSeeker(Vector2 targetPos, float dt, float speedMultiplier) {
    // High-level movement using the Seeker component
    // This mirrors the Unity AIPath behavior
    
    DungeonManager* dungeon = Game::Instance().GetDungeon();
    if (!dungeon) return;
    
    Room* currentRoom = dungeon->GetCurrentRoom();
    if (!currentRoom) return;
    
    // Update seeker timer
    m_seeker.Update(dt);
    
    // Repath if needed (based on repathRate)
    if (m_seeker.ShouldRepath() || !m_seeker.HasPath()) {
        m_seeker.StartPath(m_position, targetPos, currentRoom);
        m_seeker.ResetRepathTimer();
    }
    
    if (!m_seeker.HasPath()) return;
    
    // Use AIPathHelper for movement
    m_pathHelper.speed = m_data.moveSpeed;
    Vector2 newPos = m_pathHelper.MoveToward(m_seeker, m_position, targetPos,
                                              currentRoom, dt, speedMultiplier);
    
    // Verify walkability (constrainInsideGraph)
    if (m_seeker.constrainInsideGraph && !dungeon->IsWalkable(newPos)) {
        m_seeker.ClearPath();  // Force repath
        return;
    }
    
    m_position = newPos;
}

void Enemy::UpdateAI(float dt) {
    Player* player = Game::Instance().GetPlayer();
    if (!player) return;
    
    Vector2 playerPos = player->GetPosition();
    Vector2 toPlayer = Vector2Subtract(playerPos, m_position);
    float distToPlayer = Vector2Length(toPlayer);
    bool hasLOS = HasLineOfSight();
    
    // Track last known player position when we can see them
    if (hasLOS && distToPlayer < m_data.detectionRange) {
        m_lastKnownPlayerPos = playerPos;
    }
    
    // Ranged enemy behavior (Skeleton)
    bool isRanged = (m_data.type == EnemyType::SKELETON);
    float preferredDist = GetPreferredDistance();
    
    switch (m_aiState) {
        case AIState::IDLE:
            if (distToPlayer < m_data.detectionRange && hasLOS) {
                m_aiState = AIState::CHASE;
            }
            break;
            
        case AIState::CHASE:
            if (distToPlayer > m_data.detectionRange * 1.5f) {
                m_aiState = AIState::IDLE;
                m_currentPath.clear();
            } else if (!hasLOS) {
                // Lost sight of player - go to search mode
                m_aiState = AIState::SEARCH;
                m_searchTimer = 3.0f;  // Search for 3 seconds
            } else if (distToPlayer < GetAttackRange()) {
                // Close enough to attack
                if (isRanged && distToPlayer < preferredDist * 0.6f) {
                    // Too close for ranged - need to reposition
                    m_aiState = AIState::REPOSITION;
                    m_repositionTarget = FindRepositionTarget();
                    m_repositionTimer = 2.0f;
                    m_pathUpdateTimer = 0;  // Force path update
                } else {
                    m_aiState = AIState::ATTACK;
                }
            } else {
                // Use pathfinding to move toward player
                if (m_pathUpdateTimer <= 0) {
                    UpdatePath(playerPos);
                }
                MoveAlongPath(dt);
            }
            break;
            
        case AIState::ATTACK:
            if (!hasLOS) {
                m_aiState = AIState::SEARCH;
                m_searchTimer = 3.0f;
            } else if (distToPlayer > GetAttackRange() * 1.2f) {
                m_aiState = AIState::CHASE;
            } else if (isRanged && distToPlayer < preferredDist * 0.5f) {
                // Too close - reposition
                m_aiState = AIState::REPOSITION;
                m_repositionTarget = FindRepositionTarget();
                m_repositionTimer = 2.0f;
            } else if (m_attackTimer <= 0) {
                Attack();
                m_attackTimer = m_data.attackCooldown;
                
                // Ranged enemies reposition after attacking sometimes
                if (isRanged && Utils::RandomFloat(0, 1) < 0.4f) {
                    m_aiState = AIState::REPOSITION;
                    m_repositionTarget = FindRepositionTarget();
                    m_repositionTimer = 1.5f;
                }
            }
            break;
            
        case AIState::REPOSITION: {
            // Move toward reposition target using pathfinding
            Vector2 toTarget = Vector2Subtract(m_repositionTarget, m_position);
            float distToTarget = Vector2Length(toTarget);
            
            if (distToTarget < 10.0f || m_repositionTimer <= 0) {
                // Reached target or timeout - go back to chase/attack
                m_aiState = (distToPlayer < GetAttackRange() && hasLOS) ? AIState::ATTACK : AIState::CHASE;
                m_currentPath.clear();
            } else {
                // Use pathfinding to reach reposition target
                if (m_pathUpdateTimer <= 0) {
                    UpdatePath(m_repositionTarget);
                }
                MoveAlongPath(dt, 1.2f);  // Move faster when repositioning
                
                // If path is empty but not at target, might be unreachable
                if (m_currentPath.empty() && distToTarget > 20.0f) {
                    m_aiState = AIState::CHASE;
                }
            }
            
            // Can still attack while repositioning if in range
            if (hasLOS && distToPlayer < GetAttackRange() && m_attackTimer <= 0) {
                Attack();
                m_attackTimer = m_data.attackCooldown;
            }
            break;
        }
            
        case AIState::SEARCH: {
            // Move toward last known player position
            Vector2 toLastKnown = Vector2Subtract(m_lastKnownPlayerPos, m_position);
            float distToLastKnown = Vector2Length(toLastKnown);
            
            if (hasLOS && distToPlayer < m_data.detectionRange) {
                // Found player again
                m_aiState = AIState::CHASE;
                m_currentPath.clear();
            } else if (m_searchTimer <= 0 || distToLastKnown < 20.0f) {
                // Gave up searching or reached last known position
                m_aiState = AIState::IDLE;
                m_currentPath.clear();
            } else {
                // Use pathfinding to move toward last known position
                if (m_pathUpdateTimer <= 0) {
                    UpdatePath(m_lastKnownPlayerPos);
                }
                MoveAlongPath(dt, 0.7f);  // Slower when searching
            }
            break;
        }
            
        case AIState::SPECIAL:
            // Boss-specific behavior
            break;
    }
}

void Enemy::Attack() {
    Player* player = Game::Instance().GetPlayer();
    if (!player) return;
    
    // Different attack behavior based on type
    switch (m_data.type) {
        case EnemyType::SLIME:
        case EnemyType::GOBLIN:
            // Melee attack - direct damage if in range
            if (CollidesWith(*player)) {
                player->TakeDamage(m_data.damage);
            }
            break;
            
        case EnemyType::SKELETON: {
            // Ranged attack - shoot projectile
            Vector2 dir = Vector2Normalize(
                Vector2Subtract(player->GetPosition(), m_position));
            Game::Instance().GetProjectiles()->SpawnProjectile(
                m_position, dir, 200.0f, m_data.damage, false, false, PURPLE);
            break;
        }
            
        case EnemyType::BAT:
            // Quick dash attack
            if (CollidesWith(*player)) {
                player->TakeDamage(m_data.damage);
            }
            break;
            
        case EnemyType::MINI_BOSS_GOLEM:
            // AoE stomp (damage in radius)
            if (Vector2Distance(m_position, player->GetPosition()) < 80.0f) {
                player->TakeDamage(m_data.damage);
            }
            break;
    }
}

float Enemy::GetAttackRange() const {
    switch (m_data.type) {
        case EnemyType::SKELETON:
            return 250.0f;
        case EnemyType::MINI_BOSS_GOLEM:
            return 80.0f;
        default:
            return 30.0f;
    }
}

// Factory methods
EnemyData Enemy::CreateSlimeData() {
    return {
        EnemyType::SLIME,
        "Slime",
        30,             // maxHealth
        50.0f,          // moveSpeed
        5,              // damage
        1.5f,           // attackCooldown
        200.0f,         // detectionRange
        5,              // currencyDrop
        GREEN           // color
    };
}

EnemyData Enemy::CreateSkeletonData() {
    return {
        EnemyType::SKELETON,
        "Skeleton",
        40,
        30.0f,
        10,
        2.0f,
        300.0f,
        10,
        BEIGE
    };
}

EnemyData Enemy::CreateBatData() {
    return {
        EnemyType::BAT,
        "Bat",
        20,
        120.0f,
        8,
        0.8f,
        250.0f,
        7,
        DARKPURPLE
    };
}

EnemyData Enemy::CreateGoblinData() {
    return {
        EnemyType::GOBLIN,
        "Goblin",
        50,
        80.0f,
        12,
        1.2f,
        220.0f,
        12,
        DARKGREEN
    };
}

EnemyData Enemy::CreateGolemData() {
    return {
        EnemyType::MINI_BOSS_GOLEM,
        "Stone Golem",
        200,
        25.0f,
        25,
        3.0f,
        400.0f,
        50,
        GRAY
    };
}

// EnemyManager implementation
void EnemyManager::Update(float dt) {
    for (auto& enemy : m_enemies) {
        if (enemy) {
            enemy->Update(dt);
        }
    }
    
    // Remove dead enemies
    m_enemies.erase(
        std::remove_if(m_enemies.begin(), m_enemies.end(),
            [](const std::unique_ptr<Enemy>& e) { 
                return !e || e->IsDead(); 
            }),
        m_enemies.end()
    );
}

void EnemyManager::Render() {
    for (auto& enemy : m_enemies) {
        if (enemy) {
            enemy->Render();
        }
    }
}

void EnemyManager::Clear() {
    m_enemies.clear();
}

void EnemyManager::SpawnEnemy(EnemyType type, Vector2 pos) {
    EnemyData data;
    
    switch (type) {
        case EnemyType::SLIME:
            data = Enemy::CreateSlimeData();
            break;
        case EnemyType::SKELETON:
            data = Enemy::CreateSkeletonData();
            break;
        case EnemyType::BAT:
            data = Enemy::CreateBatData();
            break;
        case EnemyType::GOBLIN:
            data = Enemy::CreateGoblinData();
            break;
        case EnemyType::MINI_BOSS_GOLEM:
            data = Enemy::CreateGolemData();
            break;
    }
    
    m_enemies.push_back(std::make_unique<Enemy>(data, pos));
}

void EnemyManager::SpawnEnemiesInRoom(const std::vector<Vector2>& spawnPoints, int difficulty) {
    // Number of enemies scales with difficulty
    int numEnemies = std::min(static_cast<int>(spawnPoints.size()), 
                              2 + difficulty);
    
    std::vector<EnemyType> availableTypes = {
        EnemyType::SLIME,
        EnemyType::SKELETON,
        EnemyType::BAT,
        EnemyType::GOBLIN
    };
    
    for (int i = 0; i < numEnemies && i < static_cast<int>(spawnPoints.size()); ++i) {
        // Random enemy type with difficulty weighting
        int typeIndex = Utils::RandomInt(0, std::min(difficulty, 
            static_cast<int>(availableTypes.size()) - 1));
        SpawnEnemy(availableTypes[typeIndex], spawnPoints[i]);
    }
    
    // Chance to spawn miniboss on higher floors
    if (difficulty >= 3 && Utils::RandomFloat(0, 1) < 0.2f) {
        if (!spawnPoints.empty()) {
            SpawnEnemy(EnemyType::MINI_BOSS_GOLEM, 
                       spawnPoints[spawnPoints.size() / 2]);
        }
    }
}

int EnemyManager::GetActiveCount() const {
    int count = 0;
    for (const auto& enemy : m_enemies) {
        if (enemy && !enemy->IsDead()) {
            ++count;
        }
    }
    return count;
}

Enemy* EnemyManager::GetNearestEnemy(Vector2 pos, float maxRange) {
    Enemy* nearest = nullptr;
    float nearestDist = maxRange;
    
    for (auto& enemy : m_enemies) {
        if (!enemy || enemy->IsDead()) continue;
        
        float dist = Vector2Distance(pos, enemy->GetPosition());
        if (dist < nearestDist) {
            nearestDist = dist;
            nearest = enemy.get();
        }
    }
    
    return nearest;
}
