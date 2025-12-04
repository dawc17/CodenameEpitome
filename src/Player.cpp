#include "Player.hpp"
#include "Game.hpp"
#include "Projectile.hpp"
#include "Enemy.hpp"
#include "Dungeon.hpp"
#include "Utils.hpp"

// Initialize static meta currency
int Player::s_metaCurrency = 0;

Player::Player() : Entity({0, 0}, 16.0f) {
    // Default to Terrorist character
    SetCharacter(CharacterType::TERRORIST);
}

CharacterData Player::GetCharacterData(CharacterType type) {
    CharacterData data;
    data.type = type;
    
    switch (type) {
        case CharacterType::TERRORIST:
            data.name = "Terrorist";
            data.description = "Wields a pistol. Explosion ability deals AoE damage.";
            data.passiveName = "Explosive Rounds";
            data.passiveDescription = "20% chance for kills to explode, dealing 15 AoE damage.";
            data.lore = "Once a demolitions expert, now a mercenary who found purpose in the dungeon's chaos. Every explosion reminds him of home.";
            data.passive = PassiveType::EXPLOSIVE_ROUNDS;
            data.stats.name = "Terrorist";
            data.stats.maxHealth = 100;
            data.stats.maxEnergy = 100;
            data.stats.moveSpeed = 200.0f;
            data.color = Color{180, 80, 80, 255};  // Reddish
            break;
            
        case CharacterType::COUNTER_TERRORIST:
            data.name = "Counter-Terrorist";
            data.description = "Wields a burst rifle. Flashbang immobilizes enemies.";
            data.passiveName = "Tactical Reload";
            data.passiveDescription = "Weapon shots restore 2 energy on hit.";
            data.lore = "A former elite operative who lost her squad to the dungeon's horrors. She fights with precision and discipline, never wasting a bullet.";
            data.passive = PassiveType::TACTICAL_RELOAD;
            data.stats.name = "Counter-Terrorist";
            data.stats.maxHealth = 110;
            data.stats.maxEnergy = 90;
            data.stats.moveSpeed = 190.0f;
            data.color = Color{80, 80, 180, 255};  // Bluish
            break;
    }
    
    return data;
}

