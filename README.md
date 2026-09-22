# ZWSuite — Plugin Technical Documentation

Author: Lee Min-chae (Hermes) — 2026-09-22
Source: `/opt/data/projectx/ZWSuite-src/` (reconstructed from the Perforce workspace `//Felix/Felix.Main/Felix/Plugins/ZW`; missing plugins ZWStateTree / ZWUIInputBridge pulled from the GitHub repo tree)
Documentation scope: 15 plugins, 15 markdown files.

## Plugin index

| # | Plugin | File | Classes | Modules |
|---|--------|------|---------|---------|
| 1 | ZWCore | `ZWCore.md` | 3 | 1 |
| 2 | ZWDialogueSystem | `ZWDialogueSystem.md` | 22 | 5 |
| 3 | ZWGASExtensions | `ZWGASExtensions.md` | 2 | 1 |
| 4 | ZWGameplayActions | `ZWGameplayActions.md` | 6 | 2 |
| 5 | ZWInput | `ZWInput.md` | 3 | 1 |
| 6 | ZWInputStateTree | `ZWInputStateTree.md` | 5 | 1 |
| 7 | ZWStateTree | `ZWStateTree.md` | 4 | 1 |
| 8 | ZWInteraction | `ZWInteraction.md` | 9 | 1 |
| 9 | ZWInventory | `ZWInventory.md` | 25 | 2 |
| 10 | ZWQuestFactBase | `ZWQuestFactBase.md` | 13 | 2 |
| 11 | ZWScatteringTool | `ZWScatteringTool.md` | 11 | 1 |
| 12 | ZWUICore | `ZWUICore.md` | 10 | 1 |
| 13 | ZWUIHUD | `ZWUIHUD.md` | 4 | 1 |
| 14 | ZWUIInputBridge | `ZWUIInputBridge.md` | 1 | 1 |
| 15 | ZWUIStateTree | `ZWUIStateTree.md` | 11 | 1 |

Every document follows a consistent 8-section structure: Overview / Metadata (.uplugin) / Modules (Build.cs) / Public API / Implementation (Private) / Configuration (.ini) / Dependencies within ZWSuite / Notes & Risks.

## Dependency map within the suite (summary)

- **ZWCore** — lowest level: `UZWInputConfig` data asset (InputAction→InputTag mapping). Consumed by ZWInput.
- **ZWInput** → ZWCore, EnhancedInput; tag-driven input binding.
- **ZWStateTree** — abstract `UZWStateTreeSubsystemBase` foundation (LocalPlayerSubsystem + FTickableGameObject, pure-virtual asset/context hooks). Intended base for ZWInputStateTree, but not yet wired up there.
- **ZWInputStateTree** → ZWInput (+ StateTree), ZWGameplayActions; state-machine-driven input.
- **ZWGASExtensions** → ZWInput; bridge between input tags and GAS (`HandleInputTag` → `TryActivateAbilitiesByTag`).
- **ZWGameplayActions** → EnhancedInput; asset actions + `UZWActionManagerComponent` (note: no replication).
- **ZWInteraction** → interaction detection (sphere-trace, scene capture), investigation FSM.
- **ZWInventory** → ZWInteraction; item system, FastArray replication, SaveGame.
- **ZWScatteringTool** → ZWInventory, ZWInteraction; scatterer/probe (loot + enemy spawns).
- **ZWUICore** → ZWCore; panel routing, root-layout layers.
- **ZWUIHUD** → ZWUICore; HUD elements.
- **ZWUIInputBridge** → ZWInput, ZWUICore; runtime-only startup copy of input config into UI settings.
- **ZWUIStateTree** → ZWUICore (+ StateTree); state-machine UI control.
- **ZWQuestFactBase** — quest-fact registry, runtime subsystem + editor tree.
- **ZWDialogueSystem** — largest plugin; dialogues, choice system, audio generator, MovieScene dialogue track.