"""Receipt rejection tests using synthetic files, never target execution proof."""
import copy
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

import gen_feature_status as feature


class SleepFeatureReceiptTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='sleep-feature-test-')
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        source = feature.ROOT / 'kernel/tools/probes/probe-user-process-exit.py'
        spec = importlib.util.spec_from_file_location('sleep_probe_fixture', source)
        probe = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(probe)
        for relative in probe.SOURCE_FILES:
            path = self.root / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes((feature.ROOT / relative).read_bytes())
        identity = self.root / 'kernel/metadata/build-identity.json'
        identity.parent.mkdir(parents=True, exist_ok=True)
        identity.write_text(json.dumps({'identity_sha256': 'f' * 64}))
        (self.root / 'kernel/zlOS-usb.img').write_bytes(b'synthetic build bytes\n')
        probe.REPO_ROOT = str(self.root)
        probe.KERNEL_ROOT = str(self.root / 'kernel')
        fixture = self.root / 'user.bin'
        fixture.write_bytes(probe.SLEEP_PROGRAM)
        receipt = self.root / 'sleep.json'
        probe.write_receipt(str(receipt), 'synthetic transcript', str(fixture), True)
        self.receipt = json.loads(receipt.read_text())

    def validate(self, receipt):
        feature.validate_user_process_exit_receipt(
            receipt, 'f' * 64, self.root, sleep=True)

    def test_current_sleep_receipt_contract(self):
        self.validate(self.receipt)

    def test_removed_sleep_or_early_wake_claim_is_rejected(self):
        for field, value in [('minimum_guest_sleep_ticks', 0),
                             ('content_hex', '90'), ('expected_exit_status', 0)]:
            with self.subTest(field=field):
                mutant = copy.deepcopy(self.receipt)
                mutant['fixture'][field] = value
                with self.assertRaises(ValueError):
                    self.validate(mutant)

    def test_missing_sleep_observation_is_rejected(self):
        mutant = copy.deepcopy(self.receipt)
        mutant['assertions'].pop(2)
        with self.assertRaises(ValueError):
            self.validate(mutant)

    def test_plain_exit_receipt_cannot_prove_sleep(self):
        mutant = copy.deepcopy(self.receipt)
        mutant['schema'] = 'zlos.user-process-exit-native-uefi64-qemu-receipt.v1'
        mutant['result'] = 'PASS_EXTERNAL_FILE_SPAWN_EXIT_OBSERVE_REAP'
        with self.assertRaises(ValueError):
            self.validate(mutant)

    def test_foreign_service_cannot_reuse_receipt(self):
        (self.root / 'kernel/src/core/user_process_service.c').write_text('changed')
        with self.assertRaisesRegex(ValueError, 'identity drifted'):
            self.validate(self.receipt)


if __name__ == '__main__':
    unittest.main()
