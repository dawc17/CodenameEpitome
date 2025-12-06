#include "AchievementManager.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>

AchievementManager& AchievementManager::Instance() {
    static AchievementManager instance;
    return instance;
}

void AchievementManager::Init() {
    // Define achievements here
    m_achievements = {
        {"FIRST_BLOOD", "First Blood", "Kill your first enemy."},
        {"SURVIVOR", "Survivor", "Clear the first floor."},
        {"BIG_SPENDER", "Big Spender", "Spend 100 run currency in a single run."}, // Note: Tracking "single run" might need more logic, simplifying to just spending for now or triggering externally
        {"HOARDER", "Hoarder", "Accumulate 500 run currency."},
        {"VETERAN", "Veteran", "Die 10 times."} // Example of a counter-based one, would need external tracking
    };

    // Build map for fast lookup
    for (size_t i = 0; i < m_achievements.size(); ++i) {
        m_achievementMap[m_achievements[i].id] = i;
    }

    LoadAchievements();
}

void AchievementManager::UnlockAchievement(const std::string& id) {
    auto it = m_achievementMap.find(id);
    if (it != m_achievementMap.end()) {
        Achievement& achievement = m_achievements[it->second];
        if (!achievement.unlocked) {
            achievement.unlocked = true;
            std::cout << "ACHIEVEMENT UNLOCKED: " << achievement.title << " - " << achievement.description << std::endl;
            // TODO: Add a UI notification here
            SaveAchievements();
        }
    }
}

bool AchievementManager::IsAchievementUnlocked(const std::string& id) const {
    auto it = m_achievementMap.find(id);
    if (it != m_achievementMap.end()) {
        return m_achievements[it->second].unlocked;
    }
    return false;
}

const std::vector<Achievement>& AchievementManager::GetAchievements() const {
    return m_achievements;
}

void AchievementManager::SaveAchievements() {
    std::ofstream file(SAVE_FILE);
    if (file.is_open()) {
        for (const auto& achievement : m_achievements) {
            if (achievement.unlocked) {
                file << achievement.id << "\n";
            }
        }
        file.close();
    }
}

void AchievementManager::LoadAchievements() {
    std::ifstream file(SAVE_FILE);
    if (file.is_open()) {
        std::string id;
        while (std::getline(file, id)) {
            auto it = m_achievementMap.find(id);
            if (it != m_achievementMap.end()) {
                m_achievements[it->second].unlocked = true;
            }
        }
        file.close();
    }
}
