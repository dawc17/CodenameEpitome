#include "Projectile.hpp"
#include "Utils.hpp"
#include <algorithm>

// Projectile implementation
Projectile::Projectile(Vector2 pos, Vector2 dir, float speed, int damage,
                       bool playerOwned, bool piercing, Color color)
    : Entity(pos, 6.0f)
    , m_direction(Vector2Normalize(dir))
    , m_speed(speed)
    , m_damage(damage)
    , m_playerOwned(playerOwned)
    , m_piercing(piercing)
    , m_color(color)
{
}

void Projectile::Update(float dt) {
    if (!m_active) return;
    
    // Move projectile
    m_position = Vector2Add(m_position, Vector2Scale(m_direction, m_speed * dt));
    
    // Decrease lifetime
    m_lifetime -= dt;
    if (m_lifetime <= 0) {
        m_active = false;
    }
}

void Projectile::Render() {
    if (!m_active) return;
    
    // Draw projectile as a small circle with a trail effect
    DrawCircleV(m_position, m_radius, m_color);
    
    // Draw a simple trail
    Vector2 trailEnd = Vector2Subtract(m_position, Vector2Scale(m_direction, m_radius * 2));
    DrawLineEx(trailEnd, m_position, m_radius * 0.8f, 
               ColorAlpha(m_color, 0.5f));
}

// ProjectileManager implementation
void ProjectileManager::Update(float dt) {
    for (auto& projectile : m_projectiles) {
        projectile.Update(dt);
    }
    
    // Remove inactive projectiles
    m_projectiles.erase(
        std::remove_if(m_projectiles.begin(), m_projectiles.end(),
            [](const Projectile& p) { return !p.IsActive(); }),
        m_projectiles.end()
    );
}

void ProjectileManager::Render() {
    for (auto& projectile : m_projectiles) {
        projectile.Render();
    }
}

void ProjectileManager::Clear() {
    m_projectiles.clear();
}

void ProjectileManager::SpawnProjectile(Vector2 pos, Vector2 dir, float speed, 
                                         int damage, bool playerOwned, 
                                         bool piercing, Color color) {
    m_projectiles.emplace_back(pos, dir, speed, damage, playerOwned, piercing, color);
}
