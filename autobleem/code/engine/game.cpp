//
// Created by screemer on 2018-12-15.
//
#include "game.h"
#include "metadata.h"
#include "iso_dir.h"
#include "ini_file.h"
#include "cfg_processor.h"
#include "../gui/gui.h"
#include "serial_scanner.h"
#include "../utils/string_utils.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include "../log.h"
#include "../engine/scanner.h"
#include "../environment.h"
#include "../lang.h"
#include "../utils/time_utils.h"
#include <ctime>

using namespace std;

//*******************************
// USBGame::validateCue
//*******************************
bool USBGame::validateCue(string cuePath, string path) {
    vector<string> binFiles;
    string line;
    ifstream cueStream;
    bool result = true;

    cueStream.open(cuePath);
    while (getline(cueStream, line)) {
        line = trim(line);
        if (line.empty())
            continue;
        if (line.substr(0, 4) == "FILE") {
            line = line.substr(6, string::npos);
            line = line.substr(0, line.find('"'));
            binFiles.push_back(line);
        }
    }
    for (int i = 0; i < binFiles.size(); i++) {
        string binPath = path + sep + binFiles[i];
        if (!FileUtils::exists(binPath)) {
            result = false;
        } else {
            if (i == 0) {
                if (firstBinPath.empty()) {
                    firstBinPath = binPath;
                }
            }
        }
    }
    cueStream.close();
    return result;
}

//*******************************
// USBGame::valueOrDefault
//*******************************
string USBGame::valueOrDefault(string name, string def, bool setAutomationIfDefaultUsed) {
    string value;
    if (iniValues.find(name) != iniValues.end()) { // name is in the ini file
        value = trim(iniValues.find(name)->second);
        if (value.length() == 0) { // name is in the ini file but the value is empty
            if (setAutomationIfDefaultUsed)
                automationUsed = true;
            return def; // return default
        }
    } else { // name is not in the ini file
        if (setAutomationIfDefaultUsed)
            automationUsed = true;
        value = def; // return default
    }
    return value; // return value in ini file
}

//*******************************
// USBGame::verify
//*******************************
bool USBGame::verify(std::vector<std::string> *failureReasons) {
    bool result = true;

    if (discs.size() == 0) {
        if (failureReasons)
            failureReasons->emplace_back(_("No discs"));
        result = false;
    }

    for (int i = 0; i < discs.size(); i++) {
        if (discs[i].diskName.length() == 0) {
            if (failureReasons)
                failureReasons->emplace_back(_("No disc name"));
            result = false;
        }
        if (!discs[i].cueFound) {
            PLOG_DEBUG << i << discs[i].diskName << discs[i].cueFound;
            if (failureReasons)
                failureReasons->emplace_back(_("Cue file not found"));
            result = false;
        }
        if (!discs[i].binVerified) {
            if (failureReasons)
                failureReasons->emplace_back(_("Bin file failed to verify"));
            result = false;
        }
    }

    if (!gameDataFound) {
        if (failureReasons)
            failureReasons->emplace_back(_("Game file not found"));
        result = false;
    }
    if (!gameIniFound) {
        if (failureReasons)
            failureReasons->emplace_back(_("Game.ini file not found"));
        result = false;
    }
    if (!gameIniValid) {
        if (failureReasons)
            failureReasons->emplace_back(_("Game.ini file not valid"));
        result = false;
    }
    if (!coverImageFound) {
        if (failureReasons)
            failureReasons->emplace_back(_("Cover image file not found"));
        result = false;
    }
    if (!licFound) {
        if (failureReasons)
            failureReasons->emplace_back(_(".lic file not found"));
        result = false;
    }
    if (!pcsxCfgFound) {
        if (failureReasons)
            failureReasons->emplace_back(_("pcsx.cfg file not found"));
        result = false;
    }

    if (!result) {
        PLOG_WARNING << "Game: " << title << " validation failed";
    }

    return result;
}

