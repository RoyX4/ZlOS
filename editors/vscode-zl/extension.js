/* extension.js - wires ./zlfmt into VS Code as the formatter for .zl files.
 *
 * Deliberately plain CommonJS with no dependencies and no build step: the
 * `vscode` module is injected by the extension host and everything else is
 * node's stdlib, so this folder can be copied straight into
 * ~/.vscode/extensions/ and work. Adding a bundler here would mean the
 * extension could no longer be installed by `cp -r`, which is how it is
 * installed.
 *
 * The formatter is a separate process on purpose. zlfmt uses the real
 * lexer.c to decide which braces are code and which are inside a string or
 * a comment; reimplementing that classification in JavaScript would mean two
 * definitions of zl's lexical grammar that drift apart.
 *
 * We pipe the document through `zlfmt -` rather than pointing it at the file
 * on disk, because VS Code formats the in-memory buffer - which is not the
 * same bytes as the saved file whenever the user has unsaved edits, and
 * format-on-save runs BEFORE the write.
 */

const vscode = require('vscode');
const cp = require('child_process');
const fs = require('fs');
const os = require('os');
const path = require('path');

const OUTPUT = vscode.window.createOutputChannel('zl');

/* Where to find one of our binaries, in order:
 *   1. the given setting, if the user set one
 *   2. ./<name> at the root of any open workspace folder (where build.sh puts it)
 *   3. any of `names` on PATH
 * Returns null if none of those exist, so the caller can say so precisely
 * instead of failing with a bare ENOENT. */
function findBinary(setting, names) {
    const configured = vscode.workspace.getConfiguration('zl').get(setting);
    if (configured && configured.trim() !== '') {
        const expanded = configured.replace(/^~(?=$|\/)/, process.env.HOME || '~');
        return fs.existsSync(expanded) ? expanded : null;
    }

    for (const folder of vscode.workspace.workspaceFolders || []) {
        const candidate = path.join(folder.uri.fsPath, names[0]);
        if (fs.existsSync(candidate)) return candidate;
    }

    for (const dir of (process.env.PATH || '').split(path.delimiter)) {
        if (dir === '') continue;
        for (const name of names) {
            const candidate = path.join(dir, name);
            try {
                fs.accessSync(candidate, fs.constants.X_OK);
                return candidate;
            } catch (e) { /* not here; keep looking */ }
        }
    }

    return null;
}

function findFormatter() {
    return findBinary('formatterPath', ['zlfmt']);
}

/* Run zlfmt over `text` and resolve to the formatted text.
 *
 * Rejects rather than resolving on any non-zero exit. That matters: zlfmt
 * exits 1 when the file does not lex, and a half-formatted or empty buffer
 * would be worse than leaving the user's source alone. "Formatting failed"
 * must never mean "formatting ate my file". */
function runFormatter(binary, text, indent) {
    return new Promise((resolve, reject) => {
        const child = cp.spawn(binary, ['--indent', String(indent), '-'], {
            stdio: ['pipe', 'pipe', 'pipe'],
        });

        let stdout = '';
        let stderr = '';
        child.stdout.on('data', (d) => { stdout += d; });
        child.stderr.on('data', (d) => { stderr += d; });

        child.on('error', (err) => reject(new Error(`could not run ${binary}: ${err.message}`)));

        child.on('close', (code) => {
            if (code === 0) resolve(stdout);
            else reject(new Error(stderr.trim() || `zlfmt exited ${code}`));
        });

        child.stdin.on('error', () => { /* child died early; 'close' reports it */ });
        child.stdin.end(text);
    });
}

/* Replace the whole document. zlfmt only ever rewrites leading and trailing
 * whitespace, so a full-range edit produces a minimal-looking diff anyway,
 * and it keeps this side of the integration free of any assumption about
 * which lines the formatter touched. */
function wholeDocumentRange(document) {
    const lastLine = document.lineAt(document.lineCount - 1);
    return new vscode.Range(0, 0, document.lineCount - 1, lastLine.text.length);
}

let warnedMissing = false;

async function provideEdits(document, options) {
    const binary = findFormatter();

    if (binary === null) {
        if (!warnedMissing) {
            warnedMissing = true;
            vscode.window.showWarningMessage(
                'zl: formatter not found. Build it with ./build.sh in the zl repo, ' +
                'or set "zl.formatterPath" in settings.',
                'Open settings'
            ).then((choice) => {
                if (choice === 'Open settings') {
                    vscode.commands.executeCommand('workbench.action.openSettings', 'zl.formatterPath');
                }
            });
        }
        return [];
    }

    const indent = options && options.tabSize ? options.tabSize : 4;

    try {
        const formatted = await runFormatter(binary, document.getText(), indent);
        if (formatted === document.getText()) return [];    /* already formatted */
        return [vscode.TextEdit.replace(wholeDocumentRange(document), formatted)];
    } catch (err) {
        /* A lex error here is the normal case while you are mid-edit, so this
         * goes to the output channel rather than a modal. The buffer is left
         * untouched. */
        OUTPUT.appendLine(`[${new Date().toISOString()}] ${document.fileName}: ${err.message}`);
        vscode.window.setStatusBarMessage(`zl: not formatted - ${err.message}`, 5000);
        return [];
    }
}

