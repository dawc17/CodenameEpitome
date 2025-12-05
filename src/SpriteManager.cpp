#include "SpriteManager.hpp"
#include <filesystem>
#include <algorithm>
#include <cmath>

namespace fs = std::filesystem;

// ============================================================================
// AnimationData Implementation
// ============================================================================
void AnimationData::Update(float dt) {
    timer += dt;
    if (timer >= frameTime) {
        timer -= frameTime;
        currentFrame++;
        if (currentFrame >= frameCount) {
            currentFrame = loop ? 0 : frameCount - 1;
        }
    }
}

void AnimationData::Reset() {
    currentFrame = 0;
    timer = 0.0f;
}

// ============================================================================
// SpriteData Implementation
// ============================================================================
Rectangle SpriteData::GetCurrentFrame() const {
    if (!isAnimated || frameWidth == 0 || frameHeight == 0) {
        return sourceRect;
    }
    
    int framesPerRow = texture.width / frameWidth;
    int row = animation.currentFrame / framesPerRow;
    int col = animation.currentFrame % framesPerRow;
    
    return {
        static_cast<float>(col * frameWidth),
        static_cast<float>(row * frameHeight),
        static_cast<float>(frameWidth),
        static_cast<float>(frameHeight)
    };
}

// ============================================================================
// SpriteManager Implementation
// ============================================================================
SpriteManager& SpriteManager::Instance() {
    static SpriteManager instance;
    return instance;
}

void SpriteManager::Init() {
    if (m_initialized) return;
    
    m_sprites.clear();
    m_initialized = true;
    
    // Try to auto-load common sprites from asset path
    LoadFromDirectory(m_assetPath);
}

void SpriteManager::Shutdown() {
    for (auto& [type, sprite] : m_sprites) {
        if (sprite.isLoaded) {
            UnloadTexture(sprite.texture);
        }
    }
    m_sprites.clear();
    m_initialized = false;
}

void SpriteManager::Update(float dt) {
    for (auto& [type, sprite] : m_sprites) {
        if (sprite.isAnimated && sprite.isLoaded) {
            sprite.animation.Update(dt);
        }
    }
}

bool SpriteManager::LoadSprite(SpriteType type, const std::string& filename) {
    std::string fullPath = m_assetPath + filename;
    
    // Check if file exists
    if (!FileExists(fullPath.c_str())) {
        TraceLog(LOG_WARNING, "SpriteManager: File not found: %s", fullPath.c_str());
        return false;
    }
    
    // Unload existing sprite if present
    if (m_sprites.count(type) && m_sprites[type].isLoaded) {
        UnloadTexture(m_sprites[type].texture);
    }
    
    SpriteData sprite;
    sprite.texture = LoadTexture(fullPath.c_str());
    sprite.isLoaded = sprite.texture.id != 0;
    
    if (sprite.isLoaded) {
        sprite.sourceRect = {0, 0, 
                            static_cast<float>(sprite.texture.width),
                            static_cast<float>(sprite.texture.height)};
        sprite.origin = {sprite.texture.width / 2.0f, sprite.texture.height / 2.0f};
        sprite.frameWidth = sprite.texture.width;
        sprite.frameHeight = sprite.texture.height;
        
        m_sprites[type] = sprite;
        TraceLog(LOG_INFO, "SpriteManager: Loaded %s for %s", 
                 filename.c_str(), GetSpriteName(type));
        return true;
    }
    
    TraceLog(LOG_WARNING, "SpriteManager: Failed to load texture: %s", fullPath.c_str());
    return false;
}

bool SpriteManager::LoadAnimatedSprite(SpriteType type, const std::string& filename,
                                        int frameWidth, int frameHeight, int frameCount,
                                        float frameTime, bool loop) {
    if (!LoadSprite(type, filename)) {
        return false;
    }
    
    SpriteData& sprite = m_sprites[type];
    sprite.isAnimated = true;
    sprite.frameWidth = frameWidth;
    sprite.frameHeight = frameHeight;
    sprite.origin = {frameWidth / 2.0f, frameHeight / 2.0f};
    sprite.sourceRect = {0, 0, static_cast<float>(frameWidth), static_cast<float>(frameHeight)};
    
    sprite.animation.frameCount = frameCount;
    sprite.animation.frameTime = frameTime;
    sprite.animation.loop = loop;
    sprite.animation.framesPerRow = sprite.texture.width / frameWidth;
    
    return true;
}

void SpriteManager::UnloadSprite(SpriteType type) {
    if (m_sprites.count(type) && m_sprites[type].isLoaded) {
        UnloadTexture(m_sprites[type].texture);
        m_sprites.erase(type);
    }
}

bool SpriteManager::HasSprite(SpriteType type) const {
    auto it = m_sprites.find(type);
    return it != m_sprites.end() && it->second.isLoaded;
}

SpriteData* SpriteManager::GetSprite(SpriteType type) {
    auto it = m_sprites.find(type);
    return it != m_sprites.end() ? &it->second : nullptr;
}

const SpriteData* SpriteManager::GetSprite(SpriteType type) const {
    auto it = m_sprites.find(type);
    return it != m_sprites.end() ? &it->second : nullptr;
}

// ============================================================================
// Drawing Functions
// ============================================================================
void SpriteManager::Draw(SpriteType type, Vector2 position) {
    Draw(type, position, 0.0f, 1.0f, WHITE);
}

void SpriteManager::Draw(SpriteType type, Vector2 position, float scale) {
    Draw(type, position, 0.0f, scale, WHITE);
}

