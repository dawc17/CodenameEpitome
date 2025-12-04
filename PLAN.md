# PLAN.md – Dungeon Crawler Project

## 0. Tech Stack
Raylib and C++. External libraries allowed. Recommend if necessary.

## 1. Core Vision
A fast-paced, top-down dungeon crawler inspired by Soul Knight. Easy to pick up, satisfying to master. The long-term vision includes a wide roster of playable characters with unique lore, mechanics, and special skills.

## 2. Game Pillars
- ~~Fluid top-down action~~ ✅
  Same general perspective and movement feel as the attached screenshot.
- Weapons, weapons, weapons ✅
  A deep, ever-expanding arsenal with unique firing patterns, elemental variants, weird gimmicks, and high replay value.
- ~~Simple yet satisfying aiming~~ ✅
  Auto-aim system with smart targeting logic, so players focus on dodging, positioning, and ability timing.
- Character identity & abilities ✅
  Each character has:
  - A passive trait ✅
  - An active skill with cooldown ✅
  - A lore snippet that ties into the game's world ✅
- Procedurally generated dungeons ✅
  Rooms, enemies, traps, and loot vary between runs. Different biomes unlock as the game grows.
- Engaging enemy designs ✅
  Enemies with readable patterns, surprising behaviors, minibosses, and bosses.
- Dual-currency economy ✅
  - Meta Currency (kept between runs): unlock characters, skins, permanent upgrades. ✅
  - Run Currency (lost each run): used in dungeon shops for buffs, weapons, healing. ✅
- Buff selection between levels ✅
  After clearing a floor, the player chooses 1 of 3 randomized buffs.## 3. MVP Scope
### Characters (1–2)
~~Basic passive and active ability.~~ ✅
- Terrorist: Pistol + Explosion (AoE damage)
- Counter-Terrorist: Burst Rifle + Flashbang (immobilize enemies)

### Hub Area ✅
- Character selection
- Portal to start run
- Return after death with run results

### Weapons
5–8 weapons covering basic archetypes:

Pistol (single shot)✅

Shotgun (spread)✅

SMG (rapid fire)✅

Magic wand (piercing shot)✅

Heavy cannon (slow, strong)✅

Burst Rifle (3-round burst) ✅

Simple auto-targeting.✅

### Enemies
~~3–4 enemies + one miniboss.~~ ✅

~~Basic AI: move toward player, simple attack pattern.~~ ✅ (Now with advanced ranged AI)

### Dungeon
~~Procedural generator with:~~ ✅

~~Rooms connected by doors (all 4 directions)~~ ✅

~~Random enemy placement~~ ✅

Occasional loot rooms✅

Exit to next floor✅

### Economy
~~Run currency + simple vendor in dungeon.~~ ✅

~~Meta currency placeholder.~~ ✅

### UI
~~Health, energy, weapon slot, skill cooldown.~~ ✅

~~Minimap showing visited/unvisited rooms~~ ✅

~~Energy regeneration (5/sec after 3s delay)~~ ✅

## 4. Systems Breakdown
### Auto-Aim
~~Prioritize nearest enemy within cone or radius.~~ ✅

~~Sticky targeting to avoid jitter.~~ ✅

~~Smooth rotation towards target.~~ ✅

### Abilities System
~~Universal cooldown manager.~~ ✅

~~Each character ability is a modular component (easy to plug new ones in).~~ ✅

### Buffs
~~Tiered buffs (common → rare → epic).~~ ✅

~~Buff categories:~~ ✅

~~Weapon damage~~ ✅

~~Fire rate~~ ✅

~~Movement speed~~ ✅

~~Ability cooldown reduction~~ ✅

Special effects (chain lightning, burn, poison, etc.)

~~Buff selection at game start~~ ✅

### Dungeon Generator
Seed-based, weighted templates, biome support (later).

### Weapons Framework
Data-driven: damage, firerate, projectile type, modifiers✅. Weapon rarity tiers (later).

## 5. Art Direction
Pixel-art style similar to Soul Knight; clear silhouettes.

## 6. Long-Term Features
More characters with complex mechanics.

Boss rooms & multi-phase bosses.

Pets or companions.

Online or local co-op.

Crafting system or weapon upgrades.

Achievements.

Story mode or lore entries.

## 7. Development Roadmap
### Phase 1 — Core Engine ✅
~~Player movement, auto-aim, shooting, basic enemies, dungeon gen.~~

### Phase 2 — Combat Loop (In Progress)
Weapons, ~~enemy variety~~ ✅, ~~abilities~~ ✅, ~~run currency~~ ✅ + shop.

### Phase 3 — Meta Progression
Meta currency, character unlocks, ~~buff system~~ ✅.

### Phase 4 — Polish & Expansion
More content, visuals, sound, optimization.
