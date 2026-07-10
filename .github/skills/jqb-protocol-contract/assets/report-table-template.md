# Report Table Template

| Report ID | Size | Direction | Field Offsets | Notes |
|---|---|---|---|---|
| 0x01 | 1 | IN | 0: status | keep for host heartbeat |
| 0x02 | 12 | GET/SET | 0..11 config bytes | fixed-size settings |
| 0x08 | 4 | GET | 0:ver,1:keys,2:enc,3:caps | capability discovery |

## Compatibility Rules
- Never reuse a retired report ID for a different payload.
- New optional features should be gated by capabilities bitmask.
- Unknown report IDs should return a safe error path, not crash.
