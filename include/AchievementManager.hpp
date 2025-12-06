#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>

struct Achievement {
    std::string id;
    std::string title;
    std::string description;
    bool unlocked = false;
    bool hidden = false;
};

class AchievementManager {
public:
    static AchievementManager& Instance();

    void Init();
    void UnlockAchievement(const std::string& id);
    bool IsAchievementUnlocked(const std::string& id) const;
    const std::vector<Achievement>& GetAchievements() const;

    // Save/Load functionality (simple text file for now)
    void SaveAchievements();
    void LoadAchievements();

private:
    AchievementManager() = default;
    ~AchievementManager() = default;
    AchievementManager(const AchievementManager&) = delete;
    AchievementManager& operator=(const AchievementManager&) = delete;

    std::vector<Achievement> m_achievements;
    std::map<std::string, size_t> m_achievementMap; // ID to index
    
    const std::string SAVE_FILE = "achievements.dat";
};
