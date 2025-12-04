#pragma once

#include "raylib.h"
#include <string>
#include <functional>
#include <memory>

class Player;

class Ability {
public:
    Ability(const std::string& name, float cooldown, int energyCost,
            std::function<void(Player*)> effect);
    virtual ~Ability() = default;
    
    bool TryActivate(Player* player);
    void Update(float dt);
    
    const std::string& GetName() const { return m_name; }
    float GetCooldownPercent() const;
    bool IsReady() const { return m_currentCooldown <= 0.0f; }
    int GetEnergyCost() const { return m_energyCost; }
    
protected:
    std::string m_name;
    float m_cooldown;
    float m_currentCooldown = 0.0f;
    int m_energyCost;
    std::function<void(Player*)> m_effect;
};

// Predefined abilities
namespace Abilities {
    // Knight: Shield Dash - dash forward, invincible during dash
    std::unique_ptr<Ability> CreateShieldDash();
    
    // Mage: Arcane Burst - AoE damage around player
    std::unique_ptr<Ability> CreateArcaneBurst();
}
