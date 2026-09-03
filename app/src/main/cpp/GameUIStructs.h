#ifndef TOUCHPARTY_GAMEUISTRUCTS_H
#define TOUCHPARTY_GAMEUISTRUCTS_H

#include <string>

struct ServerRoomEntry {
    std::string id;
    std::string name;
    bool isPrivate = false;
    int playerCount = 1;
    int maxPlayers = 8;
    std::string pin;
};

struct PlayerInfo {
    std::string id;
    std::string name;
    std::string team;
    bool isLocal = false;
    bool connected = true;
    bool isOwner = false;
};

#endif // TOUCHPARTY_GAMEUISTRUCTS_H
