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
 * test restates by hand is a limit the test stops checking. */
#define HTML_MAX_NODES 8192

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
