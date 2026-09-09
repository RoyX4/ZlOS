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

    def test_missing_or_changed_boot_validator_cannot_reuse_receipt(self):
        for relative in ('kernel/tools/probes/exercise.py',
                         'kernel/tools/checks/write-user-process-receipt.py',
                         'kernel/tools/checks/write-scheduler-receipt.py'):
            with self.subTest(path=relative):
                mutant = copy.deepcopy(self.receipt)
                mutant['implementation'] = [row for row in mutant['implementation']
                                            if row['path'] != relative]
                with self.assertRaisesRegex(ValueError, 'identity drifted'):
                    self.validate(mutant)
                path = self.root / relative
                original = path.read_bytes()
                path.write_bytes(original + b'\n# changed boot admission\n')
                with self.assertRaisesRegex(ValueError, 'identity drifted'):
                    self.validate(self.receipt)
                path.write_bytes(original)


class UserProcessBoundaryReceiptTests(unittest.TestCase):
    """Synthetic source-bound records test the producer/consumer contract."""
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='process-boundary-receipt-')
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        spec = importlib.util.spec_from_file_location('boundary_receipt_fixture',
            feature.ROOT / 'kernel/tools/checks/write-user-process-receipt.py')
        producer = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(producer)
        receipt = json.loads(producer.DEFAULT_OUTPUT.read_text())
        receipt['build_identity'] = 'f' * 64
        receipt['implementation'] = producer.expected_files()
        receipt['source_contracts'] = copy.deepcopy(producer.SOURCE_CONTRACTS)
        receipt['assertions'] = copy.deepcopy(list(producer.ASSERTIONS))
        guarded = next(row for row in receipt['assertions']
                       if row['id'] == 'guarded-supervisor-tss-stacks')
        guarded['observed_high_water_bytes'] = {'slot0': 512, 'slot1': 512}
        for row in [receipt['artifact'], receipt['harness'], receipt['generator'],
                    receipt['host_receipt'], *receipt['implementation']]:
            path = self.root / row['path']
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes((feature.ROOT / row['path']).read_bytes()
                             if row['path'].endswith('user_syscalls.json')
                             else ('synthetic evidence for ' + row['path']).encode())
            row['sha256'] = feature.digest(path)
        self.receipt = receipt

    def validate(self, receipt):
        feature.validate_user_process_receipt(receipt, 'f' * 64, self.root)

    def test_current_producer_contract_is_accepted(self):
        self.validate(self.receipt)

    def test_stale_syscall_range_or_admitted_unknown_probe_is_rejected(self):
        for stale_range in (True, False):
            with self.subTest(stale_range=stale_range):
                mutant = copy.deepcopy(self.receipt)
                if stale_range:
                    mutant['source_contracts']['syscall_numbers']['last'] -= 2
                else:
                    mutant['assertions'][1]['probes'][1] -= 2
                with self.assertRaises(ValueError):
                    self.validate(mutant)

    def test_missing_spawn_loader_binding_is_rejected(self):
        for relative in ('kernel/src/arch/x86/user_image64.c',
                         'kernel/src/arch/x86/user_image64.h',
                         'kernel/src/arch/x86/user_process_abi.h',
                         'kernel/tests/host/userspawnwaittest.c',
                         'kernel/tools/generators/gen-user-syscalls.py'):
            with self.subTest(path=relative):
                mutant = copy.deepcopy(self.receipt)
                mutant['implementation'] = [row for row in mutant['implementation']
                                            if row['path'] != relative]
                with self.assertRaisesRegex(ValueError, 'identity drifted'):
                    self.validate(mutant)


if __name__ == '__main__':
    unittest.main()
