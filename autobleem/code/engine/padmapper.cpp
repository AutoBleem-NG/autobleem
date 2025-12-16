//
// Created by screemer on 2019-03-21.
//

#include "padmapper.h"
#include "../util.h"
#include <iostream>
#include "../log.h"
#include "../DirEntry.h"
#include "../environment.h"
#include "../gui/abl.h"
#include "../lang.h"
#include "../gui/gui.h"

extern int sdl_filter_analog_on;

void PadMapper::init() { SDL_SetEventFilter(&playstation_event_filter, nullptr); }

void PadMapper::registerPad(int joy_idx) {
    SDL_Joystick *js = SDL_JoystickOpen(joy_idx);
    if (js == nullptr)
        return;
    SDL_JoystickGUID guid = SDL_JoystickGetGUID(js);
    char guid_str[1024];
    SDL_JoystickGetGUIDString(guid, guid_str, sizeof(guid_str));
    if (!SDL_IsGameController(joy_idx)) {
        SDL_JoystickClose(js);
        return;
    }
    SDL_GameController *controller = nullptr;
    controller = SDL_GameControllerOpen(joy_idx);
    if (controller == nullptr)
        return;
    string name = SDL_GameControllerName(controller);
    PLOG_DEBUG << "New GameController name: " << name;
    char *mappingString = SDL_GameControllerMapping(controller);
    PLOG_DEBUG << "New GameController mapping: " << mappingString;
    ControllerInfo *info = new ControllerInfo();
    info->pad = controller;
    info->joy = SDL_GameControllerGetJoystick(controller);
    info->guid = guid_str;
    info->name = name;
    info->index = joy_idx;
    connectedPads.push_back(info);
    PLOG_DEBUG << "New GameController GUID: " << guid_str << "  Name:" << name;
    PLOG_DEBUG << "MAP: " << mappingString;
}
void PadMapper::handlePowerBtn(SDL_Event *event) {
    shared_ptr<Gui> gui(Gui::getInstance());
    if (event->type == SDL_KEYDOWN) {
        PLOG_DEBUG << event->key.keysym.scancode << " " << event->key.keysym.sym;
        if (event->key.keysym.scancode == SDL_SCANCODE_SLEEP || event->key.keysym.sym == SDLK_ESCAPE) {
            gui->drawText(_("POWERING OFF... PLEASE WAIT"));
            Util::powerOff();
        }
    }
}
void PadMapper::handleHotPlug(SDL_Event *event) {
    if (event->type == SDL_JOYDEVICEADDED) {
        registerPad(event->jdevice.which);
    } else if (event->type == SDL_JOYDEVICEREMOVED) {
        removePad(event->jdevice.which);
    }
}

void PadMapper::flushPads() {
    for (auto ci : connectedPads) {
        SDL_GameControllerClose(ci->pad);
        delete ci;
    }
    connectedPads.clear();
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
}

void PadMapper::removePad(int joy_idx) {
    int indexToRemove = -1;
    for (int i = 0; i < connectedPads.size(); i++) {
        ControllerInfo *ci = connectedPads[i];
        SDL_JoystickID instance_id = SDL_JoystickInstanceID(ci->joy);
        if (joy_idx == instance_id) {
            indexToRemove = i;
            PLOG_DEBUG << "Pad disconnected: " << ci->index << ":" << ci->name;
            SDL_GameControllerClose(ci->pad);
            delete ci;
            break;
        }
    }
    if (indexToRemove != -1)
        connectedPads.erase(connectedPads.begin() + indexToRemove);
}

void PadMapper::probePads() {
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    if (connectedPads.size() > 0) {
        flushPads();
    }
    bool mappingsLoaded = false;
    for (string controllerdbPath : gamedbpaths) {
        if (DirEntry::exists(controllerdbPath)) {
            int loadedMappings = SDL_GameControllerAddMappingsFromFile(controllerdbPath.c_str());
            PLOG_DEBUG << "Loaded pad mappings " << loadedMappings << " from " << controllerdbPath;
            mappingsLoaded = true;
            currentControllerdb = controllerdbPath;
            break;
        }
    }
    if (!mappingsLoaded) {
        PLOG_WARNING << "Warning: Default mapping db in use - no gamecontrollerdb.txt file found";
    }

    string zeroguid = "00000000000000000000000000000000";
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(i);
        char guid_cstr[1024];
        SDL_JoystickGetGUIDString(guid, guid_cstr, 1024);
        PLOG_DEBUG << "Checking device with GUID:" << guid_cstr;
        if (guid_cstr == zeroguid) {
            PLOG_DEBUG << "Invalid gamepad";
            continue;
        }
        if (SDL_IsGameController(i)) {
            PLOG_DEBUG << "Device is gamepad.... checking JoystickAPI name";
            string joyname = SDL_JoystickNameForIndex(i);
            PLOG_DEBUG << "Name:" << joyname;
            registerPad(i);
        }
    }
}

//*******************************
// PadMapper::isDirection
//*******************************
bool PadMapper::isDirection(SDL_Event *e, int dir) {
    if (e->type == SDL_CONTROLLERHATMOTIONDOWN) {
        if (e->cbutton.button == SDL_BTN_DUP)
            status[DUP] = true;
        if (e->cbutton.button == SDL_BTN_DDOWN)
            status[DDOWN] = true;
        if (e->cbutton.button == SDL_BTN_DLEFT)
            status[DLEFT] = true;
        if (e->cbutton.button == SDL_BTN_DRIGHT)
            status[DRIGHT] = true;
    }

    if (e->type == SDL_CONTROLLERHATMOTIONUP) {
        if (e->cbutton.button == SDL_BTN_DUP)
            status[DUP] = false;
        if (e->cbutton.button == SDL_BTN_DDOWN)
            status[DDOWN] = false;
        if (e->cbutton.button == SDL_BTN_DLEFT)
            status[DLEFT] = false;
        if (e->cbutton.button == SDL_BTN_DRIGHT)
            status[DRIGHT] = false;
    }

    switch (dir) {
    case DIR_UP:
        return status[DUP];
    case DIR_DOWN:
        return status[DDOWN];
    case DIR_LEFT:
        return status[DLEFT];
    case DIR_RIGHT:
        return status[DRIGHT];
    case DIR_NONE:
        return !status[DUP] && !status[DDOWN] && !status[DLEFT] && !status[DRIGHT];
    }

    return false;
}

//*******************************
// PadMapper::isCenter
//*******************************
bool PadMapper::isCenter(SDL_Event *event) { return isDirection(event, DIR_NONE); }

//*******************************
// PadMapper::isUp
//*******************************
bool PadMapper::isUp(SDL_Event *event) { return isDirection(event, DIR_UP); }

//*******************************
// PadMapper::isDown
//*******************************
bool PadMapper::isDown(SDL_Event *event) { return isDirection(event, DIR_DOWN); }

//*******************************
// PadMapper::isLeft
//*******************************
bool PadMapper::isLeft(SDL_Event *event) { return isDirection(event, DIR_LEFT); }

//*******************************
// PadMapper::isRight
//*******************************
bool PadMapper::isRight(SDL_Event *event) { return isDirection(event, DIR_RIGHT); }