//*******************************
// USBGame::print
//*******************************
bool USBGame::print() {
    PLOG_DEBUG << "-------------------";
    PLOG_DEBUG << "Printing game data:";
    PLOG_DEBUG << "-----------------";
    PLOG_DEBUG << "AUTOMATION: " << automationUsed;
    PLOG_DEBUG << "Game folder id: " << folder_id;
    PLOG_DEBUG << "Game: " << title;
    PLOG_DEBUG << "Players: " << players;
    PLOG_DEBUG << "Publisher: " << publisher;
    PLOG_DEBUG << "Year: " << year;
    PLOG_DEBUG << "Serial: " << serial;
    PLOG_DEBUG << "Region: " << region;
    PLOG_DEBUG << "GameData found: " << gameDataFound;
    PLOG_DEBUG << "Game.ini found: " << gameIniFound;
    PLOG_DEBUG << "Game.ini valid: " << gameIniValid;
    PLOG_DEBUG << "PNG found:" << coverImageFound;
    PLOG_DEBUG << "LIC found:" << licFound;
    PLOG_DEBUG << "pcsx.cfg found: " << pcsxCfgFound;
    PLOG_DEBUG << "TotalDiscs: " << discs.size();
    PLOG_DEBUG << "Favorite: " << favorite;
    PLOG_DEBUG << "Play Using RA: " << play_using_ra;
    PLOG_DEBUG << "Last Played: " << last_played;
    PLOG_DEBUG << "Last Played: " << TimeUtils::timeToDisplayTimeString(last_played);

    for (int i = 0; i < discs.size(); i++) {
        PLOG_DEBUG << "  Disc:" << i + 1 << "  " << discs[i].diskName;
        PLOG_DEBUG << "  CUE found: " << discs[i].cueFound;
        PLOG_DEBUG << "  BIN correct: " << discs[i].binVerified;
    }

    vector<string> failureReasons;
    bool result = verify(&failureReasons);
    if (result) {
        PLOG_DEBUG << "-------Game Verify OK-------";
    } else {
        PLOG_DEBUG << "------Game Verify FAIL------";
        for (const auto &reason : failureReasons)
            PLOG_DEBUG << "Reason: " << reason;
    }

    return result;
}

