"""Configuration loading and validation."""

import json
import sys
from pathlib import Path

REQUIRED_MQTT_KEYS = ["endpoint", "certificate_path", "private_key_path",
                      "root_ca_path"]


def load_config(path: str = "config.json") -> dict:
    p = Path(path)
    if not p.exists():
        print(f"ERROR: {path} not found. Copy config.json.template")
        sys.exit(1)

    try:
        with open(p) as f:
            config = json.load(f)
    except json.JSONDecodeError as e:
        print(f"ERROR: Invalid JSON in config file: {e}")
        sys.exit(1)

    # Validate MQTT section
    mqtt = config.get("mqtt", {})
    missing = [k for k in REQUIRED_MQTT_KEYS if k not in mqtt]
    if missing:
        print(f"ERROR: Missing MQTT config keys: {missing}")
        sys.exit(1)

    # Validate cert files exist
    for key in ["certificate_path", "private_key_path", "root_ca_path"]:
        cert_path = Path(mqtt[key])
        if not cert_path.exists():
            print(f"ERROR: Certificate not found: {cert_path}")
            sys.exit(1)

    return config
