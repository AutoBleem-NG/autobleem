//
// Created by screemer on 2/4/19.
//

#include "lang.h"

#include <fstream>

#include "DirEntry.h"
#include "environment.h"
#include "log.h"
#include "utils/string_utils.h"

using std::endl;
using std::ifstream;
using std::map;
using std::ofstream;
using std::string;
using std::vector;

// Global translation helper function
string _(const string &input) { return Lang::getInstance()->translate(input); }

string Lang::translate(const string &input) {
    string trimmed = input;
    trim(trimmed);
    if (trimmed.empty())
        return "";

    if (currentLang == DEFAULT_LANG)
        return trimmed;

    string &translated = langData[trimmed];
    if (translated.empty()) {
        translated = trimmed;
        newData.push_back(trimmed);
    }
    return translated;
}

void Lang::load(const string &languageName) {
    langData.clear();
    newData.clear();
    currentLang = languageName;

    if (languageName == DEFAULT_LANG)
        return;

    string path = Env::getWorkingPath() + sep + "lang" + sep + languageName + ".txt";
    ifstream is(path);
    if (!is.is_open()) {
        PLOG_WARNING << "Failed to open language file: " << path;
        return;
    }

    string line;
    bool firstLine = true;

    while (std::getline(is, line)) {
        // Strip UTF-8 BOM from first line if present
        if (firstLine && line.size() >= 3) {
            const auto *p = reinterpret_cast<const unsigned char *>(line.c_str());
            if (p[0] == 0xEF && p[1] == 0xBB && p[2] == 0xBF) {
                line = line.substr(3);
            }
            firstLine = false;
        }

        trim(line);

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#')
            continue;

        // Parse key=value format (first '=' is delimiter)
        size_t pos = line.find('=');
        if (pos != string::npos) {
            string key = line.substr(0, pos);
            string value = line.substr(pos + 1);
            trim(key);
            trim(value);
            if (!key.empty()) {
                langData[key] = value;
            }
        }
    }
}

// Exports untranslated strings for translators to complete
void Lang::dump(const string &fileName) {
    string fileSave = Env::getWorkingPath() + sep + fileName;
    ofstream os(fileSave);
    if (!os.is_open()) {
        PLOG_ERROR << "Failed to open dump file: " << fileSave;
        return;
    }

    os << "# Untranslated strings - add translations after the '='" << endl;
    os << "# Format: English Text=Translated Text" << endl;
    os << endl;

    for (const string &data : newData) {
        PLOG_DEBUG << data;
        os << data << "=" << data << endl;
    }
    os.flush();
    os.close();
}

vector<string> Lang::getListOfLanguages() {
    vector<string> languages;
    languages.push_back(DEFAULT_LANG);

    string langDir = Env::getWorkingPath() + sep + "lang";
    string defaultFile = DEFAULT_LANG + ".txt";
    for (const DirEntry &entry : DirEntry::diru(langDir)) {
        if (DirEntry::matchExtension(entry.name, ".txt") && !StringUtils::compareCaseInsensitive(entry.name, defaultFile)) {
            // Strip .txt extension
            languages.push_back(entry.name.substr(0, entry.name.size() - 4));
        }
    }
    return languages;
}
