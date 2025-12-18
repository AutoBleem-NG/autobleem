// Original author: steve
#include "lightgun_games.h"

#include <algorithm>
#include <iostream>

#include "engine/database.h"
#include "environment.h"
#include "gui/gui.h"
#include "launcher/ra_integrator.h"
#include "utils/file_utils.h"

using namespace std;

std::string LightgunGames::filename;
std::vector<std::string> LightgunGames::lightgunGamePaths;

LightgunGames::LightgunGames() {
    filename = Environment::getWorkingPath() + sep + string("lightguns.txt");
    lightgunGamePaths = FileUtils::readTextFile(filename, true);

    // there shouldn't be any blank lines unless someone hand edited it.  but just to be sure.
    lightgunGamePaths.erase(
        remove_if(begin(lightgunGamePaths), end(lightgunGamePaths), [](const string &str) { return str == ""; }),
        end(lightgunGamePaths));
}

// Returns the path used as identifier for lightgun game tracking
// - RetroArch roms: image_path (the ROM file path)
// - PS1 games: folder (the game directory)
string LightgunGames::PathForLightgunFile(PsGamePtr game) {
    if (game == nullptr)
        return "";

    if (game->foreign)
        return game->image_path; // Retroarch roms game
    else
        return game->folder; // /Games PS1 game
}

void LightgunGames::UpdateFile() { FileUtils::writeTextFile(filename, lightgunGamePaths, true); }

bool LightgunGames::IsGameALightgunGame(PsGamePtr game) { return IsGameALightgunGame(PathForLightgunFile(game)); }

bool LightgunGames::IsGameALightgunGame(const std::string &gamepath) {
    if (gamepath == "")
        return false;

    auto it = find(begin(lightgunGamePaths), end(lightgunGamePaths), gamepath);
    return it != end(lightgunGamePaths);
}

void LightgunGames::AddGame(PsGamePtr game) {
    if (game)
        AddGame(PathForLightgunFile(game));
}

void LightgunGames::AddGame(const std::string &gamepath) {
    if (gamepath == "")
        return;

    if (!IsGameALightgunGame(gamepath)) {
        lightgunGamePaths.emplace_back(gamepath);
        UpdateFile();
    }
}

void LightgunGames::RemoveGame(PsGamePtr game) {
    if (game)
        RemoveGame(PathForLightgunFile(game));
}

void LightgunGames::RemoveGame(const std::string &gamepath) {
    if (gamepath == "")
        return;

    lightgunGamePaths.erase(remove(begin(lightgunGamePaths), end(lightgunGamePaths), gamepath), end(lightgunGamePaths));
    UpdateFile();
}

// Removes entries for games that no longer exist on disk
void LightgunGames::PurgeGamesNotFound() {
    lightgunGamePaths.erase(remove_if(begin(lightgunGamePaths), end(lightgunGamePaths),
                                      [](const string &gamepath) { return !FileUtils::exists(gamepath); }),
                            end(lightgunGamePaths));
    UpdateFile();
}

// Returns all games marked as lightgun games (PS1 and RetroArch), sorted by title
PsGames LightgunGames::GetAllLightgunGames() {
    PsGames lightgunGames;

    PurgeGamesNotFound();

    // add PS1 lightgun games
    PsGames psgames;
    Gui::getInstance()->db->getGames(&psgames);
    copy_if(begin(psgames), end(psgames), back_inserter(lightgunGames),
            [&](PsGamePtr game) { return IsGameALightgunGame(game); });

    // add RA lightgun games
    shared_ptr<RAIntegrator> integrator = RAIntegrator::getInstance();
    PsGames raGames = integrator->getAllRAGames();
    copy_if(begin(raGames), end(raGames), back_inserter(lightgunGames),
            [&](PsGamePtr game) { return IsGameALightgunGame(game); });

    sort(begin(lightgunGames), end(lightgunGames),
         [](PsGamePtr game1, PsGamePtr game2) { return game1->title < game2->title; });

    return lightgunGames;
}
