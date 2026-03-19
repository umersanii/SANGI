#include "github_data.h"

GitHubDataStore::GitHubDataStore() {
  contributions_.dataLoaded = false;
  contributions_.totalContributions = 0;
  contributions_.currentStreak = 0;
  contributions_.longestStreak = 0;
  memset(contributions_.username, 0, sizeof(contributions_.username));
  memset(contributions_.contributions, 0, sizeof(contributions_.contributions));

  stats_.dataLoaded = false;
  stats_.repos = 0;
  stats_.followers = 0;
  stats_.following = 0;
  stats_.contributions = 0;
  stats_.commits = 0;
  stats_.prs = 0;
  stats_.issues = 0;
  stats_.stars = 0;
  stats_.timestamp = 0;
  memset(stats_.username, 0, sizeof(stats_.username));
}

void GitHubDataStore::clearContributions() {
  contributions_.dataLoaded = false;
  contributions_.totalContributions = 0;
  contributions_.currentStreak = 0;
  contributions_.longestStreak = 0;
  memset(contributions_.username, 0, sizeof(contributions_.username));
  memset(contributions_.contributions, 0, sizeof(contributions_.contributions));
  Serial.println("GitHub contribution data cleared");
}

void GitHubDataStore::setContributions(const uint8_t contributions[52][7], int total,
                                        int streak, int longest, const char* user) {
  memcpy(contributions_.contributions, contributions, sizeof(contributions_.contributions));

  contributions_.totalContributions = total;
  contributions_.currentStreak = streak;
  contributions_.longestStreak = longest;

  if (user) {
    strncpy(contributions_.username, user, 31);
    contributions_.username[31] = '\0';
  } else {
    strcpy(contributions_.username, "user");
  }

  contributions_.dataLoaded = true;

  Serial.printf("GitHub data loaded: %d contributions, %d day streak\n", total, streak);
}

GitHubContributionData* GitHubDataStore::getContributions() {
  if (!contributions_.dataLoaded) {
    return nullptr;
  }
  return &contributions_;
}

void GitHubDataStore::setStats(const char* user, int repos, int followers, int following,
                                int contributions, int commits, int prs, int issues, int stars) {
  if (user) {
    strncpy(stats_.username, user, 31);
    stats_.username[31] = '\0';
  } else {
    strcpy(stats_.username, "user");
  }

  stats_.repos = repos;
  stats_.followers = followers;
  stats_.following = following;
  stats_.contributions = contributions;
  stats_.commits = commits;
  stats_.prs = prs;
  stats_.issues = issues;
  stats_.stars = stars;
  stats_.timestamp = millis();
  stats_.dataLoaded = true;

  Serial.printf("GitHub stats loaded: %s - %d repos, %d followers, %d contributions\n",
                user, repos, followers, contributions);
}

const GitHubStatsData* GitHubDataStore::getStats() const {
  if (!stats_.dataLoaded) {
    return nullptr;
  }
  return &stats_;
}