void SpriteManager::Draw(SpriteType type, Vector2 position, float rotation, float scale) {
    Draw(type, position, rotation, scale, WHITE);
}

void SpriteManager::Draw(SpriteType type, Vector2 position, float rotation, float scale, Color tint) {
    if (!HasSprite(type)) return;
    
    const SpriteData& sprite = m_sprites[type];
    DrawInternal(sprite, position, rotation, scale, tint);
}

void SpriteManager::DrawFitRadius(SpriteType type, Vector2 position, float radius) {
    DrawFitRadius(type, position, radius, 0.0f, WHITE);
}

void SpriteManager::DrawFitRadius(SpriteType type, Vector2 position, float radius, 
                                   float rotation, Color tint) {
    if (!HasSprite(type)) return;
    
    const SpriteData& sprite = m_sprites[type];
    
    // Calculate scale to fit the sprite within the radius
    Rectangle srcRect = sprite.GetCurrentFrame();
    float maxDim = fmaxf(srcRect.width, srcRect.height);
    float scale = (radius * 2.0f) / maxDim;
    
    DrawInternal(sprite, position, rotation, scale, tint);
}

void SpriteManager::DrawRect(SpriteType type, Rectangle destRect) {
    DrawRect(type, destRect, WHITE);
}

void SpriteManager::DrawRect(SpriteType type, Rectangle destRect, Color tint) {
    if (!HasSprite(type)) return;
    
    const SpriteData& sprite = m_sprites[type];
    Rectangle srcRect = sprite.GetCurrentFrame();
    
    Vector2 origin = {0, 0};
    DrawTexturePro(sprite.texture, srcRect, destRect, origin, 0.0f, tint);
}

void SpriteManager::DrawInternal(const SpriteData& sprite, Vector2 position,
                                  float rotation, float scale, Color tint) {
    Rectangle srcRect = sprite.GetCurrentFrame();
    
    Rectangle destRect = {
        position.x,
        position.y,
        srcRect.width * scale * sprite.scale,
        srcRect.height * scale * sprite.scale
    };
    
    Vector2 origin = {
        sprite.origin.x * scale * sprite.scale,
        sprite.origin.y * scale * sprite.scale
    };
    
    // Combine tints
    Color finalTint = {
        static_cast<unsigned char>((tint.r * sprite.tint.r) / 255),
        static_cast<unsigned char>((tint.g * sprite.tint.g) / 255),
        static_cast<unsigned char>((tint.b * sprite.tint.b) / 255),
        static_cast<unsigned char>((tint.a * sprite.tint.a) / 255)
    };
    
    DrawTexturePro(sprite.texture, srcRect, destRect, origin, 
                   rotation + sprite.rotation, finalTint);
}

// ============================================================================
// Configuration Functions
// ============================================================================
void SpriteManager::SetSpriteScale(SpriteType type, float scale) {
    if (m_sprites.count(type)) {
        m_sprites[type].scale = scale;
    }
}

void SpriteManager::SetSpriteOrigin(SpriteType type, Vector2 origin) {
    if (m_sprites.count(type)) {
        m_sprites[type].origin = origin;
    }
}

void SpriteManager::SetSpriteTint(SpriteType type, Color tint) {
    if (m_sprites.count(type)) {
        m_sprites[type].tint = tint;
    }
}

int SpriteManager::LoadFromDirectory(const std::string& directory) {
    int loaded = 0;
    
    // Try to load default sprites based on naming convention
    for (int i = 0; i < static_cast<int>(SpriteType::COUNT); ++i) {
        SpriteType type = static_cast<SpriteType>(i);
        std::string filename = GetDefaultFilename(type);
        std::string fullPath = directory + filename;
        
        if (FileExists(fullPath.c_str())) {
            if (LoadSprite(type, filename)) {
                loaded++;
            }
        }
    }
    
    TraceLog(LOG_INFO, "SpriteManager: Auto-loaded %d sprites from %s", 
             loaded, directory.c_str());
    return loaded;
}

// ============================================================================
// Utility Functions
// ============================================================================
const char* SpriteManager::GetSpriteName(SpriteType type) {
    switch (type) {
        case SpriteType::PLAYER_TERRORIST: return "player_terrorist";
        case SpriteType::PLAYER_COUNTER_TERRORIST: return "player_counter_terrorist";
        case SpriteType::ENEMY_SLIME: return "enemy_slime";
        case SpriteType::ENEMY_SKELETON: return "enemy_skeleton";
        case SpriteType::ENEMY_BAT: return "enemy_bat";
        case SpriteType::ENEMY_GOBLIN: return "enemy_goblin";
        case SpriteType::ENEMY_GOLEM: return "enemy_golem";
        case SpriteType::PORTAL: return "portal";
        case SpriteType::TREASURE_CHEST: return "treasure_chest";
        case SpriteType::SHOP_STAND: return "shop_stand";
        case SpriteType::PROJECTILE_BULLET: return "projectile_bullet";
        case SpriteType::PROJECTILE_ENEMY: return "projectile_enemy";
        case SpriteType::TILE_FLOOR: return "tile_floor";
        case SpriteType::TILE_WALL: return "tile_wall";
        case SpriteType::TILE_DOOR: return "tile_door";
        case SpriteType::EFFECT_EXPLOSION: return "effect_explosion";
        case SpriteType::EFFECT_FLASHBANG: return "effect_flashbang";
        default: return "unknown";
    }
}

std::string SpriteManager::GetDefaultFilename(SpriteType type) {
    return std::string(GetSpriteName(type)) + ".png";
}
