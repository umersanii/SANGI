"""Abstract base class for all notification monitors."""

from abc import ABC, abstractmethod
import logging
import threading


class Monitor(ABC):
    """Base class for all notification monitors."""

    def __init__(self, config: dict, name: str):
        self.config = config
        self.logger = logging.getLogger(name)
        self._stop_event = threading.Event()

    @abstractmethod
    def poll(self) -> None:
        """Execute one polling cycle."""
        ...

    @property
    @abstractmethod
    def interval(self) -> float:
        """Seconds between poll() calls."""
        ...

    def start(self) -> None:
        """Blocking polling loop. Call from a daemon thread."""
        self.logger.info(f"{self.__class__.__name__} started")
        while not self._stop_event.is_set():
            try:
                self.poll()
            except Exception:
                self.logger.exception("Error in poll cycle")
            self._stop_event.wait(self.interval)

    def stop(self) -> None:
        self._stop_event.set()
