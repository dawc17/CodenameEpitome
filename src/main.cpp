#include "Game.hpp"

int main() {
    Game& game = Game::Instance();
    
    game.Init();
    game.Run();
    game.Shutdown();
    
    return 0;
}
