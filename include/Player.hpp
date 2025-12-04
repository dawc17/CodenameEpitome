#pragma once

#include "Entity.hpp"
#include "Weapon.hpp"
#include "Ability.hpp"
#include <memory>
#include <string>
#include <functional>
#include <vector>

// Buff definitions
struct BuffData {
    std::string name;
    std::string description;
    std::function<void(class Player*)> applyFunc;
};

enum class CharacterType {
    TERRORIST,
    COUNTER_TERRORIST
};

struct CharacterStats {
    std::string name = "Terrorist";
    int maxHealth = 100;
    int maxEnergy = 100;
    float moveSpeed = 200.0f;
    float energyRegen = 10.0f; // per second
    float damageMultiplier = 1.0f;  // For buff system
    float fireRateMultiplier = 1.0f;  // For buff system
    float cooldownMultiplier = 1.0f;  // For ability cooldown reduction
};

// Passive ability types
enum class PassiveType {
    NONE,
    EXPLOSIVE_ROUNDS,    // Terrorist: kills have 20% chance to explode
    TACTICAL_RELOAD      // Counter-Terrorist: reloading restores 10 energy
};

// Character data definitions
struct CharacterData {
    CharacterType type;
    std::string name;
    std::string description;
    std::string passiveName;
    std::string passiveDescription;
    std::string lore;
    PassiveType passive;
    CharacterStats stats;
    Color color;
};

class Player : public Entity {
public:
    Player();
    ~Player() override = default;
    
    void Update(float dt) override;
    void Render() override;
    
    // Combat
    void TakeDamage(int amount);
    void Heal(int amount);
    void UseEnergy(int amount);
    void RestoreFullEnergy() { m_energy = m_stats.maxEnergy; m_energyRegenAccumulator = 0.0f; }
    void Shoot();
    void UseAbility();
    
    // Getters
    int GetHealth() const { return m_health; }
    int GetMaxHealth() const { return m_stats.maxHealth; }
    int GetEnergy() const { return m_energy; }
    int GetMaxEnergy() const { return m_stats.maxEnergy; }
    float GetAbilityCooldownPercent() const;
    Weapon* GetWeapon() { return m_weapon.get(); }
    
    // Weapon management
    void EquipWeapon(std::unique_ptr<Weapon> weapon);
    void SetAbility(std::unique_ptr<Ability> ability);
    
    // Run Currency (lost on death)
    int GetRunCurrency() const { return m_runCurrency; }
    void AddRunCurrency(int amount) { m_runCurrency += amount; }
    bool SpendRunCurrency(int amount);
    
    // Meta Currency (persists between runs)
    static int GetMetaCurrency() { return s_metaCurrency; }
    static void AddMetaCurrency(int amount) { s_metaCurrency += amount; }
    static bool SpendMetaCurrency(int amount);
    
    // Passive ability
    PassiveType GetPassive() const { return m_passive; }
    void TriggerPassiveOnKill();
    
    // Buffs
    void ApplyBuff(const BuffData& buff);
    CharacterStats& GetStats() { return m_stats; }
    static std::vector<BuffData> GetStartingBuffs();
    static std::vector<BuffData> GetFloorBuffs();
    static std::vector<BuffData> GetRandomBuffs(int count);
    static std::vector<BuffData> GetRandomFloorBuffs(int count);
    
    // Auto-aim
    Vector2 GetAimDirection() const { return m_aimDirection; }
    Entity* GetTarget() const { return m_currentTarget; }
    
    // Respawn
    void Reset();
    
    // Character selection
    void SetCharacter(CharacterType type);
    CharacterType GetCharacterType() const { return m_characterType; }
    static CharacterData GetCharacterData(CharacterType type);
    
private:
    void HandleMovement(float dt);
    void UpdateAutoAim();
    void RegenerateEnergy(float dt);
    
    CharacterStats m_stats;
    CharacterType m_characterType = CharacterType::TERRORIST;
    PassiveType m_passive = PassiveType::NONE;
    int m_health = 100;
    int m_energy = 100;
    int m_runCurrency = 0;
    
    // Meta currency persists between runs (static)
    static int s_metaCurrency;
    
    std::unique_ptr<Weapon> m_weapon;
    std::unique_ptr<Ability> m_ability;
    
    Vector2 m_aimDirection = {1, 0};
    Entity* m_currentTarget = nullptr;
    
    // Visual
    Color m_color = BLUE;
    float m_shootCooldown = 0.0f;
    
    // Energy regeneration
    float m_energyRegenDelay = 0.0f;  // Time before regen starts
    float m_energyRegenAccumulator = 0.0f;  // Accumulate fractional energy
    static constexpr float ENERGY_REGEN_DELAY = 1.5f;  // 3 seconds after use
    static constexpr float ENERGY_REGEN_RATE = 9.0f;  // 5 energy per second
    
    // Auto-aim settings
    static constexpr float AIM_RANGE = 300.0f;
    static constexpr float AIM_SMOOTHING = 10.0f;
};
