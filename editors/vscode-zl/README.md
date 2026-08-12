# zl language — VS Code extension

Syntax highlighting for `.zl` files. Turns zl source from plain text into properly coloured code:
keywords, strings, f-strings with interpolation, numbers, the 93 builtins, function definitions and
calls, operators and comments all get distinct colours from whatever theme you use.

## What it highlights

| Element | Example | Scope (how your theme colours it) |
|---|---|---|
| Comments | `# note` | `comment.line` |
| Keywords | `if elif else while for do loop return break continue fn` | `keyword.control` / `storage.type.function` |
| Word operators | `and or not in to step` | `keyword.operator.word` |
| Constants | `true false nil` | `constant.language` |
| Strings | `"hi"` with `\n \t \xNN \0` escapes | `string.quoted.double` |
| F-strings | `f"x is {v}"` — the `{v}` is highlighted as code | `string.interpolated` + `meta.interpolation` |
| Numbers | `42  3.14  0xFF  1e6` | `constant.numeric` |
| Builtins | `print len push sort sqrt …` (all 93) | `support.function.builtin` |
| Function defs | `fn greet(...)` — name highlighted | `entity.name.function` |
| Function calls | `greet(...)` | `entity.name.function.call` |
| Operators | `+ - * / % == != <= += ? :` | `keyword.operator` |

## Install (development / local use)

VS Code loads any extension placed in its extensions folder — no build, no marketplace needed.

**Linux:**
1. Copy this whole `vscode-zl` folder into `~/.vscode/extensions/`
   (so you end up with `~/.vscode/extensions/vscode-zl/package.json`).
2. Restart VS Code.
3. Open any `.zl` file — it is coloured automatically.

From the repo root:

    cp -r editors/vscode-zl ~/.vscode/extensions/vscode-zl

**Windows:**
1. Copy this whole `vscode-zl` folder into `%USERPROFILE%\.vscode\extensions\`.
2. Restart VS Code.

Or from PowerShell, from the repo root:

    Copy-Item -Recurse editors\vscode-zl "$env:USERPROFILE\.vscode\extensions\vscode-zl"

Then reload VS Code (Ctrl+Shift+P → "Developer: Reload Window").

## Verify it loaded

Open a `.zl` file and check the bottom-right of the VS Code status bar — it should say **zl**, not
"Plain Text". If it says Plain Text, click it and pick "zl", or confirm the folder copied correctly.

## Next steps (not done yet — see docs/design/design_tooling.md)

This is syntax highlighting only. A fuller extension would add:
- A **formatter** (the parser already builds a full AST — a pretty-printer over it is the real work)
- An **LSP** for go-to-definition, errors-as-you-type, autocomplete — blocked on the parser gaining
  error recovery (it exits on the first error today)
- **Snippets** for `fn`, `if`, `for`, etc.
- **Bracket-pair colouring** (VS Code does this natively once the language is registered — already works)
