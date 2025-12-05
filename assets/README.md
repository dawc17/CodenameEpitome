# Assets Directory

This directory contains all game assets (sprites, sounds, etc.)

## Sprites (`sprites/`)

The **SpriteManager** automatically loads sprites from `assets/sprites/` on game startup.

### Naming Convention

Simply name your PNG file to match the entity type, and it will be auto-loaded:

| Entity Type | Filename |
|-------------|----------|
| **Player** | |
| Terrorist | `player_terrorist.png` |
| Counter-Terrorist | `player_counter_terrorist.png` |
| **Enemies** | |
| Slime | `enemy_slime.png` |
| Skeleton | `enemy_skeleton.png` |
| Bat | `enemy_bat.png` |
| Goblin | `enemy_goblin.png` |
| Golem | `enemy_golem.png` |
| **Environment** | |
| Portal | `portal.png` |
| Treasure Chest | `treasure_chest.png` |
| Shop Stand | `shop_stand.png` |
| **Projectiles** | |
| Player Bullet | `projectile_bullet.png` |
| Enemy Bullet | `projectile_enemy.png` |
| **Tiles** | |
| Floor | `tile_floor.png` |
| Wall | `tile_wall.png` |
| Door | `tile_door.png` |
| **Effects** | |
| Explosion | `effect_explosion.png` |
| Flashbang | `effect_flashbang.png` |

### How It Works

1. **Drop-in Replacement**: Just add a PNG with the correct name and the game will use it automatically
2. **Fallback Rendering**: If no sprite is found, the game uses primitive shapes (circles, rectangles)
3. **Auto-Scaling**: Sprites are automatically scaled to fit entity radius/size
4. **Animation Support**: Use sprite sheets for animated sprites

### Manual Loading (Code)

```cpp
#include "SpriteManager.hpp"

// Load a single sprite
SpriteManager::Instance().LoadSprite(SpriteType::ENEMY_SLIME, "my_slime.png");

// Load an animated sprite (sprite sheet)
SpriteManager::Instance().LoadAnimatedSprite(
    SpriteType::PORTAL,
    "portal_animated.png",
    32,    // frame width
    32,    // frame height
    8,     // frame count
    0.1f,  // seconds per frame
    true   // loop animation
);

// Set sprite properties
SpriteManager::Instance().SetSpriteScale(SpriteType::PORTAL, 1.5f);
SpriteManager::Instance().SetSpriteTint(SpriteType::ENEMY_SLIME, GREEN);
```

### Tips

- **Transparency**: Use PNG with alpha channel for transparent backgrounds
- **Size**: Sprites will be scaled to fit - use any reasonable size
- **Orientation**: Sprites should face RIGHT (rotation 0°) as the default
- **Center Origin**: The sprite's center will be used as the pivot point

## Directory Structure

- `sprites/` - Character and enemy sprites (auto-loaded)
- `tiles/` - Dungeon tile textures  
- `ui/` - UI elements
- `sounds/` - Sound effects
- `music/` - Background music
