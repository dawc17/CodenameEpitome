#include "Player.hpp"
#include "Game.hpp"
#include "Projectile.hpp"
#include "Enemy.hpp"
#include "Dungeon.hpp"
#include "Utils.hpp"

Player::Player() : Entity({0, 0}, 16.0f) {
    m_health = m_stats.maxHealth;
    m_energy = m_stats.maxEnergy;
    
    // Start with a pistol
    m_weapon = std::make_unique<Weapon>(Weapon::CreatePistolData());
    
    // Start with Shield Dash ability
    m_ability = Abilities::CreateShieldDash();
}

void Player::Update(float dt) {
    HandleMovement(dt);
    UpdateAutoAim();
    RegenerateEnergy(dt);
    
    if (m_weapon) {
        m_weapon->Update(dt);
    }
    if (m_ability) {
        m_ability->Update(dt);
    }
}

void Player::Render() {
    // Draw player body
    DrawCircleV(m_position, m_radius, m_color);
    
    // Draw aim direction indicator
    Vector2 aimEnd = Vector2Add(m_position, Vector2Scale(m_aimDirection, m_radius + 10));
    DrawLineEx(m_position, aimEnd, 3.0f, WHITE);
    
    // Draw target indicator if we have a target
    if (m_currentTarget && m_currentTarget->IsActive()) {
        DrawCircleLinesV(m_currentTarget->GetPosition(), 
                         m_currentTarget->GetRadius() + 5, RED);
    }
}

void Player::HandleMovement(float dt) {
    Vector2 moveDir = {0, 0};
    
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) moveDir.y -= 1;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) moveDir.y += 1;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) moveDir.x -= 1;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) moveDir.x += 1;
    
    // Normalize diagonal movement
    if (Vector2Length(moveDir) > 0) {
        moveDir = Vector2Normalize(moveDir);
    }
    
    // Apply movement
    Vector2 newPos = Vector2Add(m_position, Vector2Scale(moveDir, m_stats.moveSpeed * dt));
    
    // Check wall collision
    DungeonManager* dungeon = Game::Instance().GetDungeon();
    if (dungeon && dungeon->IsWalkable(newPos)) {
        m_position = newPos;
    } else {
        // Try sliding along walls
        Vector2 testX = {newPos.x, m_position.y};
        Vector2 testY = {m_position.x, newPos.y};
        
        if (dungeon->IsWalkable(testX)) {
            m_position.x = newPos.x;
        }
        if (dungeon->IsWalkable(testY)) {
            m_position.y = newPos.y;
        }
    }
    
    m_velocity = moveDir;
}

void Player::UpdateAutoAim() {
    EnemyManager* enemies = Game::Instance().GetEnemies();
    if (!enemies) return;
    
    Enemy* nearest = enemies->GetNearestEnemy(m_position, AIM_RANGE);
    
    if (nearest) {
        m_currentTarget = nearest;
        Vector2 toTarget = Vector2Subtract(nearest->GetPosition(), m_position);
        Vector2 targetDir = Vector2Normalize(toTarget);
        
        // Smooth aim transition
        float dt = Game::Instance().GetDeltaTime();
        m_aimDirection.x = Utils::Lerp(m_aimDirection.x, targetDir.x, AIM_SMOOTHING * dt);
        m_aimDirection.y = Utils::Lerp(m_aimDirection.y, targetDir.y, AIM_SMOOTHING * dt);
        m_aimDirection = Vector2Normalize(m_aimDirection);
    } else {
        m_currentTarget = nullptr;
        
        // Default to movement direction or last aim direction
        if (Vector2Length(m_velocity) > 0.1f) {
            Vector2 targetDir = Vector2Normalize(m_velocity);
            float dt = Game::Instance().GetDeltaTime();
            m_aimDirection.x = Utils::Lerp(m_aimDirection.x, targetDir.x, AIM_SMOOTHING * dt);
            m_aimDirection.y = Utils::Lerp(m_aimDirection.y, targetDir.y, AIM_SMOOTHING * dt);
            m_aimDirection = Vector2Normalize(m_aimDirection);
        }
    }
}

