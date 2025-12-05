#include "Pathfinding.hpp"
#include "Dungeon.hpp"
#include "raymath.h"
#include <cmath>
#include <algorithm>
#include <functional>
#include <random>

// ============================================================================
// Default Traversal Provider Implementation
// ============================================================================
bool DefaultTraversalProvider::CanTraverse(Room* room, int x, int y) const {
    return room && room->IsWalkable(x, y);
}

// ============================================================================
// Weighted Traversal Provider Implementation
// ============================================================================
void WeightedTraversalProvider::AddPenaltyZone(Vector2 worldPos, float radius, float penalty) {
    m_penaltyZones.push_back({worldPos, radius, penalty});
}

void WeightedTraversalProvider::ClearPenaltyZones() {
    m_penaltyZones.clear();
}

bool WeightedTraversalProvider::CanTraverse(Room* room, int x, int y) const {
    return room && room->IsWalkable(x, y);
}

float WeightedTraversalProvider::GetTraversalCost(Room* room, int x, int y) const {
    if (!room) return 1.0f;
    
    Vector2 worldPos = room->TileToWorld(x, y);
    float totalPenalty = 1.0f;
    
    for (const auto& zone : m_penaltyZones) {
        float dist = Vector2Distance(worldPos, zone.worldPos);
        if (dist < zone.radius) {
            // Linear falloff from center
            float influence = 1.0f - (dist / zone.radius);
            totalPenalty += zone.penalty * influence;
        }
    }
    
    return totalPenalty;
}

// ============================================================================
// Path Smoother Implementation
// ============================================================================
void PathSmoother::Apply(Path& path) {
    if (path.vectorPath.size() < 3) return;
    
    std::vector<Vector2> smoothed;
    smoothed.push_back(path.vectorPath[0]);
    
    size_t i = 0;
    while (i < path.vectorPath.size() - 1) {
        // Try to find the furthest visible point from current position
        size_t furthest = i + 1;
        for (size_t j = i + 2; j < path.vectorPath.size(); ++j) {
            // Simple line-of-sight check (could be enhanced with actual raycast)
            Vector2 dir = Vector2Subtract(path.vectorPath[j], path.vectorPath[i]);
            float len = Vector2Length(dir);
            if (len < 1.0f) continue;
            
            // For now, just check if waypoints are roughly aligned
            Vector2 midDir = Vector2Subtract(path.vectorPath[(i + j) / 2], path.vectorPath[i]);
            float dot = Vector2DotProduct(Vector2Normalize(dir), Vector2Normalize(midDir));
            if (dot > 0.9f) {
                furthest = j;
            }
        }
        
        smoothed.push_back(path.vectorPath[furthest]);
        i = furthest;
    }
    
    path.vectorPath = smoothed;
}

// ============================================================================
// Alternative Path Modifier Implementation
// ============================================================================
void AlternativePathModifier::Apply(Path& path) {
    if (path.vectorPath.size() < 2) return;
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-randomOffset, randomOffset);
    
    // Don't modify start and end points
    for (size_t i = 1; i < path.vectorPath.size() - 1; ++i) {
        // Add perpendicular offset
        Vector2 prev = path.vectorPath[i - 1];
        Vector2 next = path.vectorPath[i + 1];
        Vector2 dir = Vector2Normalize(Vector2Subtract(next, prev));
        Vector2 perpendicular = {-dir.y, dir.x};
        
        float offset = dist(gen);
        path.vectorPath[i] = Vector2Add(path.vectorPath[i], 
                                         Vector2Scale(perpendicular, offset));
    }
}

// ============================================================================
// Pathfinder Implementation
// ============================================================================
Pathfinder& Pathfinder::Instance() {
    static Pathfinder instance;
    return instance;
}

float Pathfinder::Heuristic(int x1, int y1, int x2, int y2) const {
    // Euclidean distance with configurable heuristic scale
    float dx = static_cast<float>(x2 - x1);
    float dy = static_cast<float>(y2 - y1);
    return std::sqrt(dx * dx + dy * dy) * config.heuristicScale;
}

std::vector<std::pair<int, int>> Pathfinder::GetNeighbors(Room* room, int x, int y) const {
    std::vector<std::pair<int, int>> neighbors;
    
    ITraversalProvider* traversal = config.traversalProvider ? 
        config.traversalProvider : const_cast<DefaultTraversalProvider*>(&m_defaultTraversal);
    
    if (config.allowDiagonal) {
        // 8-directional movement
        const int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
        const int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};
        
        for (int i = 0; i < 8; ++i) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            
            if (traversal->CanTraverse(room, nx, ny)) {
                // For diagonal movement, check corner cutting
                if (dx[i] != 0 && dy[i] != 0 && !config.cutCorners) {
                    if (!traversal->CanTraverse(room, x + dx[i], y) || 
                        !traversal->CanTraverse(room, x, y + dy[i])) {
                        continue;  // Can't cut corner
                    }
                }
                neighbors.push_back({nx, ny});
            }
        }
    } else {
        // 4-directional movement
        const int dx[] = {-1, 0, 1, 0};
        const int dy[] = {0, -1, 0, 1};
        
        for (int i = 0; i < 4; ++i) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            
            if (traversal->CanTraverse(room, nx, ny)) {
                neighbors.push_back({nx, ny});
            }
        }
    }
    
    return neighbors;
}