//*******************************
// USBGame::recoverMissingFiles
//*******************************
void USBGame::recoverMissingFiles() {
    string workingPath = Env::getWorkingPath();

    Metadata md;
    bool metadataLoaded = false;

    if (this->imageType == IMAGE_PBP) {
        // disc link
        string destinationDir = fullPath;
        string pbpFileName = FileUtils::findFirstFile(EXT_PBP, destinationDir);
        if (pbpFileName != "") {
            if (discs.size() == 0) {
                automationUsed = false;
                Disc disc;
                disc.diskName = pbpFileName; // the full filename including the .PBP
                disc.cueFound = true;
                disc.cueName = pbpFileName;
                disc.binVerified = true;
                discs.push_back(disc);
            }
        } else {
            automationUsed = true;
            PLOG_DEBUG << "Switching automation in PBP";
        }
    } else if (this->imageType == IMAGE_CHD) {
        // disc link
        string destinationDir = fullPath;
        string chdFileName = FileUtils::findFirstFile(EXT_CHD, destinationDir);
        if (chdFileName != "") {
            firstBinPath = destinationDir + sep + chdFileName;
            if (discs.size() == 0) {
                vector<string> extensions;
                extensions.push_back("chd");
                DirEntries allFiles = FileUtils::diru(destinationDir);
                DirEntries fileList = FileUtils::getFilesWithExtension(destinationDir, allFiles, extensions);
                automationUsed = false;
                for (DirEntry dirEntry : fileList) {
                    Disc *disc = new Disc();
                    disc->diskName = dirEntry.name; // the full filename including the .CHD
                    disc->cueFound = true;
                    disc->cueName = dirEntry.name;
                    disc->binVerified = true;
                    discs.push_back(*disc);
                    delete disc;
                }
            }
            if (this->imageType == IMAGE_CHD)
                imageType = IMAGE_CHD;
        } else {
            automationUsed = true;
            PLOG_DEBUG << "Switching automation in CHD";
        }
    }
    if (FileUtils::imageTypeUsesACueFile(this->imageType)) {
        if (discs.size() == 0) {
            automationUsed = true;
            PLOG_DEBUG << "Switching automation no discs";
            // find cue files
            string destination = fullPath;
            for (const DirEntry &entry : FileUtils::diru(destination)) {
                if (FileUtils::matchExtension(entry.name, EXT_CUE)) {
                    Disc disc;
                    string discEntry = entry.name.substr(0, entry.name.size() - 4); // remove .CUE
                    disc.diskName = discEntry;                                      // the CUE filename without the .CUE
                    disc.cueFound = true;
                    disc.cueName = discEntry; // the CUE filename without the .CUE
                    disc.binVerified = validateCue(destination + sep + entry.name, fullPath);
                    discs.push_back(disc);
                }
            }
        }
    }

    if (discs.size() > 0) {
        if (!licFound) {
            automationUsed = true;
            PLOG_DEBUG << "Switching automation no lic";
            string source = workingPath + sep + "default.lic";
            string destination = fullPath + sep + discs[0].diskName + ".lic";
            PLOG_DEBUG << "Copy: " << source << " -> " << destination;
            FileUtils::copy(source, destination);
            licFound = true;
        }
        if (!coverImageFound) {
            automationUsed = true;
            PLOG_DEBUG << "Switching automation no image";
            string source = workingPath + sep + "default.png";
            string destination = fullPath + sep + discs[0].diskName + ".png";
            PLOG_DEBUG << "Copy: " << source << " -> " << destination;
            FileUtils::copy(source, destination);
            // maybe we can do better ?
            PLOG_DEBUG << "getting serial from Image File";

            string serial = SerialScanner::scanSerial(imageType, fullPath, firstBinPath);
            if (serial != "") {

                if (md.lookupBySerial(serial)) {
                    metadataLoaded = true;
                    PLOG_DEBUG << "Updating cover in recoverMissingFiles()" << destination;
                    ofstream pngFile;
                    pngFile.open(destination, std::ios::out | std::ios::binary);
                    pngFile.write(md.bytes, md.dataSize);
                    pngFile.flush();
                    pngFile.close();
                    automationUsed = false;
                    coverImageFound = true;
                };
                md.clean();
            }
            coverImageFound = true;
        }
    }

    if (!pcsxCfgFound) {
        automationUsed = true;
        PLOG_DEBUG << "Switching automation no pcsx";
        string source = workingPath + sep + PCSX_CFG;
        string destination = fullPath + sep + PCSX_CFG;
        PLOG_DEBUG << "Copy: " << source << " -> " << destination;

        int region = 0;
        bool japan = false;

        if (!metadataLoaded) {
            string serial = SerialScanner::scanSerial(imageType, fullPath, firstBinPath);
            if (serial != "") {
                metadataLoaded = md.lookupBySerial(serial);
            }
        }

        if (metadataLoaded) {
            if (md.lastRegion == "U") {
                japan = false;
                region = 1;
            }
            if (md.lastRegion == "J") {
                japan = true;
                region = 1;
            }
            if (md.lastRegion == "P") {
                japan = false;
                region = 2;
            }
        }
        md.clean();
        shared_ptr<Gui> gui(Gui::getInstance());
        FileUtils::copy(source, destination);

        CfgProcessor *processor = new CfgProcessor();
        processor->replaceUSB(gameDirName, fullPath, "region", "region = " + to_string(region));
        delete (processor);
        pcsxCfgFound = true;
    }
}

