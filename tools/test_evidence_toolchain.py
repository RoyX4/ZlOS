"""Join-contract regressions with synthetic manifests, never toolchain proof."""
import copy
import importlib.util
import json
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location(
    'evidence_registry', ROOT / 'kernel/tools/generators/gen-evidence-registry.py')
REGISTRY = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(REGISTRY)


class ToolchainJoinTests(unittest.TestCase):
    def setUp(self):
        self.manifest = json.loads(
            (ROOT / 'kernel/metadata/toolchain-manifest.json').read_text())

    def test_measured_header_closures_are_not_pinned_to_one_host(self):
        for count in (81, 82, 83, 84):
            with self.subTest(headers=count):
                value = copy.deepcopy(self.manifest)
                template = value['external_headers'][0]
                value['external_headers'] = [dict(template, path=f'/synthetic/{i}.h')
                                             for i in range(count)]
                value['counts']['external_headers'] = count
                self.assertEqual(REGISTRY.toolchain_manifest_counts(value),
                                 value['counts'])

    def test_invented_count_is_rejected(self):
        value = copy.deepcopy(self.manifest)
        value['external_headers'].pop()
        with self.assertRaises(ValueError):
            REGISTRY.toolchain_manifest_counts(value)

    def test_missing_header_identity_is_rejected(self):
        value = copy.deepcopy(self.manifest)
        value['external_headers'][0]['sha256'] = ''
        with self.assertRaises(ValueError):
            REGISTRY.toolchain_manifest_counts(value)

    def test_empty_closure_is_rejected(self):
        value = copy.deepcopy(self.manifest)
        value['external_headers'] = []
        value['counts']['external_headers'] = 0
        with self.assertRaises(ValueError):
            REGISTRY.toolchain_manifest_counts(value)

    def test_missing_tools_and_lanes_are_rejected(self):
        for field in ('tools', 'target_lanes'):
            with self.subTest(field=field):
                value = copy.deepcopy(self.manifest)
                value[field].pop()
                value['counts'][field] = len(value[field])
                with self.assertRaises(ValueError):
                    REGISTRY.toolchain_manifest_counts(value)

    def test_portability_and_environment_limits_remain_required(self):
        for field, replacement in (
            ('result', 'PASS_HERMETIC'), ('open_gaps', []),
            ('environment', {'forbidden_present': ['CPATH']}),
        ):
            with self.subTest(field=field):
                value = copy.deepcopy(self.manifest)
                value[field] = replacement
                with self.assertRaises(ValueError):
                    REGISTRY.toolchain_manifest_counts(value)


if __name__ == '__main__':
    unittest.main()
