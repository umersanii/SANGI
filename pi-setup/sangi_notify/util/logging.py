"""Shared logging setup."""

import logging
import sys
from pathlib import Path


def setup_logging(config: dict) -> None:
    """Configure logging from the config's 'logging' section."""
    log_config = config.get("logging", {})
    log_level = getattr(logging, log_config.get("level", "INFO"))
    log_file = log_config.get("file", "./logs/notification_monitor.log")

    # Create logs directory if needed
    log_dir = Path(log_file).parent
    log_dir.mkdir(parents=True, exist_ok=True)

    logging.basicConfig(
        level=log_level,
        format="%(asctime)s - %(name)s - %(levelname)s - %(message)s",
        handlers=[
            logging.FileHandler(log_file),
            logging.StreamHandler(sys.stdout),
        ],
    )
