#include "debug_timer.h"

#include <SDL2/SDL_timer.h>

#include <iostream>

#include "log.h"

using namespace std;

#ifndef NDEBUG
DebugTimer::DebugTimer(const string &desc) : description(desc), ticks_start(SDL_GetTicks()) {
    PLOG_DEBUG << description << ": start timer";
}

DebugTimer::~DebugTimer() {
    ticks_end = SDL_GetTicks();
    float time = static_cast<float>(ticks_end - ticks_start) / 1000.0f;
    PLOG_DEBUG << description << ": " << time << " seconds";
}
#endif
