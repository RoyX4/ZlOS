# Gen9 3D pipeline for one alpha-blended quad — the packet recipe

Captured from the vendor driver on this exact silicon (`8086:9B41`), 2026-08-19.
**Not derived from a manual this tree does not have.**

```bash
cd kernel/hosttest
MESA_SHADER_CACHE_DISABLE=true INTEL_DEBUG=bat ./gpu_fillrate 2>batch.txt
```

The full decode is ~15 000 lines and 500 KB, so it is **not committed** — the
command above reproduces it in a few seconds. What is committed is the part a
person actually needs: the order, and the state that is not a default.

`MESA_SHADER_CACHE_DISABLE` matters here too — on a cache hit iris skips the
compile and the interesting state never appears. That one variable is what made
`INTEL_DEBUG` look broken for most of a day.

## Two shaders, two byte counts — not a contradiction

`gen9-shader-source.md` says the kernel is **5 instructions, 48 bytes**;
`gpu_shader.inc` here says **5 instructions, 80 bytes**. Both are right, and the
difference is the point:

- **48 bytes** comes from a shader that reads the colour from a **push
  constant** — supplied at draw time, nothing to patch.
- **80 bytes** comes from `gl_FragColor = vec4(<literal>)`, where the four
  channels are **float immediates inside the movs**. Mesa reports
  `Compacted 80 to 80 bytes (0%)` — an immediate cannot be compacted, which is
  exactly why it is bigger.

Same five instructions either way, because blending is the fixed-function output
merger and not the shader's job. The push-constant version is the better design
for a driver that fills in many colours; the immediate version is the simpler
one to embed and patch. Pick per use, and do not "fix" either to match the
other's byte count.

## Why this exists

`gpu-driver.md` had the render engine blocked on *"no Gen9 assembler"*. The
shader turned out to be liftable (`gpu_shader.inc`, 80 bytes). This is the other
half: the pipeline state around it. Deriving 76 packets from a PRM is how a
driver project stalls; reading a working one is not.

## The blend state zlOS has to match

```
Color Buffer Blend Enable: true
Source Blend Factor:       3  (SRC_ALPHA)
Destination Blend Factor: 19  (INV_SRC_ALPHA)
```

Ordinary source-over — the same operation `fb.c`'s `fill_blend` does at 122
Mpix/s and the GPU does at 5930.

## The 77 packets, in order

