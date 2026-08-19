/* support.js — a shim for the `x-dc` template runtime that ds.html was written for.
 *
 * ds.html is a desktop-environment mockup authored against a proprietary
 * React-based template runtime that ships as `./support.js`. That file was never
 * given to us, so the reference could be read but not rendered. This is a
 * reimplementation of just enough of that runtime to make it render.
 *
 * What it does, in order:
 *   1. loads React + ReactDOM 18 from ./vendor/ (no CDN at render time),
 *   2. hoists <helmet> children into <head>,
 *   3. compiles the <x-dc> template into a React element factory,
 *   4. defines the global `DCLogic` base class whose render() instantiates that
 *      template with the object returned by the subclass's renderVals(),
 *   5. evaluates the <script type="text/x-dc"> body in real global scope,
 *   6. mounts the resulting `Component` with the defaults declared in data-props.
 *
 * See README.md for the list of template features supported and not supported.
 */
(function () {
  'use strict';

  var SELF = document.currentScript && document.currentScript.src;
  var BASE = SELF ? SELF.replace(/[^/]*$/, '') : './';

  var DC = (window.__DC__ = {
    ready: false,
    errors: [],
    warnings: [],
    instance: null,
    props: null,
    preview: null,
    /* Driver hook: patch component state and resolve once React has committed. */
    setState: function (patch) {
      return new Promise(function (resolve, reject) {
        if (!DC.instance) return reject(new Error('component not mounted'));
        DC.instance.setState(patch, function () { resolve(true); });
      });
    }
  });

  function note(kind, msg) {
    DC[kind].push(msg);
    (kind === 'errors' ? console.error : console.warn)('[support.js] ' + msg);
  }

  window.addEventListener('error', function (e) {
    DC.errors.push(String((e && e.error && e.error.stack) || (e && e.message) || e));
  });
  window.addEventListener('unhandledrejection', function (e) {
    DC.errors.push('unhandled rejection: ' + String((e && e.reason && e.reason.message) || (e && e.reason)));
  });

  /* ------------------------------------------------------------------ *
   * loading
   * ------------------------------------------------------------------ */

  function loadScript(url) {
    return new Promise(function (resolve, reject) {
      var s = document.createElement('script');
      s.src = url;
      s.async = false;
      s.onload = function () { resolve(); };
      s.onerror = function () { reject(new Error('could not load ' + url)); };
      (document.head || document.documentElement).appendChild(s);
    });
  }

  function domReady() {
    return new Promise(function (resolve) {
      if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', function () { resolve(); });
      } else resolve();
    });
  }

  /* ------------------------------------------------------------------ *
   * CSS: literal style="..." strings -> React style objects
   *
   * Kept as inline styles rather than generated classes on purpose: the
   * helmet stylesheet contains `input[type=range]{...}` (specificity 0,1,1),
   * which would beat a generated single class and change the rendering.
   * ------------------------------------------------------------------ */

  function splitTop(str, ch) {
    var out = [], depth = 0, cur = '';
    for (var i = 0; i < str.length; i++) {
      var c = str[i];
      if (c === '(') depth++;
      else if (c === ')') depth--;
      if (c === ch && depth === 0) { out.push(cur); cur = ''; } else cur += c;
    }
    out.push(cur);
    return out;
  }

  function indexOfTop(str, ch) {
    var depth = 0;
    for (var i = 0; i < str.length; i++) {
      var c = str[i];
      if (c === '(') depth++;
      else if (c === ')') depth--;
      else if (c === ch && depth === 0) return i;
    }
    return -1;
  }

  function dashToCamel(name) {
    return name.replace(/-([a-z])/g, function (_, c) { return c.toUpperCase(); });
  }

  function cssPropName(name) {
    if (name.slice(0, 2) === '--') return name;                       // custom property
    if (name[0] === '-') {                                            // vendor prefix
      var m = /^-(webkit|moz|ms|o)-(.*)$/.exec(name);
      if (m) {
        var head = m[1] === 'ms' ? 'ms' : m[1][0].toUpperCase() + m[1].slice(1);
        return head + dashToCamel(m[2]).replace(/^([a-z])/, function (_, c) { return c.toUpperCase(); });
      }
    }
    return dashToCamel(name);
  }

  var cssCache = Object.create(null);

  function cssToObj(css) {
    if (cssCache[css]) return cssCache[css];
    var obj = {}, decls = splitTop(css, ';');
    for (var i = 0; i < decls.length; i++) {
      var d = decls[i].trim();
      if (!d) continue;
      var k = indexOfTop(d, ':');
      if (k < 0) { note('warnings', 'unparseable css declaration: ' + d); continue; }
      obj[cssPropName(d.slice(0, k).trim())] = d.slice(k + 1).trim();
    }
    return (cssCache[css] = obj);
  }

  /* style-hover="..." becomes a real generated `.dchN:hover` rule.
   * !important is required: three of the four style-hover sites sit on
   * elements whose `style` is a dynamic object, and an inline style would
   * otherwise beat the class rule. */
  var hoverSheet = null, hoverN = 0;

  function hoverClass(css) {
    if (!hoverSheet) {
      hoverSheet = document.createElement('style');
      hoverSheet.setAttribute('data-support-shim', 'style-hover');
      document.head.appendChild(hoverSheet);
    }
    var cls = 'dch' + hoverN++;
    var decls = splitTop(css, ';').map(function (d) { return d.trim(); }).filter(Boolean)
      .map(function (d) { return d + ' !important'; }).join(';');
    hoverSheet.appendChild(document.createTextNode('.' + cls + ':hover{' + decls + '}\n'));
    return cls;
  }

  /* ------------------------------------------------------------------ *
   * attribute names
   *
   * The HTML parser lowercases attribute names on HTML elements, so the
   * template's `onPointerDown` arrives as `onpointerdown`. This maps back to
   * the React prop names. (Inside <svg>, foreign-content parsing already
   * preserves viewBox/preserveAspectRatio, so both spellings are listed.)
   * ------------------------------------------------------------------ */

  var ATTR = {
    // template directives, consumed by the compiler and never emitted
    'hint-placeholder-count': null,
    'hint-placeholder-val': null,
    // React quirks
    'class': 'className',
    'for': 'htmlFor',
    'spellcheck': 'spellCheck',
    'tabindex': 'tabIndex',
    'readonly': 'readOnly',
    'maxlength': 'maxLength',
    'autofocus': 'autoFocus',
    'contenteditable': 'contentEditable',
    'srcset': 'srcSet',
    'crossorigin': 'crossOrigin',
    // svg
    'viewbox': 'viewBox',
    'viewBox': 'viewBox',
    'preserveaspectratio': 'preserveAspectRatio',
    'preserveAspectRatio': 'preserveAspectRatio',
    'stroke-width': 'strokeWidth',
    'stroke-linecap': 'strokeLinecap',
    'stroke-linejoin': 'strokeLinejoin',
    'stroke-dasharray': 'strokeDasharray',
    'fill-rule': 'fillRule',
    'clip-rule': 'clipRule',
    'clip-path': 'clipPath',
    'stop-color': 'stopColor',
    'stop-opacity': 'stopOpacity',
    'text-anchor': 'textAnchor',
    'dominant-baseline': 'dominantBaseline',
    'font-family': 'fontFamily',
    'font-size': 'fontSize'
  };

  /* Event names the template uses, plus every other DOM event React knows, so
   * an unanticipated onFoo still binds instead of becoming a string attribute. */
  var EVENTS = ('click dblclick doubleclick contextmenu change input submit reset focus blur keydown keyup ' +
    'keypress scroll wheel copy cut paste drag dragend dragenter dragleave dragover dragstart drop ' +
    'mousedown mouseup mousemove mouseenter mouseleave mouseover mouseout ' +
    'pointerdown pointerup pointermove pointerenter pointerleave pointerover pointerout pointercancel ' +
    'touchstart touchend touchmove touchcancel animationend animationstart transitionend load error').split(' ');
  var EVENT_MAP = Object.create(null);
  EVENTS.forEach(function (e) {
    var camel = e === 'doubleclick' ? 'DoubleClick'
      : e === 'dblclick' ? 'DoubleClick'
      : e === 'contextmenu' ? 'ContextMenu'
      : e.replace(/^([a-z])/, function (_, c) { return c.toUpperCase(); })
         .replace(/(down|up|move|enter|leave|over|out|cancel|start|end|press)$/,
                  function (m) { return m[0].toUpperCase() + m.slice(1); });
    EVENT_MAP['on' + e] = 'on' + camel;
  });
  // The regex above cannot see word boundaries inside compounds; pin the ones
  // the reference actually uses so nothing depends on that guess.
  Object.assign(EVENT_MAP, {
    onclick: 'onClick', ondblclick: 'onDoubleClick', ondoubleclick: 'onDoubleClick',
    oncontextmenu: 'onContextMenu', onchange: 'onChange', oninput: 'onInput',
    onkeydown: 'onKeyDown', onkeyup: 'onKeyUp', onkeypress: 'onKeyPress',
    onscroll: 'onScroll', onwheel: 'onWheel', onfocus: 'onFocus', onblur: 'onBlur',
    onsubmit: 'onSubmit', onreset: 'onReset',
    onpointerdown: 'onPointerDown', onpointerup: 'onPointerUp', onpointermove: 'onPointerMove',
    onpointerenter: 'onPointerEnter', onpointerleave: 'onPointerLeave',
    onpointerover: 'onPointerOver', onpointerout: 'onPointerOut', onpointercancel: 'onPointerCancel',
    onmousedown: 'onMouseDown', onmouseup: 'onMouseUp', onmousemove: 'onMouseMove',
    onmouseenter: 'onMouseEnter', onmouseleave: 'onMouseLeave',
    onmouseover: 'onMouseOver', onmouseout: 'onMouseOut',
    ontouchstart: 'onTouchStart', ontouchend: 'onTouchEnd', ontouchmove: 'onTouchMove',
    onanimationend: 'onAnimationEnd', onanimationstart: 'onAnimationStart',
    ontransitionend: 'onTransitionEnd'
  });

  function propName(attr) {
    var lower = attr.toLowerCase();
    if (EVENT_MAP[lower]) return EVENT_MAP[lower];
    if (Object.prototype.hasOwnProperty.call(ATTR, attr)) return ATTR[attr];
    if (Object.prototype.hasOwnProperty.call(ATTR, lower)) return ATTR[lower];
    if (lower.slice(0, 5) === 'data-' || lower.slice(0, 5) === 'aria-') return attr;
    if (lower.slice(0, 2) === 'on') {
      note('warnings', 'unmapped event attribute ' + attr + ' — bound as on' +
        attr.slice(2, 3).toUpperCase() + attr.slice(3));
      return 'on' + attr.slice(2, 3).toUpperCase() + attr.slice(3);
    }
    return attr;
  }

  /* ------------------------------------------------------------------ *
   * template compilation
   * ------------------------------------------------------------------ */

  var MUSTACHE = /\{\{([^}]*)\}\}/g;   // stateful (g): only ever used with .exec in a reset loop
  var HAS_MUSTACHE = /\{\{/;           // stateless: safe for .test anywhere
  var PATH_RE = /^[A-Za-z_$][\w$]*(\.[A-Za-z_$][\w$]*)*$/;
  var nodeId = 0;

  function wholeMustache(str) {
    var m = /^\s*\{\{([^}]*)\}\}\s*$/.exec(str);
    return m ? m[1].trim() : null;
  }

  /* Babel's cleanJSXElementLiteralChild, reproduced. The original runtime was a
   * React template compiler, so template whitespace must collapse the way JSX
   * collapses it or every flex row gains stray text nodes. */
  function cleanText(text) {
    var lines = text.split(/\r\n|\n|\r/), last = -1, out = '';
    for (var i = 0; i < lines.length; i++) if (/[^ \t]/.test(lines[i])) last = i;
    for (var j = 0; j < lines.length; j++) {
      var line = lines[j].replace(/\t/g, ' ');
      if (j !== 0) line = line.replace(/^ +/, '');
      if (j !== lines.length - 1) line = line.replace(/ +$/, '');
      if (!line) continue;
      if (j !== last) line += ' ';
      out += line;
    }
    return out;
  }

  function compileText(raw) {
    var parts = [], idx = 0, m;
    MUSTACHE.lastIndex = 0;
    while ((m = MUSTACHE.exec(raw)) !== null) {
      var lit = cleanText(raw.slice(idx, m.index));
      if (lit) parts.push(lit);
      var expr = m[1].trim();
      if (PATH_RE.test(expr)) parts.push({ p: expr.split('.') });
      else note('warnings', 'non-path expression in text, rendered empty: {{ ' + expr + ' }}');
      idx = m.index + m[0].length;
    }
    var tail = cleanText(raw.slice(idx));
    if (tail) parts.push(tail);
    return parts;
  }

  function compileChildren(parent) {
    var out = [];
    for (var n = parent.firstChild; n; n = n.nextSibling) {
      if (n.nodeType === 3) {
        var parts = compileText(n.nodeValue);
        if (parts.length) out.push({ t: 'txt', parts: parts });
      } else if (n.nodeType === 1) {
        var c = compileEl(n);
        if (c) out.push(c);
      }
      /* comments and everything else are dropped, as the lexer would */
    }
    return out;
  }

  function compileEl(el) {
    var tag = el.localName;

    if (tag === 'helmet') return null;   // hoisted before compilation

    if (tag === 'sc-for') {
      var listExpr = wholeMustache(el.getAttribute('list') || '');
      var as = el.getAttribute('as');
      if (!listExpr || !PATH_RE.test(listExpr) || !as) {
        note('errors', '<sc-for> needs list="{{ path }}" and as="name"; got list=' +
          el.getAttribute('list') + ' as=' + as);
        return null;
      }
      return { t: 'for', id: 'f' + nodeId++, path: listExpr.split('.'), as: as, children: compileChildren(el) };
    }

    if (tag === 'sc-if') {
      var condExpr = wholeMustache(el.getAttribute('value') || '');
      if (!condExpr || !PATH_RE.test(condExpr)) {
        note('errors', '<sc-if> needs value="{{ path }}"; got ' + el.getAttribute('value'));
        return null;
      }
      return { t: 'if', id: 'c' + nodeId++, path: condExpr.split('.'), children: compileChildren(el) };
    }

    var sprops = {}, dprops = [];
    for (var i = 0; i < el.attributes.length; i++) {
      var a = el.attributes[i], name = a.name, value = a.value;
      if (name === 'hint-placeholder-count' || name === 'hint-placeholder-val') continue;
      if (name === 'style-hover') {
        sprops.className = (sprops.className ? sprops.className + ' ' : '') + hoverClass(value);
        continue;
      }
      var expr = wholeMustache(value);
      if (expr !== null) {
        if (!PATH_RE.test(expr)) {
          note('warnings', 'non-path expression in attribute ' + name + ', dropped: {{ ' + expr + ' }}');
          continue;
        }
        var pn = name === 'style' ? 'style' : name === 'ref' ? 'ref' : propName(name);
        if (pn === null) continue;
        dprops.push({ name: pn, path: expr.split('.'), style: name === 'style' });
      } else if (HAS_MUSTACHE.test(value)) {
        note('warnings', 'partial interpolation in attribute ' + name + ' is not supported: ' + value);
      } else {
        if (name === 'style') sprops.style = cssToObj(value);
        else {
          var sn = propName(name);
          if (sn !== null) sprops[sn] = value;
        }
      }
    }

    return { t: 'el', id: 'e' + nodeId++, tag: tag, sprops: sprops, dprops: dprops, children: compileChildren(el) };
  }

  /* ------------------------------------------------------------------ *
   * instantiation
   * ------------------------------------------------------------------ */

  var ROOT_SCOPE = Object.create(null);

  function resolve(path, scope, vals) {
    var cur = (path[0] in scope) ? scope[path[0]] : vals[path[0]];
    for (var i = 1; i < path.length; i++) {
      if (cur === null || cur === undefined) return undefined;
      cur = cur[path[i]];
    }
    return cur;
  }

  function emit(node, scope, vals, ksuf, out) {
    if (node.t === 'txt') {
      for (var i = 0; i < node.parts.length; i++) {
        var p = node.parts[i];
        if (typeof p === 'string') out.push(p);
        else {
          var v = resolve(p.p, scope, vals);
          if (v !== null && v !== undefined && v !== false) out.push(v);
        }
      }
      return;
    }
    if (node.t === 'if') {
      if (resolve(node.path, scope, vals)) {
        for (var j = 0; j < node.children.length; j++) emit(node.children[j], scope, vals, ksuf, out);
      }
      return;
    }
    if (node.t === 'for') {
      var list = resolve(node.path, scope, vals);
      if (list === null || list === undefined) return;
      if (!Array.isArray(list)) {
        note('warnings', '<sc-for list="' + node.path.join('.') + '"> got a non-array');
        return;
      }
      for (var k = 0; k < list.length; k++) {
        var sub = Object.create(scope);
        sub[node.as] = list[k];
        var ks = ksuf + '.' + node.id + '_' + k;
        for (var m = 0; m < node.children.length; m++) emit(node.children[m], sub, vals, ks, out);
      }
      return;
    }
    out.push(build(node, scope, vals, ksuf));
  }

  function build(node, scope, vals, ksuf) {
    var props = { key: node.id + ksuf };
    for (var k in node.sprops) props[k] = node.sprops[k];
    for (var i = 0; i < node.dprops.length; i++) {
      var d = node.dprops[i], v = resolve(d.path, scope, vals);
      if (d.style) {
        if (v === null || v === undefined) continue;
        props.style = (typeof v === 'string') ? cssToObj(v) : v;
      } else if (v !== undefined) {
        props[d.name] = v;
      }
    }
    if (!node.children.length) return React.createElement(node.tag, props);
    var kids = [];
    for (var j = 0; j < node.children.length; j++) emit(node.children[j], scope, vals, ksuf, kids);
    if (!kids.length) return React.createElement(node.tag, props);
    return React.createElement(node.tag, props, kids);
  }

  function instantiate(roots, vals) {
    var out = [];
    for (var i = 0; i < roots.length; i++) emit(roots[i], ROOT_SCOPE, vals, '', out);
    return out.length === 1 ? out[0] : React.createElement(React.Fragment, null, out);
  }

  /* ------------------------------------------------------------------ *
   * boot
   * ------------------------------------------------------------------ */

  Promise.all([
    domReady(),
    loadScript(BASE + 'vendor/react.js').then(function () { return loadScript(BASE + 'vendor/react-dom.js'); })
  ]).then(function () {
    if (!window.React || !window.ReactDOM) throw new Error('React did not define its globals');

    var xdc = document.querySelector('x-dc');
    if (!xdc) throw new Error('no <x-dc> element in the document');

    // 1. hoist helmet children into <head>
    var helmets = xdc.querySelectorAll('helmet');
    for (var h = 0; h < helmets.length; h++) {
      var hel = helmets[h];
      while (hel.firstChild) {
        var n = hel.firstChild;
        hel.removeChild(n);
        if (n.nodeType === 1) document.head.appendChild(n);
      }
      hel.parentNode.removeChild(hel);
    }

    // 2. compile
    var roots = compileChildren(xdc);
    DC.templateNodes = nodeId;

    // 3. swap <x-dc> for a mount point
    var mount = document.createElement('div');
    mount.id = 'dc-root';
    xdc.parentNode.replaceChild(mount, xdc);

    // 4. props declared in data-props
    var scriptEl = document.querySelector('script[type="text/x-dc"]');
    if (!scriptEl) throw new Error('no <script type="text/x-dc"> in the document');
    var decl = {};
    try { decl = JSON.parse(scriptEl.getAttribute('data-props') || '{}'); }
    catch (err) { note('warnings', 'data-props is not valid JSON: ' + err.message); }
    var props = {};
    Object.keys(decl).forEach(function (k) {
      if (k === '$preview') { DC.preview = decl[k]; return; }
      if (decl[k] && typeof decl[k] === 'object' && 'default' in decl[k]) props[k] = decl[k]['default'];
    });
    DC.props = props;

    // 5. the base class the reference extends
    window.DCLogic = class DCLogic extends React.Component {
      renderVals() { return {}; }
      render() {
        var vals = this.renderVals();
        if (!vals || typeof vals !== 'object') {
          note('errors', 'renderVals() did not return an object');
          vals = {};
        }
        return instantiate(roots, vals);
      }
    };

    // 6. evaluate the x-dc script in real global scope
    var run = document.createElement('script');
    run.setAttribute('data-support-shim', 'x-dc-script');
    run.textContent = scriptEl.textContent + '\n;window.__DC_COMPONENT__ = Component;\n';
    document.head.appendChild(run);
    if (typeof window.__DC_COMPONENT__ !== 'function') {
      throw new Error('the x-dc script did not define a `Component` class');
    }

    // 7. mount
    var root = ReactDOM.createRoot(mount);
    root.render(React.createElement(window.__DC_COMPONENT__, Object.assign({
      ref: function (inst) { if (inst) DC.instance = inst; }
    }, props)));

    DC.root = root;
    DC.ready = true;
    document.documentElement.setAttribute('data-dc-ready', '1');
  }).catch(function (err) {
    DC.errors.push('boot: ' + (err && err.stack || err));
    console.error('[support.js] boot failed', err);
    document.documentElement.setAttribute('data-dc-failed', String(err && err.message || err));
  });
})();
