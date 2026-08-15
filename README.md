# SpiderHero — Unreal Engine 5 Superhero Game Prototype

SpiderHero is an original, AAA-inspired, modular third-person spider-themed superhero action game architecture built entirely in C++ for Unreal Engine 5.4+.

It features custom superhero kinematic physics, multi-trace parkour traversal, realistic pendulum web-swinging, ballistic point-zipping, a combo-driven melee/web combat system with Spider-Sense cues, perception-driven enemy AI coordinated via a combat token director, procedural HISM city generation, lightweight vehicle and pedestrian crowd simulations, dynamic 24-hour day/night cycles, dynamic weather, a 4-tier skill tree progression system, modern minimalist HUD, versioned SaveGame serialization, and automated verification suites.

---

## 📋 Table of Contents
1. [Project Overview & Architecture](#project-overview--architecture)
2. [Unreal Engine Version & Prerequisites](#unreal-engine-version--prerequisites)
3. [Folder Structure](#folder-structure)
4. [Controls & Input Mapping](#controls--input-mapping)
5. [Core Gameplay Systems](#core-gameplay-systems)
   - [Movement & Traversal (Phase 2)](#1-movement--traversal)
   - [Web Mechanics & Physics (Phase 3)](#2-web-mechanics--pendulum-physics)
   - [Superhero Camera (Phase 4)](#3-superhero-camera-system)
   - [Combat & Abilities (Phase 5)](#4-melee--web-combat-system)
   - [Enemy AI & Token Director (Phase 6)](#5-enemy-ai--attack-token-director)
   - [World, Traffic & Civilians (Phases 7 & 8)](#6-world-generation-traffic--civilians)
   - [Missions & Progression (Phases 9 & 10)](#7-missions--progression)
   - [UI, Audio & VFX (Phases 11 & 12)](#8-ui-audio--vfx)
   - [Day/Night, Weather & Cinematics (Phases 13 & 14)](#9-daynight-weather--cinematics)
   - [SaveGame, Scalability & Testing (Phases 15 & 16)](#10-savegame-scalability--testing)
6. [Build Instructions](#build-instructions)
7. [Developer Debug Commands](#developer-debug-commands)
8. [Performance & Scalability Notes](#performance--scalability-notes)
9. [Future Improvements & Art Pipeline](#future-improvements--art-pipeline)

---

## 🏗️ Project Overview & Architecture

SpiderHero is engineered with a strict **Component-Based, Interface-Driven C++ Architecture**. The core player pawn (`ASpiderHeroCharacter`) acts as an orchestrator delegating gameplay logic to dedicated, decoupled actor components:

```
ASpiderHeroCharacter
 ├── USpiderMovementComponent      (Custom superhero kinematic physics)
 ├── USpiderTraversalComponent     (Multi-stage ledge/obstacle raycasting)
 ├── USpiderWebTargetingComponent  (Frustum view cone anchor scoring)
 ├── USpiderWebSwingComponent      (Pendulum tension & centripetal physics)
 ├── USpiderWebZipComponent        (Ballistic point-zip & point-launch)
 ├── USpiderWebShootingComponent   (Web fluid ammo & projectile firing)
 ├── USpiderCombatComponent        (3-hit combos, launchers, juggles, dodges)
 ├── USpiderAbilityComponent       (Web shot, burst, pull, wall pin)
 ├── USpiderHealthComponent        (Health, I-Frames, damage mitigation)
 ├── USpiderStaminaComponent       (Stamina pool, consumption, recovery)
 ├── USpiderProgressionComponent   (XP, level curves, 4 skill trees)
 ├── USpiderAudioComponent         (Dynamic wind whoosh & combat audio)
 └── USpiderCameraComponent        (Speed FOV, dynamic lag, camera trauma)
```

---

## ⚙️ Unreal Engine Version & Prerequisites

* **Engine Version**: Unreal Engine 5.4 or 5.5
* **Build Configuration**: Development / Shipping (Win64)
* **Compiler**: Microsoft Visual Studio 2022 (v143 toolset) with C++ Game Development workload
* **Plugins Required**:
  - `EnhancedInput` (Enabled)
  - `Niagara` (Enabled)
  - `ChaosVehiclesPlugin` (Enabled)
  - `MotionWarping` (Enabled)
  - `ControlRig` (Enabled)

---

## 📁 Folder Structure

```
d:/spidy-game/
├── SpiderHero.uproject
├── Config/
│   ├── DefaultEngine.ini          (Trace channels, collision profiles, renderer settings)
│   ├── DefaultGame.ini            (Project metadata)
│   ├── DefaultInput.ini           (Enhanced Input developer settings)
│   └── DefaultEditor.ini          (Editor optimization settings)
└── Source/
    ├── SpiderHero.Target.cs
    ├── SpiderHeroEditor.Target.cs
    └── SpiderHero/
        ├── SpiderHero.Build.cs
        ├── SpiderHero.h / .cpp    (Primary module & custom log categories)
        ├── Abilities/             (Health, Stamina, and Web Ability components)
        ├── AI/                    (AI Controller, Base Enemy, Melee Grunt, Director)
        ├── Audio/                 (Dynamic velocity-reactive audio component)
        ├── Camera/                (Superhero camera with dynamic FOV, lag, trauma)
        ├── Character/             (SpiderHeroCharacter and SpiderHeroAnimInstance)
        ├── Civilians/             (Pooled pedestrian crowd simulation)
        ├── Combat/                (Combat component, Data assets, Interfaces)
        │   └── Interfaces/        (IDamageable, ICombatTarget, IWebAttachable)
        ├── Core/                  (GameMode, GameState, PlayerController, Types)
        ├── Missions/              (Mission manager, Data assets, Tutorial mission)
        ├── Movement/              (SpiderMovementComponent with custom physics)
        ├── Progression/           (Progression component & 4-tree skill matrix)
        ├── Save/                  (SpiderSaveGame binary serialization)
        ├── Traffic/               (Lightweight instanced traffic simulation)
        ├── Traversal/             (SpiderTraversalComponent with ledge raycasting)
        ├── UI/                    (HUD widget, Pause menu, Upgrade menu)
        ├── Utilities/             (Debug manager, Scalability, Test automation, VFX)
        ├── Web/                   (Targeting, Swing physics, Zip, Shooting, Projectiles)
        └── World/                 (HISM City generator, Day/Night, Weather, Cinematics)
```

---

## 🎮 Controls & Input Mapping

| Action | Keyboard / Mouse | Gamepad (Xbox / PS) |
|---|---|---|
| **Move** | W / A / S / D | Left Analog Stick |
| **Look** | Mouse Delta | Right Analog Stick |
| **Jump / Wall Jump / Point Launch** | Spacebar | Face Button Bottom (A / Cross) |
| **Sprint / Wall Run / Slide** | Left Shift | Right Trigger (RT / R2) |
| **Web Swing (Hold/Release)** | Right Mouse Button / Shift | Right Trigger (Hold/Release) |
| **Point Zip / Perch Zip** | E (Tap) | Left Trigger + Right Trigger |
| **Web Shoot** | F (Tap) | Right Shoulder (RB / R1) |
| **Light Attack / Combo** | Left Mouse Button | Face Button Left (X / Square) |
| **Heavy Launcher / Ground Slam** | Hold Left Mouse / C | Face Button Top (Y / Triangle) |
| **Dodge / Perfect Counter** | Left Ctrl | Face Button Right (B / Circle) |
| **Web Ability (Burst / Pull)** | Q | Left Shoulder (LB / L1) |
| **Skill Tree / Upgrades Menu** | Tab | View / Touchpad |
| **Pause Menu** | Escape | Menu / Options |

---

## 🕹️ Core Gameplay Systems

### 1. Movement & Traversal
- **Kinematic Physics**: Overrides `PhysCustom` to execute wall running, wall climbing, parkour clearance, high-speed ground sliding, and vertical dive-bombing.
- **Apex Float**: Gravity scales to `0.65x` when vertical velocity approaches zero at the apex of a jump.
- **Multi-Trace Ledge Detection**: Forward eye/chest/waist traces paired with top-down ledge sweeps accurately classify obstacle heights (`VaultLow`, `VaultHigh`, `Mantle`, `LedgeClimb`).

### 2. Web Mechanics & Pendulum Physics
- **View Cone Raycasting**: Analyzes geometry across concentric angular rings, scoring targets by distance, screen-center alignment, and elevation advantage.
- **Pendulum Swing Equation**: Implements dynamic rope constraint $L$ with net tension $T = U \cdot (mg \cos\theta + m\frac{v^2}{L})$ and tangential steering torque.
- **Release Momentum Boost**: Grants up to a 1.35x velocity amplification on upward swing releases.
- **Point Zip**: High-speed ballistic transit (3800 cm/s) with a timed jump release window granting an explosive launch boost (1850 cm/s).

### 3. Superhero Camera System
- **Contextual FOV**: Seamless blending between Base (90°), Sprint (98°), High-Speed Swing (110°), Dive (115°), and Combat (82°).
- **Procedural Trauma Shakes**: Quadratic trauma decay (`Trauma²`) powering directional impulses on light hits, heavy strikes, hard landings, and perfect dodges.
- **Roll Tilts**: Computes centrifugal roll angles and wall-run banking.

### 4. Melee & Web Combat System
- **3-Hit Combo Chain**: Buffer queuing with hit-stop micro-freezes.
- **Heavy Launcher**: Launches ground enemies airborne and carries the hero upward for aerial juggle combos.
- **Frame-Perfect Dodge**: Bullet-time slow motion, invulnerability window, and Spider-Sense acoustic/visual cues.
- **Web Abilities**: Web Shot (web gauge accumulation), Web Burst (360° radial bind), Web Pull (yank light enemies or web strike heavy foes), and Web Wall Pin (permanently pinning webbed targets to walls).

### 5. Enemy AI & Attack Token Director
- **Combat Token Manager**: Limits simultaneous attackers (Max 1–2) with timeouts and stagger revocation to eliminate enemy dogpiling.
- **Radial Combat Slots**: Inner ring (6 slots @ 380u) and outer ring (8 slots @ 700u) orbiting the player.
- **Perception-Driven**: Sight (2500u) and hearing (2000u) integration with StateTree / BehaviorTree tasks.

### 6. World Generation, Traffic & Civilians
- **HISM City Generator**: Renders dense skyscraper districts (Modern, Art Deco, Neo-Gothic) and rooftop superhero props (water towers, HVAC, satellite dishes) with minimal draw calls and Nanite compatibility.
- **Traffic Simulation**: Lightweight vehicle pooling across road grid networks with 4-way synchronized traffic lights.
- **Civilian Crowds**: Sidewalk pedestrians that cheer when the hero swings overhead, take photos, or flee during villain threats.

### 7. Missions & Progression
- **Tutorial Mission ("Rooftop Disturbance")**: Guides player through vantage traversal, web-shooting lookouts, melee combat, and squad leader finishers.
- **Dynamic Crime Events**: Spawns procedural rooftop ambushes and armed robberies around the player.
- **4-Tier Skill Tree**: 12 unlockable upgrades across Traversal, Combat, Web, and Survival disciplines.

### 8. UI, Audio & VFX
- **Minimalist Modern HUD**: Smooth animated health, stamina, XP, web cartridges, dynamic reticle tracking, and Spider-Sense threat halos.
- **Audio Subsystem**: Speed-reactive wind whoosh volume, wrist-shooter discharges, and layered combat impacts.
- **Niagara VFX Subsystem**: Web beam ribbons, impact bursts, ground slam shockwaves, and wet surface splashes.

### 9. Day/Night, Weather & Cinematics
- **Celestial Cycle**: Dynamic solar/lunar azimuth driving sky atmosphere, real-time Skylight captures, and automatic skyscraper window emissives.
- **Dynamic Weather**: Clear, Overcast, Rain, Storms, and Fog driving Global Material Parameter Collections for road puddles and roughness.
- **Cinematic Intro**: Bezier camera flight path descending from high skyline canyon to the hero perched on a rooftop before seamlessly transitioning to gameplay.

### 10. SaveGame, Scalability & Testing
- **Versioned Serialization**: Saves player transform, stats, level, unlocked skills, mission progression, and city crime rates.
- **Scalability Presets**: Low, Medium, High, and Ultra presets managing Lumen, Nanite, shadows, and crowd density.
- **Automated Test Suite**: Validates movement state transitions, swing physics math stability, damage routing, mission objective chaining, and SaveGame serialization.

---

## 🔨 Build Instructions

1. Open `SpiderHero.uproject` in Unreal Engine 5.4+.
2. If prompted, select **Generate Visual Studio Project Files**.
3. Open `SpiderHero.sln` in Visual Studio 2022.
4. Set Build Configuration to **Development Editor** / **Win64**.
5. Build Solution (`Ctrl + Shift + B`).
6. Launch the Editor from Visual Studio or double-click `SpiderHero.uproject`.
7. Press **Play in Editor (PIE)** to experience the full gameplay prototype.

---

## 🛠️ Developer Debug Commands

Open the in-game console (`~`) and enter:

* `ToggleDebugMovement` — Toggles on-screen movement state, ground speed, and wall-normal gizmos.
* `ToggleDebugWeb` — Visualizes web raycast view cone, candidate scores, and pendulum tension vectors.
* `ToggleDebugCombat` — Displays combo windows, hitboxes, and target lock-on markers.
* `ToggleDebugTraversal` — Shows ledge detection traces, obstacle classifications, and vault trajectories.
* `ToggleDebugAll` — Toggles the full diagnostic debug HUD overlay.

---

## ⚡ Performance & Scalability Notes

* **HISM Meshes**: Reusable modular building blocks share instance buffers, reducing thousands of potential draw calls to single-digit render batches.
* **Throttled Ticks**: Civilian crowds and traffic vehicles beyond 18,000 units are tick-throttled and visibility-culled.
* **Physics Substepping**: Enabled in `DefaultEngine.ini` (`bSubstepping=True`, `MaxSubstepDeltaTime=0.016667`) for rock-solid pendulum swing constraint solver stability.

---

## 🚀 Future Improvements & Art Pipeline

1. **Character Art Swap**: Replace the placeholder humanoid mannequin with custom skeletal meshes and blendspaces using the modular socket conventions (`Socket_Wrist_L`, `Socket_Wrist_R`, `Socket_Back`).
2. **Animation Motion Warping**: Bind custom root motion montages to `USpiderTraversalComponent` and `USpiderCombatComponent` using Unreal's Motion Warping plugin.
3. **World Partition Data Layers**: Partition city districts into streaming layers for even larger metropolitan world sizes.