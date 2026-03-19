"""Discord message monitor — selfbot approach for DMs and mentions."""

import asyncio
import logging
from datetime import datetime, timezone
from typing import Callable, Dict, List, Optional

import discord

from sangi_notify.monitors.base import Monitor


class DiscordMonitor(Monitor):
    """Monitor Discord messages for personal account.

    WARNING: Using user tokens (selfbot) violates Discord ToS.
    """

    def __init__(self, config: dict, callback: Callable[[dict], None]):
        super().__init__(config, "DiscordMonitor")

        dc_config = config.get("discord_messages", {})
        self.enabled = dc_config.get("enabled", False)
        self.token = dc_config.get("user_token", "")
        self.poll_interval = dc_config.get("poll_interval", 300)
        self.monitor_dms = dc_config.get("monitor_dms", True)
        self.monitor_mentions = dc_config.get("monitor_mentions", True)

        self.callback = callback
        self.client: Optional[discord.Client] = None
        self.last_check_time: Optional[datetime] = None
        self.message_buffer: List[Dict] = []
        self.running = False

        if not self.enabled:
            self.logger.warning("Discord message monitoring is disabled in config")
        elif not self.token:
            self.logger.error("Discord user token not configured")
            self.enabled = False
        else:
            self.logger.info("DiscordMonitor initialized")
            self.logger.warning("Using Discord selfbot - this violates Discord ToS!")

    @property
    def interval(self) -> float:
        return self.poll_interval

    def poll(self) -> None:
        # Not used — Discord uses its own async event loop via start() override.
        pass

    def start(self) -> None:
        """Override base start() to run the async Discord client."""
        if not self.enabled:
            self.logger.warning("Discord message monitoring disabled, not starting")
            return

        self.logger.info("Starting Discord message monitoring...")
        try:
            asyncio.run(self._run_client())
        except KeyboardInterrupt:
            self.logger.info("Discord monitoring stopped by user")
        except Exception as e:
            self.logger.error(f"Error starting Discord monitoring: {e}", exc_info=True)
            raise

    async def _run_client(self):
        try:
            await self._setup_client()
            asyncio.create_task(self._periodic_check_loop())
            await self.client.start(self.token)
        except discord.LoginFailure:
            self.logger.error("Failed to login to Discord - invalid token")
            self.running = False
        except Exception as e:
            self.logger.error(f"Discord client error: {e}", exc_info=True)
            self.running = False

    async def _setup_client(self):
        self.client = discord.Client()

        @self.client.event
        async def on_ready():
            self.logger.info(f"Discord monitor connected as {self.client.user.name}#{self.client.user.discriminator}")
            self.running = True
            self.last_check_time = datetime.now(timezone.utc)

        @self.client.event
        async def on_message(message):
            if message.author.id == self.client.user.id:
                return

            is_dm = isinstance(message.channel, discord.DMChannel)
            is_mentioned = self.client.user.mentioned_in(message)

            should_track = False
            message_type = None

            if is_dm and self.monitor_dms:
                should_track = True
                message_type = "DM"
            elif is_mentioned and self.monitor_mentions:
                should_track = True
                message_type = "Mention"

            if should_track:
                self.message_buffer.append({
                    "id": message.id,
                    "type": message_type,
                    "author": str(message.author),
                    "author_id": message.author.id,
                    "content": message.content[:200],
                    "timestamp": message.created_at,
                    "channel": str(message.channel),
                    "channel_id": message.channel.id,
                    "guild": str(message.guild) if message.guild else None,
                })
                self.logger.info(f"Captured Discord {message_type} from {message.author}: {message.content[:50]}...")

        @self.client.event
        async def on_error(event, *args, **kwargs):
            self.logger.error(f"Discord client error in {event}", exc_info=True)

    async def _periodic_check_loop(self):
        await self.client.wait_until_ready()
        self.logger.info(f"Starting periodic Discord message checks (interval: {self.poll_interval}s)")
        try:
            while not self.client.is_closed():
                await asyncio.sleep(self.poll_interval)
                await self._check_and_report()
        except asyncio.CancelledError:
            self.logger.info("Periodic check loop cancelled")
        except Exception as e:
            self.logger.error(f"Error in periodic check loop: {e}", exc_info=True)

    async def _check_and_report(self):
        try:
            new_messages = [m for m in self.message_buffer
                           if self.last_check_time and m["timestamp"] > self.last_check_time]
            if new_messages:
                stats = self._calculate_message_stats(new_messages)
                if stats:
                    self.logger.info(f"Discord message stats: {stats['total_new_messages']} new "
                                     f"({stats['dms']} DMs, {stats['mentions']} mentions)")
                    self.callback(stats)

            self.last_check_time = datetime.now(timezone.utc)

            if len(self.message_buffer) > 100:
                self.message_buffer = self.message_buffer[-100:]
        except Exception as e:
            self.logger.error(f"Error checking Discord messages: {e}", exc_info=True)

    def _calculate_message_stats(self, messages: List[Dict]) -> Optional[Dict]:
        if not messages:
            return None
        stats = {
            "username": str(self.client.user) if self.client else "Unknown",
            "user_id": self.client.user.id if self.client else 0,
            "timestamp": datetime.now(timezone.utc).isoformat(),
            "total_new_messages": len(messages),
            "dms": len([m for m in messages if m["type"] == "DM"]),
            "mentions": len([m for m in messages if m["type"] == "Mention"]),
            "unique_senders": len(set(m["author_id"] for m in messages)),
            "messages": [],
        }
        for msg in messages[-5:]:
            stats["messages"].append({
                "type": msg["type"],
                "from": msg["author"],
                "preview": msg["content"][:100],
                "channel": msg["channel"],
                "time": msg["timestamp"].isoformat(),
            })
        return stats

    async def async_stop(self):
        if self.client:
            await self.client.close()
        self.running = False
        self.logger.info("Discord monitoring stopped")
