#include "Dungeon.hpp"
#include "Utils.hpp"
#include "Game.hpp"
#include "Player.hpp"

// Room implementation
Room::Room(int id, RoomType type, int gridX, int gridY)
    : m_id(id), m_type(type), m_gridX(gridX), m_gridY(gridY)
{
    // Initialize tile grid
    m_tiles.resize(HEIGHT, std::vector<TileType>(WIDTH, TileType::FLOOR));
}

void Room::Generate(unsigned int seed) {
    Utils::SeedRNG(seed + m_id);
    
    // Create walls around the room
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            if (x == 0 || x == WIDTH - 1 || y == 0 || y == HEIGHT - 1) {
                m_tiles[y][x] = TileType::WALL;
            } else {
                m_tiles[y][x] = TileType::FLOOR;
            }
        }
    }
    
    // Add some random obstacles for normal rooms
    if (m_type == RoomType::NORMAL) {
        int numObstacles = Utils::RandomInt(0, 4);
        for (int i = 0; i < numObstacles; ++i) {
            int x = Utils::RandomInt(3, WIDTH - 4);
            int y = Utils::RandomInt(3, HEIGHT - 4);
            
            // Small pillar or obstacle
            m_tiles[y][x] = TileType::WALL;
        }
    }
    
    // Set up door positions based on connected rooms
    for (auto& door : m_doors) {
        int doorX, doorY;
        switch (door.direction) {
            case 0: // Up
                doorX = WIDTH / 2;
                doorY = 0;
                break;
            case 1: // Right
                doorX = WIDTH - 1;
                doorY = HEIGHT / 2;
                break;
            case 2: // Down
                doorX = WIDTH / 2;
                doorY = HEIGHT - 1;
                break;
            case 3: // Left
                doorX = 0;
                doorY = HEIGHT / 2;
                break;
            default:
                continue;
        }
        m_tiles[doorY][doorX] = TileType::DOOR;
        door.position = TileToWorld(doorX, doorY);
    }
    
    // Set player spawn point (center of room for start room)
    m_playerSpawn = TileToWorld(WIDTH / 2, HEIGHT / 2);
    
    // Generate enemy spawn points
    m_enemySpawns.clear();
    if (m_type == RoomType::NORMAL || m_type == RoomType::BOSS) {
        int numSpawns = (m_type == RoomType::BOSS) ? 1 : Utils::RandomInt(3, 6);
        for (int i = 0; i < numSpawns; ++i) {
            int x = Utils::RandomInt(2, WIDTH - 3);
            int y = Utils::RandomInt(2, HEIGHT - 3);
            if (m_tiles[y][x] == TileType::FLOOR) {
                m_enemySpawns.push_back(TileToWorld(x, y));
            }
        }
    }
}

void Room::Render(Vector2 offset) {
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            Vector2 worldPos = TileToWorld(x, y);
            worldPos.x += offset.x;
            worldPos.y += offset.y;
            
            Rectangle rect = {
                worldPos.x - TILE_SIZE / 2,
                worldPos.y - TILE_SIZE / 2,
                static_cast<float>(TILE_SIZE),
                static_cast<float>(TILE_SIZE)
            };
            
            Color color;
            switch (m_tiles[y][x]) {
                case TileType::FLOOR:
                    color = Color{40, 40, 50, 255};
                    break;
                case TileType::WALL:
                    color = Color{80, 80, 100, 255};
                    break;
                case TileType::DOOR:
                    color = m_cleared ? Color{60, 120, 60, 255} : Color{120, 60, 60, 255};
                    break;
                case TileType::VOID:
                    color = BLACK;
                    break;
            }
            
            DrawRectangleRec(rect, color);
            
            // Draw grid lines
            DrawRectangleLinesEx(rect, 1, Color{60, 60, 70, 255});
        }
    }
}

TileType Room::GetTile(int x, int y) const {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
        return TileType::VOID;
    }
    return m_tiles[y][x];
}

void Room::SetTile(int x, int y, TileType tile) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        m_tiles[y][x] = tile;
    }
}

bool Room::IsWalkable(int x, int y) const {
    TileType tile = GetTile(x, y);
    return tile == TileType::FLOOR || tile == TileType::DOOR;
}

Vector2 Room::GetWorldPosition() const {
    return {
        static_cast<float>(m_gridX * WIDTH * TILE_SIZE),
        static_cast<float>(m_gridY * HEIGHT * TILE_SIZE)
    };
}

