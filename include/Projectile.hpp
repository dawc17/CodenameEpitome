#pragma once

#include "Entity.hpp"
#include <vector>

class Projectile : public Entity {
public:
    Projectile(Vector2 pos, Vector2 dir, float speed, int damage, 
               bool playerOwned, bool piercing = false, Color color = WHITE);
    ~Projectile() override = default;
    
    void Update(float dt) override;
    void Render() override;
    
    int GetDamage() const { return m_damage; }
    bool IsPlayerOwned() const { return m_playerOwned; }
    bool IsPiercing() const { return m_piercing; }
    
    void MarkForDestroy() { m_active = false; }
    
private:
    Vector2 m_direction;
    float m_speed;
    int m_damage;
    bool m_playerOwned;
    bool m_piercing;
    Color m_color;
    float m_lifetime = 3.0f; // auto-destroy after this time
};

class ProjectileManager {
public:
    ProjectileManager() = default;
    ~ProjectileManager() = default;
    
    void Update(float dt);
    void Render();
    void Clear();
    
    void SpawnProjectile(Vector2 pos, Vector2 dir, float speed, int damage,
                         bool playerOwned, bool piercing = false, Color color = WHITE);
    
    // Returns all projectiles for collision checking
    std::vector<Projectile>& GetProjectiles() { return m_projectiles; }
    
private:
    std::vector<Projectile> m_projectiles;
};
