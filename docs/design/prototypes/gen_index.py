#!/usr/bin/env python3
"""Regenerate index.html from one table, so adding a direction is one row.

The cards are generated rather than hand-written because there are sixteen of
them and hand-maintained duplicates drift - the same argument design.h makes
about colour literals.
"""
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "index.html")

# key, name, modes, group, pitch
D = [
 ("plate","PLATE","light + dark","r1","A printed technical document. Warm paper, black ink, hard 2px rules, one vermilion overprint. The left rail is a numbered register; the module grid is ruled onto the desk where you can see it. Focus knocks the header plate out to solid ink. Dark is ortho lith film."),
 ("folio","Folio","light + dark","r1","The desktop as a printed page. Square-cornered sheets on a 12-column grid, a left margin rail carrying the running table of contents, and every distinction made with weight, case, scale, rule-thickness and whitespace - never with colour."),
 ("datum","Datum","light + dark","r1","Paper and ink are two different materials on screen at once, by law: paper is the user's, ink is the machine's. A full-height ink rail, a full-width raster strip, a microsecond timing in every window's status band, a memory-map ruler along the bottom edge."),
 ("raking","Raking","dark only","r1","A warm-graphite machine lit by one cold light from the upper left. Depth is a 1px lit edge plus a 1px cut groove, never a shadow. Focus means turned toward the light. Radius encodes how much an object can move, and nesting halves it."),

 ("blueline","Blueline","light + dark","r2","An engineering drawing in diazo whiteprint. Blue line on white, no black anywhere, cyanotype as the other mode. All chrome collapses into a bottom-right title block. Window state is line type: present continuous, unfocused hidden, minimised phantom."),
 ("neatline","Neatline","light + dark","r2","One map sheet. A full-perimeter apron carries grid coordinates on three edges and the key on the fourth. Five inks, each lexical and each defined by the on-screen key. Dark comes from the IHO S-52 night table real ECDIS bridges use."),
 ("apparatus","Apparatus","light + dark","r2","Every window is a scholarly text with a critical apparatus: numbered lines in the outer margin, the machine's commentary at the foot keyed to line numbers. Focus means the apparatus opens. Its cut edge is drawn by the occluder, never the occluded."),
 ("deck","Deck","light + dark","r2","A front page that has been made up. No window frames at all - a story is bounded only by the rules it shares with its neighbours. Hierarchy by rank rather than z-order, so one window really is the lead story. Focus means the deck line gets set."),
 ("spread","SPREAD","light + dark","r2","Folio's 12-column page with PLATE's knockout lifted off the window entirely and hoisted into a running head shared by the whole screen. One head, one baseline, one page."),
 ("shelf","Shelf","light + dark","r2","A workstation, from the NeXTSTEP and IRIX line rather than the GNOME one. No bar anywhere: a mid-grey hatched desk, a left menu column that belongs to the focused application, and a right-edge dock where every item is a live instrument rather than an icon."),

 ("blendpaper","the paper blend","light + dark","bl","PLATE's ground, register rail, ruled desk grid and knockout, with Datum's instrumentation bolted on as a free rider. Its own implementer: an improvement of degree, not of kind. PLATE plus the instrumentation it should have had."),
 ("blendgraphite","the graphite blend","light + dark","bl","Raking's warm graphite and its lit-edge depth model, with Raking's two named defects repaired at zero cost to the premise: the focus signal gains 17.6x the area, and the rail can no longer be covered."),
 ("blendpage","the page blend","light + dark","bl","Folio's no-colour law kept whole, with its overlap weakness fixed by Apparatus's cut edge - legal under the law because it needs no colour, no shadow and no new primitive."),

 ("mixpress","PRESSWORK","dark primary","mx","Graphite widened until a knockout fits, then PLATE's knockout taken whole. Same 7.133% of a plate as graphite, 4.63x the ratio inside it: it does not add a mark, it makes graphite's existing mark legible. Focus is a 54 L* excursion at 6.4796:1."),
 ("mixstock","STOCK","dark primary","mx","Graphite's ladder untouched, and a vermilion overprint instead of a knockout - ink laid on top of the material rather than the material changing value, so the raking light never inverts. Focus costs 1.997% of a plate."),
 ("mixink","INKSTOCK","dark primary","mx","Two materials, both dark: warm screened stock for the user, cool flat plate for the machine, separated by hue and texture rather than luminance. Its own readout says so: NOT separated by luminance, by design."),
]

GROUPS = [("r1","Round one - the original four"),
          ("r2","Round two - six more"),
          ("bl","The blends - one position picked per axis"),
          ("mx","The mixes - graphite base carrying paper character")]

