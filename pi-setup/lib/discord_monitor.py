"""
Discord message monitor for personal account (umersanii)
Monitors DMs and mentions, sends periodic notifications every 5 minutes
"""

import logging
import time
import discord
import asyncio
from datetime import datetime, timezone
from typing import Dict, List, Optional


class DiscordMessageMonitor:
    """Monitor Discord messages for personal account using selfbot approach"""
    
    def __init__(self, config, callback):
        """
        Initialize Discord message monitor
        
        Args:
            config: Configuration dictionary with Discord settings
            callback: Function to call when new messages detected (stats_dict)
        
        WARNING: Using user tokens (selfbot) violates Discord ToS.
                 Use at your own risk. Consider using a bot account instead.
        """
        self.config = config
        self.callback = callback
        self.logger = logging.getLogger('DiscordMessageMonitor')
        
        discord_config = config.get('discord_messages', {})
        self.enabled = discord_config.get('enabled', False)
        self.token = discord_config.get('user_token', '')
        self.poll_interval = discord_config.get('poll_interval', 300)  # 5 minutes default
        self.monitor_dms = discord_config.get('monitor_dms', True)
        self.monitor_mentions = discord_config.get('monitor_mentions', True)
        
        self.client: Optional[discord.Client] = None
        self.last_check_time = None
        self.message_buffer: List[Dict] = []
        self.running = False
        
        if not self.enabled:
            self.logger.warning("Discord message monitoring is disabled in config")
        elif not self.token:
            self.logger.error("Discord user token not configured")
            self.enabled = False
        else:
            self.logger.info("DiscordMessageMonitor initialized")
            self.logger.warning("Using Discord selfbot - this violates Discord ToS!")
    
    async def _setup_client(self):
        """Setup Discord client with event handlers"""
        # Use selfbot-compatible client
        self.client = discord.Client()
        
        @self.client.event
        async def on_ready():
            self.logger.info(f"Discord monitor connected as {self.client.user.name}#{self.client.user.discriminator}")
            self.logger.info(f"User ID: {self.client.user.id}")
            self.running = True
            self.last_check_time = datetime.now(timezone.utc)
        
        @self.client.event
        async def on_message(message):
            # Ignore own messages
            if message.author.id == self.client.user.id:
                return
            
            # Check if it's a DM
            is_dm = isinstance(message.channel, discord.DMChannel)
            
            # Check if mentioned
            is_mentioned = self.client.user.mentioned_in(message)
            
            # Only track DMs or mentions based on config
            should_track = False
            message_type = None
            
            if is_dm and self.monitor_dms:
                should_track = True
                message_type = 'DM'
            elif is_mentioned and self.monitor_mentions:
                should_track = True
                message_type = 'Mention'
            
            if should_track:
                message_data = {
                    'id': message.id,
                    'type': message_type,
                    'author': str(message.author),
                    'author_id': message.author.id,
                    'content': message.content[:200],  # First 200 chars
                    'timestamp': message.created_at,
                    'channel': str(message.channel),
                    'channel_id': message.channel.id,
                    'guild': str(message.guild) if message.guild else None
                }
                
                self.message_buffer.append(message_data)
                self.logger.info(f"Captured Discord {message_type} from {message.author}: {message.content[:50]}...")
        
        @self.client.event
        async def on_error(event, *args, **kwargs):
            self.logger.error(f"Discord client error in {event}", exc_info=True)
    
    def _get_new_messages_since_last_check(self) -> List[Dict]:
        """Get messages received since last check"""
        if not self.last_check_time:
            return []
        
        new_messages = [
            msg for msg in self.message_buffer
            if msg['timestamp'] > self.last_check_time
        ]
        
        return new_messages
    
    def _calculate_message_stats(self, messages: List[Dict]) -> Dict:
        """Calculate statistics from messages"""
        if not messages:
            return None
        
        stats = {
            'username': str(self.client.user) if self.client else 'Unknown',
            'user_id': self.client.user.id if self.client else 0,
            'timestamp': datetime.now(timezone.utc).isoformat(),
            'total_new_messages': len(messages),
            'dms': len([m for m in messages if m['type'] == 'DM']),
            'mentions': len([m for m in messages if m['type'] == 'Mention']),
            'unique_senders': len(set(m['author_id'] for m in messages)),
            'messages': []
        }
        
        # Add preview of recent messages (last 5)
        for msg in messages[-5:]:
            stats['messages'].append({
                'type': msg['type'],
                'from': msg['author'],
                'preview': msg['content'][:100],
                'channel': msg['channel'],
                'time': msg['timestamp'].isoformat()
            })
        
        return stats
    
    async def _check_and_report(self):
        """Check for new messages and report via callback"""
        try:
            # Get new messages since last check
            new_messages = self._get_new_messages_since_last_check()
            
            if new_messages:
                self.logger.info(f"Found {len(new_messages)} new messages since last check")
                
                # Calculate stats
                stats = self._calculate_message_stats(new_messages)
                
                if stats:
                    self.logger.info(f"Discord message stats: {stats['total_new_messages']} new "
                                   f"({stats['dms']} DMs, {stats['mentions']} mentions)")
                    
                    # Call callback with stats
                    self.callback(stats)
            else:
                self.logger.debug("No new Discord messages since last check")
            
            # Update last check time
            self.last_check_time = datetime.now(timezone.utc)
            
            # Cleanup old messages (keep last 100)
            if len(self.message_buffer) > 100:
                self.message_buffer = self.message_buffer[-100:]
            
        except Exception as e:
            self.logger.error(f"Error checking Discord messages: {e}", exc_info=True)
    
    async def _periodic_check_loop(self):
        """Periodic check loop for reporting (runs while client is connected)"""
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
    
    async def _run_client(self):
        """Run Discord client"""
        try:
            await self._setup_client()
            
            # Start periodic check loop as a background task
            asyncio.create_task(self._periodic_check_loop())
            
            # Run client (blocking) - discord.py-self doesn't use bot parameter
            await self.client.start(self.token)
            
        except discord.LoginFailure:
            self.logger.error("Failed to login to Discord - invalid token")
            self.running = False
        except Exception as e:
            self.logger.error(f"Discord client error: {e}", exc_info=True)
            self.running = False
    
    def start_monitoring(self):
        """Start Discord message monitoring (blocking)"""
        if not self.enabled:
            self.logger.warning("Discord message monitoring disabled, not starting")
            return
        
        self.logger.info("Starting Discord message monitoring...")
        self.logger.warning("⚠️  Using Discord selfbot violates Discord Terms of Service")
        
        try:
            # Run async client in event loop
            asyncio.run(self._run_client())
            
        except KeyboardInterrupt:
            self.logger.info("Discord monitoring stopped by user")
        except Exception as e:
            self.logger.error(f"Error starting Discord monitoring: {e}", exc_info=True)
            raise
    
    async def stop(self):
        """Stop Discord monitoring"""
        if self.client:
            await self.client.close()
        self.running = False
        self.logger.info("Discord monitoring stopped")
    
    def get_stats(self) -> Optional[Dict]:
        """Get current message statistics"""
        if not self.message_buffer:
            return None
        
        return self._calculate_message_stats(self.message_buffer)
