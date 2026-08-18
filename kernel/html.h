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
    HT_UNKNOWN
};

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
const char *html_title(int *len);

#endif
