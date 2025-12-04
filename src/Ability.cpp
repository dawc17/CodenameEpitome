#include "Ability.hpp"
#include "Player.hpp"
#include "Game.hpp"
#include "Dungeon.hpp"
#include "Enemy.hpp"
#include "Utils.hpp"

Ability::Ability(const std::string& name, float cooldown, int energyCost,
                 std::function<void(Player*)> effect)
    : m_name(name)
    , m_cooldown(cooldown)
    , m_currentCooldown(0.0f)
    , m_energyCost(energyCost)
    , m_effect(effect)
{
}

bool Ability::TryActivate(Player* player) {
    if (!player) return false;
    if (!IsReady()) return false;
    if (player->GetEnergy() < m_energyCost) return false;
    
    player->UseEnergy(m_energyCost);
    m_currentCooldown = m_cooldown;
    
    if (m_effect) {
        m_effect(player);
    }
    
    return true;
}

void Ability::Update(float dt) {
    if (m_currentCooldown > 0) {
        m_currentCooldown -= dt;
    }
}

float Ability::GetCooldownPercent() const {
    if (m_cooldown <= 0) return 0.0f;
    return m_currentCooldown / m_cooldown;
}

// Predefined abilities
namespace Abilities {
    std::unique_ptr<Ability> CreateShieldDash() {
        return std::make_unique<Ability>(
            "Shield Dash",
            3.0f,   // cooldown
            20,     // energy cost
            [](Player* player) {
                if (!player) return;
                
                // Dash in aim direction
                Vector2 dashDir = player->GetAimDirection();
                Vector2 dashOffset = Vector2Scale(dashDir, 150.0f);
                Vector2 newPos = Vector2Add(player->GetPosition(), dashOffset);
                
                // Check if we can dash there (simplified - just check endpoint)
                DungeonManager* dungeon = Game::Instance().GetDungeon();
                if (dungeon && dungeon->IsWalkable(newPos)) {
                    player->SetPosition(newPos);
                } else {
                    // Try partial dash
                    for (float t = 0.9f; t > 0.1f; t -= 0.1f) {
                        Vector2 testPos = Vector2Add(player->GetPosition(), 
                            Vector2Scale(dashOffset, t));
                        if (dungeon->IsWalkable(testPos)) {
                            player->SetPosition(testPos);
                            break;
                        }
                    }
                }
                
                // TODO: Add invincibility frames during dash
            }
        );
    }
    
    std::unique_ptr<Ability> CreateArcaneBurst() {
        return std::make_unique<Ability>(
            "Arcane Burst",
            5.0f,   // cooldown
            35,     // energy cost
            [](Player* player) {
                if (!player) return;
                
                // AoE damage around player
                EnemyManager* enemies = Game::Instance().GetEnemies();
                if (!enemies) return;
                
                float burstRadius = 120.0f;
                int burstDamage = 30;
                
                for (auto& enemy : enemies->GetEnemies()) {
                    if (!enemy || enemy->IsDead()) continue;
                    
                    float dist = Vector2Distance(player->GetPosition(), 
                                                  enemy->GetPosition());
                    if (dist <= burstRadius) {
                        enemy->TakeDamage(burstDamage);
                    }
                }
                
                // Visual effect would go here
            }
        );
    }
    
    std::unique_ptr<Ability> CreateExplosion() {
        return std::make_unique<Ability>(
            "Explosion",
            4.0f,   // cooldown
            30,     // energy cost
            [](Player* player) {
                if (!player) return;
                
                // Heavy AoE damage around player
                EnemyManager* enemies = Game::Instance().GetEnemies();
                if (!enemies) return;
                
                float explosionRadius = 100.0f;
                int explosionDamage = 50;
                
                for (auto& enemy : enemies->GetEnemies()) {
                    if (!enemy || enemy->IsDead()) continue;
                    
                    float dist = Vector2Distance(player->GetPosition(), 
                                                  enemy->GetPosition());
                    if (dist <= explosionRadius) {
                        enemy->TakeDamage(explosionDamage);
                    }
                }
                
                // Visual effect would go here (explosion effect)
            }
        );
    }
    
    std::unique_ptr<Ability> CreateFlashbang() {
        return std::make_unique<Ability>(
            "Flashbang",
            5.0f,   // cooldown
            25,     // energy cost
            [](Player* player) {
                if (!player) return;
                
                // Immobilize enemies within radius
                EnemyManager* enemies = Game::Instance().GetEnemies();
                if (!enemies) return;
                
                float flashRadius = 150.0f;
                float stunDuration = 3.0f;
                
                for (auto& enemy : enemies->GetEnemies()) {
                    if (!enemy || enemy->IsDead()) continue;
                    
                    float dist = Vector2Distance(player->GetPosition(), 
                                                  enemy->GetPosition());
                    if (dist <= flashRadius) {
                        enemy->Immobilize(stunDuration);
                    }
                }
                
                // Visual effect would go here (flash effect)
            }
        );
    }
}
