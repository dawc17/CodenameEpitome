#pragma once

#include "raylib.h"
#include "raymath.h"

class Entity {
public:
    Entity() = default;
    Entity(Vector2 pos, float radius) : m_position(pos), m_radius(radius) {}
    virtual ~Entity() = default;
    
    virtual void Update(float dt) = 0;
    virtual void Render() = 0;
    
    // Position
    Vector2 GetPosition() const { return m_position; }
    void SetPosition(Vector2 pos) { m_position = pos; }
    
    // Velocity
    Vector2 GetVelocity() const { return m_velocity; }
    void SetVelocity(Vector2 vel) { m_velocity = vel; }
    
    // Collision
    float GetRadius() const { return m_radius; }
    bool IsActive() const { return m_active; }
    void SetActive(bool active) { m_active = active; }
    
    // Simple circle collision
    bool CollidesWith(const Entity& other) const {
        float dist = Vector2Distance(m_position, other.m_position);
        return dist < (m_radius + other.m_radius);
    }
    
    Rectangle GetBounds() const {
        return { m_position.x - m_radius, m_position.y - m_radius, 
                 m_radius * 2, m_radius * 2 };
    }

protected:
    Vector2 m_position = {0, 0};
    Vector2 m_velocity = {0, 0};
    float m_radius = 16.0f;
    bool m_active = true;
};