Vector2 Room::TileToWorld(int tileX, int tileY) const {
    Vector2 roomPos = GetWorldPosition();
    return {
        roomPos.x + tileX * TILE_SIZE + TILE_SIZE / 2.0f,
        roomPos.y + tileY * TILE_SIZE + TILE_SIZE / 2.0f
    };
}

bool Room::WorldToTile(Vector2 worldPos, int& tileX, int& tileY) const {
    Vector2 roomPos = GetWorldPosition();
    Vector2 localPos = Vector2Subtract(worldPos, roomPos);
    
    tileX = static_cast<int>(localPos.x / TILE_SIZE);
    tileY = static_cast<int>(localPos.y / TILE_SIZE);
    
    return tileX >= 0 && tileX < WIDTH && tileY >= 0 && tileY < HEIGHT;
}

void Room::AddDoor(int direction, int connectedRoomId) {
    Door door;
    door.direction = direction;
    door.connectedRoomId = connectedRoomId;
    door.isOpen = false;
    m_doors.push_back(door);
}

// DungeonManager implementation
DungeonManager::DungeonManager() {
}

void DungeonManager::Generate(unsigned int seed, int floorNumber) {
    Utils::SeedRNG(seed);
    m_floorNumber = floorNumber;
    m_rooms.clear();
    
    GenerateLayout(seed);
    ConnectRooms();
    
    // Generate each room
    for (auto& room : m_rooms) {
        room->Generate(seed + room->GetId());
    }
    
    // Set current room to start room
    SetCurrentRoom(0);
}

void DungeonManager::GenerateLayout(unsigned int seed) {
    // Generate a more complex layout with both horizontal and vertical connections
    int numRooms = 5 + m_floorNumber; // More rooms on higher floors
    
    // Create start room at center
    m_rooms.push_back(std::make_unique<Room>(0, RoomType::START, 0, 0));
    
    // Track occupied grid positions
    std::vector<std::pair<int, int>> occupiedPositions;
    occupiedPositions.push_back({0, 0});
    
    // Possible directions: right, down, left, up
    int dx[] = {1, 0, -1, 0};
    int dy[] = {0, 1, 0, -1};
    
    int currentX = 0, currentY = 0;
    
    for (int i = 1; i < numRooms - 1; ++i) {
        RoomType type = RoomType::NORMAL;
        
        // Occasional treasure room
        if (Utils::RandomFloat(0, 1) < 0.15f) {
            type = RoomType::TREASURE;
        }
        
        // Try to find a valid adjacent position
        bool placed = false;
        int attempts = 0;
        while (!placed && attempts < 20) {
            // Pick a random direction - favor variety in directions
            int dir = Utils::RandomInt(0, 3);
            int newX = currentX + dx[dir];
            int newY = currentY + dy[dir];
            
            // Check if position is available
            bool occupied = false;
            for (const auto& pos : occupiedPositions) {
                if (pos.first == newX && pos.second == newY) {
                    occupied = true;
                    break;
                }
            }
            
            if (!occupied) {
                m_rooms.push_back(std::make_unique<Room>(i, type, newX, newY));
                occupiedPositions.push_back({newX, newY});
                currentX = newX;
                currentY = newY;
                placed = true;
            }
            attempts++;
        }
        
        // Fallback: find any adjacent free position
        if (!placed) {
            for (const auto& existingPos : occupiedPositions) {
                for (int dir = 0; dir < 4; ++dir) {
                    int newX = existingPos.first + dx[dir];
                    int newY = existingPos.second + dy[dir];
                    
                    bool occupied = false;
                    for (const auto& pos : occupiedPositions) {
                        if (pos.first == newX && pos.second == newY) {
                            occupied = true;
                            break;
                        }
                    }
                    
                    if (!occupied) {
                        m_rooms.push_back(std::make_unique<Room>(i, type, newX, newY));
                        occupiedPositions.push_back({newX, newY});
                        currentX = newX;
                        currentY = newY;
                        placed = true;
                        break;
                    }
                }
                if (placed) break;
            }
        }
    }
    
    // Create boss/exit room at the last position
    // Find a position adjacent to the current room that's not occupied
    bool bossPlaced = false;
    for (int dir = 0; dir < 4; ++dir) {
        int newX = currentX + dx[dir];
        int newY = currentY + dy[dir];
        
        bool occupied = false;
        for (const auto& pos : occupiedPositions) {
            if (pos.first == newX && pos.second == newY) {
                occupied = true;
                break;
            }
        }
        
        if (!occupied) {
            m_rooms.push_back(std::make_unique<Room>(numRooms - 1, RoomType::EXIT, newX, newY));
            bossPlaced = true;
            break;
        }
    }
    
    if (!bossPlaced) {
        // Fallback to adjacent to start
        m_rooms.push_back(std::make_unique<Room>(numRooms - 1, RoomType::EXIT, 1, 1));
    }
}