void Player::SetCharacter(CharacterType type) {
    m_characterType = type;
    CharacterData charData = GetCharacterData(type);
    
    m_stats = charData.stats;
    m_color = charData.color;
    m_passive = charData.passive;
    m_health = m_stats.maxHealth;
    m_energy = m_stats.maxEnergy;
    
    // Set weapon and ability based on character
    switch (type) {
        case CharacterType::TERRORIST:
            m_weapon = std::make_unique<Weapon>(Weapon::CreatePistolData());
            m_ability = Abilities::CreateExplosion();
            break;
            
        case CharacterType::COUNTER_TERRORIST:
            m_weapon = std::make_unique<Weapon>(Weapon::CreateBurstRifleData());
            m_ability = Abilities::CreateFlashbang();
            break;
    }
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

bool Player::SpendMetaCurrency(int amount) {
    if (s_metaCurrency >= amount) {
        s_metaCurrency -= amount;
        return true;
    }
    return false;
}

void Player::TriggerPassiveOnKill() {
    switch (m_passive) {
        case PassiveType::EXPLOSIVE_ROUNDS:
            // 20% chance to explode on kill - handled at kill location
            if (Utils::RandomFloat(0.0f, 1.0f) < 0.2f) {
                // The explosion is created at the enemy position by the caller
                // Just return true to indicate explosion should happen
            }
            break;
            
        case PassiveType::TACTICAL_RELOAD:
            // Energy restored on hit, not on kill
            break;
            
        case PassiveType::NONE:
        default:
            break;
    }
}

void Player::Reset() {
    // Get character data for current character type
    CharacterData charData = GetCharacterData(m_characterType);
    
    m_stats = charData.stats;
    m_color = charData.color;
    m_passive = charData.passive;
    m_health = m_stats.maxHealth;
    m_energy = m_stats.maxEnergy;
    m_runCurrency = 0;
    m_position = {0, 0};
    m_velocity = {0, 0};
    m_aimDirection = {1, 0};
    m_currentTarget = nullptr;
    m_energyRegenDelay = 0.0f;
    m_energyRegenAccumulator = 0.0f;
    
    // Reset weapon and ability based on character
    switch (m_characterType) {
        case CharacterType::TERRORIST:
            m_weapon = std::make_unique<Weapon>(Weapon::CreatePistolData());
            m_ability = Abilities::CreateExplosion();
            break;
            
        case CharacterType::COUNTER_TERRORIST:
            m_weapon = std::make_unique<Weapon>(Weapon::CreateBurstRifleData());
            m_ability = Abilities::CreateFlashbang();
            break;
    }
}

void Player::ApplyBuff(const BuffData& buff) {
    if (buff.applyFunc) {
        buff.applyFunc(this);
    }
}

std::vector<BuffData> Player::GetStartingBuffs() {
    return {
        // Basic stat buffs
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

std::vector<BuffData> Player::GetFloorBuffs() {
    return {
        // Basic buffs (common)
        {"Minor Heal", "Restore 30 HP", [](Player* p) { 
            p->Heal(30);
        }},
        {"Health Boost", "+15 Max Health", [](Player* p) { 
            p->GetStats().maxHealth += 15; 
            p->Heal(15);
        }},
        {"Energy Boost", "+15 Max Energy", [](Player* p) { 
            p->GetStats().maxEnergy += 15; 
        }},
        {"Quick Feet", "+10% Movement Speed", [](Player* p) { 
            p->GetStats().moveSpeed *= 1.10f; 
        }},
        {"Sharpshooter", "+10% Weapon Damage", [](Player* p) { 
            p->GetStats().damageMultiplier *= 1.10f; 
        }},
        {"Rapid Fire", "+10% Fire Rate", [](Player* p) { 
            p->GetStats().fireRateMultiplier *= 1.10f; 
        }},
        {"Cooldown Reduction", "-10% Ability Cooldown", [](Player* p) { 
            p->GetStats().cooldownMultiplier *= 0.90f; 
        }},
        
        // Special effect buffs (rare)
        {"Vampiric Touch", "Kills restore 5 HP", [](Player* p) { 
            // This is handled by a flag - for now just heal a bit
            p->Heal(10);  // Placeholder until proper implementation
        }},
        {"Energy Thief", "Kills restore 10 Energy", [](Player* p) { 
            // Placeholder - restore some energy now
            p->RestoreFullEnergy();
        }},
        {"Glass Cannon", "+40% Damage, -20 Max HP", [](Player* p) { 
            p->GetStats().damageMultiplier *= 1.40f;
            p->GetStats().maxHealth -= 20;
            if (p->GetHealth() > p->GetMaxHealth()) {
                // Reduce health if over new max
            }
        }},
        {"Tank Mode", "+30 Max HP, -10% Speed", [](Player* p) { 
            p->GetStats().maxHealth += 30;
            p->Heal(30);
            p->GetStats().moveSpeed *= 0.90f;
        }},
        {"Berserker", "+25% Damage, +15% Fire Rate at low HP", [](Player* p) { 
            // Buff that activates when below 30% HP - for now just give stats
            p->GetStats().damageMultiplier *= 1.15f;
            p->GetStats().fireRateMultiplier *= 1.10f;
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

std::vector<BuffData> Player::GetRandomFloorBuffs(int count) {
    std::vector<BuffData> allBuffs = GetFloorBuffs();
    std::vector<BuffData> result;
    
    // Shuffle and pick 'count' buffs
    for (int i = 0; i < count && !allBuffs.empty(); ++i) {
        int idx = Utils::RandomInt(0, static_cast<int>(allBuffs.size()) - 1);
        result.push_back(allBuffs[idx]);
        allBuffs.erase(allBuffs.begin() + idx);
    }
    
    return result;
}
