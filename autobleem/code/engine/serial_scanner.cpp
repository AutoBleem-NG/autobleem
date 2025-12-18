//
// Created by screemer on 2/3/19.
//

#include "../main.h"
#include "serial_scanner.h"
#include "iso_dir.h"
#include <sstream>
#include "../io/binary_reader.h"
#include "../system/process_utils.h"
#include <fstream>
#include <iostream>
#include "../log.h"
#include "../utils/file_utils.h"

using namespace std;

// The SerialScanner class reads the serial number in a CDROM BIN file which is an ISO 9660 image of a CDROM.
// https://en.wikipedia.org/wiki/ISO_9660

//*******************************
// SerialScanner::fixSerial
//*******************************
string SerialScanner::fixSerial(string serial) {
    replace(serial.begin(), serial.end(), '_', '-');
    serial.erase(remove(serial.begin(), serial.end(), '.'), serial.end());
    string fixed = "";
    stringstream alpha;
    stringstream digits;
    bool digitsProcessing = false;
    for (int i = 0; i < serial.size(); i++) {
        int maxchars = serial[0] == 'L' ? 3 : 4;
        if (!isdigit(serial[i])) {

            if (digitsProcessing)
                continue;
            if (serial[i] == '-')
                continue;
            if (alpha.str().length() < maxchars) {
                alpha << serial[i];
            }
        } else {
            digitsProcessing = true;
            digits << serial[i];
        }
    }
    return alpha.str() + "-" + digits.str();
}

//*******************************
// SerialScanner::scanSerial
//*******************************
string SerialScanner::scanSerial(ImageType imageType, string path, string firstBinPath) {
    string serial = scanSerialInternal(imageType, path, firstBinPath);
    PLOG_DEBUG << serial;
    if (serial.empty()) {
        serial = workarounds(imageType, path, firstBinPath);
    }
    return serial;
}

//*******************************
// SerialScanner::scanSerialInternal
//*******************************
string SerialScanner::scanSerialInternal(ImageType imageType, string path, string firstBinPath) {
    PLOG_DEBUG << imageType << "   " << path << "   " << firstBinPath;
    if (imageType == IMAGE_PBP) {
        string destinationDir = path;
        string pbpFileName = FileUtils::findFirstFile(EXT_PBP, destinationDir);
        if (pbpFileName != "") {
            ifstream is;
            is.open(destinationDir + sep + pbpFileName);
            BinaryIO::Reader reader(&is);

            long magic = reader.readDword();
            if (magic != 0x50425000) {
                return "";
            }
            long second = reader.readDword();
            if (second != 0x10000) {
                return "";
            }
            long sfoStart = reader.readDword();

            is.seekg(sfoStart, ios::beg);

            unsigned int signature = reader.readDword();
            if (signature != 1179865088) {
                return "";
            }
            unsigned int version = reader.readDword();
            ;
            unsigned int fields_table_offs = reader.readDword();
            unsigned int values_table_offs = reader.readDword();
            int nitems = reader.readDword();

            vector<string> fields;
            vector<string> values;
            fields.clear();
            values.clear();
            is.seekg(sfoStart, ios::beg);
            is.seekg(fields_table_offs, ios::cur);
            for (int i = 0; i < nitems; i++) {
                string fieldName = reader.readNullTerminatedString();
                reader.skipZeros();
                fields.push_back(fieldName);
            }

            is.seekg(sfoStart, ios::beg);
            is.seekg(values_table_offs, ios::cur);
            for (int i = 0; i < nitems; i++) {
                string valueName = reader.readNullTerminatedString();
                reader.skipZeros();
                values.push_back(valueName);
            }

            is.close();

            for (int i = 0; i < nitems; i++) {
                if (fields[i] == "DISC_ID") {
                    string potentialSerial = values[i];
                    return fixSerial(potentialSerial);
                }
            }
        }
    }
    if (FileUtils::imageTypeUsesACueFile(imageType) || (imageType == IMAGE_CHD)) {
        string prefixes[] = {"CPCS", "ESPM", "HPS",  "LPS",  "LSP",  "SCAJ", "SCED", "SCES",
                             "SCPS", "SCUS", "SIPS", "SLES", "SLKA", "SLPM", "SLPS", "SLUS"};
        if (firstBinPath == "") {
            return ""; // not at this stage
        }

        for (int level = 1; level < 4; level++) {
            Isodir *dirLoader = new Isodir();

            IsoDirectory dir = dirLoader->getDir(firstBinPath, level, imageType == IMAGE_CHD);
            delete dirLoader;
            string serialFound = "";
            if (!dir.rootDir.empty()) {
                for (const string &entry : dir.rootDir) {
                    //   PLOG_DEBUG << entry ;
                    string potentialSerial = fixSerial(entry);
                    for (const string &prefix : prefixes) {
                        int pos = potentialSerial.find(prefix.c_str(), 0);
                        if (pos == 0) {
                            serialFound = potentialSerial;
                            PLOG_DEBUG << "Serial number: " << serialFound;
                            return serialFound;
                        }
                    }
                }
                string volume = fixSerial(dir.volumeName);
                for (const string &prefix : prefixes) {
                    int pos = volume.find(prefix.c_str(), 0);
                    if (pos == 0) {
                        serialFound = volume;
                        PLOG_DEBUG << "Serial number: " << serialFound;
                        return serialFound;
                    }
                }

            } else {
                return "";
            }
        }
    }
    return "";
}

//*******************************
// SerialScanner::workarounds
//*******************************
string SerialScanner::workarounds(ImageType imageType, string path, string firstBinPath) {
    string fileToScan = "";
    if (FileUtils::imageTypeUsesACueFile(imageType)) {
        fileToScan = firstBinPath;
    }
    if (imageType == IMAGE_PBP) {
        fileToScan = FileUtils::findFirstFile(EXT_PBP, path);
    }
    if (imageType == IMAGE_CHD) {
        fileToScan = FileUtils::findFirstFile(EXT_CHD, path);
    }

    // BH2 - Resident Evil 1.5
    if (fileToScan.find("BH2") != string::npos) {
        return serialByMd5(fileToScan);
    }
    return "";
}

//*******************************
// SerialScanner::serialByMd5
//*******************************
string SerialScanner::serialByMd5(string scanFile) {
    string head = System::executeCommand(("head -c 1M \"" + scanFile + "\" | md5sum | awk '{print $1}'").c_str());
    string tail = System::executeCommand(("tail -c 1M \"" + scanFile + "\" | md5sum | awk '{print $1}'").c_str());

    return head + tail;
}

//*******************************
// SerialScanner::serialToRegion
//*******************************
string SerialScanner::serialToRegion(const string &serial) {
    string region;
    if (serial.length() >= 3) {
        char regionCode = serial[2];
        if (regionCode == 'U')
            region = "US"; // SLUS, SCUS = NTSC-U
        else if (regionCode == 'E')
            region = "Europe-Aus"; // SLES, SCES = PAL
        else if (regionCode == 'P')
            region = "Japan"; // SLPS, SLPM, SCPS = NTSC-J
    }

    return region;
}
