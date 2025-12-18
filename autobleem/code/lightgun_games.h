// Original author: steve
// Manages the list of games that use lightgun controllers.
// Stored in lightguns.txt with one game path per line.
#pragma once

#include <string>
#include <vector>

#include "launcher/ps_game.h"

struct LightgunGames {
    static std::string filename;
    static std::vector<std::string> lightgunGamePaths;

    LightgunGames();
    void UpdateFile();

    static std::string PathForLightgunFile(PsGamePtr game);

    bool IsGameALightgunGame(PsGamePtr game);
    bool IsGameALightgunGame(const std::string &gamepath);

    void AddGame(PsGamePtr game);
    void AddGame(const std::string &gamepath);

    void RemoveGame(PsGamePtr game);
    void RemoveGame(const std::string &gamepath);

    void PurgeGamesNotFound();

    PsGames GetAllLightgunGames();
};
