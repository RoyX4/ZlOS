/* js.h - a JavaScript interpreter, bounded on purpose.
 *
 * WHAT THIS IS AND IS NOT, first, because "JavaScript" is the word in this
 * project most likely to be read as a claim it cannot support.
 *
 * A JS ENGINE FOR THE MODERN WEB IS UNBOUNDED and that judgement stands - it
 * is the one thing feature-catalogue.md called unbounded that really is. The
 * target moves, it is defined by whatever Chrome shipped last Tuesday, and it
 * drags in a DOM, an event loop, promises, generators, proxies, prototypes,
 * getters, regular expressions, Unicode, a garbage collector and a JIT. There
 * is no version of this file that runs YouTube.
 *
 * A JS INTERPRETER IS NOT UNBOUNDED. A tokenizer, a precedence-climbing parser
 * and a tree-walking evaluator over a fixed value pool is a well-understood
 * shape with a finish line, and it is the shape interp_kernel.c in this same
 * kernel already has for zl. THAT is what this is: it runs the kind of script a
 * document actually carries.
 *
 * WHAT IT RUNS
 *   numbers, strings, booleans, null, undefined
 *   var/let/const, assignment and compound assignment
 *   + - * / % , comparisons, && || ! , ternary, ++ --
 *   if/else, while, for, break, continue, blocks
 *   function declarations and expressions, calls, return, RECURSION
 *   arrays: literals, indexing, .length, .push
 *   strings: concatenation, .length, indexing, .charAt
 *   console.log and document.write
 *
 * WHAT IT DOES NOT RUN, and will not without a different design
 *   objects with prototypes, `this`, `new`, classes
 *   closures over a parent frame after it returns
 *   async, promises, generators, exceptions
 *   regular expressions, eval, the real DOM
 *   a garbage collector - values live in a fixed pool and a program that
 *     exhausts it STOPS with an error rather than growing
 *
 * NO HEAP, like everything else here. Every array is fixed and every limit
 * refuses rather than grows.
 */
#ifndef ZL_JS_H
#define ZL_JS_H

void js_reset(void);

/* Run a script. Returns 0 on success, -1 on a parse or runtime error - in
 * which case js_error() says what and roughly where. A script that fails
 * leaves whatever output it had already produced, which is what a browser
 * does. */
int  js_eval(const char *src, int len);

/* The value of the last expression evaluated, rendered as text. Mostly for
 * the harness; a page uses document.write. */
const char *js_result(int *len);

/* What console.log and document.write emitted, concatenated. */
const char *js_output(int *len);

const char *js_error(void);

/* how much of each fixed pool the last run used - the harness asserts these
 * stay inside their bounds rather than trusting that they did */
int js_nodes_used(void);
int js_vars_used(void);
int js_arena_used(void);

#endif
