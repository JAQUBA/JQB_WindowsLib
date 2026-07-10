# Tab Layout Checklist

## Structure
- Add clear tab title and one-line intent summary.
- Split content into cards (connection, settings, actions, results/logs).
- Keep 8/16/24 spacing rhythm.

## Controls
- Labels: wide text (L"...").
- Inputs/selects/buttons: UTF-8 text.
- One primary action per card section.
- Use secondary buttons for non-critical actions.

## Wiring
- UI callback should call AppState method.
- AppState method handles validation and side effects.
- Long-running work must be delegated to worker thread.

## Observability
- Emit info/error logs via TextLogger.
- Show visible operation status near actions.
