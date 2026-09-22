# ZWSuite — Dokumentacja techniczna pluginów

Autor: Lee Min-chae (Hermes) — 2026-09-22
Źródło: `/opt/data/projectx/ZWSuite-src/` (zrekonstruowane z workspace P4 `//Felix/Felix.Main/Felix/Plugins/ZW`)
Moduły dokumentacji: 13 pluginów, ~356 KB markdown.

## Spis pluginów

| # | Plugin | Plik | Klasy | Moduły |
|---|--------|------|-------|--------|
| 1 | ZWCore | `ZWCore.md` | 3 | 1 |
| 2 | ZWDialogueSystem | `ZWDialogueSystem.md` | 22 | 5 |
| 3 | ZWGASExtensions | `ZWGASExtensions.md` | 2 | 1 |
| 4 | ZWGameplayActions | `ZWGameplayActions.md` | 6 | 2 |
| 5 | ZWInput | `ZWInput.md` | 3 | 1 |
| 6 | ZWInputStateTree | `ZWInputStateTree.md` | 5 | 1 |
| 7 | ZWInteraction | `ZWInteraction.md` | 9 | 1 |
| 8 | ZWInventory | `ZWInventory.md` | 25 | 2 |
| 9 | ZWQuestFactBase | `ZWQuestFactBase.md` | 13 | 2 |
| 10 | ZWScatteringTool | `ZWScatteringTool.md` | 11 | 1 |
| 11 | ZWUICore | `ZWUICore.md` | 10 | 1 |
| 12 | ZWUIHUD | `ZWUIHUD.md` | 4 | 1 |
| 13 | ZWUIStateTree | `ZWUIStateTree.md` | 11 | 1 |

Każdy dokument ma spójną strukturę (8 sekcji): Overview / Metadata (.uplugin) / Sub-modules (Build.cs) / Public API / Implementation (Private) / Configuration (.ini) / Dependencies within ZWSuite / Notes & risks.

## Zależności wewnątrz suite (skrót)
- **ZWCore** — najniższy poziom: data asset `UZWInputConfig` (InputAction→InputTag mapowanie). Używany przez ZWInput.
- **ZWInput** → ZWCore, EnhancedInput; tagiem steruje bindingi.
- **ZWInputStateTree** → ZWInput (+ StateTree), ZWGameplayActions; stan-to-maszyna inputu.
- **ZWGASExtensions** → ZWInput; most między tagami input a GAS (`HandleInputTag`→`TryActivateAbilitiesByTag`).
- **ZWGameplayActions** → EnhancedInput; asset actions + ActionManagerComponent (gotowe pero ryzykowne: brak replikacji).
- **ZWInteraction** → detekcja interakcji (sphere-trace, scene capture), FSM inspekcji; zależny od wspólnych tagów.
- **ZWInventory** → ZWInteraction; system przedmiotów, FastArray replikacja, SaveGame.
- **ZWScatteringTool** → ZWInventory, ZWInteraction; scatterer/probe (loot + enemies).
- **ZWUICore** → ZWCore; routing paneli, warstwy root layout.
- **ZWUIHUD** → ZWUICore; HUD.
- **ZWUIStateTree** → ZWUICore (+ StateTree); state-machine UI.
- **ZWQuestFactBase** → baza faktów questowych, subsystem + edytor.
- **ZWDialogueSystem** → największy; dialogi, choice system, audio generator, MovieScene track.