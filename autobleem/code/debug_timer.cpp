#include "debug_timer.h"
#include <SDL2/SDL_timer.h>
#include <stdio.h>
#include <iostream>
#include "log.h"

using namespace std;

#ifndef NDEBUG // if debug build
//*******************************
// DebugTimer::DebugTimer
//*******************************
// to use create a DebugTimer variable passing it the name of the function or other text.
// When the object goes out of scope it will output the time delay that has passed to cout.
//
DebugTimer::DebugTimer(const string &_description) : description(_description) {
    ticks_start = SDL_GetTicks();
    PLOG_DEBUG << description << ": start timer";
}

//*******************************
// DebugTimer::~DebugTimer
//*******************************
DebugTimer::~DebugTimer() {
    ticks_end = SDL_GetTicks();
    float time = float(ticks_end - ticks_start) / 1000.0;
    PLOG_DEBUG << description << ": " << time << " seconds";
};
#endif
