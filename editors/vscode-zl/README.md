# zl language — VS Code extension

Syntax highlighting, **running**, **formatting**, and snippets for `.zl` files.

## Install

```bash
./install.sh
```

Then reload VS Code (Ctrl+Shift+P → "Developer: Reload Window").

That builds `zlfmt`, packages a `.vsix`, and installs it with
`code --install-extension`.

### Do not install it by copying the folder

Older versions of this README said to `cp -r` this directory into
`~/.vscode/extensions/`. **That does not work on current VS Code and silently
does nothing.** The editor keeps a manifest at
`~/.vscode/extensions/extensions.json` and loads only what is listed there; a
folder dropped alongside it is never scanned. `code --list-extensions` reads
the same manifest, so the extension is invisible to both — you get a `.zl`
file that stays "Plain Text" with no error anywhere to explain why.

`code --install-extension` is the thing that writes the manifest entry.
Verified on VS Code 1.132.1.

### Verify it loaded

```bash
code --list-extensions --show-versions | grep zl
```

Expect `zl.zl-lang@0.3.2`. In the editor, the status bar of an open `.zl`
file should read **zl**, not "Plain Text".

## Running a file

With a `.zl` file open, the ▷ button in the editor title bar runs it. Two
commands, both also in the palette under "zl:":

| Command | Keybinding | What it runs |
|---|---|---|
| **zl: Run file (interpreter)** | `Ctrl+F5` | `interp file.zl` — the reference engine, runs the whole language |
| **zl: Run file (compiled, LLVM backend)** | `Ctrl+Shift+F5` | `compilel file.zl` → `clang -O2 out.ll` → the executable |

Output goes to a terminal named **zl**, not to an output channel, because zl
programs read stdin — `examples/calc_repl.zl` is interactive and an output
channel is write-only.

The file is saved first (`zl.saveBeforeRun`, default on). Both engines read
from disk, so running an unsaved buffer would run stale bytes.

Binaries are found the same way the formatter is: the setting
(`zl.interpreterPath`, `zl.compilerPath`), then the workspace root, then
`PATH` (`interp` or `zl` for the interpreter). **The command runs from the
directory the binary was found in** when that is a workspace root — `import
<stdlib module>` resolves against the process's cwd, not against the source
file, which is exactly why the `zl` wrapper on `PATH` cds to the repo root
before exec'ing `./interp`.

The compiled path is the **unboxed subset**, not the whole language: `compilel
hello.zl` fails with "only user-defined functions can be called in an
expression". That is the backend's documented scope, not a broken integration
— when it rejects a file, use the interpreter. `bench/b2_arith.zl` runs
through it in 20 ms. Its `out.ll` lands in the workspace root (gitignored) and
the executable in `$TMPDIR`, so running a file never drops a binary into your
tree.

## Formatting

The extension registers a document formatter, so **Format Document**
(Shift+Alt+F), format-on-save, and Format Selection all work on `.zl`.

It shells out to `zlfmt`, built by `build.sh` at the repo root. The extension
finds it in this order:

1. the `zl.formatterPath` setting
2. `./zlfmt` at the root of any open workspace folder
3. `zlfmt` on `PATH`

To format on save, add to your VS Code settings:

```json
"zl.formatterPath": "/path/to/zl-linux/zlfmt",
"[zl]": {
    "editor.formatOnSave": true,
    "editor.defaultFormatter": "zl.zl-lang"
}
```

### What the formatter does, and what it deliberately does not

`zlfmt` is a **re-indenter**, not a pretty-printer — Option C from
`docs/design/design_tooling.md` §3. It rewrites each line's leading
whitespace and strips trailing whitespace. Every other byte is copied through
from the original buffer, untouched.

- **does:** fix indentation to a consistent width, including mixed 2/4-space
  files, and nest blocks correctly
- **does not:** wrap long lines, put spaces around operators, align `=`, or
  reorder anything

That restraint is the point rather than a shortcut. zl's lexer discards
comments entirely (`lexer.c:272-273`) and truncates any token's text at 128
bytes (`lexer.c:88`), so a formatter that *rebuilt* source from the token
stream or the AST would delete every comment in the corpus and silently
corrupt any string literal of 128 characters or more. Copying the bytes makes
both classes of damage impossible rather than merely unlikely.

`verify_fmt.sh` at the repo root proves it on all 133 `.zl` files: the output
lexes, is idempotent, has identical non-whitespace bytes, and produces a
**byte-identical token stream including line numbers** — meaning the parser
cannot distinguish the file before and after.

The formatter is a separate process on purpose. It uses the real `lexer.c` to
decide which braces are code and which are inside a string or a comment;
reimplementing that in JavaScript would create a second definition of zl's
lexical grammar, free to drift from the first.

**If a buffer does not lex, nothing is formatted and the buffer is left
exactly as it was** — the error goes to the "zl" output channel and the
status bar. That is the normal case while you are mid-edit.

## What it highlights

| Element | Example | Scope (how your theme colours it) |
|---|---|---|
| Comments | `# note` | `comment.line` |
| Keywords | `if elif else while for return break continue fn import` | `keyword.control` / `storage.type.function` |
| Word operators | `and or not in` | `keyword.operator.word` |
| Constants | `true false` | `constant.language` |
| Strings | `"hi"` with `\n \t \xNN \0` escapes | `string.quoted.double` |
| F-strings | `f"x is {v}"` — the `{v}` is highlighted as code | `string.interpolated` + `meta.interpolation` |
| Numbers | `42  3.14  0xFF  1e6` | `constant.numeric` |
| Builtins | all 104, `print len push sort sqrt pi …` | `support.function.builtin` |
| Function defs | `fn greet(...)` — name highlighted | `entity.name.function` |
| Function calls | `greet(...)` | `entity.name.function.call` |
| Operators | `+ - * / % == != <= += ? :` | `keyword.operator` |

The keyword list is exactly the 16 in `lexer.c:48-55`. Earlier versions of
this grammar also coloured `do`, `loop`, `to`, `step` and `nil`, none of which
are zl keywords — all five are ordinary identifiers and `x = nil` is a runtime
error, not a constant. `nil` *is* a builtin **function**: `nil()` returns nil.
`import` is keyword #16 and was missing. 15 builtins were missing too.

## Snippets

`fn`, `if`, `ifelse`, `elif`, `for`, `forr`, `while`, `print`, `fs`
(f-string), and `check` (the assertion helper that ten `tests/*.zl` files
each define by hand).

## Completions

Ctrl+Space (and as-you-type) offers keywords, `true`/`false`, and all builtins
with `name()` snippet insert. Static list in `extension.js`, kept in sync with
the TextMate grammar — not an LSP. No go-to-definition, no errors-as-you-type.

## Still missing

- **An LSP** for go-to-definition and errors-as-you-type. Blocked on the parser
  gaining error recovery — `parse_error` calls `exit(1)` today
  (`parser.c:67-72`), and on `Node` carrying a line number, which it does not
  (`parser.h:33-42`). See `docs/design/design_tooling.md` §5.
- **Line wrapping / operator spacing.** Needs the comment-aware pretty-printer
  (Option B, 550-700 lines) and a `T_COMMENT` token in the lexer.
