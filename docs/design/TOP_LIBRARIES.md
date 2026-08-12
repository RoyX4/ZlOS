# The top libraries of Python, C and C++ — and what gates a zl equivalent

Compiled 2026-08-03. ~200 of the most-used libraries across the three ecosystems, grouped by
domain, each tagged with the zl feature that gates a zl equivalent. Tags: **[FFI]** wrap the
existing C lib once zl has FFI (item 16) · **[bytes]** needs a real byte buffer (GAPS_REALWORLD 1.1)
· **[closures]** item 11 · **[async]** items 18/multiplexing · **[types]** generics/structs ·
**[pure]** writable in pure zl today or soon · **[hard]** genuinely large even with the spine.

**THE FINDING UP FRONT, and it reframes the whole ecosystem question:** a huge fraction of Python's
and C++'s "top libraries" are **thin wrappers around a C library.** requests→libcurl-ish,
cryptography→OpenSSL, pillow→libjpeg/libpng, numpy→BLAS/LAPACK, lxml→libxml2, psycopg2→libpq,
pyyaml→libyaml, sqlite3→SQLite. So the honest path to an ecosystem is NOT reimplementing 300
libraries — it is **FFI (item 16), after which zl binds the SAME C libraries Python binds.** The
work becomes writing `.zl` binding files, which is library code, not engine work.

---

## WEB & HTTP
- **libcurl** (C) — the HTTP engine behind half of everything. [FFI]
- **requests / urllib3 / httpx / aiohttp** (Py) — HTTP clients; mostly libcurl or raw sockets. [FFI][bytes][async]
- **OpenSSL / LibreSSL / mbedTLS / BoringSSL** (C) — TLS. Every HTTPS call needs one. [FFI][hard]
- **nghttp2** (C) — HTTP/2. [FFI]
- **Flask / Django / FastAPI / Starlette** (Py) — web frameworks. [async][types][bytes] + a lot of zl
- **uvicorn / gunicorn** (Py) — ASGI/WSGI servers. [async]
- **websockets / socket.io** — realtime. [bytes][async]
- **beautifulsoup4 / lxml / scrapy / selenium** (Py) — scraping; lxml→libxml2. [FFI]
zl gate: **sockets need FFI; bodies need byte buffers; many clients need multiplexing.** All three
are exactly the web-server blockers GAPS_REALWORLD found. This is the most FFI-gated domain.

## DATA & SCIENCE (the reason Python won)
- **numpy** (Py) — n-dim arrays; wraps BLAS/LAPACK/OpenBLAS. [FFI][hard][types]
- **pandas** (Py) — dataframes; on numpy. [hard]
- **scipy** (Py) — scientific computing; wraps Fortran/C. [FFI][hard]
- **BLAS / LAPACK / OpenBLAS / Eigen** (C/C++/Fortran) — linear algebra kernels. [FFI][hard]
- **matplotlib / seaborn / plotly** (Py) — plotting. [bytes] + a lot of zl (or [FFI] to a backend)
- **scikit-learn / xgboost / lightgbm** — classic ML. [FFI][hard]
- **tensorflow / pytorch / keras / jax** — deep learning; huge C++/CUDA cores. [FFI][hard]
zl gate: **the kernels are C/Fortran — FFI reaches them.** zl's own math stdlib (linalg, matrix,
stats, complex, bignum) is a real head-start for the PURE side. But numpy-class performance needs
typed unboxed arrays (the type system) AND FFI to BLAS. This is the hardest domain, and the one
where "as fast as C" actually gets tested on real workloads.

## DATABASES
- **SQLite** (C) — the world's most-deployed database, one file, embeddable. [FFI] — the FIRST db to bind
- **libpq / psycopg2** (C/Py) — PostgreSQL. [FFI]
- **MySQL/MariaDB client** (C) — [FFI]
- **hiredis** (C) — Redis client. [FFI]
- **SQLAlchemy** (Py) — ORM over the above. [types] + a lot of zl
- **pymongo / mongo-c-driver** — MongoDB. [FFI]
- **LevelDB / RocksDB** (C++) — embedded key-value. [FFI]
zl gate: **SQLite via FFI is the single highest-value database win** — one binding, and zl has real
persistent storage. A pure-zl kv-store is also cheap. [FFI] dominates.

