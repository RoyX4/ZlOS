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
cd "$(dirname "$0")" || exit 2
ZL=kernel.zl
RT=../freestanding/runtime_kernel.c
[ -f "$ZL" ] || { echo "no $ZL"; exit 2; }
[ -f "$RT" ] || { echo "no $RT"; exit 2; }

# builtins the runtime registers
grep -oE 'streq\(name, "[a-z_0-9]+"\)' "$RT" | sed 's/.*"\(.*\)".*/\1/' | sort -u > /tmp/zl_reg.$$

# zl's own functions, and the language's keywords/builtins that are not
# registered by name in the runtime
grep -oE '^fn [a-z_0-9]+' "$ZL" | awk '{print $2}' | sort -u > /tmp/zl_fns.$$
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
sed 's/"[^"]*"/""/g; s/#.*//' "$ZL" \
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
    echo "UNRESOLVED - kernel.zl calls these and nothing defines them:"
    echo "$missing" | sed 's/^/    /'
    echo
    echo "FAIL: $(echo "$missing" | grep -c .) unresolved call site(s)"
    exit 1
fi

echo "ok: every kernel.zl call resolves to a builtin or a zl fn"
[ -n "$known_hit" ] && echo "KNOWN UNRESOLVED (pre-existing, see the header): $(echo "$known_hit" | tr '\n' ' ')"
[ -n "$unused" ] && echo "note: $(echo "$unused" | grep -c .) registered builtin(s) with no caller in kernel.zl"
exit 0