```
 1. PIPELINE_SELECT
      Media Sampler DOP Clock Gate Enable: false
 2. MI_LOAD_REGISTER_IMM
      SLM Enable: false
 3. PIPE_CONTROL
      Depth Cache Flush Enable: true
      State Cache Invalidation Enable: false
      Constant Cache Invalidation Enable: false
 4. STATE_BASE_ADDRESS
      General State Base Address Modify Enable: true
      Surface State Base Address Modify Enable: true
      Dynamic State Base Address Modify Enable: true
 5. PIPE_CONTROL
      Depth Cache Flush Enable: false
      State Cache Invalidation Enable: true
      Constant Cache Invalidation Enable: true
 6. MI_LOAD_REGISTER_IMM
 7. MI_LOAD_REGISTER_IMM
      Float Blend Optimization Enable: true
      NP PMA Fix Enable: 0
      Blend Optimization Fix Disable: false
 8. 3DSTATE_DRAWING_RECTANGLE
 9. 3DSTATE_SAMPLE_PATTERN
10. 3DSTATE_AA_LINE_PARAMETERS
11. 3DSTATE_WM_CHROMAKEY
      ChromaKey Kill Enable: false
12. 3DSTATE_WM_HZ_OP
      Number of Multisamples: 0
      Pixel Position Offset Enable: false
      Hierarchical Depth Buffer Resolve Enable: false
13. 3DSTATE_POLY_STIPPLE_OFFSET
14. 3DSTATE_PUSH_CONSTANT_ALLOC_VS
15. 3DSTATE_PUSH_CONSTANT_ALLOC_HS
16. 3DSTATE_PUSH_CONSTANT_ALLOC_DS
17. 3DSTATE_PUSH_CONSTANT_ALLOC_GS
18. 3DSTATE_PUSH_CONSTANT_ALLOC_PS
19. PIPE_CONTROL
      Depth Cache Flush Enable: false
      State Cache Invalidation Enable: false
      Constant Cache Invalidation Enable: false
20. MI_LOAD_REGISTER_IMM
21. PIPE_CONTROL
      Depth Cache Flush Enable: true
      State Cache Invalidation Enable: false
      Constant Cache Invalidation Enable: false
22. STATE_BASE_ADDRESS
      General State Base Address Modify Enable: false
      Surface State Base Address Modify Enable: true
      Dynamic State Base Address Modify Enable: false
23. PIPE_CONTROL
      Depth Cache Flush Enable: false
      State Cache Invalidation Enable: true
      Constant Cache Invalidation Enable: true
24. 3DSTATE_VIEWPORT_STATE_POINTERS_CC
25. 3DSTATE_VIEWPORT_STATE_POINTERS_SF_CLIP
26. 3DSTATE_URB_VS
27. 3DSTATE_URB_HS
28. 3DSTATE_URB_DS
29. 3DSTATE_URB_GS
30. 3DSTATE_BLEND_STATE_POINTERS
      Blend State Pointer Valid: true
      Blend State Pointer: 0xfffef080
      Color Dither Enable: true
31. 3DSTATE_CC_STATE_POINTERS
      Blend Constant Color Red: 0.000000
      Blend Constant Color Green: 0.000000
      Blend Constant Color Blue: 0.000000
32. 3DSTATE_CONSTANT_VS
33. 3DSTATE_CONSTANT_PS
34. 3DSTATE_BINDING_TABLE_POINTERS_VS
      Cube Face Enable - Positive Z: false
      Cube Face Enable - Negative Z: false
      Cube Face Enable - Positive Y: false
35. 3DSTATE_SAMPLER_STATE_POINTERS_VS
36. 3DSTATE_BINDING_TABLE_POINTERS_HS
      Cube Face Enable - Positive Z: false
      Cube Face Enable - Negative Z: false
      Cube Face Enable - Positive Y: false
37. 3DSTATE_BINDING_TABLE_POINTERS_DS
      Cube Face Enable - Positive Z: false
      Cube Face Enable - Negative Z: false
      Cube Face Enable - Positive Y: false
38. 3DSTATE_BINDING_TABLE_POINTERS_GS
      Cube Face Enable - Positive Z: false
      Cube Face Enable - Negative Z: false
      Cube Face Enable - Positive Y: false
39. 3DSTATE_BINDING_TABLE_POINTERS_PS
      Cube Face Enable - Positive Z: false
      Cube Face Enable - Negative Z: false
      Cube Face Enable - Positive Y: false
40. 3DSTATE_SAMPLER_STATE_POINTERS_PS
41. 3DSTATE_MULTISAMPLE
      Number of Multisamples: 0
      Pixel Position Offset Enable: false
42. 3DSTATE_SAMPLE_MASK
      Sample Mask: 65535
43. 3DSTATE_VS
      Kernel Start Pointer: 0xfffef1c0
      Software Exception Enable: false
      Illegal Opcode Exception Enable: false
44. 3DSTATE_HS
      Software Exception Enable: false
      Illegal Opcode Exception Enable: false
      Thread Dispatch Priority: 0
45. 3DSTATE_TE
      TE Enable: false
      Output Topology: 0 (POINT)
46. 3DSTATE_DS
      Kernel Start Pointer: 0x00000000
      Software Exception Enable: false
      Illegal Opcode Exception Enable: false
47. 3DSTATE_GS
      Kernel Start Pointer: 0x00000000
      Expected Vertex Count: 0
      Software Exception Enable: false
48. 3DSTATE_PS
      Kernel Start Pointer 0: 0xfffef000
      Software Exception Enable: false
      Mask Stack Exception Enable: false
49. 3DSTATE_PS_EXTRA
      Attribute Enable: false
50. 3DSTATE_STREAMOUT
      SO Statistics Enable: false
      SO Function Enable: false
      Buffer 0 Surface Pitch: 0
51. 3DSTATE_CLIP
      User Clip Distance Cull Test Enable Bitmask: 0
      Statistics Enable: true
      Force Clip Mode: false
52. 3DSTATE_RASTER
      Viewport Z Near Clip Test Enable: true
      Scissor Rectangle Enable: false
      Antialiasing Enable: false
53. 3DSTATE_SF
      Viewport Transform Enable: true
      Statistics Enable: true
      Legacy Global Depth Bias Enable: false
54. 3DSTATE_WM
      Force Kill Pixel Enable: 0
      Line Stipple Enable: false
      Polygon Stipple Enable: false
55. 3DSTATE_SBE
      Primitive ID Override Attribute Select: 0
      Primitive ID Override Component X: false
      Primitive ID Override Component Y: false
56. 3DSTATE_SBE_SWIZ
      Attribute Wrap Shortest Enables[0]: 0
      Attribute Wrap Shortest Enables[1]: 0
      Attribute Wrap Shortest Enables[2]: 0
57. 3DSTATE_PS_BLEND
      Independent Alpha Blend Enable: false
      Alpha Test Enable: false
      Destination Blend Factor: 19 (INV_SRC_ALPHA)
58. 3DSTATE_WM_DEPTH_STENCIL
      Depth Buffer Write Enable: false
      Depth Test Enable: false
      Stencil Buffer Write Enable: false
59. 3DSTATE_SCISSOR_STATE_POINTERS
60. 3DSTATE_DEPTH_BUFFER
      Surface Pitch: 7679
      Surface Format: 3 (D24_UNORM_X8_UINT)
      Hierarchical Depth Buffer Enable: true
61. 3DSTATE_STENCIL_BUFFER
      Surface Pitch: 0
      Stencil Buffer Enable: false
      Surface Base Address: 0x00000000
62. 3DSTATE_HIER_DEPTH_BUFFER
      Surface Pitch: 1919
      Surface Base Address: 0xfffffffefe6e8000
      Surface QPitch: 300
63. 3DSTATE_CLEAR_PARAMS
64. 3DSTATE_POLY_STIPPLE_PATTERN
65. 3DSTATE_LINE_STIPPLE
      Modify Enable (Current Repeat Counter, Current Stipple Index): false
66. 3DSTATE_VF_TOPOLOGY
      Primitive Topology Type: 7 (QUADLIST)
67. PIPE_CONTROL
      Depth Cache Flush Enable: false
      State Cache Invalidation Enable: false
      Constant Cache Invalidation Enable: false
68. PIPE_CONTROL
      Depth Cache Flush Enable: false
      State Cache Invalidation Enable: false
      Constant Cache Invalidation Enable: false
69. 3DSTATE_VERTEX_BUFFERS
      Buffer Pitch: 8
      Address Modify Enable: true
70. 3DSTATE_VERTEX_ELEMENTS
      Edge Flag Enable: false
71. 3DSTATE_VF_INSTANCING
      Instancing Enable: false
72. 3DSTATE_VF_SGVS
      VertexID Enable: false
      InstanceID Enable: false
73. 3DSTATE_VF_STATISTICS
      Statistics Enable: true
74. 3DSTATE_VF
      Indexed Draw Cut Index Enable: false
      Component Packing Enable: false
      Sequential Draw Cut Index Enable: false
75. PIPE_CONTROL
      Depth Cache Flush Enable: false
      State Cache Invalidation Enable: false
      Constant Cache Invalidation Enable: false
76. MI_LOAD_REGISTER_IMM
77. 3DPRIMITIVE
      Predicate Enable: false
      Indirect Parameter Enable: false
      Primitive Topology Type: 0
```

## Reading it

Packets 1–23 are one-time setup: pipeline select, `STATE_BASE_ADDRESS`, URB
allocation, push-constant allocation. Packets 24–66 are the per-draw state —
this is where the blend, the pixel shader pointer and the render target live.
67–76 are the vertex buffer and the draw itself.

A zlOS implementation does not need all 76. Many are disables (`3DSTATE_HS`,
`3DSTATE_DS`, `3DSTATE_GS`, `3DSTATE_STREAMOUT`) that still have to be *emitted*
as disabled, because the hardware keeps whatever the last batch left there —
which is the trap that makes a half-built pipeline draw nothing with no error.
