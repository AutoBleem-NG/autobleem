//
// Created by screemer on 2/4/19.
//
// Localization support for AutoBleem UI strings.
// Language files are stored in lang/*.txt with alternating lines:
//   - Odd lines: English source string
//   - Even lines: Translated string
//

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

// Global translation helper - use _("string") in code
std::string _(const std::string &input);

// Default/base language (no translation file needed)
static const std::string DEFAULT_LANG = "English";

// Singleton class managing translations
class Lang {
  public:
    std::string currentLang;

    std::string translate(const std::string &input);
    void load(const std::string &languageName);
    void dump(const std::string &fileName);
    std::vector<std::string> getListOfLanguages();

    Lang(const Lang &) = delete;
    Lang &operator=(const Lang &) = delete;

    static std::shared_ptr<Lang> getInstance() {
        static std::shared_ptr<Lang> s{new Lang};
        return s;
    }

  private:
    Lang() = default;
    std::map<std::string, std::string> langData;
    std::vector<std::string> newData; // Tracks untranslated strings for export
};