Path Pathfinder::FindPath(Room* room, Vector2 startWorld, Vector2 goalWorld) {
    Path result;
    
    if (!room) {
        result.error = true;
        result.errorMessage = "No room provided";
        return result;
    }
    
    // Get traversal provider
    ITraversalProvider* traversal = config.traversalProvider ? 
        config.traversalProvider : const_cast<DefaultTraversalProvider*>(&m_defaultTraversal);
    
    // Convert world positions to tile coordinates
    int startX, startY, goalX, goalY;
    if (!room->WorldToTile(startWorld, startX, startY)) {
        result.error = true;
        result.errorMessage = "Start position outside room";
        return result;
    }
    if (!room->WorldToTile(goalWorld, goalX, goalY)) {
        result.error = true;
        result.errorMessage = "Goal position outside room";
        return result;
    }
    
    // Check walkability
    if (!traversal->CanTraverse(room, startX, startY)) {
        result.error = true;
        result.errorMessage = "Start position not walkable";
        return result;
    }
    if (!traversal->CanTraverse(room, goalX, goalY)) {
        result.error = true;
        result.errorMessage = "Goal position not walkable";
        return result;
    }
    
    // Already at goal
    if (startX == goalX && startY == goalY) {
        return result;  // Empty path, no error
    }
    
    // A* implementation
    auto hashCoord = [](int x, int y) { return y * 10000 + x; };
    
    std::priority_queue<PathNode, std::vector<PathNode>, std::greater<PathNode>> openSet;
    std::unordered_map<int, PathNode> allNodes;
    std::unordered_map<int, bool> closedSet;
    
    PathNode startNode;
    startNode.x = startX;
    startNode.y = startY;
    startNode.gCost = 0;
    startNode.hCost = Heuristic(startX, startY, goalX, goalY);
    startNode.parentX = -1;
    startNode.parentY = -1;
    
    openSet.push(startNode);
    allNodes[hashCoord(startX, startY)] = startNode;
    
    int iterations = 0;
    
    while (!openSet.empty() && iterations < config.maxIterations) {
        ++iterations;
        
        PathNode current = openSet.top();
        openSet.pop();
        
        int currentHash = hashCoord(current.x, current.y);
        
        // Skip if already processed
        if (closedSet[currentHash]) continue;
        closedSet[currentHash] = true;
        
        // Found the goal
        if (current.x == goalX && current.y == goalY) {
            // Reconstruct path
            std::vector<Vector2> reversePath;
            int cx = current.x;
            int cy = current.y;
            
            while (cx != -1 && cy != -1) {
                reversePath.push_back(room->TileToWorld(cx, cy));
                int hash = hashCoord(cx, cy);
                int px = allNodes[hash].parentX;
                int py = allNodes[hash].parentY;
                cx = px;
                cy = py;
            }
            
            // Reverse path, skipping start position
            for (int i = static_cast<int>(reversePath.size()) - 2; i >= 0; --i) {
                result.vectorPath.push_back(reversePath[i]);
            }
            
            // Apply modifiers
            for (auto& modifier : m_modifiers) {
                modifier->Apply(result);
            }
            
            return result;
        }
        
        // Explore neighbors
        for (auto& [nx, ny] : GetNeighbors(room, current.x, current.y)) {
            int neighborHash = hashCoord(nx, ny);
            
            if (closedSet[neighborHash]) continue;
            
            // Base cost: 1.0 for cardinal, 1.414 for diagonal
            float baseCost = (nx != current.x && ny != current.y) ? 1.414f : 1.0f;
            
            // Apply traversal cost
            float traversalCost = traversal->GetTraversalCost(room, nx, ny);
            float newGCost = current.gCost + baseCost * traversalCost;
            
            auto it = allNodes.find(neighborHash);
            if (it == allNodes.end() || newGCost < it->second.gCost) {
                PathNode neighbor;
                neighbor.x = nx;
                neighbor.y = ny;
                neighbor.gCost = newGCost;
                neighbor.hCost = Heuristic(nx, ny, goalX, goalY);
                neighbor.parentX = current.x;
                neighbor.parentY = current.y;
                
                allNodes[neighborHash] = neighbor;
                openSet.push(neighbor);
            }
        }
    }
    
    // No path found
    result.error = true;
    result.errorMessage = iterations >= config.maxIterations ? 
        "Max iterations reached" : "No path exists";
    return result;
}

std::vector<Vector2> Pathfinder::FindPathStatic(Room* room, Vector2 startWorld, Vector2 goalWorld) {
    Path path = Instance().FindPath(room, startWorld, goalWorld);
    return path.vectorPath;
}

Vector2 Pathfinder::GetNextWaypoint(const std::vector<Vector2>& path, Vector2 currentPos, float waypointRadius) {
    if (path.empty()) {
        return currentPos;
    }
    
    // Return the first waypoint that we haven't reached yet
    for (size_t i = 0; i < path.size(); ++i) {
        float dist = Vector2Distance(currentPos, path[i]);
        if (dist > waypointRadius) {
            return path[i];
        }
    }
    
    // If we're close to all waypoints, return the last one
    return path.back();
}

