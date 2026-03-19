"""Entry point: python -m sangi_notify"""

from sangi_notify.config import load_config
from sangi_notify.service import NotificationService


def main():
    config = load_config()
    service = NotificationService(config)
    service.start()


if __name__ == "__main__":
    main()
