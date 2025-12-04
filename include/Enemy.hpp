#pragma once

#include "Entity.hpp"
#include <vector>
#include <memory>
#include <string>

enum class EnemyType {
    SLIME,          // Basic melee, slow
    SKELETON,       // Ranged, stationary shooter  
    BAT,            // Fast, erratic movement
    GOBLIN,         // Melee, charges at player
    MINI_BOSS_GOLEM // Tanky, AoE attacks
};

struct EnemyData {
    EnemyType type;
    std::string name;
    int maxHealth;
    float moveSpeed;
    int damage;
    float attackCooldown;
    float detectionRange;
    int currencyDrop;
    Color color;
};

class Enemy : public Entity {
public:
    Enemy(const EnemyData& data, Vector2 pos);
    ~Enemy() override = default;
    
    void Update(float dt) override;
    void Render() override;
    
    void TakeDamage(int amount);
    bool IsDead() const { return m_health <= 0; }
    
    const EnemyData& GetData() const { return m_data; }
    int GetHealth() const { return m_health; }
    int GetMaxHealth() const { return m_data.maxHealth; }
    
    // Factory methods
    static EnemyData CreateSlimeData();
    static EnemyData CreateSkeletonData();
    static EnemyData CreateBatData();
    static EnemyData CreateGoblinData();
    static EnemyData CreateGolemData();
    
protected:
    virtual void UpdateAI(float dt);
    virtual void Attack();
    float GetAttackRange() const;
    float GetPreferredDistance() const;  // For ranged enemies to maintain distance
    bool HasLineOfSight() const;
    Vector2 FindRepositionTarget() const;
    
    EnemyData m_data;
    int m_health;
    float m_attackTimer = 0.0f;
    float m_stateTimer = 0.0f;
    float m_repositionTimer = 0.0f;  // Timer for repositioning behavior
    Vector2 m_repositionTarget = {0, 0};  // Target position for repositioning
    Vector2 m_lastKnownPlayerPos = {0, 0};  // Last known player position
    float m_searchTimer = 0.0f;  // Timer for searching behavior
    
    // AI state
    enum class AIState { IDLE, CHASE, ATTACK, SPECIAL, REPOSITION, SEARCH };
    AIState m_aiState = AIState::IDLE;
};

class EnemyManager {
public:
    EnemyManager() = default;
    ~EnemyManager() = default;
    
    void Update(float dt);
    void Render();
    void Clear();
    
    void SpawnEnemy(EnemyType type, Vector2 pos);
    void SpawnEnemiesInRoom(const std::vector<Vector2>& spawnPoints, int difficulty);
    
    std::vector<std::unique_ptr<Enemy>>& GetEnemies() { return m_enemies; }
    int GetActiveCount() const;
    
    // For auto-aim
    Enemy* GetNearestEnemy(Vector2 pos, float maxRange);
    
private:
    std::vector<std::unique_ptr<Enemy>> m_enemies;
};
