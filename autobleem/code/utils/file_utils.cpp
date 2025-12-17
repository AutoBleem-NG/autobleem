#include "file_utils.h"
#include "string_utils.h"
#include "../log.h"
#include <fstream>

using namespace std;

namespace FileUtils {

vector<string> readTextFile(const string &filePath, bool removeCRLF) {
    ifstream file;
    string line;
    vector<string> contents;

    file.open(filePath);
    if (!file.good()) {
        PLOG_ERROR << "Error opening file: " << filePath;
        return contents;
    }

    while (getline(file, line)) {
        if (removeCRLF) {
            // Remove CR and LF characters
            size_t crPos = line.find('\r');
            size_t lfPos = line.find('\n');
            if (crPos != string::npos || lfPos != string::npos) {
                size_t endPos = min(crPos, lfPos);
                if (endPos != string::npos) {
                    line = line.substr(0, endPos);
                }
            }
        }
        contents.emplace_back(line);
    }
    file.close();

    return contents;
}

bool writeTextFile(const string &filePath, const vector<string> &lines, bool appendLineEnding) {
    ofstream os(filePath, ios_base::trunc);
    if (!os.is_open()) {
        return false;
    }

    for (const string &s : lines) {
        if (appendLineEnding) {
            os << s << endl;
        } else {
            os << s;
        }
    }
    os << flush;
    return true;
}

istream &getlineRemoveCR(istream &is, string &str) {
    getline(is, str);
    // Remove carriage return if present
    if (!str.empty() && str.back() == '\r') {
        str.pop_back();
    }
    return is;
}

} // namespace FileUtils
