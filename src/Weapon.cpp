#include "Weapon.hpp"
#include "Game.hpp"
#include "Projectile.hpp"
#include "Utils.hpp"

Weapon::Weapon(const WeaponData& data) : m_data(data), m_cooldown(0.0f) {
}

bool Weapon::TryFire(Vector2 position, Vector2 direction) {
    if (!CanFire()) return false;
    
    SpawnProjectiles(position, direction);
    m_cooldown = 1.0f / m_data.fireRate;
    
    return true;
}

void Weapon::Update(float dt) {
    if (m_cooldown > 0) {
        m_cooldown -= dt;
    }
}

float Weapon::GetCooldownPercent() const {
    if (m_data.fireRate <= 0) return 0.0f;
    float maxCooldown = 1.0f / m_data.fireRate;
    return m_cooldown / maxCooldown;
}

void Weapon::SpawnProjectiles(Vector2 position, Vector2 direction) {
    ProjectileManager* projectiles = Game::Instance().GetProjectiles();
    if (!projectiles) return;
    
    if (m_data.projectilesPerShot == 1) {
        // Single projectile with optional spread
        Vector2 dir = direction;
        if (m_data.spread > 0) {
            float angleOffset = Utils::RandomFloat(-m_data.spread/2, m_data.spread/2);
            dir = Utils::RotateVector(direction, angleOffset);
        }
        
        projectiles->SpawnProjectile(position, dir, m_data.projectileSpeed,
            m_data.damage, true, m_data.piercing, m_data.projectileColor);
    } else {
        // Multiple projectiles (shotgun style)
        float totalSpread = m_data.spread;
        float angleStep = totalSpread / (m_data.projectilesPerShot - 1);
        float startAngle = -totalSpread / 2;
        
        for (int i = 0; i < m_data.projectilesPerShot; ++i) {
            float angle = startAngle + angleStep * i;
            Vector2 dir = Utils::RotateVector(direction, angle);
            
            projectiles->SpawnProjectile(position, dir, m_data.projectileSpeed,
                m_data.damage, true, m_data.piercing, m_data.projectileColor);
        }
    }
}

// Factory methods for standard weapons
WeaponData Weapon::CreatePistolData() {
    return {
        "Pistol",
        WeaponType::PISTOL,
        15,             // damage
        3.0f,           // fireRate
        400.0f,         // projectileSpeed
        1,              // projectilesPerShot
        5.0f,           // spread
        false,          // piercing
        5,              // energyCost
        YELLOW          // projectileColor
    };
}

WeaponData Weapon::CreateShotgunData() {
    return {
        "Shotgun",
        WeaponType::SHOTGUN,
        8,              // damage per pellet
        1.2f,           // fireRate
        350.0f,         // projectileSpeed
        5,              // projectilesPerShot
        40.0f,          // spread
        false,          // piercing
        15,             // energyCost
        ORANGE          // projectileColor
    };
}

WeaponData Weapon::CreateSMGData() {
    return {
        "SMG",
        WeaponType::SMG,
        8,              // damage
        10.0f,          // fireRate
        450.0f,         // projectileSpeed
        1,              // projectilesPerShot
        10.0f,          // spread
        false,          // piercing
        3,              // energyCost
        YELLOW          // projectileColor
    };
}

WeaponData Weapon::CreateMagicWandData() {
    return {
        "Magic Wand",
        WeaponType::MAGIC_WAND,
        20,             // damage
        2.0f,           // fireRate
        300.0f,         // projectileSpeed
        1,              // projectilesPerShot
        0.0f,           // spread
        true,           // piercing
        12,             // energyCost
        PURPLE          // projectileColor
    };
}

WeaponData Weapon::CreateHeavyCannonData() {
    return {
        "Heavy Cannon",
        WeaponType::HEAVY_CANNON,
        50,             // damage
        0.7f,           // fireRate
        250.0f,         // projectileSpeed
        1,              // projectilesPerShot
        2.0f,           // spread
        false,          // piercing
        25,             // energyCost
        RED             // projectileColor
    };
}