## CRYPTO & SECURITY (ties to Aegis)
- **OpenSSL / libsodium / mbedTLS** (C) — the crypto foundations. [FFI][hard]
- **cryptography / pyjwt / bcrypt / passlib** (Py) — mostly OpenSSL wrappers. [FFI]
- **BoringSSL, wolfSSL** (C) — embedded TLS. [FFI]
zl status: zl has real **sha256** in pure zl (NIST-verified) — a genuine asset. md5/sha1/crc32/hmac
are cheap pure-zl adds. But AES/RSA/TLS want OpenSSL via FFI. [pure] for hashes, [FFI][hard] for TLS.

## IMAGES, MEDIA, GRAPHICS
- **libpng / libjpeg-turbo / libwebp / giflib** (C) — image codecs. [FFI]
- **pillow** (Py) — wraps those. [FFI]
- **FFmpeg** (C) — the video/audio swiss army knife. [FFI][hard]
- **SDL2 / SFML** (C/C++) — games, windows, input, audio. [FFI]
- **OpenGL / Vulkan / DirectX** — GPU. [FFI][hard]
- **Dear ImGui** (C++) — immediate-mode GUI. [FFI]
- **freetype** (C) — font rendering. [FFI]
- **stb_image / stb_*** (C) — single-header everything; EASIEST to bind or even port. [FFI][pure]
zl status: has **bmp** in pure zl. png/jpg = [FFI] to libpng, OR port stb_image (single-header, a
real pure-zl porting candidate). SDL2 via FFI would give windows+input+audio+games in one binding.

## CONCURRENCY & ASYNC I/O
- **libuv** (C) — the async I/O core behind Node.js. [FFI][async]
- **libevent / libev** (C) — event loops. [FFI]
- **Boost.Asio / asio** (C++) — async networking. [FFI][async]
- **Intel TBB / OpenMP** — parallelism. [FFI]
- **asyncio / trio / gevent** (Py) — async frameworks. [async]
- **celery / kombu** (Py) — distributed task queues. [async] + a lot of zl
zl gate: **item 18 (threads) + I/O multiplexing** — the two-part concurrency gap GAPS_REALWORLD
sharpened. libuv via FFI is the pragmatic route to an event loop.

