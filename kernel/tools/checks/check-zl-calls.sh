#!/bin/bash
# check-zl-calls.sh - every builtin kernel.zl CALLS must be REGISTERED in
# freestanding/runtime_kernel.c.
#
# zl has no compile-time check that a call site resolves. A kernel.zl hunk that
# survives a merge while the builtin behind it does not is a RUNTIME fault, and
# the merge that produced it is clean, links, and boots right up until the code
# path runs. That is not hypothetical: desktop/overnight-compositor removed the
# bg_snap/bg_rest/grab/stamp builtins while other branches still called them,
# and desktop/exec-track and main disagreed about whether `wm_focus` was the
# getter or the setter - a one-argument call to a zero-argument builtin, which
# in this language is silence.
#
# Reads source and does arithmetic. No build, no QEMU, so it cannot fail
# because the host is busy.
set -u
cd "$(dirname "$0")/../.." || exit 2
ZL=src/kernel.zl
RT=../freestanding/runtime_kernel.c
[ -f "$ZL" ] || { echo "no $ZL"; exit 2; }
[ -f "$RT" ] || { echo "no $RT"; exit 2; }

# EVERY MODULE kernel.zl ACTUALLY `import`s, not just kernel.zl itself.
# parser.c splices an imported module's top level into the program at parse
# time (docs/design/design_imports.md), so the real compiled program is
# kernel.zl PLUS every "./<name>.zl" its import lines name - the app suite
# (apps_registry.zl, apps_games1.zl, ...) lives there, not in kernel.zl. A
# checker that only read kernel.zl would report every fn in those modules as
# "unresolved" the moment kernel.zl called one, which is a false alarm this
# script must not raise. Same resolution rule the parser uses: ./<name>.zl.
ZLFILES="$ZL"
for m in $(grep -oP '^import\s+\K[a-zA-Z0-9_, ]+' "$ZL" | tr ',' '\n' | tr -d ' '); do
    [ -f "apps/$m.zl" ] && ZLFILES="$ZLFILES apps/$m.zl"
done

# builtins the runtime registers
grep -oE 'streq\(name, "[a-z_0-9]+"\)' "$RT" | sed 's/.*"\(.*\)".*/\1/' | sort -u > /tmp/zl_reg.$$

# zl's own functions, and the language's keywords/builtins that are not
# registered by name in the runtime
grep -hoE '^fn [a-z_0-9]+' $ZLFILES | awk '{print $2}' | sort -u > /tmp/zl_fns.$$
cat > /tmp/zl_kw.$$ <<'KW'
if
elif
else
while
for
in
fn
return
break
continue
not
and
or
import
true
false
print
put
len
KW
sort -u -o /tmp/zl_kw.$$ /tmp/zl_kw.$$

# Call sites. STRINGS AND COMMENTS COME OUT FIRST: prose is full of words
# followed by a bracket - "a bootloader (512 bytes)" reads as a call to
# bootloader() otherwise, and this check is worthless the moment it cries wolf.
sed 's/"[^"]*"/""/g; s/#.*//' $ZLFILES \
  | grep -oE '\b[a-z_][a-z_0-9]*[ ]*\(' \
  | tr -d ' (' | sort -u > /tmp/zl_calls.$$

# Empty on purpose. `key()` on the panel-handover path used to live here as a
# tracked hole; it now waits on in_char() like every other blocking read.
# A new unresolved call is a FAIL, not a footnote.
cat > /tmp/zl_known.$$ <<'KNOWN'
KNOWN

missing=$(comm -23 /tmp/zl_calls.$$ <(cat /tmp/zl_reg.$$ /tmp/zl_fns.$$ /tmp/zl_kw.$$ /tmp/zl_known.$$ | sort -u))
known_hit=$(comm -12 /tmp/zl_calls.$$ <(sort -u /tmp/zl_known.$$))

# and the other direction, which is only ever informational: a builtin nothing
# calls is dead weight, not a fault
unused=$(comm -13 /tmp/zl_calls.$$ /tmp/zl_reg.$$)

rm -f /tmp/zl_reg.$$ /tmp/zl_fns.$$ /tmp/zl_kw.$$ /tmp/zl_calls.$$ /tmp/zl_known.$$

if [ -n "$missing" ]; then
    echo "UNRESOLVED - kernel.zl (+ its imports) call these and nothing defines them:"
    echo "$missing" | sed 's/^/    /'
    echo
    echo "FAIL: $(echo "$missing" | grep -c .) unresolved call site(s)"
    exit 1
fi

echo "ok: every call in kernel.zl and its imports ($ZLFILES) resolves to a builtin or a zl fn"
[ -n "$known_hit" ] && echo "KNOWN UNRESOLVED (pre-existing, see the header): $(echo "$known_hit" | tr '\n' ' ')"
[ -n "$unused" ] && echo "note: $(echo "$unused" | grep -c .) registered builtin(s) with no caller"
exit 0
