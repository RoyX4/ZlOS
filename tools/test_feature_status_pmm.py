"""PMM feature-join regressions with synthetic evidence, never a boot claim."""
import copy
import hashlib
import json
from pathlib import Path
import tempfile
import unittest

import gen_feature_status as feature


class PmmFeatureReceiptTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='pmm-feature-test-')
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.receipt = json.loads((feature.ROOT / 'kernel/docs/receipts/'
            'physical-page-allocator-native-uefi64-qemu-2026-08-30.json').read_text())
        rows = [self.receipt['artifact'], self.receipt['harness'],
                *self.receipt['implementation'], self.receipt['host_receipt'],
                self.receipt['consumer_receipt'], self.receipt['generator']]
        for row in rows:
            path = self.root / row['path']
            path.parent.mkdir(parents=True, exist_ok=True)
            data = ((feature.ROOT / row['path']).read_bytes()
                    if row is self.receipt['generator'] else b'synthetic evidence\n')
            path.write_bytes(data)
            row['sha256'] = hashlib.sha256(data).hexdigest()

    def validate(self, receipt):
        feature.validate_pmm_receipt(receipt, receipt['build_identity'], self.root)

    def test_current_declared_limitations_are_accepted(self):
        self.validate(self.receipt)

    def test_each_missing_limitation_is_rejected(self):
        for index in range(len(self.receipt['known_gaps'])):
            with self.subTest(index=index):
                mutant = copy.deepcopy(self.receipt)
                mutant['known_gaps'].pop(index)
                with self.assertRaisesRegex(ValueError, 'hides its open gaps'):
                    self.validate(mutant)

    def test_same_count_falsified_limitations_are_rejected(self):
        for index in range(len(self.receipt['known_gaps'])):
            with self.subTest(index=index):
                mutant = copy.deepcopy(self.receipt)
                mutant['known_gaps'][index] = 'consumer and physical proof are complete'
                with self.assertRaisesRegex(ValueError, 'hides its open gaps'):
                    self.validate(mutant)

    def test_non_list_limitations_are_rejected(self):
        mutant = copy.deepcopy(self.receipt)
        mutant['known_gaps'] = {'consumer': 'physical'}
        with self.assertRaisesRegex(ValueError, 'hides its open gaps'):
            self.validate(mutant)

    def test_generator_identity_remains_enforced(self):
        (self.root / self.receipt['generator']['path']).write_text('KNOWN_GAPS = ()\n')
        with self.assertRaisesRegex(ValueError, 'identity drifted'):
            self.validate(self.receipt)

    def test_writer_contract_is_read_without_executing_it(self):
        row = self.receipt['generator']
        path = self.root / row['path']
        data = path.read_bytes() + b'\nraise RuntimeError("must not execute evidence")\n'
        path.write_bytes(data)
        row['sha256'] = hashlib.sha256(data).hexdigest()
        self.validate(self.receipt)


if __name__ == '__main__':
    unittest.main()
