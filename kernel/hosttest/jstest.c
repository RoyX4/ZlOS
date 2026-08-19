/* jstest.c - the bounded JavaScript interpreter.
 *
 * The SCOPE CLAIM in js.h is as much under test as the code. This asserts what
 * js.h says it runs, and - section 7 - that what it cannot do fails cleanly
 * with a message rather than doing something plausible and wrong. A parser
 * that accepts garbage and evaluates it to 0 is worse than one that refuses.
 *
 * No kernel, no boot, no DOM: js.c is a string in and a string out.
 */
#include <stdio.h>
#include <string.h>
#include "../js.h"

static int checks, fails;

static void expr(const char *src, const char *want)
{
    checks++;
    if (js_eval(src, (int)strlen(src)) != 0) {
        fails++;
        printf("  FAIL %-46s error: %s\n", src, js_error());
        return;
    }
    int n;
    const char *got = js_result(&n);
    if (strcmp(got, want)) {
        fails++;
        printf("  FAIL %-46s got '%s' want '%s'\n", src, got, want);
    }
}

static void outp(const char *what, const char *src, const char *want)
{
    checks++;
    if (js_eval(src, (int)strlen(src)) != 0) {
        fails++;
        printf("  FAIL %-46s error: %s\n", what, js_error());
        return;
    }
    int n;
    const char *got = js_output(&n);
    if (strcmp(got, want)) {
        fails++;
        printf("  FAIL %-46s output '%s' want '%s'\n", what, got, want);
    }
}

static void refuses(const char *what, const char *src)
{
    checks++;
    int r = js_eval(src, (int)strlen(src));
    if (r == 0) { fails++; printf("  FAIL %s: accepted something it cannot do\n", what); }
    else if (!js_error()[0]) { fails++; printf("  FAIL %s: failed with no message\n", what); }
}

int main(void)
{
    printf("jstest: the bounded JavaScript interpreter\n\n1. expressions\n");
    expr("1 + 2 * 3", "7");
    expr("(1 + 2) * 3", "9");
    expr("10 / 4", "2.5");
    expr("7 % 3", "1");
    expr("-5 + 2", "-3");
    expr("2 < 3", "true");
    expr("3 <= 3", "true");
    expr("1 == 1", "true");
    expr("1 != 2", "true");
    expr("!0", "true");
    expr("1 && 2", "2");
    expr("0 || 5", "5");
    expr("true ? 'y' : 'n'", "y");
    expr("'a' + 'b'", "ab");
    expr("'n=' + 42", "n=42");
    expr("'5' * 2", "10");

    printf("\n2. variables and control flow\n");
    expr("var x = 5; x * 2", "10");
    expr("let a = 1; a += 4; a", "5");
    expr("var i = 0; while (i < 5) { i = i + 1; } i", "5");
    expr("var s = 0; for (var i = 1; i <= 10; i = i + 1) { s = s + i; } s", "55");
    expr("var s = 0; for (var i = 0; i < 10; i++) { if (i % 2 == 0) continue; s += i; } s", "25");
    expr("var s = 0; for (var i = 0; i < 100; i++) { if (i > 4) break; s += i; } s", "10");
    expr("var x = 3; if (x > 2) { x = 100; } else { x = 0; } x", "100");
    expr("var n = 0; n++; n++; n", "2");
    expr("var n = 5; --n", "4");

    printf("\n3. functions, including recursion\n");
    expr("function sq(x) { return x * x; } sq(7)", "49");
    expr("function add(a, b) { return a + b; } add(3, 4)", "7");
    /* the two that prove scopes are FRAMES rather than one flat table - a
     * flat table gets fact(10) wrong because the recursive call overwrites n */
    expr("function fact(n) { if (n <= 1) return 1; return n * fact(n - 1); } fact(10)",
         "3628800");
    expr("function fib(n) { if (n < 2) return n; return fib(n-1) + fib(n-2); } fib(18)",
         "2584");
    expr("var r = twice(21); function twice(x) { return x + x; } r", "42");

    printf("\n4. arrays and strings\n");
    expr("var a = [1,2,3]; a[1]", "2");
    expr("var a = [1,2,3]; a.length", "3");
    expr("var a = [1,2,3]; a[0] = 9; a[0]", "9");
    expr("var a = []; a.push(5); a.push(6); a.length", "2");
    expr("var a = [1,2,3]; var s = 0; for (var i=0;i<a.length;i++) s += a[i]; s", "6");
    expr("'hello'.length", "5");
    expr("'hello'.charAt(1)", "e");
    expr("'abc'[2]", "c");
    expr("'a' == 'a'", "true");
    expr("'a' == 'b'", "false");

    printf("\n5. output\n");
    outp("console.log", "console.log('hi')", "hi\n");
    outp("console.log, two args", "console.log(1, 2)", "1 2\n");
    outp("document.write", "document.write('<b>x</b>')", "<b>x</b>");
    outp("write in a loop", "for (var i=1;i<=3;i++) document.write(i)", "123");
    outp("an array prints as a list", "var a=[1,2]; console.log(a)", "1,2\n");

    printf("\n6. a script a document might actually carry\n");
    outp("zero-padded sequence",
         "function fmt(n) { return n < 10 ? '0' + n : '' + n; }\n"
         "var out = '';\n"
         "for (var i = 8; i <= 11; i++) { out += fmt(i) + ' '; }\n"
         "document.write(out);",
         "08 09 10 11 ");

    printf("\n7. it refuses rather than guesses\n");
    refuses("an unterminated block", "if (1) {");
    refuses("calling a non-function", "var x = 5; x();");
    refuses("a runaway loop is stopped", "while (true) { }");
    refuses("string arena exhaustion is an error, not corruption",
            "var s = 'x'; for (var i = 0; i < 100000; i++) { s += 'yyyyyyyyyy'; } s");
    {
        /* deep nesting must hit the depth ceiling rather than the guard page.
         * This kernel's compositor already overflowed a stack once. */
        char deep[1024];
        int n = 0;
        for (int i = 0; i < 400; i++) deep[n++] = '(';
        deep[n++] = '1';
        for (int i = 0; i < 400; i++) deep[n++] = ')';
        deep[n] = 0;
        refuses("deep nesting is refused, not crashed into", deep);
    }

    printf("\n8. the pools stayed inside themselves\n");
    checks++;
    if (js_nodes_used() > 4096 || js_vars_used() > 256 || js_arena_used() > 16384) {
        fails++;
        printf("  FAIL a pool overran: nodes=%d vars=%d arena=%d\n",
               js_nodes_used(), js_vars_used(), js_arena_used());
    }

    printf("\n%d checks, %d failed\n", checks, fails);
    return fails ? 1 : 0;
}
