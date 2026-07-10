# Pipeline Stage Template

| Stage | Input | Output | Failure Modes | Logging |
|---|---|---|---|---|
| detect | source directory/files | typed source list | missing files | info/error |
| parse | source list | domain model | syntax/format errors | info/error |
| normalize | domain model | normalized model | invalid bounds | info/error |
| generate | normalized model + config | toolpaths/jobs | geometry failures | info/error |
| optimize | toolpaths/jobs | ordered plan | optimization fallback | info/warn |
| export | ordered plan + machine profile | output file | write/validation errors | info/error |

## Rule
Every stage should have clear input/output structs and should be independently testable.