void Pathfinder::AddModifier(std::shared_ptr<PathModifier> modifier) {
    m_modifiers.push_back(modifier);
}

void Pathfinder::ClearModifiers() {
    m_modifiers.clear();
}

// ============================================================================
// Seeker Implementation
// ============================================================================
void Seeker::StartPath(Vector2 start, Vector2 end, Room* room, OnPathCompleteCallback callback) {
    m_destination = end;
    m_room = room;
    m_callback = callback;
    m_calculating = true;
    
    // Calculate path immediately (could be made async in future)
    m_currentPath = Pathfinder::Instance().FindPath(room, start, end);
    m_currentWaypoint = 0;
    m_calculating = false;
    
    if (m_callback) {
        m_callback(m_currentPath);
    }
}

Vector2 Seeker::GetNextWaypoint(Vector2 currentPos) const {
    if (!HasPath() || m_currentWaypoint >= static_cast<int>(m_currentPath.vectorPath.size())) {
        return currentPos;
    }
    
    return m_currentPath.vectorPath[m_currentWaypoint];
}

void Seeker::AdvanceWaypoint(Vector2 currentPos) {
    if (!HasPath()) return;
    
    while (m_currentWaypoint < static_cast<int>(m_currentPath.vectorPath.size())) {
        float dist = Vector2Distance(currentPos, m_currentPath.vectorPath[m_currentWaypoint]);
        if (dist < pickNextWaypointDist) {
            ++m_currentWaypoint;
        } else {
            break;
        }
    }
}

bool Seeker::ReachedEndOfPath(Vector2 currentPos, float threshold) const {
    if (!HasPath()) return true;
    if (m_currentPath.vectorPath.empty()) return true;
    
    float dist = Vector2Distance(currentPos, m_currentPath.vectorPath.back());
    return dist < threshold;
}

void Seeker::ClearPath() {
    m_currentPath.vectorPath.clear();
    m_currentPath.error = false;
    m_currentPath.errorMessage.clear();
    m_currentWaypoint = 0;
}

void Seeker::Update(float dt) {
    m_repathTimer -= dt;
}

// ============================================================================
// AIPathHelper Implementation
// ============================================================================
Vector2 AIPathHelper::MoveToward(Seeker& seeker, Vector2 currentPos, Vector2 destination, 
                                  Room* room, float dt, float speedMultiplier) {
    // Update seeker timer
    seeker.Update(dt);
    
    // Repath if needed
    if (seeker.ShouldRepath() || !seeker.HasPath()) {
        seeker.StartPath(currentPos, destination, room);
        seeker.ResetRepathTimer();
    }
    
    // Advance waypoint if we've reached the current one
    seeker.AdvanceWaypoint(currentPos);
    
    // Get next waypoint to move toward
    Vector2 nextWaypoint = seeker.GetNextWaypoint(currentPos);
    
    // Check if we've reached the destination
    if (seeker.ReachedEndOfPath(currentPos, endReachedDistance)) {
        return currentPos;
    }
    
    // Calculate direction and movement
    Vector2 toWaypoint = Vector2Subtract(nextWaypoint, currentPos);
    float distToWaypoint = Vector2Length(toWaypoint);
    
    if (distToWaypoint < 0.1f) {
        return currentPos;
    }
    
    // Calculate speed with slowdown near destination
    float remainingDist = GetRemainingDistance(seeker, currentPos);
    float currentSpeed = speed * speedMultiplier;
    
    if (remainingDist < slowdownDistance) {
        float slowdownFactor = remainingDist / slowdownDistance;
        currentSpeed *= std::max(0.3f, slowdownFactor);  // Minimum 30% speed
    }
    
    // Move toward waypoint
    Vector2 moveDir = Vector2Normalize(toWaypoint);
    float moveDistance = std::min(currentSpeed * dt, distToWaypoint);
    
    return Vector2Add(currentPos, Vector2Scale(moveDir, moveDistance));
}

bool AIPathHelper::HasReachedDestination(const Seeker& seeker, Vector2 currentPos) const {
    return seeker.ReachedEndOfPath(currentPos, endReachedDistance);
}

float AIPathHelper::GetRemainingDistance(const Seeker& seeker, Vector2 currentPos) const {
    if (!seeker.HasPath()) return 0.0f;
    
    const auto& path = seeker.GetCurrentPath().vectorPath;
    if (path.empty()) return 0.0f;
    
    int currentIdx = seeker.GetCurrentWaypointIndex();
    if (currentIdx >= static_cast<int>(path.size())) return 0.0f;
    
    // Distance to current waypoint
    float totalDist = Vector2Distance(currentPos, path[currentIdx]);
    
    // Add distances between remaining waypoints
    for (int i = currentIdx; i < static_cast<int>(path.size()) - 1; ++i) {
        totalDist += Vector2Distance(path[i], path[i + 1]);
    }
    
    return totalDist;
}
