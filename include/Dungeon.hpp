#pragma once

#include "raylib.h"
#include <vector>
#include <memory>
#include <random>

enum class RoomType {
    START,
    NORMAL,
    TREASURE,
    SHOP,
    BOSS,
    EXIT
};

enum class TileType {
    FLOOR,
    WALL,
    DOOR,
    VOID
};

struct Door {
    Vector2 position;
    int direction; // 0=up, 1=right, 2=down, 3=left
    int connectedRoomId = -1;
    bool isOpen = false;
};

class Room {
public:
    Room(int id, RoomType type, int gridX, int gridY);
    
    void Generate(unsigned int seed);
    void Render(Vector2 offset);
    
    // Tile access
    TileType GetTile(int x, int y) const;
    void SetTile(int x, int y, TileType tile);
    bool IsWalkable(int x, int y) const;
    
    // World position conversion
    Vector2 GetWorldPosition() const;
    Vector2 TileToWorld(int tileX, int tileY) const;
    bool WorldToTile(Vector2 worldPos, int& tileX, int& tileY) const;
    
    // Properties
    int GetId() const { return m_id; }
    RoomType GetType() const { return m_type; }
    int GetGridX() const { return m_gridX; }
    int GetGridY() const { return m_gridY; }
    bool IsCleared() const { return m_cleared; }
    void SetCleared(bool cleared) { m_cleared = cleared; }
    bool IsVisited() const { return m_visited; }
    void SetVisited(bool visited) { m_visited = visited; }
    
    // Doors
    std::vector<Door>& GetDoors() { return m_doors; }
    void AddDoor(int direction, int connectedRoomId);
    
    // Spawn points
    std::vector<Vector2> GetEnemySpawnPoints() const { return m_enemySpawns; }
    Vector2 GetPlayerSpawnPoint() const { return m_playerSpawn; }
    
    // Room dimensions
    static constexpr int WIDTH = 15;
    static constexpr int HEIGHT = 11;
    static constexpr int TILE_SIZE = 48;
    
private:
    int m_id;
    RoomType m_type;
    int m_gridX, m_gridY;
    bool m_cleared = false;
    bool m_visited = false;
    
    std::vector<std::vector<TileType>> m_tiles;
    std::vector<Door> m_doors;
    std::vector<Vector2> m_enemySpawns;
    Vector2 m_playerSpawn;
};

class DungeonManager {
public:
    DungeonManager();
    ~DungeonManager() = default;
    
    void Generate(unsigned int seed, int floorNumber);
    void Update(float dt);
    void Render();
    void RenderMinimap(float x, float y, float scale);
    
    // Room access
    Room* GetCurrentRoom() { return m_currentRoom; }
    Room* GetRoom(int id);
    const std::vector<std::unique_ptr<Room>>& GetAllRooms() const { return m_rooms; }
    void SetCurrentRoom(int id);
    void TransitionToRoom(int roomId, int fromDirection);
    
    // Collision
    bool IsWalkable(Vector2 worldPos) const;
    bool CheckDoorCollision(Vector2 worldPos, int& roomId, int& direction);
    
    // Floor info
    int GetFloorNumber() const { return m_floorNumber; }
    int GetRoomCount() const { return static_cast<int>(m_rooms.size()); }
    
private:
    void GenerateLayout(unsigned int seed);
    void ConnectRooms();
    
    std::vector<std::unique_ptr<Room>> m_rooms;
    Room* m_currentRoom = nullptr;
    int m_floorNumber = 1;
    
    // Camera offset for smooth transitions
    Vector2 m_cameraTarget = {0, 0};
    Vector2 m_cameraOffset = {0, 0};
    bool m_transitioning = false;
};
