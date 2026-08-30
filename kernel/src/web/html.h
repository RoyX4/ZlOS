/* html.h - the tree, as the layout sees it.
 *
 * Indices, not pointers - see the comment at the top of html.c. -1 is null.
 * Nothing here allocates and nothing here can fail, so every accessor is total:
 * an out-of-range index returns an empty answer rather than faulting.
 */
#ifndef HTML_H
#define HTML_H

#define HN_ELEM 0
#define HN_TEXT 1

enum {
    HT_NONE = 0,
    HT_HTML, HT_HEAD, HT_BODY, HT_TITLE,
    HT_H1, HT_H2, HT_H3, HT_H4, HT_H5, HT_H6,
    HT_P, HT_BR, HT_HR, HT_A,
    HT_UL, HT_OL, HT_LI,
    HT_STRONG, HT_EM, HT_B, HT_I,
    HT_CODE, HT_PRE,
    HT_DIV, HT_SPAN, HT_IMG,
    HT_SCRIPT, HT_STYLE,
    HT_TABLE, HT_TR, HT_TD, HT_TH, HT_THEAD, HT_TBODY, HT_TFOOT, HT_CAPTION,
    HT_UNKNOWN
};

/* The parser's hard limits, EXPOSED rather than duplicated. htmltest asserted
 * `html_count() <= 1024` against a literal, so raising MAX_NODES turned a real
 * bounds check into a failing test that said nothing about bounds. A limit a
 * test restates by hand is a limit the test stops checking.
 *
 * 8192 WAS THE CEILING A REAL PAGE HIT FIRST, and it hit it hard: the English
 * Wikipedia article on Linux (982,395 bytes) filled the array and dropped
 * 7,807 further nodes - it parsed a little over half the document. The text
 * arena finished at 196,607 of 196,608 in the same run. Neither could be
 * raised, because both were BSS in a kernel with 126,336 bytes of link
 * headroom, and 32768 nodes is 2.25 MiB on its own.
 *
 * So the storage moved and these are now what the CALLER must supply. */
#define HTML_MAX_NODES 32768
#define HTML_ARENA     (1024 * 1024)

/* sizeof(struct node), which html.c keeps to itself. It is here because the
 * caller has to lay out a region in BYTES before it can hand over an array,
 * and it is a literal rather than a sizeof because the struct is private.
 *
 * A DUPLICATED NUMBER IS A NUMBER THAT DRIFTS, which is the whole complaint
 * memmap.h's header makes - so html.c _Static_asserts this against the real
 * sizeof. Add a field to struct node and the build stops here rather than
 * silently handing the parser an array 5/6 the size it thinks it has.
 * Measured 72 on -m32 and on x86-64 alike: struct node holds no pointers. */
#define HTML_NODE_BYTES  72
#define HTML_NODES_BYTES ((long)HTML_MAX_NODES * HTML_NODE_BYTES)

/* WHERE THE TREE AND THE TEXT LIVE IS THE CALLER'S DECISION, which is the one
 * thing this header asks of anyone using it, and it is the same bargain png.h
 * states: png.c owns the slot bookkeeping and the caller owns the storage.
 *
 * `nodes` is void * because struct node is private; it must be aligned for a
 * 4-byte int and hold at least HTML_NODE_BYTES * max_nodes bytes. `arena` is
 * the text the tree points into.
 *
 * UNTIL THIS IS CALLED, html_parse PARSES NOTHING AND RETURNS 0. Deliberate
 * and loud, for png.h's reason restated one layer up: a parser that fell back
 * to a small built-in array would work on the home page, truncate every real
 * one, and give nobody a reason - which is precisely the bug this change
 * exists to remove, reintroduced as a default. */
void html_set_arena(void *nodes, int max_nodes, char *arena, int arena_bytes);

/* The caps actually in force, so a harness asserts against what was handed
 * over rather than against a constant it restated. Same lesson as
 * HTML_MAX_NODES above, one level further in. */
int  html_node_cap(void);
int  html_arena_cap(void);

void html_reset(void);
int  html_parse(const char *src, int len);   /* returns the node count */

int  html_count(void);
int  html_root(void);
int  html_kind(int i);
int  html_tag(int i);
int  html_parent(int i);
int  html_first(int i);
int  html_next(int i);
int  html_depth(int i);
int  html_max_depth(void);
int  html_dropped(void);                     /* recoveries from bad markup */
int  html_arena_used(void);
int  html_is_block(int tag);

const char *html_text(int i, int *len);      /* NOT nul-terminated */
const char *html_href(int i, int *len);
const char *html_src(int i, int *len);       /* an <img>'s src, "" otherwise */
int         html_attr_w(int i);              /* width=,  0 when not given    */
int         html_attr_h(int i);              /* height=, 0 when not given    */
const char *html_title(int *len);

/* What a stylesheet matches against. All total: an element with no class, no
 * id or no style= answers "" and 0. */
const char *html_tagname(int i, int *len);   /* the element's own name */
const char *html_class(int i, int *len);
const char *html_id(int i, int *len);
const char *html_style_attr(int i, int *len);

/* The document's own <style> blocks, in document order = cascade order.
 * These are spans of the SOURCE buffer, not copies - it must outlive the
 * parse, which it does: browser.c holds the document. */
int         html_sheets(void);
int         html_css_links(void);          /* <link rel=stylesheet> URLs */
const char *html_css_link(int k, int *len);
const char *html_sheet(int k, int *len);

/* the document's <script> blocks, in document order. Kept for js.c; still
 * never rendered as text. */
int         html_scripts(void);
const char *html_script(int k, int *len);

#endif
