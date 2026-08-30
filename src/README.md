# Host Language Source

This tree contains the zl host toolchain. Folder names describe ownership, not
completion state.

| Path | Owns |
|---|---|
| `frontend/` | Tokens, lexing, parsing, and the shared AST. |
| `runtime/` | Reference execution, boxed compiled-program support, and host OS calls. |
| `backends/c/` | C emitters. `compile.c` is the boxed backend; `compilef.c` is archived. |
| `backends/llvm/` | LLVM IR emission and its unboxed speed path. |
| `backends/native/` | Direct executable emitters and native runtime experiments. |
| `selfhost/` | The bounded zl-written bootstrap lane. It is not the production compiler. |
| `tools/` | Language-aware developer tools that share the frontend. |

`build.sh` is the authoritative host build entry point. It deliberately leaves
tool binaries at the repository root so existing commands such as `./interp`,
`./compile`, and `./zlfmt` remain stable.

Do not create `done/`, `half-done/`, or similar source folders. Completion is an
evidence claim and belongs in status ledgers; source folders identify ownership.