void DungeonManager::ConnectRooms() {
    // Connect all adjacent rooms (not just sequential ones)
    for (size_t i = 0; i < m_rooms.size(); ++i) {
        Room* current = m_rooms[i].get();
        
        for (size_t j = i + 1; j < m_rooms.size(); ++j) {
            Room* other = m_rooms[j].get();
            
            int dx = other->GetGridX() - current->GetGridX();
            int dy = other->GetGridY() - current->GetGridY();
            
            // Check if rooms are adjacent (exactly 1 tile apart in one direction)
            if ((abs(dx) == 1 && dy == 0) || (dx == 0 && abs(dy) == 1)) {
                int dirFromCurrent, dirFromOther;
                
                if (dx > 0) {
                    dirFromCurrent = 1; // Right
                    dirFromOther = 3;   // Left
                } else if (dx < 0) {
                    dirFromCurrent = 3; // Left
                    dirFromOther = 1;   // Right
                } else if (dy > 0) {
                    dirFromCurrent = 2; // Down
                    dirFromOther = 0;   // Up
                } else {
                    dirFromCurrent = 0; // Up
                    dirFromOther = 2;   // Down
                }
                
                // Check if door already exists
                bool doorExists = false;
                for (const auto& door : current->GetDoors()) {
                    if (door.connectedRoomId == other->GetId()) {
                        doorExists = true;
                        break;
                    }
                }
                
                if (!doorExists) {
                    current->AddDoor(dirFromCurrent, other->GetId());
                    other->AddDoor(dirFromOther, current->GetId());
                }
            }
        }
    }
}

void DungeonManager::Update(float dt) {
    // Handle room transitions, etc.
    if (m_transitioning) {
        // Smooth camera transition
        m_cameraOffset = Utils::LerpV(m_cameraOffset, m_cameraTarget, 5.0f * dt);
        
        if (Vector2Distance(m_cameraOffset, m_cameraTarget) < 1.0f) {
            m_transitioning = false;
        }
    }
}

void DungeonManager::Render() {
    if (m_currentRoom) {
        m_currentRoom->Render({0, 0});
    }
}

Room* DungeonManager::GetRoom(int id) {
    for (auto& room : m_rooms) {
        if (room->GetId() == id) {
            return room.get();
        }
    }
    return nullptr;
}

void DungeonManager::SetCurrentRoom(int id) {
    Room* room = GetRoom(id);
    if (room) {
        m_currentRoom = room;
        m_currentRoom->SetVisited(true);  // Mark as visited when entering
        m_cameraTarget = room->GetWorldPosition();
        m_cameraOffset = m_cameraTarget;
    }
}

void DungeonManager::TransitionToRoom(int roomId, int fromDirection) {
    Room* newRoom = GetRoom(roomId);
    if (!newRoom) return;
    
    m_currentRoom = newRoom;
    m_currentRoom->SetVisited(true);  // Mark as visited when entering
    m_cameraTarget = newRoom->GetWorldPosition();
    m_transitioning = true;
    
    // Position player at the opposite door
    Player* player = Game::Instance().GetPlayer();
    if (player) {
        int oppositeDir = (fromDirection + 2) % 4;
        
        // Find the door we came from
        for (auto& door : newRoom->GetDoors()) {
            if (door.direction == oppositeDir) {
                // Offset player from door
                Vector2 offset = {0, 0};
                switch (oppositeDir) {
                    case 0: offset.y = 60; break;  // Coming from up, place below door
                    case 1: offset.x = -60; break; // Coming from right, place left
                    case 2: offset.y = -60; break; // Coming from down, place above
                    case 3: offset.x = 60; break;  // Coming from left, place right
                }
                player->SetPosition(Vector2Add(door.position, offset));
                break;
            }
        }
    }
}

bool DungeonManager::IsWalkable(Vector2 worldPos) const {
    if (!m_currentRoom) return false;
    
    int tileX, tileY;
    if (m_currentRoom->WorldToTile(worldPos, tileX, tileY)) {
        return m_currentRoom->IsWalkable(tileX, tileY);
    }
    return false;
}

