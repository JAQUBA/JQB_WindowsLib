# Settings Keys Template

| Key | Default | Scope | Notes |
|---|---|---|---|
| port | COM1 | connection | active COM port |
| baud | 9600 | connection | serial speed |
| auto_poll | 0 | monitoring | 1 enables periodic polling |
| poll_interval_ms | 250 | monitoring | polling group period |
| theme_name | catppuccin_mocha | ui | startup theme |
| last_output_path |  | export | last saved file path |

## Implementation Rule
- Every key listed here must be handled in both load() and save().
