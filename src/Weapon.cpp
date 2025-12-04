#include "Weapon.hpp"
#include "Game.hpp"
#include "Player.hpp"
#include "Projectile.hpp"
#include "Utils.hpp"

Weapon::Weapon(const WeaponData& data) : m_data(data), m_cooldown(0.0f) {
}

bool Weapon::TryFire(Vector2 position, Vector2 direction) {
    if (!CanFire()) return false;
    
    // Check if this is a burst weapon
    if (m_data.burstDelay > 0 && m_data.projectilesPerShot > 1) {
        // Start burst fire
        m_burstShotsRemaining = m_data.projectilesPerShot;
        m_burstTimer = 0.0f;
        m_burstPosition = position;
        m_burstDirection = direction;
        
        // Fire first shot immediately
        SpawnSingleProjectile(position, direction);
        m_burstShotsRemaining--;
        m_burstTimer = m_data.burstDelay;
    } else {
        SpawnProjectiles(position, direction);
    }
    
    m_cooldown = 1.0f / m_data.fireRate;
    return true;
}

void Weapon::Update(float dt) {
    if (m_cooldown > 0) {
        m_cooldown -= dt;
    }
    
    // Handle burst fire
    if (m_burstShotsRemaining > 0) {
        m_burstTimer -= dt;
        if (m_burstTimer <= 0) {
            // Get current player position and direction for accurate burst
            Player* player = Game::Instance().GetPlayer();
            if (player) {
                m_burstPosition = player->GetPosition();
                m_burstDirection = player->GetAimDirection();
            }
            SpawnSingleProjectile(m_burstPosition, m_burstDirection);
            m_burstShotsRemaining--;
            m_burstTimer = m_data.burstDelay;
        }
    }
}

float Weapon::GetCooldownPercent() const {
    if (m_data.fireRate <= 0) return 0.0f;
    float maxCooldown = 1.0f / m_data.fireRate;
    return m_cooldown / maxCooldown;
}

void Weapon::SpawnSingleProjectile(Vector2 position, Vector2 direction, float angleOffset) {
    ProjectileManager* projectiles = Game::Instance().GetProjectiles();
    if (!projectiles) return;
    
    Vector2 dir = direction;
    if (m_data.spread > 0 || angleOffset != 0) {
        float randomSpread = m_data.spread > 0 ? Utils::RandomFloat(-m_data.spread/2, m_data.spread/2) : 0.0f;
        dir = Utils::RotateVector(direction, angleOffset + randomSpread);
    }
    
    projectiles->SpawnProjectile(position, dir, m_data.projectileSpeed,
        m_data.damage, true, m_data.piercing, m_data.projectileColor, m_data.projectileSize);
}

void Weapon::SpawnProjectiles(Vector2 position, Vector2 direction) {
    ProjectileManager* projectiles = Game::Instance().GetProjectiles();
    if (!projectiles) return;
    
    if (m_data.projectilesPerShot == 1) {
        // Single projectile with optional spread
        SpawnSingleProjectile(position, direction);
    } else {
        // Multiple projectiles (shotgun style - all at once)
        float totalSpread = m_data.spread;
        float angleStep = totalSpread / (m_data.projectilesPerShot - 1);
        float startAngle = -totalSpread / 2;
        
        for (int i = 0; i < m_data.projectilesPerShot; ++i) {
            float angle = startAngle + angleStep * i;
            Vector2 dir = Utils::RotateVector(direction, angle);
            
            projectiles->SpawnProjectile(position, dir, m_data.projectileSpeed,
                m_data.damage, true, m_data.piercing, m_data.projectileColor, m_data.projectileSize);
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
        450.0f,         // projectileSpeed
        1,              // projectilesPerShot
        5.0f,           // spread
        false,          // piercing
        5,              // energyCost
        YELLOW,         // projectileColor
        4.0f,           // projectileSize (small)
        0.0f            // burstDelay (no burst)
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
        ORANGE,         // projectileColor
        5.0f,           // projectileSize (small pellets)
        0.0f            // burstDelay (simultaneous)
    };
}

WeaponData Weapon::CreateSMGData() {
    return {
        "SMG",
        WeaponType::SMG,
        8,              // damage
        10.0f,          // fireRate
        500.0f,         // projectileSpeed
        1,              // projectilesPerShot
        10.0f,          // spread
        false,          // piercing
        3,              // energyCost
        YELLOW,         // projectileColor
        3.0f,           // projectileSize (tiny)
        0.0f            // burstDelay (no burst)
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
        PURPLE,         // projectileColor
        8.0f,           // projectileSize (medium magic orb)
        0.0f            // burstDelay (no burst)
    };
}

WeaponData Weapon::CreateHeavyCannonData() {
    return {
        "Heavy Cannon",
        WeaponType::HEAVY_CANNON,
        50,             // damage
        0.7f,           // fireRate
        200.0f,         // projectileSpeed (slow)
        1,              // projectilesPerShot
        2.0f,           // spread
        false,          // piercing
        25,             // energyCost
        RED,            // projectileColor
        16.0f,          // projectileSize (large cannonball)
        0.0f            // burstDelay (no burst)
    };
}

WeaponData Weapon::CreateBurstRifleData() {
    return {
        "Burst Rifle",
        WeaponType::BURST_RIFLE,
        12,             // damage per bullet
        2.0f,           // fireRate (bursts per second)
        420.0f,         // projectileSpeed
        3,              // projectilesPerShot (3-round burst)
        4.0f,           // spread
        false,          // piercing
        8,              // energyCost
        ORANGE,         // projectileColor
        5.0f,           // projectileSize
        0.06f           // burstDelay (60ms between shots in burst)
    };
}
