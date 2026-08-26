# Vendor provenance

Checked 2026-08-26 from the tracked bytes in this directory.

| Local file | Embedded identity | SHA-256 |
|---|---|---|
| `react.js` | `react.production.min.js`, React `18.3.1` | `d949f1c3687aedadcedac85261865f29b17cd273997e7f6b2bfc53b2f9d4c4dd` |
| `react-dom.js` | `react-dom.production.min.js`, reconciler `18.3.1` prerelease identity | `35f4f974f4b2bcd44da73963347f8952e341f83909e4498227d4e26b98f66f0d` |

Both files carry a React license header that identifies the MIT license and
refers to a `LICENSE` file at the upstream source-tree root. This repository does
not currently track that license text or a receipt naming the exact download
URL. Do not treat the current vendor shelf as publication-ready provenance.

## Update procedure

1. Record the exact upstream URL, release or commit, download date, and license.
2. Retain the matching license text beside the vendored files.
3. Replace both runtime files as one versioned unit.
4. Record fresh SHA-256 values in this file.
5. Run the reference-renderer comparison and preserve its result separately.

No upstream refresh, browser render, or license resolution was performed while
writing this receipt.
