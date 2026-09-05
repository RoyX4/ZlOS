"""Exercise installed hook custody only in disposable repositories."""
import os
from pathlib import Path
import shutil
import shlex
import subprocess
import tempfile
import unittest


SOURCE = Path(__file__).with_name('install-hooks.sh')
GIT = shutil.which('git')


class HookTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='hook-test-')
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name) / 'repo'
        self.root.mkdir()
        self.env = dict(os.environ, GIT_CONFIG_NOSYSTEM='1',
                        GIT_CONFIG_GLOBAL='/dev/null')
        self.env.pop('ZL_HOOK_RUNNING', None)
        self.git('init', '-q')
        self.git('config', 'user.name', 'Hook fixture')
        self.git('config', 'user.email', 'hook@example.invalid')
        self.git('config', 'commit.gpgsign', 'false')
        (self.root / 'tools').mkdir()
        (self.root / 'docs').mkdir()
        shutil.copy2(SOURCE, self.root / 'tools/install-hooks.sh')
        self.write_script('tools/journal.sh',
                          'echo called >> hook-called\necho generated > docs/JOURNAL.md')
        self.write_script('tools/todo.sh', 'echo generated > TODO.md')
        self.write_script('tools/doc-check.sh', 'echo agree')
        (self.root / 'TODO.md').write_text('original todo\n')
        (self.root / 'docs/JOURNAL.md').write_text('original journal\n')
        self.git('add', '.')
        self.git('commit', '-qm', 'fixture')
        subprocess.run(['bash', 'tools/install-hooks.sh'], cwd=self.root,
                       env=self.env, check=True, capture_output=True, timeout=5)

    def git(self, *args, cwd=None):
        return subprocess.run([GIT, *args], cwd=cwd or self.root, env=self.env,
                              check=True, capture_output=True, text=True, timeout=5)

    def write_script(self, relative, body):
        p = self.root / relative
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text('#!/bin/sh\n' + body + '\n')
        p.chmod(0o755)

    def commit(self, cwd=None):
        return self.git('commit', '--allow-empty', '-m', 'exercise hook', cwd=cwd)

    def assert_no_generation(self, root=None):
        self.assertFalse(((root or self.root) / 'hook-called').exists())

    def test_clean_managed_files_are_refreshed(self):
        self.commit()
        self.assertEqual((self.root / 'TODO.md').read_text(), 'generated\n')
        self.assertEqual((self.root / 'docs/JOURNAL.md').read_text(), 'generated\n')

    def test_pending_todo_preserves_both_files(self):
        (self.root / 'TODO.md').write_text('pending todo\n')
        self.commit()
        self.assertEqual((self.root / 'TODO.md').read_text(), 'pending todo\n')
        self.assertEqual((self.root / 'docs/JOURNAL.md').read_text(), 'original journal\n')
        self.assert_no_generation()

    def test_pending_journal_preserves_both_files(self):
        (self.root / 'docs/JOURNAL.md').write_text('pending journal\n')
        self.commit()
        self.assertEqual((self.root / 'docs/JOURNAL.md').read_text(), 'pending journal\n')
        self.assertEqual((self.root / 'TODO.md').read_text(), 'original todo\n')
        self.assert_no_generation()

    def test_untracked_managed_file_is_preserved(self):
        self.git('rm', '--cached', 'TODO.md')
        self.commit()
        self.assertEqual((self.root / 'TODO.md').read_text(), 'original todo\n')
        self.assert_no_generation()

    def test_shared_hook_checks_the_calling_worktree(self):
        linked = Path(self.temp.name) / 'linked'
        self.git('worktree', 'add', '-q', '-b', 'linked', str(linked))
        (linked / 'docs/JOURNAL.md').write_text('other worktree pending\n')
        self.commit(cwd=linked)
        self.assertEqual((linked / 'docs/JOURNAL.md').read_text(),
                         'other worktree pending\n')
        self.assertEqual((self.root / 'docs/JOURNAL.md').read_text(), 'original journal\n')
        self.assert_no_generation(linked)

    def test_unavailable_status_preserves_files(self):
        self.write_script('bin/git',
            'if [ "$1" = status ]; then exit 1; fi\nexec ' + shlex.quote(GIT) + ' "$@"')
        self.env['PATH'] = str(self.root / 'bin') + os.pathsep + self.env['PATH']
        # Git prepends its own exec path when launching a hook. Invoke the
        # installed hook directly for this controlled status-command failure.
        result = subprocess.run(['bash', '.git/hooks/post-commit'], cwd=self.root,
                                env=self.env, capture_output=True, text=True,
                                check=True, timeout=5)
        self.assertIn('could not read pending documentation', result.stdout + result.stderr)
        self.assertEqual((self.root / 'TODO.md').read_text(), 'original todo\n')
        self.assert_no_generation()


if __name__ == '__main__':
    unittest.main()