//*******************************
// USBGame::updateObj
//*******************************
void USBGame::updateObj() {
    string tmp;
    discs.clear();
    title = valueOrDefault("title", gameDirName);
    memcard = valueOrDefault("memcard", "");

    region = valueOrDefault("region", "");
    serial = valueOrDefault("serial", "");

    publisher = valueOrDefault("publisher", "Other");
    string automation = valueOrDefault("automation", "0");
    automationUsed = atoi(automation.c_str());
    tmp = valueOrDefault("players", "1");
    if (StringUtils::isInteger(tmp.c_str()))
        players = atoi(tmp.c_str());
    else
        players = 1;
    tmp = valueOrDefault("year", "2018");

    if (StringUtils::isInteger(tmp.c_str()))
        year = atoi(tmp.c_str());
    else
        year = 2018;
    tmp = valueOrDefault("highres", "0");
    if (StringUtils::isInteger(tmp.c_str()))
        highRes = atoi(tmp.c_str());
    else
        highRes = false;
    favorite = valueOrDefault("favorite", "0", false); // favorite is a new field that didn't exist before so
    play_using_ra =
        valueOrDefault("play_using_ra", "false", false); // favorite is a new field that didn't exist before so
    // don't set automationUsed if it doesn't exist

    tmp = valueOrDefault("discs", "");
    if (!tmp.empty()) {
        vector<string> strings;
        istringstream f(tmp);
        string s;
        while (getline(f, s, ',')) {
            s = StringUtils::decode(s);
            strings.push_back(s);
        }
        for (const auto &diskName : strings) {
            Disc disc;
            disc.diskName = diskName;
            if (FileUtils::imageTypeUsesACueFile(imageType)) {
                string cueFile = fullPath + sep + disc.diskName + EXT_CUE;
                bool discCueExists = FileUtils::exists(cueFile);
                if (discCueExists) {
                    disc.binVerified = validateCue(cueFile, fullPath);
                    disc.cueFound = true;
                    disc.cueName = disc.diskName;
                }
                discs.push_back(disc);
            }
            if (imageType == IMAGE_PBP) {
                string pbpName = FileUtils::findFirstFile(EXT_PBP, fullPath);
                if (pbpName == disc.diskName) {
                    disc.cueFound = true;
                } else {
                    disc.cueFound = false;
                }

                disc.binVerified = true;
                disc.cueName = disc.diskName;
                discs.push_back(disc);
            }
            if (imageType == IMAGE_CHD) {
                string chdName = FileUtils::findFirstFile(EXT_CHD, fullPath);
                disc.cueFound = true;
                disc.binVerified = true;
                disc.cueName = disc.diskName;
                discs.push_back(disc);
            }
        }
    }
    gameIniValid = true;
}

//*******************************
// USBGame::saveIni
//*******************************
void USBGame::saveIni(string path) {
    // PLOG_DEBUG << "Overwritting ini file" << path ;
    Inifile *ini = new Inifile();
    ini->section = "Game";
    ini->values["title"] = title;
    ini->values["publisher"] = publisher;
    ini->values["year"] = to_string(year);
    ini->values["serial"] = serial;
    ini->values["region"] = region;
    ini->values["players"] = to_string(players);
    ini->values["automation"] = to_string(automationUsed);
    ini->values["imagetype"] = to_string(imageType);
    ini->values["highres"] = to_string(highRes);
    if (memcard.empty())
        ini->values["memcard"] = "SONY";
    else
        ini->values["memcard"] = memcard;

    ini->values["Favorite"] = favorite;
    ini->values["Play_using_ra"] = play_using_ra;

    stringstream ss;
    for (int i = 0; i < discs.size(); i++) {
        ss << StringUtils::escape(discs[i].diskName);
        if (i != discs.size() - 1) {
            ss << ",";
        }
    }
    ini->values["discs"] = ss.str();
    ini->save(path);
    delete ini;
    gameIniFound = true;
}

//*******************************
// USBGame::parseIni
//*******************************
void USBGame::parseIni(string path) {
    iniValues.clear();
    Inifile *ini = new Inifile();
    ini->load(path);
    if (ini->values.empty()) {
        gameIniFound = false;
        delete ini;
        return;
    }
    gameIniFound = true;
    iniValues = ini->values;
    delete ini;
}

//*******************************
// USBGame::readIni
//*******************************
void USBGame::readIni(string path) {
    parseIni(path);
    updateObj();
}

//*******************************
// USBGames += USBGames
//*******************************
void operator+=(USBGames &dest, const USBGames &src) { copy(begin(src), end(src), back_inserter(dest)); }
