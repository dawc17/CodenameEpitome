#pragma once

#include "raylib.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

// ============================================================================
// Sprite Types - Add new entity types here for easy sprite assignment
// ============================================================================
enum class SpriteType {
    // Player
    PLAYER_TERRORIST,
    PLAYER_COUNTER_TERRORIST,
    
    // Enemies
    ENEMY_SLIME,
    ENEMY_SKELETON,
    ENEMY_BAT,
    ENEMY_GOBLIN,
    ENEMY_GOLEM,
    
    // Environment
    PORTAL,
    TREASURE_CHEST,
    SHOP_STAND,
    
    // Projectiles
    PROJECTILE_BULLET,
    PROJECTILE_ENEMY,
    
    // Tiles
    TILE_FLOOR,
    TILE_WALL,
    TILE_DOOR,
    
    // UI/Effects
    EFFECT_EXPLOSION,
    EFFECT_FLASHBANG,
    
    // Add more sprite types as needed
    COUNT  // Keep this last
};

// ============================================================================
// Animation Data - For animated sprites
// ============================================================================
struct AnimationData {
    int frameCount = 1;
    int currentFrame = 0;
    float frameTime = 0.1f;      // Seconds per frame
    float timer = 0.0f;
    bool loop = true;
    int framesPerRow = 1;        // For sprite sheets with multiple rows
    
    void Update(float dt);
    void Reset();
};

// ============================================================================
// Sprite Data - Contains texture and rendering info
// ============================================================================
struct SpriteData {
    Texture2D texture = {0};
    bool isLoaded = false;
    
    // Source rectangle (for sprite sheets)
    Rectangle sourceRect = {0, 0, 0, 0};
    
    // Rendering settings
    Vector2 origin = {0, 0};     // Center point for rotation
    float scale = 1.0f;
    float rotation = 0.0f;
    Color tint = WHITE;
    
    // Animation
    AnimationData animation;
    bool isAnimated = false;
    
    // Frame size for sprite sheets
    int frameWidth = 0;
    int frameHeight = 0;
    
    // Get current frame source rect
    Rectangle GetCurrentFrame() const;
};

// ============================================================================
// Sprite Manager - Singleton that handles all sprite loading and rendering
// ============================================================================
class SpriteManager {
public:
    static SpriteManager& Instance();
    
    // Initialization and cleanup
    void Init();
    void Shutdown();
    void Update(float dt);  // Update animations
    
    // Load sprites from files
    bool LoadSprite(SpriteType type, const std::string& filename);
    bool LoadAnimatedSprite(SpriteType type, const std::string& filename, 
                           int frameWidth, int frameHeight, int frameCount,
                           float frameTime = 0.1f, bool loop = true);
    
    // Unload specific sprite
    void UnloadSprite(SpriteType type);
    
    // Check if sprite is loaded
    bool HasSprite(SpriteType type) const;
    
    // Get sprite data for custom rendering
    SpriteData* GetSprite(SpriteType type);
    const SpriteData* GetSprite(SpriteType type) const;
    
    // ========================================================================
    // Rendering helpers - Draw sprites easily
    // ========================================================================
    
    // Draw at position with automatic centering
    void Draw(SpriteType type, Vector2 position);
    
    // Draw with custom scale
    void Draw(SpriteType type, Vector2 position, float scale);
    
    // Draw with custom rotation and scale
    void Draw(SpriteType type, Vector2 position, float rotation, float scale);
    
    // Draw with all parameters
    void Draw(SpriteType type, Vector2 position, float rotation, float scale, Color tint);
    
    // Draw to fit within a radius (for entity sprites)
    void DrawFitRadius(SpriteType type, Vector2 position, float radius);
    void DrawFitRadius(SpriteType type, Vector2 position, float radius, float rotation, Color tint);
    
    // Draw stretched to fill a rectangle
    void DrawRect(SpriteType type, Rectangle destRect);
    void DrawRect(SpriteType type, Rectangle destRect, Color tint);
    
    // ========================================================================
    // Configuration
    // ========================================================================
    
    // Set default sprite directory
    void SetAssetPath(const std::string& path) { m_assetPath = path; }
    std::string GetAssetPath() const { return m_assetPath; }
    
    // Sprite configuration helpers
    void SetSpriteScale(SpriteType type, float scale);
    void SetSpriteOrigin(SpriteType type, Vector2 origin);
    void SetSpriteTint(SpriteType type, Color tint);
    
    // Load all sprites from a config (returns number loaded)
    int LoadFromDirectory(const std::string& directory);
    
    // ========================================================================
    // Utility
    // ========================================================================
    
    // Get string name for sprite type (for loading/debugging)
    static const char* GetSpriteName(SpriteType type);
    
    // Get expected filename for sprite type
    static std::string GetDefaultFilename(SpriteType type);
    
private:
    SpriteManager() = default;
    ~SpriteManager() = default;
    SpriteManager(const SpriteManager&) = delete;
    SpriteManager& operator=(const SpriteManager&) = delete;
    
    std::unordered_map<SpriteType, SpriteData> m_sprites;
    std::string m_assetPath = "assets/sprites/";
    bool m_initialized = false;
    
    // Internal helpers
    void DrawInternal(const SpriteData& sprite, Vector2 position, 
                      float rotation, float scale, Color tint);
};

// ============================================================================
// Convenience macros for quick sprite rendering
// ============================================================================
#define SPRITE_MANAGER SpriteManager::Instance()
#define DRAW_SPRITE(type, pos) SpriteManager::Instance().Draw(type, pos)
#define DRAW_SPRITE_RADIUS(type, pos, radius) SpriteManager::Instance().DrawFitRadius(type, pos, radius)
