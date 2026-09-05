"""Exercise TODO publication without builds, GitHub access, or repository edits."""
import os
from pathlib import Path
import shutil
import signal
import subprocess
import tempfile
import time
import unittest

SOURCE = Path(__file__).with_name('todo.sh')
ORIGINAL = '# Existing backlog\n<!-- BEGIN HAND-WRITTEN -->\nKeep this task.\n<!-- END HAND-WRITTEN -->\n'


class TodoTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='todo-test-')
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / 'tools').mkdir()
        (self.root / 'bin').mkdir()
        shutil.copy2(SOURCE, self.root / 'tools/todo.sh')
        self.out = self.root / 'TODO.md'
        self.out.write_text(ORIGINAL)
        self.env = dict(os.environ, PATH=f"{self.root / 'bin'}:{os.environ['PATH']}")
        self.stub('git', 'exit 0')
        self.stub('gh', 'exit 0')
        self.stub('clang', 'exit 0')
        self.hazard('echo 0')

    def stub(self, name, body):
        path = self.root / 'bin' / name
        path.write_text('#!/bin/sh\n' + body + '\n')
        path.chmod(0o755)

    def hazard(self, body):
        path = self.root / 'tools/hazard-scan.sh'
        path.write_text('#!/bin/sh\n' + body + '\n')
        path.chmod(0o755)

    def run_todo(self, *args):
        return subprocess.run(['bash', str(self.root / 'tools/todo.sh'), *args],
                              env=self.env, capture_output=True, text=True, timeout=10)

    def assert_preserved(self):
        self.assertEqual(self.out.read_text(), ORIGINAL)
        self.assertEqual(list(self.root.glob('.TODO.md.*')), [])

    def test_interrupted_generation_preserves_backlog(self):
        self.hazard('touch started\nexec sleep 30')
        proc = subprocess.Popen(['bash', str(self.root / 'tools/todo.sh')],
                                env=self.env, stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE, start_new_session=True)
        try:
            deadline = time.monotonic() + 5
            while not (self.root / 'started').exists():
                if proc.poll() is not None or time.monotonic() > deadline:
                    self.fail('generator never reached the controlled interruption')
                time.sleep(0.01)
            os.killpg(proc.pid, signal.SIGTERM)
            proc.communicate(timeout=5)
            self.assertNotEqual(proc.returncode, 0)
            self.assert_preserved()
        finally:
            if proc.poll() is None:
                os.killpg(proc.pid, signal.SIGKILL)
                proc.communicate()

    def test_success_preserves_handwritten_block_and_mode(self):
        self.out.chmod(0o640)
        result = self.run_todo()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn(ORIGINAL.split('\n', 1)[1], self.out.read_text())
        self.assertEqual(self.out.stat().st_mode & 0o777, 0o640)
        self.assertEqual(list(self.root.glob('.TODO.md.*')), [])

    def test_stdout_does_not_write(self):
        result = self.run_todo('--stdout')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('Keep this task.', result.stdout)
        self.assert_preserved()

    def test_first_generation_creates_handwritten_markers(self):
        self.out.unlink()
        result = self.run_todo()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('<!-- BEGIN HAND-WRITTEN -->', self.out.read_text())
        self.assertIn('<!-- END HAND-WRITTEN -->', self.out.read_text())

    def test_hard_kill_preserves_backlog(self):
        self.hazard('kill -KILL "$TODO_GENERATOR_PID"\necho 0')
        result = subprocess.run(
            ['bash', '-c', 'export TODO_GENERATOR_PID=$$; exec bash "$1"',
             'todo-test', str(self.root / 'tools/todo.sh')],
            env=self.env, capture_output=True, text=True, timeout=10)
        self.assertEqual(result.returncode, -signal.SIGKILL)
        self.assertEqual(self.out.read_text(), ORIGINAL)

    def test_permission_copy_failure_preserves_backlog(self):
        self.stub('chmod', 'exit 1')
        result = self.run_todo()
        self.assertNotEqual(result.returncode, 0)
        self.assert_preserved()

    def test_generation_write_failure_preserves_backlog(self):
        (self.root / '.ultra').mkdir()
        (self.root / '.ultra/TENSIONS.md').write_text(
            ''.join(f'## T-{i} — unfinished item. OPEN.\n' for i in range(100)))
        result = subprocess.run(
            ['bash', '-c', 'trap "" XFSZ; ulimit -f 1; exec bash "$1"',
             'todo-test', str(self.root / 'tools/todo.sh')],
            env=self.env, capture_output=True, text=True, timeout=10)
        self.assertNotEqual(result.returncode, 0)
        self.assertNotIn('wrote TODO.md', result.stdout)
        self.assert_preserved()

    def test_failed_publish_is_reported(self):
        self.stub('mv', 'exit 1')
        result = self.run_todo()
        self.assertNotEqual(result.returncode, 0)
        self.assertNotIn('wrote TODO.md', result.stdout)
        self.assert_preserved()

    def test_failed_temp_creation_is_reported(self):
        self.stub('mktemp', 'exit 1')
        result = self.run_todo()
        self.assertNotEqual(result.returncode, 0)
        self.assert_preserved()


if __name__ == '__main__':
    unittest.main()
