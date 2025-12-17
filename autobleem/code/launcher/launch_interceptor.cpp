//
// Created by screemer on 2019-12-01.
//

#include "launch_interceptor.h"
#include "../utils/time_utils.h"
#include "../gui/gui.h"
#include <iostream>
#include "../log.h"
#include <unistd.h>
#include "../environment.h"

#ifndef __APPLE__
#include <wait.h>
#endif
using namespace std;

bool LaunchInterceptor::execute(PsGamePtr &game, int resumepoint) {
    PLOG_DEBUG << "calling LaunchInterceptor::execute()";

    shared_ptr<Gui> gui(Gui::getInstance());
    PLOG_INFO << "Starting External App";

    if (game->internal) {
        gui->internalDB->updateDatePlayed(game->gameId, TimeUtils::getCurrentTime());
    } else {
        gui->db->updateDatePlayed(game->gameId, TimeUtils::getCurrentTime());
    }

    if (game->foreign) {
        PLOG_DEBUG << "FOREIGN MODE";
    }

    gui->saveSelection();
    std::vector<const char *> argvNew;

    string link = game->base + sep + game->startup;
    argvNew.push_back(link.c_str());
    argvNew.push_back(nullptr);

    PLOG_DEBUG << "CMD line to execute: ";
    for (const char *s : argvNew) {
        if (s != nullptr) {
            PLOG_DEBUG << s << " ";
        }
    }
    PLOG_DEBUG << endl;

#if defined(__x86_64__) || defined(_M_X64) || defined(PI_DEBUG)
    Gui::splash("I'm sorry Dave.  I'm afraid I can't do that.");
#else
    int pid = fork();
    if (!pid) {
        execvp(link.c_str(), (char **)argvNew.data());
    }
    waitpid(pid, NULL, 0);
    usleep(3 * 1000);
#endif

    return true;
}
