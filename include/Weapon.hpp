#pragma once

#include "raylib.h"
#include <string>

enum class WeaponType {
    PISTOL,
    SHOTGUN,
    SMG,
    MAGIC_WAND,
    HEAVY_CANNON
};

struct WeaponData {
    std::string name;
    WeaponType type;
    int damage;
    float fireRate;         // shots per second
    float projectileSpeed;
    int projectilesPerShot; // for shotgun spread
    float spread;           // angle in degrees
    bool piercing;          // for magic wand
    int energyCost;
    Color projectileColor;
};

class Weapon {
public:
    Weapon(const WeaponData& data);
    virtual ~Weapon() = default;
    
    // Returns true if weapon fired
    bool TryFire(Vector2 position, Vector2 direction);
    void Update(float dt);
    
    // Getters
    const WeaponData& GetData() const { return m_data; }
    const std::string& GetName() const { return m_data.name; }
    bool CanFire() const { return m_cooldown <= 0.0f; }
    float GetCooldownPercent() const;
    
    // Factory methods for standard weapons
    static WeaponData CreatePistolData();
    static WeaponData CreateShotgunData();
    static WeaponData CreateSMGData();
    static WeaponData CreateMagicWandData();
    static WeaponData CreateHeavyCannonData();
    
protected:
    virtual void SpawnProjectiles(Vector2 position, Vector2 direction);
    
    WeaponData m_data;
    float m_cooldown = 0.0f;
};
