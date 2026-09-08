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
        self.probe = probe
        self.receipt = self.make_receipt(sleep=True)

    def make_receipt(self, *, sleep):
        fixture = self.root / ('sleep.bin' if sleep else 'exit.bin')
        fixture.write_bytes(self.probe.SLEEP_PROGRAM if sleep else self.probe.PROGRAM)
        receipt = fixture.with_suffix('.json')
        self.probe.write_receipt(str(receipt), 'synthetic transcript', str(fixture), sleep)
        return json.loads(receipt.read_text())

    def validate(self, receipt):
        feature.validate_user_process_exit_receipt(
            receipt, 'f' * 64, self.root, sleep=True)

    def test_current_sleep_receipt_contract(self):
        self.validate(self.receipt)

    def test_current_normal_exit_receipt_contract(self):
        feature.validate_user_process_exit_receipt(
            self.make_receipt(sleep=False), 'f' * 64, self.root)

    def test_each_gap_must_remain_exact(self):
        for sleep in (False, True):
            receipt = self.make_receipt(sleep=sleep)
            for index in range(len(receipt['known_gaps'])):
                for remove in (False, True):
                    with self.subTest(sleep=sleep, index=index, remove=remove):
                        mutant = copy.deepcopy(receipt)
                        if remove:
                            mutant['known_gaps'].pop(index)
                        else:
                            mutant['known_gaps'][index] = 'this obligation is fully proved'
                        with self.assertRaisesRegex(ValueError, 'known gaps'):
                            feature.validate_user_process_exit_receipt(
                                mutant, 'f' * 64, self.root, sleep=sleep)

    def test_keyword_stuffing_cannot_hide_scope_limits(self):
        mutant = copy.deepcopy(self.receipt)
        mutant['known_gaps'] = ['physical process-handle claim'] * 5
        with self.assertRaisesRegex(ValueError, 'known gaps'):
            self.validate(mutant)

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