CSS = """
:root{--ground:#121110;--panel:#1a1917;--line:#2e2c29;--line-hi:#443f39;
 --ink:#e8e4dc;--ink-2:#a8a29a;--ink-3:#78726a;--mark:#c4552c;
 --mono:ui-monospace,"DejaVu Sans Mono",monospace;
 --sans:ui-sans-serif,system-ui,"DejaVu Sans",sans-serif}
*{box-sizing:border-box}
html,body{margin:0;background:var(--ground);color:var(--ink);font-family:var(--sans)}
body{padding:40px clamp(16px,4vw,64px) 96px;max-width:1500px;margin:0 auto}
h1{font-size:23px;font-weight:600;margin:0 0 6px}
.sub{color:var(--ink-2);font-size:14px;line-height:1.55;max-width:74ch;margin:0 0 8px}
.note{color:var(--ink-3);font-size:12.5px;line-height:1.6;max-width:74ch;margin:0}
.rule{height:1px;background:var(--line);margin:26px 0 22px}
h2{font-size:11px;font-weight:600;letter-spacing:.16em;text-transform:uppercase;
 color:var(--ink-3);margin:38px 0 14px}
.grid{display:grid;gap:20px;grid-template-columns:repeat(auto-fill,minmax(min(100%,420px),1fr))}
.card{background:var(--panel);border:1px solid var(--line);display:flex;flex-direction:column}
.card img{width:100%;height:auto;display:block;border-bottom:1px solid var(--line)}
.body{padding:13px 15px 15px}
.name{font-size:15px;font-weight:600;display:flex;align-items:baseline;gap:9px;margin-bottom:5px}
.tag{font-family:var(--mono);font-size:10.5px;letter-spacing:.09em;text-transform:uppercase;
 color:var(--ink-3);font-weight:400}
.pitch{font-size:13px;line-height:1.55;color:var(--ink-2);margin:0 0 11px}
.links{display:flex;flex-wrap:wrap;gap:5px;font-family:var(--mono);font-size:11px}
.links a{color:var(--ink-2);text-decoration:none;border:1px solid var(--line-hi);padding:3px 7px}
.links a:hover,.links a:focus-visible{color:var(--ground);background:var(--ink);
 border-color:var(--ink);outline:none}
.links a.dk{color:var(--ink-3)}
.open{color:var(--mark);border-color:var(--mark)}
.open:hover,.open:focus-visible{background:var(--mark);border-color:var(--mark);color:#120d0a}
footer{margin-top:48px;padding-top:20px;border-top:1px solid var(--line);
 color:var(--ink-3);font-size:12.5px;line-height:1.65;max-width:74ch}
code{font-family:var(--mono);font-size:.92em;color:var(--ink-2)}
"""

def card(k, name, modes, pitch):
    dark = "-dark" if "dark" in modes and modes != "dark only" else ""
    alt  = f'<a class="dk" href="proto-{k}.html#desktop{dark}">{"light" if modes=="dark primary" else "dark"}</a>' if dark or modes=="dark primary" else ""
    if modes == "dark primary":
        alt = f'<a class="dk" href="proto-{k}.html#desktop-light">light</a>'
    elif modes == "dark only":
        alt = ""
    else:
        alt = f'<a class="dk" href="proto-{k}.html#desktop-dark">dark</a>'
    scenes = "".join(f'<a href="proto-{k}.html#{s}">{s}</a>'
                     for s in ("overview","palette","monitor","lock"))
    return f"""  <div class="card">
    <img src="render/{k}-desktop.png" alt="{name} desktop">
    <div class="body">
      <div class="name">{name} <span class="tag">{modes}</span></div>
      <p class="pitch">{pitch}</p>
      <div class="links"><a class="open" href="proto-{k}.html">open</a>{scenes}{alt}</div>
    </div>
  </div>"""

parts = ['<!doctype html>', '<meta charset="utf-8">',
         '<title>zlOS desktop - sixteen directions</title>', f'<style>{CSS}</style>',
         '<h1>zlOS desktop - sixteen directions</h1>',
         '<p class="sub">Sixteen complete replacements for the zlOS desktop, each a working '
         'interactive prototype rather than a picture. Windows focus, move and close; the '
         'overview and command palette open; the settings toggles and sliders change real '
         'values. Every colour is a <code>:root</code> custom property named after the '
         '<code>design.h</code> token it would become.</p>',
         '<p class="note">None of this is implemented in the kernel. <code>design.h</code>, '
         '<code>ui.c</code>, <code>wm.c</code>, <code>uikit.c</code> and <code>kernel.zl</code> '
         'are untouched on this branch. The reasoning is in '
         '<a href="../deciding.md" style="color:var(--ink-2)">deciding.md</a>; the axis picker '
         'is <a href="decide.html" style="color:var(--mark)">decide.html</a>.</p>',
         '<div class="rule"></div>']

for g, title in GROUPS:
    parts.append(f"<h2>{title}</h2>")
    parts.append('<div class="grid">')
    parts += [card(k, n, m, p) for k, n, m, gg, p in D if gg == g]
    parts.append("</div>")

parts.append("""<footer>
Every prototype takes the same scene deep-links, on load and on hashchange:
<code>#desktop</code> <code>#overview</code> <code>#palette</code> <code>#files</code>
<code>#monitor</code> <code>#settings</code> <code>#lock</code>, each with
<code>-dark</code> (or <code>-light</code>, for the dark-primary mixes) appended.
<code>shift+D</code> toggles.
<br><br>
Each second mode is a second semantic ladder, not an inversion:
<code>:root[data-theme=...]</code> overriding the base block with every custom-property
name identical between the two, which is the shape <code>ui_theme_init_q8()</code> would
need to map one set of role names onto two primitive ladders.
<br><br>
All sixteen have only ever been judged against headless browser renders, never against
<code>fb.c</code> on a 2560x1440 panel. The prototypes flatter the target in at least one
known way: CSS gives letter-spacing free, where <code>fb_text_role</code> cannot track
text without a new primitive.
</footer>""")

open(OUT, "w").write("\n".join(parts) + "\n")
print(f"wrote {OUT}  ({len(D)} directions, {sum(1 for _ in open(OUT))} lines)")
