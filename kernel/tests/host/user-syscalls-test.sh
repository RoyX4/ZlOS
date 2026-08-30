#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."

python3 tools/generators/gen-user-syscalls.py --check --selftest

tmp=$(mktemp --suffix=.c)
bin=$(mktemp)
trap 'rm -f "$tmp" "$bin"' EXIT
cat >"$tmp" <<'C'
#include "src/arch/x86/user_syscalls_generated.h"

static int checks;
static int failures;

static void check(int condition)
{
    checks++;
    if (!condition) failures++;
}

int main(void)
{
    check(ZLOS_U64_SYSCALL_ABI_VERSION == 1ULL);
    check(ZLOS_U64_SYSCALL_FIRST == 1ULL);
    check(ZLOS_U64_SYSCALL_LAST == 24ULL);
    check(U64_SYS_ANON_RESERVE == 22ULL);
    check(U64_SYS_ANON_COMMIT == 23ULL);
    check(U64_SYS_ANON_RELEASE == 24ULL);
    for (unsigned long long number = ZLOS_U64_SYSCALL_FIRST;
         number <= ZLOS_U64_SYSCALL_LAST; number++)
        check(zlos_u64_syscall_known(number));
    check(!zlos_u64_syscall_known(0ULL));
    check(!zlos_u64_syscall_known(ZLOS_U64_SYSCALL_LAST + 1ULL));
    check(!zlos_u64_syscall_known(1ULL << 63));
    check(!zlos_u64_syscall_known(~0ULL));
    if (failures) return 1;
    return checks == 34 ? 0 : 2;
}
C

${CC:-cc} -x c -std=c11 -Wall -Wextra -Werror -I. "$tmp" -o "$bin"
"$bin"
echo "user-syscalls host gate: 34 checks, 0 failed"
