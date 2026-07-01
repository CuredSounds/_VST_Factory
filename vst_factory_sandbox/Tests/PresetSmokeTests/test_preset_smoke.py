"""Preset smoke tests — Layer B from 07_PRESET_SMOKE_TESTING.md."""

from __future__ import annotations

import glob
import os
from pathlib import Path

import numpy as np
import pytest
import yaml

pytest.importorskip("dawdreamer")
import dawdreamer as daw  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
PLUGIN_PATH = ROOT / "build/ci/vst_factory_sandbox_artefacts/Release/VST3/VST_Factory_Sandbox.vst3"
MANIFEST_DIR = Path(__file__).resolve().parent


def _plugin_path() -> Path:
    if not PLUGIN_PATH.is_dir():
        pytest.skip(f"Built VST3 not found: {PLUGIN_PATH}")
    return PLUGIN_PATH


def _load_manifest(path: Path) -> dict:
    with open(path, encoding="utf-8") as f:
        return yaml.safe_load(f)


def _render(manifest: dict) -> np.ndarray:
    sr = manifest["sample_rate"]
    block = manifest["block_size"]
    engine = daw.RenderEngine(sr, block)
    plugin = engine.make_plugin_processor("effect", str(_plugin_path()))

    preset_file = MANIFEST_DIR / manifest["preset_file"]
    if preset_file.exists():
        plugin.load_state(str(preset_file))

    graph = [(plugin, [])]
    engine.load_graph(graph)
    duration = manifest["note_duration_sec"] + manifest["render_tail_sec"]
    engine.render(duration)
    return engine.get_audio()


@pytest.mark.parametrize(
    "manifest_path",
    sorted(glob.glob(str(MANIFEST_DIR / "*.smoketest.yaml"))),
)
def test_baseline_preset_smoke(manifest_path: str) -> None:
    manifest = _load_manifest(Path(manifest_path))
    audio = _render(manifest)

    assert not np.isnan(audio).any(), manifest.get("notes", "")
    assert not np.isinf(audio).any(), manifest.get("notes", "")

    rms = float(np.sqrt(np.mean(np.square(audio)) + 1e-12))
    rms_dbfs = 20.0 * np.log10(rms + 1e-12)
    peak_dbfs = 20.0 * np.log10(float(np.max(np.abs(audio))) + 1e-12)

    assertions = manifest["assertions"]
    assert rms_dbfs > assertions["min_rms_dbfs"], (
        f"{manifest_path}: near-silent ({rms_dbfs:.1f} dBFS). {manifest.get('notes', '')}"
    )
    assert peak_dbfs < assertions["max_peak_dbfs"], (
        f"{manifest_path}: clipping ({peak_dbfs:.1f} dBFS)"
    )
