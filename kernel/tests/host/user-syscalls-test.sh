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
    check(ZLOS_U64_SYSCALL_LAST == 27ULL);
    check(U64_SYS_ANON_RESERVE == 22ULL);
    check(U64_SYS_ANON_COMMIT == 23ULL);
    check(U64_SYS_ANON_RELEASE == 24ULL);
    check(U64_SYS_SLEEP == 25ULL);
    check(U64_SYS_SPAWN == 26ULL);
    check(U64_SYS_WAIT == 27ULL);
    for (unsigned long long number = ZLOS_U64_SYSCALL_FIRST;
         number <= ZLOS_U64_SYSCALL_LAST; number++)
        check(zlos_u64_syscall_known(number));
    check(!zlos_u64_syscall_known(0ULL));
    check(!zlos_u64_syscall_known(ZLOS_U64_SYSCALL_LAST + 1ULL));
    check(!zlos_u64_syscall_known(1ULL << 63));
    check(!zlos_u64_syscall_known(~0ULL));
    if (failures) return 1;
    return checks == 40 ? 0 : 2;
}
C

${CC:-cc} -x c -std=c11 -Wall -Wextra -Werror -I. "$tmp" -o "$bin"
"$bin"
echo "user-syscalls host gate: 40 checks, 0 failed"

# Receipt metadata must name the same range and actual negative probe as the
# generated admission table. This catches stale evidence after ABI extension.
python3 - <<'PYTEST'
import importlib.util
import json
from pathlib import Path
path = Path("tools/checks/write-user-process-receipt.py")
spec = importlib.util.spec_from_file_location("process_receipt", path)
writer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(writer)
contract = json.loads(Path("src/arch/x86/user_syscalls.json").read_text())
numbers = [row["number"] for row in contract["entries"]]
recorded = writer.SOURCE_CONTRACTS["syscall_numbers"]
assert recorded["abi_version"] == contract["abi_version"]
assert (recorded["first"], recorded["last"]) == (min(numbers), max(numbers))
probe = next(row for row in writer.ASSERTIONS if row["id"] == "unknown-syscall-admission")
assert probe["probes"] == [0, max(numbers) + 1, 1 << 63, (1 << 64) - 1]
print("user-syscalls receipt range and unknown probe: PASS")
PYTEST
