#ifndef GITHUB_DATA_H
#define GITHUB_DATA_H

#include <Arduino.h>

// ===== GITHUB CONTRIBUTION STRUCTURE =====
// Stores GitHub contribution heat map data (52 weeks x 7 days)
struct GitHubContributionData {
  uint8_t contributions[52][7];  // 52 weeks x 7 days grid (0-4+ scale)
  int totalContributions;        // Total count for the year
  int currentStreak;             // Current consecutive days
  int longestStreak;             // Longest streak this year
  char username[32];             // GitHub username
  bool dataLoaded;               // Data is valid
};

// ===== GITHUB STATS STRUCTURE =====
// Stores current GitHub profile statistics (from GitHub API)
struct GitHubStatsData {
  char username[32];             // GitHub username
  int repos;                     // Public repositories
  int followers;                 // Follower count
  int following;                 // Following count
  int contributions;             // Total contributions (last year)
  int commits;                   // Total commits (last year)
  int prs;                       // Total pull requests
  int issues;                    // Total issues
  int stars;                     // Total stars received
  unsigned long timestamp;       // Last update time
  bool dataLoaded;               // Data is valid
};

// ===== GITHUB DATA STORE =====
class GitHubDataStore {
public:
  GitHubDataStore();

  void setContributions(const uint8_t contributions[52][7], int total,
                        int streak, int longest, const char* user);
  void setStats(const char* user, int repos, int followers, int following,
                int contributions, int commits, int prs, int issues, int stars);

  const GitHubContributionData* getContributions() const;
  const GitHubStatsData* getStats() const;
  bool hasContributions() const { return contributions_.dataLoaded; }
  bool hasStats() const { return stats_.dataLoaded; }
  void clearContributions();

private:
  GitHubContributionData contributions_;
  GitHubStatsData stats_;
};

#endif // GITHUB_DATA_H
