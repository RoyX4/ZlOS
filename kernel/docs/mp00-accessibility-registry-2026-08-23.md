# MP-00 accessibility proof registry

`kernel/accessibility-registry.json` separates ordinary input/UI tests from
complete accessibility workflows. Four host primitives pass: keyboard parity,
focus lifecycle, visible focus and non-stealing notifications. Density scaling
and animations-off behavior are partial host proof.

Nine capabilities are absent, including semantic trees, screen-reader/speech/
braille providers, magnification, high contrast, captions, switch/dwell,
accessibility key policy, complete keyboard-only app workflows and localization/
RTL qualification. There are zero current-build-bound QEMU workflows and zero
complete target workflows.

The self-test rejects missing capability rows, invented semantic-tree proof,
hidden assistive gaps, target overclaim and unearned visual proof.