bool DungeonManager::CheckDoorCollision(Vector2 worldPos, int& roomId, int& direction) {
    if (!m_currentRoom) return false;
    
    for (auto& door : m_currentRoom->GetDoors()) {
        float dist = Vector2Distance(worldPos, door.position);
        if (dist < 30.0f) {
            roomId = door.connectedRoomId;
            direction = door.direction;
            return true;
        }
    }
    return false;
}

void DungeonManager::RenderMinimap(float x, float y, float scale) {
    if (m_rooms.empty()) return;
    
    // Find bounds of the dungeon
    int minGridX = 0, maxGridX = 0;
    int minGridY = 0, maxGridY = 0;
    
    for (const auto& room : m_rooms) {
        if (room->GetGridX() < minGridX) minGridX = room->GetGridX();
        if (room->GetGridX() > maxGridX) maxGridX = room->GetGridX();
        if (room->GetGridY() < minGridY) minGridY = room->GetGridY();
        if (room->GetGridY() > maxGridY) maxGridY = room->GetGridY();
    }
    
    float roomSize = 20.0f * scale;
    float padding = 4.0f * scale;
    
    // Draw background
    float mapWidth = (maxGridX - minGridX + 1) * (roomSize + padding) + padding;
    float mapHeight = (maxGridY - minGridY + 1) * (roomSize + padding) + padding;
    DrawRectangle(static_cast<int>(x - padding), static_cast<int>(y - padding),
                  static_cast<int>(mapWidth + padding), static_cast<int>(mapHeight + padding),
                  ColorAlpha(BLACK, 0.7f));
    
    // Draw rooms
    for (const auto& room : m_rooms) {
        float roomX = x + (room->GetGridX() - minGridX) * (roomSize + padding);
        float roomY = y + (room->GetGridY() - minGridY) * (roomSize + padding);
        
        Rectangle roomRect = {roomX, roomY, roomSize, roomSize};
        
        // Determine room color based on state
        Color roomColor;
        if (room.get() == m_currentRoom) {
            roomColor = WHITE;  // Current room
        } else if (room->IsVisited()) {
            // Visited rooms - color by type
            switch (room->GetType()) {
                case RoomType::START:
                    roomColor = GREEN;
                    break;
                case RoomType::TREASURE:
                    roomColor = GOLD;
                    break;
                case RoomType::SHOP:
                    roomColor = SKYBLUE;
                    break;
                case RoomType::BOSS:
                    roomColor = RED;
                    break;
                case RoomType::EXIT:
                    roomColor = PURPLE;
                    break;
                default:
                    roomColor = GRAY;
                    break;
            }
        } else {
            // Unvisited but adjacent to visited - show as unknown
            bool adjacentToVisited = false;
            for (const auto& door : room->GetDoors()) {
                Room* connected = GetRoom(door.connectedRoomId);
                if (connected && connected->IsVisited()) {
                    adjacentToVisited = true;
                    break;
                }
            }
            
            if (adjacentToVisited) {
                roomColor = ColorAlpha(DARKGRAY, 0.5f);  // Unknown but accessible
            } else {
                continue;  // Don't show rooms that aren't adjacent to visited ones
            }
        }
        
        DrawRectangleRec(roomRect, roomColor);
        
        // Draw connections/doors
        for (const auto& door : room->GetDoors()) {
            Room* connected = GetRoom(door.connectedRoomId);
            if (!connected) continue;
            
            // Only draw if both rooms should be visible
            bool thisVisible = room->IsVisited() || room.get() == m_currentRoom;
            bool otherVisited = connected->IsVisited();
            
            if (!thisVisible && !otherVisited) continue;
            
            float lineThickness = 2.0f * scale;
            Vector2 start, end;
            
            switch (door.direction) {
                case 0: // Up
                    start = {roomX + roomSize / 2, roomY};
                    end = {roomX + roomSize / 2, roomY - padding};
                    break;
                case 1: // Right
                    start = {roomX + roomSize, roomY + roomSize / 2};
                    end = {roomX + roomSize + padding, roomY + roomSize / 2};
                    break;
                case 2: // Down
                    start = {roomX + roomSize / 2, roomY + roomSize};
                    end = {roomX + roomSize / 2, roomY + roomSize + padding};
                    break;
                case 3: // Left
                    start = {roomX, roomY + roomSize / 2};
                    end = {roomX - padding, roomY + roomSize / 2};
                    break;
                default:
                    continue;
            }
            
            DrawLineEx(start, end, lineThickness, LIGHTGRAY);
        }
    }
}