void Player::RegenerateEnergy(float dt) {
    // Only regen after delay has passed
    if (m_energyRegenDelay > 0) {
        m_energyRegenDelay -= dt;
        return;
    }
    
    if (m_energy < m_stats.maxEnergy) {
        // Accumulate fractional energy to avoid truncation issues
        m_energyRegenAccumulator += ENERGY_REGEN_RATE * dt;
        
        // Convert accumulated energy to integer and add to energy
        if (m_energyRegenAccumulator >= 1.0f) {
            int energyToAdd = static_cast<int>(m_energyRegenAccumulator);
            m_energy += energyToAdd;
            m_energyRegenAccumulator -= static_cast<float>(energyToAdd);
            
            if (m_energy > m_stats.maxEnergy) {
                m_energy = m_stats.maxEnergy;
                m_energyRegenAccumulator = 0.0f;
            }
        }
    }
}

void Player::TakeDamage(int amount) {
    m_health -= amount;
    if (m_health < 0) m_health = 0;
    
    // Visual feedback - flash red
    m_color = RED;
    // Reset color after a short time would need a timer
}

void Player::Heal(int amount) {
    m_health += amount;
    if (m_health > m_stats.maxHealth) {
        m_health = m_stats.maxHealth;
    }
}

void Player::UseEnergy(int amount) {
    m_energy -= amount;
    if (m_energy < 0) m_energy = 0;
    m_energyRegenDelay = ENERGY_REGEN_DELAY;  // Reset regen delay when energy is used
    m_energyRegenAccumulator = 0.0f;  // Reset accumulator
}

void Player::Shoot() {
    if (!m_weapon) return;
    
    int energyCost = m_weapon->GetData().energyCost;
    if (m_energy < energyCost) return;
    
    if (m_weapon->TryFire(m_position, m_aimDirection)) {
        UseEnergy(energyCost);
    }
}

void Player::UseAbility() {
    if (!m_ability) return;
    m_ability->TryActivate(this);
}

float Player::GetAbilityCooldownPercent() const {
    if (!m_ability) return 0.0f;
    return m_ability->GetCooldownPercent();
}

void Player::EquipWeapon(std::unique_ptr<Weapon> weapon) {
    m_weapon = std::move(weapon);
}

void Player::SetAbility(std::unique_ptr<Ability> ability) {
    m_ability = std::move(ability);
}

bool Player::SpendRunCurrency(int amount) {
    if (m_runCurrency >= amount) {
        m_runCurrency -= amount;
        return true;
    }
    return false;
}

void Player::Reset() {
    m_health = m_stats.maxHealth;
    m_energy = m_stats.maxEnergy;
    m_runCurrency = 0;
    m_position = {0, 0};
    m_velocity = {0, 0};
    m_aimDirection = {1, 0};
    m_currentTarget = nullptr;
    m_color = BLUE;
    m_energyRegenDelay = 0.0f;
    m_energyRegenAccumulator = 0.0f;
    
    // Reset stats to default
    m_stats = CharacterStats();
    
    // Reset weapon to pistol
    m_weapon = std::make_unique<Weapon>(Weapon::CreatePistolData());
    m_ability = Abilities::CreateShieldDash();
}

void Player::ApplyBuff(const BuffData& buff) {
    if (buff.applyFunc) {
        buff.applyFunc(this);
    }
}

std::vector<BuffData> Player::GetStartingBuffs() {
    return {
        {"Health Boost", "+20 Max Health", [](Player* p) { 
            p->GetStats().maxHealth += 20; 
            p->Heal(20);  // Also heal for the bonus
        }},
        {"Speed Demon", "+15% Movement Speed", [](Player* p) { 
            p->GetStats().moveSpeed *= 1.15f; 
        }},
        {"Power Strike", "+20% Weapon Damage", [](Player* p) { 
            p->GetStats().damageMultiplier *= 1.20f; 
        }},
        {"Quick Trigger", "+15% Fire Rate", [](Player* p) { 
            p->GetStats().fireRateMultiplier *= 1.15f; 
        }},
        {"Energy Surge", "+25 Max Energy", [](Player* p) { 
            p->GetStats().maxEnergy += 25; 
        }},
        {"Swift Recovery", "-20% Ability Cooldown", [](Player* p) { 
            p->GetStats().cooldownMultiplier *= 0.80f; 
        }},
    };
}

std::vector<BuffData> Player::GetRandomBuffs(int count) {
    std::vector<BuffData> allBuffs = GetStartingBuffs();
    std::vector<BuffData> result;
    
    // Shuffle and pick 'count' buffs
    for (int i = 0; i < count && !allBuffs.empty(); ++i) {
        int idx = Utils::RandomInt(0, static_cast<int>(allBuffs.size()) - 1);
        result.push_back(allBuffs[idx]);
        allBuffs.erase(allBuffs.begin() + idx);
    }
    
    return result;
}
