# Validation Matrix Template

| Check | Input | Condition | On Fail | Severity |
|---|---|---|---|---|
| config range | numeric fields | min <= value <= max | block export | error |
| machine bounds | planned coords | within workspace | block export | error |
| stage completeness | pipeline outputs | non-empty required sets | block export | error |
| output syntax | generated text | required headers/footer present | block export | error |
| optimization health | plan stats | fallback accepted if optimization skipped | continue + warn | warn |

## Rule
Validation must happen before writing final artifact whenever possible.
