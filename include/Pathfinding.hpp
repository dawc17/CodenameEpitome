#pragma once

#include "raylib.h"
#include <vector>
#include <queue>
#include <unordered_map>
#include <functional>
#include <memory>
#include <cmath>

class Room;

// Forward declarations
class Path;
class Seeker;
class PathModifier;

// ============================================================================
// Path Result - Similar to Unity's Path class
// ============================================================================
class Path {
public:
    std::vector<Vector2> vectorPath;  // World positions to follow
    bool error = false;
    std::string errorMessage;
    
    bool IsComplete() const { return !error && !vectorPath.empty(); }
    int GetWaypointCount() const { return static_cast<int>(vectorPath.size()); }
};

// ============================================================================
// Traversal Provider - Controls which nodes can be traversed and their costs
// Similar to Unity's ITraversalProvider
// ============================================================================
class ITraversalProvider {
public:
    virtual ~ITraversalProvider() = default;
    
    // Check if a tile can be traversed
    virtual bool CanTraverse(Room* room, int x, int y) const = 0;
    
    // Get the cost to traverse a tile (higher = less desirable)
    virtual float GetTraversalCost(Room* room, int x, int y) const = 0;
};

// Default traversal provider - just checks walkability
class DefaultTraversalProvider : public ITraversalProvider {
public:
    bool CanTraverse(Room* room, int x, int y) const override;
    float GetTraversalCost(Room* room, int x, int y) const override { return 1.0f; }
};

// Weighted traversal provider - adds penalties for certain areas
class WeightedTraversalProvider : public ITraversalProvider {
public:
    // Add a penalty zone at world position with radius
    void AddPenaltyZone(Vector2 worldPos, float radius, float penalty);
    void ClearPenaltyZones();
    
    bool CanTraverse(Room* room, int x, int y) const override;
    float GetTraversalCost(Room* room, int x, int y) const override;
    
private:
    struct PenaltyZone {
        Vector2 worldPos;
        float radius;
        float penalty;
    };
    std::vector<PenaltyZone> m_penaltyZones;
};

// ============================================================================
// Path Modifier - Post-processes paths for variety/smoothing
// Similar to Unity's AlternativePath modifier
// ============================================================================
class PathModifier {
public:
    virtual ~PathModifier() = default;
    virtual void Apply(Path& path) = 0;
};

// Smooths the path by removing unnecessary waypoints
class PathSmoother : public PathModifier {
public:
    void Apply(Path& path) override;
};

// Adds slight randomization to prevent all enemies from taking identical paths
class AlternativePathModifier : public PathModifier {
public:
    float randomOffset = 10.0f;  // Max random offset in world units
    void Apply(Path& path) override;
};

// ============================================================================
// Pathfinder Configuration - Similar to AstarPath settings
// ============================================================================
struct PathfinderConfig {
    float heuristicScale = 1.0f;       // A* heuristic weight (1.0 = balanced)
    bool allowDiagonal = true;          // Allow 8-directional movement
    bool cutCorners = false;            // Allow cutting through wall corners
    int maxIterations = 1000;           // Max A* iterations before giving up
    ITraversalProvider* traversalProvider = nullptr;  // Custom traversal logic
};

// ============================================================================
// Pathfinder Node - Internal A* node structure
// ============================================================================
struct PathNode {
    int x, y;
    float gCost;  // Cost from start
    float hCost;  // Heuristic cost to goal
    float fCost() const { return gCost + hCost; }
    int parentX, parentY;
    
    bool operator>(const PathNode& other) const {
        return fCost() > other.fCost();
    }
};

// ============================================================================
// Pathfinder - Core A* pathfinding implementation
// Similar to Unity's ABPath + AstarPath
// ============================================================================
class Pathfinder {
public:
    // Singleton access for global configuration
    static Pathfinder& Instance();
    
    // Configuration
    PathfinderConfig config;
    
    // Find path from start to goal in world coordinates
    // Returns a Path object with the result
    Path FindPath(Room* room, Vector2 startWorld, Vector2 goalWorld);
    
    // Legacy static method for compatibility
    static std::vector<Vector2> FindPathStatic(Room* room, Vector2 startWorld, Vector2 goalWorld);
    
    // Get next position to move toward on the path
    static Vector2 GetNextWaypoint(const std::vector<Vector2>& path, Vector2 currentPos, float waypointRadius = 20.0f);
    
    // Apply path modifiers
    void AddModifier(std::shared_ptr<PathModifier> modifier);
    void ClearModifiers();
    
private:
    Pathfinder() = default;
    
    float Heuristic(int x1, int y1, int x2, int y2) const;
    std::vector<std::pair<int, int>> GetNeighbors(Room* room, int x, int y) const;
    
    std::vector<std::shared_ptr<PathModifier>> m_modifiers;
    DefaultTraversalProvider m_defaultTraversal;
};

// ============================================================================
// Seeker - Component that handles path requests for an entity
// Similar to Unity's Seeker component
// ============================================================================
using OnPathCompleteCallback = std::function<void(const Path&)>;

class Seeker {
public:
    Seeker() = default;
    
    // Configuration (similar to AIPath settings)
    float repathRate = 0.3f;              // How often to recalculate paths (seconds)
    float pickNextWaypointDist = 20.0f;   // Distance to pick next waypoint
    bool constrainInsideGraph = true;     // Keep agent on walkable tiles
    
    // Start a new path request
    void StartPath(Vector2 start, Vector2 end, Room* room, OnPathCompleteCallback callback = nullptr);
    
    // Check if the seeker has finished calculating
    bool IsDone() const { return !m_calculating; }
    
    // Get the current path
    const Path& GetCurrentPath() const { return m_currentPath; }
    bool HasPath() const { return m_currentPath.IsComplete(); }
    
    // Path following helpers
    Vector2 GetNextWaypoint(Vector2 currentPos) const;
    void AdvanceWaypoint(Vector2 currentPos);
    bool ReachedEndOfPath(Vector2 currentPos, float threshold = 10.0f) const;
    int GetCurrentWaypointIndex() const { return m_currentWaypoint; }
    
    // Clear the current path
    void ClearPath();
    
    // Update timer for automatic repathing
    void Update(float dt);
    bool ShouldRepath() const { return m_repathTimer <= 0; }
    void ResetRepathTimer() { m_repathTimer = repathRate; }
    
private:
    Path m_currentPath;
    int m_currentWaypoint = 0;
    bool m_calculating = false;
    float m_repathTimer = 0.0f;
    Vector2 m_destination = {0, 0};
    Room* m_room = nullptr;
    OnPathCompleteCallback m_callback = nullptr;
};

// ============================================================================
// AIPath-style Movement Helper
// Provides high-level movement logic similar to Unity's AIPath component
// ============================================================================
class AIPathHelper {
public:
    // Configuration
    float speed = 100.0f;
    float rotationSpeed = 360.0f;
    float slowdownDistance = 50.0f;
    float endReachedDistance = 10.0f;
    
    // Move toward destination using the seeker's path
    // Returns the new position
    Vector2 MoveToward(Seeker& seeker, Vector2 currentPos, Vector2 destination, 
                       Room* room, float dt, float speedMultiplier = 1.0f);
    
    // Check if we've reached the destination
    bool HasReachedDestination(const Seeker& seeker, Vector2 currentPos) const;
    
    // Get remaining distance along the path
    float GetRemainingDistance(const Seeker& seeker, Vector2 currentPos) const;
};
