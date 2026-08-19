# Where a Gen9 pixel shader comes from — answered, on this box

2026-08-19. `gpu-driver.md` closes with *"Settle where a pixel shader comes from
before committing weeks to RCS"*, and lists the blocker as **no Gen9 shader
assembler in this tree or on this box — no `intel_clc`, no `aubinator`, and
`INTEL_DEBUG=fs` silent.**

Two of those three are true. The third is not, and it was the one that mattered.

## `INTEL_DEBUG=fs` is not silent. The shader cache was eating it.

Reproduced three times in a row with one program, `scratch/fsdump.c` — a
surfaceless-EGL harness that compiles one fragment shader and draws one
triangle, so nothing appears on anyone's screen:

| run | environment | stderr |
|---|---|---|
| 1 | `INTEL_DEBUG=fs` (shader never seen before) | **82 lines** — NIR, then Gen9 assembly |
| 2 | `INTEL_DEBUG=fs` again, same shader | **0 lines** |
| 3 | `MESA_SHADER_CACHE_DISABLE=true INTEL_DEBUG=fs` | **82 lines** again |

`~/.cache/mesa_shader_cache` is 7.8 MB. On a hit, iris loads the compiled binary
and the compiler never runs, so there is nothing for `INTEL_DEBUG` to print. Any
attempt on an app that had been run before — which is every app on this
machine — sees exactly the silence recorded as a missing capability.

**The whole fix is one environment variable:**

```bash
MESA_SHADER_CACHE_DISABLE=true INTEL_DEBUG=fs,hex ./yourprogram
```

The box has had a working Gen9 shader compiler *and* disassembler the entire
time: Mesa 26.1.5, iris, reporting `Mesa Intel(R) UHD Graphics (CML GT2)` —
the target part, 8086:9B41, not an emulation of it.

## The shader zlOS needs, in full

The 48× measurement is about **alpha blending**, and blending on Gen9 is the
fixed-function output merger, not the shader. So the kernel is a constant-colour
write and the blend unit does `dst = src·a + dst·(1−a)` for free:

```glsl
#version 300 es
precision highp float;
uniform vec4 ucol;
out vec4 frag;
void main(){ frag = ucol; }
```

Compiled for this part, SIMD8, **five instructions, 48 bytes**:

```
01 0b 01 20 00 7b 02 00                          mov(8) g123<1>F g2<0,1,0>F    { align1 1Q compacted }
01 0b 15 20 00 7c 02 00                          mov(8) g124<1>F g2.1<0,1,0>F  { align1 1Q compacted }
01 0b 19 20 00 7d 02 00                          mov(8) g125<1>F g2.2<0,1,0>F  { align1 1Q compacted }
01 0b 1d 20 00 7e 02 00                          mov(8) g126<1>F g2.3<0,1,0>F  { align1 1Q compacted }
34 00 60 05 00 00 00 00 60 0f 00 00 00 14 03 88  sendsc(8) nullUD g123UD nullUD 0x08031400
        render MsgDesc: RT write SIMD8 LastRT Surface = 0  mlen 4 ex_mlen 0 rlen 0 { align1 1Q EOT }
```

Four moves of the push constant into the render-target write payload, then one
`send` with EOT. 4×8 compacted + 16 uncompacted = 48, which is what Mesa's own
line says: *"Compacted 80 to 48 bytes (40%)"*. A SIMD16 variant is in the same
dump, also five instructions.

**It was verified running, not just compiled.** The harness draws with
`glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)` and reads the centre pixel
back:

```
uniform  = (0.376, 0.824, 0.922, 0.5)  over black
predicted  30 69 76      (v · 0.5 · 255, per channel)
read back  30 69 76 40
```

Exact on all three channels. The five instructions plus the blend unit do the
operation the 48× is about, on the real part.

## What this does and does not settle

**Settled:** the kernel is not the hard part and never was. "No shader
assembler" was a false blocker, and the *specific* kernel zlOS wants is 48 bytes
that can be embedded as a byte array, with a reference disassembly to check any
hand-encoder against.

**Not settled, and this is now the real cost of RCS:** those 48 bytes run inside
a state vector zlOS does not have. `g2` holds push constants because Mesa
programmed `3DSTATE_CONSTANT_PS` to put them there; the `send` names render
target surface 0 because Mesa built a binding table and a `RENDER_SURFACE_STATE`
for it. Standing that up means `3DSTATE_PS`, `3DSTATE_PS_EXTRA`,
`3DSTATE_CONSTANT_PS`, `3DSTATE_BLEND_STATE_POINTERS`, binding tables, viewport
and scissor state, `3DSTATE_VF`/VS for the two triangles, and a `STATE_BASE_ADDRESS`
that makes all of it reachable. **That** is the weeks of work, not the shader.

So the question `gpu-driver.md` asked is answered, and the answer changes what to
weigh: RCS is expensive because of state setup, and any estimate that treated
"write a Gen9 shader" as the expensive part was weighing the wrong thing.

**Nothing here has run on zlOS.** These bytes were produced by Mesa and executed
by Mesa. That zlOS can submit them depends on the ring, which is milestone 2 and
still needs a hardware run. This document is about where a kernel comes from, and
that is all it claims.

## Reproducing it

`scratch/fsdump.c` is deliberately not in the tree — it needs `libEGL`/`libGLESv2`
and belongs to the host, not the kernel. It is 100 lines and the recipe is the
whole content of it: surfaceless EGL, an FBO, one constant-colour fragment
shader, `glDrawArrays`, `glReadPixels`. The compile happens at **draw** time in
iris, not at `glLinkProgram`, so a program that only links prints nothing and
looks like another silent run.

Related: [`gpu-driver.md`](gpu-driver.md) · [`gpu-blitter.md`](gpu-blitter.md) ·
[`intel-graphics-stack.md`](intel-graphics-stack.md)
