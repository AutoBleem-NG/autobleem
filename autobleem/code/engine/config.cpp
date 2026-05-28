//
// Created by screemer on 23.12.18.
//

#include "config.h"
#include "../utils/file_utils.h"
#include "../environment.h"
#include "../lang.h"
#include "../launcher/gui_notification_line.h"

//*******************************
// Config::Config()
//*******************************
Config::Config() {
    std::string path = Env::getWorkingPath() + sep + "config.ini";
    inifile.load(path);

    bool aDefaultWasSet{false};
    if (inifile.values["language"] == "") {
        inifile.values["language"] = DEFAULT_LANG;
        aDefaultWasSet = true;
    }
    if (inifile.values["aspect"] == "") {
        inifile.values["aspect"] = "false";
        aDefaultWasSet = true;
    }
    if (inifile.values["jewel"] == "") {
        inifile.values["jewel"] = "default";
        aDefaultWasSet = true;
    }
    if (inifile.values["music"] == "") {
        inifile.values["music"] = "--";
        aDefaultWasSet = true;
    }
    if (inifile.values["font"] == "") {
        inifile.values["font"] = "--";
        aDefaultWasSet = true;
    }
    if (inifile.values["themefont"] == "") {
        inifile.values["themefont"] = "true";
        aDefaultWasSet = true;
    }
    if (inifile.values["showingtimeout"] == "") {
        inifile.values["showingtimeout"] = DefaultShowingTimeoutText;
        aDefaultWasSet = true;
    }

    if (inifile.values["raconfig"] == "") {
        inifile.values["raconfig"] = "true";
        aDefaultWasSet = true;
    }

    if (aDefaultWasSet)
        save();
}

//*******************************
// Config::save
//*******************************
void Config::save() {
    std::string path = Env::getWorkingPath() + sep + "config.ini";
    inifile.save(path);
}