/* Running a .zl file.
 *
 * This goes through a VS Code terminal rather than an output channel on
 * purpose: zl programs read stdin and print as they go, and an output channel
 * is write-only. A terminal is the only surface where an interactive program
 * behaves the same inside the editor as it does in a shell.
 *
 * The working directory matters. `import <stdlib module>` resolves relative to
 * the process's cwd, not to the source file - that is why the `zl` wrapper on
 * PATH cds to the repo root before exec'ing ./interp. We do the same: when the
 * binary we found lives at a repo root, run from there; otherwise fall back to
 * the file's own directory.
 */
let terminal = null;

function runTerminal() {
    if (terminal === null || terminal.exitStatus !== undefined) {
        terminal = vscode.window.createTerminal('zl');
    }
    return terminal;
}

function shellQuote(s) {
    return `'${s.replace(/'/g, `'\\''`)}'`;
}

/* A wrapper script (`zl`) already cds for itself and takes the file directly;
 * a bare `interp`/`compilel` binary does not. Anything found at a workspace
 * root is the repo's own build output, so its directory IS the root. */
function workingDirectoryFor(binary, filePath) {
    for (const folder of vscode.workspace.workspaceFolders || []) {
        if (path.dirname(binary) === folder.uri.fsPath) return folder.uri.fsPath;
    }
    return path.dirname(filePath);
}

async function currentZlFile() {
    const editor = vscode.window.activeTextEditor;
    if (!editor || editor.document.languageId !== 'zl') {
        vscode.window.showErrorMessage('zl: no .zl file is active.');
        return null;
    }
    /* Untitled is checked BEFORE saving, not after. `save()` on an untitled
     * buffer opens a Save-As dialog and then swaps in a *different* document,
     * leaving this `editor.document` still untitled - so a post-save check
     * would report failure on a file the user had just successfully saved.
     * This is also the failure the tasks.json runner gives as the useless
     * `can't open 'Untitled-1'`: ${file} on an unsaved buffer is not a path. */
    if (editor.document.isUntitled) {
        vscode.window.showErrorMessage(
            'zl: this buffer has never been saved, so there is no file to run. ' +
            'Save it as a .zl file first.'
        );
        return null;
    }
    if (vscode.workspace.getConfiguration('zl').get('saveBeforeRun') !== false) {
        /* Both runners read from disk; an unsaved buffer would run stale bytes. */
        if (editor.document.isDirty && !(await editor.document.save())) {
            vscode.window.showErrorMessage('zl: could not save the file before running.');
            return null;
        }
    }
    return editor.document.uri.fsPath;
}

function missing(what, hint) {
    vscode.window.showErrorMessage(`zl: ${what} not found. ${hint}`);
}

async function runInterpreted() {
    const file = await currentZlFile();
    if (file === null) return;

    const binary = findBinary('interpreterPath', ['interp', 'zl']);
    if (binary === null) {
        missing('interpreter', 'Build it with ./build.sh in the zl repo, or set "zl.interpreterPath".');
        return;
    }

    const term = runTerminal();
    term.show(true);
    term.sendText(`cd ${shellQuote(workingDirectoryFor(binary, file))} && ` +
                  `${shellQuote(binary)} ${shellQuote(file)}`);
}

/* compilel emits ./out.ll in the cwd and clang turns that into a binary. The
 * executable goes to a temp path rather than next to the source so that
 * running a file never drops an untracked binary into the user's tree. */
async function runCompiled() {
    const file = await currentZlFile();
    if (file === null) return;

    const binary = findBinary('compilerPath', ['compilel']);
    if (binary === null) {
        missing('compilel', 'Build it with ./build.sh in the zl repo, or set "zl.compilerPath".');
        return;
    }
    if (findBinary('__none__', ['clang']) === null) {
        missing('clang', 'The LLVM backend needs clang to turn out.ll into an executable.');
        return;
    }

    const cwd = workingDirectoryFor(binary, file);
    const exe = path.join(os.tmpdir(), `zlrun-${path.basename(file, '.zl')}`);

    const term = runTerminal();
    term.show(true);
    term.sendText(`cd ${shellQuote(cwd)} && ` +
                  `${shellQuote(binary)} ${shellQuote(file)} && ` +
                  `clang -O2 ${shellQuote(path.join(cwd, 'out.ll'))} -o ${shellQuote(exe)} && ` +
                  shellQuote(exe));
}

function activate(context) {
    context.subscriptions.push(
        vscode.languages.registerDocumentFormattingEditProvider('zl', {
            provideDocumentFormattingEdits: provideEdits,
        })
    );

    context.subscriptions.push(
        vscode.commands.registerCommand('zl.showFormatterPath', () => {
            const binary = findFormatter();
            vscode.window.showInformationMessage(
                binary ? `zl formatter: ${binary}` : 'zl formatter: not found'
            );
        })
    );

    context.subscriptions.push(
        vscode.commands.registerCommand('zl.run', runInterpreted),
        vscode.commands.registerCommand('zl.runCompiled', runCompiled)
    );

    context.subscriptions.push(OUTPUT);
}

function deactivate() { /* nothing to tear down; the child process is per-call */ }

module.exports = { activate, deactivate };
