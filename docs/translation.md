# Translation Guide

AutoBleem supports 17 languages. This guide explains how to work with translations.

## Supported Languages

| Language | File |
|----------|------|
| Chinese (Simplified) | `Chinese_Simplified.txt` |
| Czech | `Czech.txt` |
| Danish | `Danish.txt` |
| Dutch | `Dutch.txt` |
| English | `English.txt` |
| Finnish | `Finnish.txt` |
| French | `French.txt` |
| German | `German.txt` |
| Italian | `Italian.txt` |
| Occitan | `Occitan.txt` |
| Polish | `Polish.txt` |
| Portuguese (Brazil) | `Portuguese_BR.txt` |
| Romanian | `Romanian.txt` |
| Slovak | `Slovak.txt` |
| Spanish | `Spanish.txt` |
| Swedish | `Swedish.txt` |
| Turkish | `Turkish.txt` |

## File Format

Translation files use a simple key=value format:

```
# Comments start with #
English Text=Translated Text
Another String=Another Translation
```

- **Key**: The English text (must match exactly)
- **Value**: The translated text
- **Empty value** (`Key=`): Untranslated string
- **Special key** `|@lang|`: Language code passed to PCSX emulator

## Translation Tools

The `lang_tools.py` script manages translations:

```bash
# Extract strings from source and sync all language files
make lang-update

# Validate all language files
make lang-validate

# Compare translations against English (show missing)
make lang-compare

# Validate a specific file
python3 autobleem/scripts/lang_tools.py validate autobleem/resources/lang/French.txt

# Compare a specific file
python3 autobleem/scripts/lang_tools.py compare autobleem/resources/lang/French.txt
```

## Adding Translations

1. Open the language file in `autobleem/resources/lang/`
2. Find strings with empty values (e.g., `Some Text=`)
3. Add the translation after the `=`
4. Save the file

Example:
```
# Before
Loading... Please Wait...=

# After
Loading... Please Wait...=Chargement... Veuillez patienter...
```

## Adding a New Language

1. Copy `English.txt` to `NewLanguage.txt`
2. Update the header comment
3. Set the locale code: `|@lang|=xx` (e.g., `ja` for Japanese)
4. Translate all strings
5. Run `make lang-validate` to check for errors

## Style Guidelines

- **Brand names**: Keep unchanged (AutoBleem, RetroArch, PlayStation, etc.)
- **Placeholders**: Preserve format strings like `%s`, `%d`
- **Punctuation**: Match the English style (periods, ellipsis, etc.)
- **Case**: Follow natural casing for the target language
- **Length**: Keep translations concise for UI display

## How It Works

The `_()` macro marks strings for translation in C++ code:

```cpp
Gui::splash(_("Loading... Please Wait..."));
```

At runtime, AutoBleem loads the selected language file and replaces English strings with translations.

## Font Support

AutoBleem includes fonts for:
- Latin characters (Western European languages)
- Japanese characters (partial CJK support)

For full CJK support (Chinese, Japanese, Korean), additional fonts may be needed.
