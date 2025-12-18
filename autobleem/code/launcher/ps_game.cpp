//
// Created by screemer on 2/12/19.
//

#include "ps_game.h"
#include "../engine/ini_file.h"
#include "../gui/gui.h"
#include <fstream>
#include <iostream>
#include "../log.h"

using namespace std;

//*******************************
// PsGame::isCleanExit
//*******************************
bool PsGame::isCleanExit() {
    if (!foreign) {
        string filenamefile = ssFolder + sep + "filename.txt";
        bool ret = FileUtils::exists(filenamefile);
        if (!ret)
            PLOG_DEBUG << "PsGame::isCleanExit() failed '" << filenamefile << "' not found";
        return ret;
    } else {
        return true;
    }
}

//*******************************
// PsGame::setMemCard
//*******************************
void PsGame::setMemCard(string name) {
    if (!foreign) {
        this->memcard = name;
        Inifile ini;
        ini.load(this->folder + sep + GAME_INI);
        ini.values["memcard"] = name;
        ini.save(this->folder + sep + GAME_INI);
        shared_ptr<Gui> gui(Gui::getInstance());
        gui->db->updateMemcard(this->gameId, name);
    }
}

//*******************************
// PsGame::removeResumePoint
//*******************************
void PsGame::removeResumePoint(int slot) {
    if (!foreign) {
        string filenamefile = ssFolder + sep + "filename.txt.res";
        string filenamepoint = ssFolder + sep + "filename." + to_string(slot) + ".txt.res";
        if (FileUtils::exists(filenamepoint)) {
            filenamefile = filenamepoint;
        }
        if (FileUtils::exists(filenamefile)) {
            ifstream is(filenamefile.c_str());
            if (is.is_open()) {

                std::string line;
                std::getline(is, line);
                std::getline(is, line);

                string ssfile = ssFolder + sep + "sstates/" + line + ".00" + to_string(slot) + ".res";
                FileUtils::removeFile(ssfile);
                // last line is our filename
                if (slot == 0) {
                    string slot0img = ssFolder + sep + "screenshots/" + line + ".png.res";
                    FileUtils::removeFile(slot0img);

                } else {
                    string slotnimg = ssFolder + sep + "screenshots/" + line + "." + to_string(slot) + ".png.res";
                    FileUtils::removeFile(slotnimg);
                }
                is.close();
            }
        }
    }
}

//*******************************
// PsGame::isResumeSlotActive
//*******************************
bool PsGame::isResumeSlotActive(int slot) {
    if (!foreign) {
        string filenamefile = ssFolder + sep + "filename.txt.res";
        string filenamepoint = ssFolder + sep + "filename." + to_string(slot) + ".txt.res";
        if (FileUtils::exists(filenamepoint)) {
            filenamefile = filenamepoint;
        }
        if (FileUtils::exists(filenamefile)) {
            ifstream is(filenamefile.c_str());
            if (is.is_open()) {

                std::string line;
                std::getline(is, line);
                std::getline(is, line);

                // last line is our filename
                if (slot == 0) {
                    string slot0img = ssFolder + sep + "screenshots/" + line + ".png.res";
                    if (FileUtils::exists(slot0img)) {
                        return true;
                    }
                } else {
                    string slotnimg = ssFolder + sep + "screenshots/" + line + "." + to_string(slot) + ".png.res";
                    if (FileUtils::exists(slotnimg)) {
                        return true;
                    }
                }
                is.close();
            }
        }
    }
    return false;
}

//*******************************
// PsGame::storeResumePicture
//*******************************
void PsGame::storeResumePicture(int slot) {
    if (!foreign) {
        string filenamefile = ssFolder + sep + "filename.txt.res";
        string filenamepoint = ssFolder + sep + "filename." + to_string(slot) + ".txt.res";
        if (FileUtils::exists(filenamepoint)) {
            filenamefile = filenamepoint;
        }
        if (FileUtils::exists(filenamefile)) {
            ifstream is(filenamefile.c_str());
            if (is.is_open()) {

                std::string line;
                std::getline(is, line);
                std::getline(is, line);

                string inputImg = ssFolder + sep + "screenshots/" + line + ".png";
                if (!FileUtils::exists(inputImg)) {
                    return;
                }
                string slotImg;
                // last line is our filename
                if (slot == 0) {
                    slotImg = ssFolder + sep + "screenshots/" + line + ".png.res";

                } else {
                    slotImg = ssFolder + sep + "screenshots/" + line + "." + to_string(slot) + ".png.res";
                }
                is.close();

                FileUtils::removeFile(slotImg);
                FileUtils::copy(inputImg, slotImg);
                FileUtils::removeFile(inputImg);
            }
        }
    }
}

//*******************************
// PsGame::findResumePicture
//*******************************
string PsGame::findResumePicture(int slot) {
    if (!foreign) {
        string filenamefile = ssFolder + sep + "filename.txt.res";
        string filenamepoint = ssFolder + sep + "filename." + to_string(slot) + ".txt.res";
        if (FileUtils::exists(filenamepoint)) {
            filenamefile = filenamepoint;
        }
        if (FileUtils::exists(filenamefile)) {
            ifstream is(filenamefile.c_str());
            if (is.is_open()) {

                std::string line;
                std::getline(is, line);
                std::getline(is, line);

                // last line is our filename
                if (slot == 0) {
                    string slot0img = ssFolder + sep + "screenshots/" + line + ".png.res";
                    if (FileUtils::exists(slot0img)) {
                        return slot0img;
                    }
                } else {
                    string slotnimg = ssFolder + sep + "screenshots/" + line + "." + to_string(slot) + ".png.res";
                    if (FileUtils::exists(slotnimg)) {
                        return slotnimg;
                    }
                }
                is.close();
            }
        }
    }
    return "";
}

//*******************************
// PsGame::findResumePicture
//*******************************
string PsGame::findResumePicture() {
    // try to do it in silly Sony way
    if (!foreign) {
        string filenamefile = ssFolder + sep + "filename.txt.res";
        for (int i = 0; i < 4; i++) {
            string filenamepoint = ssFolder + sep + "filename." + to_string(i) + ".txt.res";
            if (FileUtils::exists(filenamepoint)) {
                filenamefile = filenamepoint;
                break;
            }
        }
        if (FileUtils::exists(filenamefile)) {
            ifstream is(filenamefile.c_str());
            if (is.is_open()) {

                std::string line;
                std::getline(is, line);
                std::getline(is, line);

                // last line is our filename
                string pngfile = ssFolder + sep + "screenshots/" + line + ".png.res";
                if (FileUtils::exists(pngfile)) {
                    return pngfile;
                }
                is.close();
            }
        }
    }
    return "";
}

//*******************************
// PsGames += PsGames
//*******************************
void operator+=(PsGames &dest, const PsGames &src) { copy(begin(src), end(src), back_inserter(dest)); }