## SERIALIZATION & CONFIG
- **nlohmann/json / RapidJSON / jansson / cJSON** (C/C++) — JSON. [pure] — zl HAS json (but it's BROKEN, fix first)
- **protobuf / flatbuffers / cap'n proto** — schema RPC formats. [types][bytes][FFI]
- **libyaml / pyyaml** — YAML. [FFI] or [pure]
- **toml / tomllib** — TOML. [pure] — cheap, high-value (build configs)
- **msgpack / cbor** — binary serialization. [bytes]
- **pydantic** (Py) — validation/parsing. [types] — maps onto zl's planned type system nicely
zl status: JSON exists but broken (fix per STDLIB_PLAN); TOML/YAML/msgpack are [pure]/[bytes] adds.

## CLI, TERMINAL, TEXT
- **click / typer / argparse** (Py) — CLI parsing. [pure] — but needs `args` builtin (item 8) first!
- **rich / colorama / termcolor** (Py) — terminal styling. [pure] — zl has ansi, needs VT-mode fix
- **ncurses / termbox** (C) — full TUI. [FFI] or [pure] with raw-mode + kbhit (GAPS_REALWORLD win)
- **tqdm** (Py) — progress bars. [pure] — needs flush() (GAPS_REALWORLD gap)
- **PCRE2 / RE2 / Oniguruma** (C) — real regex engines. [FFI] — zl's regex_match is basic
- **fmt / spdlog** (C++) — formatting + logging. [pure] — logging is a cheap high-value pure-zl add
- **jinja2 / mustache** — templating. [pure] — zl has template_engine
zl status: mostly [pure], gated on small builtins (args, flush, kbhit) — the cheap cluster.

## SYSTEM, PROCESS, FILES
- **psutil** (Py) — process/system info. [FFI]
- **watchdog** (Py) — file watching. [FFI]
- **paramiko / libssh2** — SSH. [FFI]
- **libgit2** (C) — git operations. [FFI]
- **glib** (C) — portable system utilities. [FFI]
- **boost::filesystem / std::filesystem** — file ops. [FFI] (or pure via OS calls)
zl gate: [FFI] for almost all — this is the "control the PC" block (roadmap 50–66).

## TESTING & DEV TOOLS
- **pytest / unittest / mock** (Py) — [pure] — zl has testlib, needs assert-that-errors (item 5)
- **GoogleTest / Catch2 / doctest** (C++) — [pure]
- **coverage / gcov** — coverage. [hard-ish] — needs interpreter instrumentation
- **valgrind / ASan** — memory checking. [FFI]/[hard] — relevant to zl's own leak problem
- **gdb / lldb** — debuggers. [hard]
zl status: testlib exists; a coverage tool and a debugger are on the roadmap (items 30, 24).

## EMBEDDABLE / LANGUAGE INFRA (zl is one of these)
- **Lua** (C) — the embeddable scripting language; the model zl's interpreter resembles. [reference]
- **LLVM** (C++) — compiler infra; **zl already uses it.** [using]
- **pybind11 / cffi / ctypes** (Py) — FFI machinery. [reference] — this IS zl's item 16
- **tree-sitter** (C) — incremental parsing; relevant to a zl LSP. [FFI]/[reference]

---

## THE THREE CONCLUSIONS

**1. Most top libraries are C libraries with a wrapper.** OpenSSL, SQLite, libcurl, libpng, zlib,
libxml2, BLAS, libuv, PCRE, ffmpeg, freetype, SDL. Python and C++ did not reimplement these — they
wrapped them. **zl + FFI can wrap the identical libraries.** The ecosystem question is therefore
mostly the FFI question. Ship item 16 and the world's C libraries become zl-reachable.

**2. The rest cluster onto zl's known spine.** Everything not FFI-gated needs one of: byte buffers
(serialization, media, net bodies), closures (data pipelines, functional), the type system
(numpy-class arrays, pydantic-style validation, ORMs), or async (servers, task queues). No new
unlock appears that the roadmap didn't already have — this survey CONFIRMS the spine rather than
extending it.

**3. The cheap pure-zl wins are concentrated in CLI/text/config.** logging, TOML, more hashes
(md5/sha1/crc32), progress bars, better regex, argparse-that-works — all [pure], all gated only on
small builtins (`args`, `flush`, `kbhit`) rather than big features. A "batteries" pass here would
make zl feel complete for scripting long before FFI lands.

**Ranked ecosystem unlocks, by how many top-library domains each opens:**
1. **FFI (item 16)** — web, databases, crypto/TLS, media, system, SSH, async cores. The overwhelming winner.
2. **Byte buffers (GAPS_REALWORLD 1.1)** — serialization, media, network bodies, binary formats.
3. **The type system** — numpy-class arrays, validation, ORMs, typed collections.
4. **Closures (item 11)** — data pipelines, functional libraries, callbacks.
5. **Async + threads (item 18)** — servers, task queues, event loops.
6. **The cheap builtins (`args`/`flush`/`kbhit`/`sleep`)** — the entire CLI/TUI/scripting tier.

The "top 300 libraries" reduce, once more, to the SAME small set of unlocks — with **FFI now clearly
the single highest-leverage one**, because it turns "reimplement the world" into "write binding files."
