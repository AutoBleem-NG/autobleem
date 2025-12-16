#!/usr/bin/env python3
"""
Language file management tools for AutoBleem.

Commands:
    extract     Extract translatable strings from source code
    update      Sync all language files with English.txt keys
    convert     Convert old alternating-line format to key=value format
    validate    Validate a language file
    compare     Compare a language file against English.txt
    help        Show this help message

File format (key=value):
    # Comments start with #
    English Text=Translated Text
    Another String=Another Translation

Workflow:
    1. make english          # Extract strings from source
    2. make lang-update      # Add missing keys to all language files
    3. Translate strings where key=value (untranslated)
"""

import argparse
import os
import re
import sys
from pathlib import Path


def extract_strings(src_dir: Path) -> set[str]:
    """Extract all _("...") strings from C++ source files, skipping comments."""
    pattern = re.compile(r'_\("([^"]+)"\)')
    strings = set()

    for ext in ('*.cpp', '*.h'):
        for filepath in src_dir.rglob(ext):
            # Skip libs directory
            if 'libs' in filepath.parts:
                continue
            try:
                content = filepath.read_text(encoding='utf-8')
                # Process line by line to skip commented lines
                for line in content.splitlines():
                    stripped = line.lstrip()
                    # Skip single-line comments
                    if stripped.startswith('//'):
                        continue
                    # Skip lines where _( appears after //
                    comment_pos = line.find('//')
                    if comment_pos != -1:
                        line = line[:comment_pos]
                    matches = pattern.findall(line)
                    strings.update(matches)
            except Exception as e:
                print(f"Warning: Could not read {filepath}: {e}", file=sys.stderr)

    return strings


def parse_old_format(filepath: Path) -> dict[str, str]:
    """Parse old alternating-line format (key\\nvalue\\nkey\\nvalue...)."""
    translations = {}
    lines = []

    content = filepath.read_text(encoding='utf-8-sig')  # Handle BOM
    for line in content.splitlines():
        line = line.strip()
        if line:  # Skip empty lines
            lines.append(line)

    # Parse alternating lines
    for i in range(0, len(lines) - 1, 2):
        key = lines[i]
        value = lines[i + 1]
        translations[key] = value

    return translations


def parse_new_format(filepath: Path) -> dict[str, str]:
    """Parse new key=value format."""
    translations = {}

    content = filepath.read_text(encoding='utf-8-sig')  # Handle BOM
    for line in content.splitlines():
        line = line.strip()

        # Skip empty lines and comments
        if not line or line.startswith('#'):
            continue

        # Parse key=value (first = is delimiter)
        pos = line.find('=')
        if pos != -1:
            key = line[:pos].strip()
            value = line[pos + 1:].strip()
            if key:
                translations[key] = value

    return translations


def write_new_format(filepath: Path, translations: dict[str, str], header: str = None):
    """Write translations in new key=value format."""
    with open(filepath, 'w', encoding='utf-8') as f:
        if header:
            for line in header.splitlines():
                f.write(f"# {line}\n")
            f.write("\n")

        # Sort keys for consistent output
        for key in sorted(translations.keys()):
            value = translations[key]
            f.write(f"{key}={value}\n")


def cmd_extract(args):
    """Extract strings from source code and generate English.txt."""
    src_dir = Path(args.src_dir)
    output = Path(args.output)

    if not src_dir.exists():
        print(f"Error: Source directory not found: {src_dir}", file=sys.stderr)
        return 1

    print(f"Extracting strings from {src_dir}...")
    strings = extract_strings(src_dir)

    # Create translations dict (English -> English)
    translations = {s: s for s in strings}

    header = f"AutoBleem English Strings\nExtracted from source code\nTotal: {len(strings)} strings"
    write_new_format(output, translations, header)

    print(f"Generated {output} with {len(strings)} strings")
    return 0


def cmd_convert(args):
    """Convert old format file to new key=value format."""
    input_path = Path(args.input)
    output_path = Path(args.output) if args.output else input_path

    if not input_path.exists():
        print(f"Error: Input file not found: {input_path}", file=sys.stderr)
        return 1

    print(f"Converting {input_path}...")
    translations = parse_old_format(input_path)

    # Detect language name from filename
    lang_name = input_path.stem

    header = f"AutoBleem {lang_name} Translation\nFormat: English Text=Translated Text\nTotal: {len(translations)} translations"
    write_new_format(output_path, translations, header)

    print(f"Converted {len(translations)} translations to {output_path}")
    return 0


def cmd_validate(args):
    """Validate a language file for formatting issues."""
    filepath = Path(args.file)

    if not filepath.exists():
        print(f"Error: File not found: {filepath}", file=sys.stderr)
        return 1

    errors = []
    warnings = []

    content = filepath.read_text(encoding='utf-8-sig')
    lines = content.splitlines()

    seen_keys = {}
    line_num = 0

    for line in lines:
        line_num += 1
        stripped = line.strip()

        # Check for trailing whitespace
        if line != stripped and stripped:
            warnings.append(f"Line {line_num}: Trailing whitespace")

        # Skip empty lines and comments
        if not stripped or stripped.startswith('#'):
            continue

        # Check for = delimiter
        pos = stripped.find('=')
        if pos == -1:
            errors.append(f"Line {line_num}: Missing '=' delimiter: {stripped[:50]}...")
            continue

        key = stripped[:pos].strip()
        value = stripped[pos + 1:].strip()

        if not key:
            errors.append(f"Line {line_num}: Empty key")
            continue

        # Check for duplicate keys
        if key in seen_keys:
            warnings.append(f"Line {line_num}: Duplicate key '{key}' (first at line {seen_keys[key]})")
        else:
            seen_keys[key] = line_num

        # Check for untranslated (empty value)
        if not value:
            warnings.append(f"Line {line_num}: Untranslated: '{key}'")

    # Report results
    if errors:
        print("ERRORS:")
        for e in errors:
            print(f"  {e}")

    if warnings and args.verbose:
        print("WARNINGS:")
        for w in warnings:
            print(f"  {w}")

    if not errors and not warnings:
        print(f"OK: {filepath} ({len(seen_keys)} translations)")
    elif not errors:
        print(f"OK with {len(warnings)} warnings: {filepath} ({len(seen_keys)} translations)")

    return 1 if errors else 0


def cmd_compare(args):
    """Compare a language file against English.txt to find missing/extra translations."""
    lang_path = Path(args.lang_file)
    english_path = Path(args.english_file)

    if not lang_path.exists():
        print(f"Error: Language file not found: {lang_path}", file=sys.stderr)
        return 1
    if not english_path.exists():
        print(f"Error: English file not found: {english_path}", file=sys.stderr)
        return 1

    english = parse_new_format(english_path)
    lang = parse_new_format(lang_path)

    english_keys = set(english.keys())
    lang_keys = set(lang.keys())

    missing = english_keys - lang_keys
    extra = lang_keys - english_keys

    if missing:
        print(f"MISSING ({len(missing)} strings not translated):")
        for key in sorted(missing):
            print(f"  {key}")

    if extra:
        print(f"\nEXTRA ({len(extra)} strings not in English.txt):")
        for key in sorted(extra):
            print(f"  {key}")

    if not missing and not extra:
        print(f"OK: {lang_path.stem} has all {len(english_keys)} translations")

    # Summary
    translated = len(lang_keys & english_keys)
    total = len(english_keys)
    pct = (translated / total * 100) if total > 0 else 0
    print(f"\nCoverage: {translated}/{total} ({pct:.1f}%)")

    return 0


def cmd_update(args):
    """Sync all language files with English.txt keys."""
    lang_dir = Path(args.lang_dir)
    english_path = lang_dir / "English.txt"

    if not english_path.exists():
        print(f"Error: English.txt not found at {english_path}", file=sys.stderr)
        print("Run 'make english' first to generate it.", file=sys.stderr)
        return 1

    english = parse_new_format(english_path)
    english_keys = set(english.keys())

    print(f"English.txt has {len(english_keys)} strings\n")

    total_added = 0
    total_removed = 0

    # Process each language file
    for lang_file in sorted(lang_dir.glob("*.txt")):
        if lang_file.name == "English.txt":
            continue

        lang_name = lang_file.stem
        lang = parse_new_format(lang_file)
        lang_keys = set(lang.keys())

        # Find missing and extra keys
        missing = english_keys - lang_keys
        extra = lang_keys - english_keys

        # Build updated translations
        updated = {}

        # Keep existing translations that are still in English
        for key in english_keys:
            if key in lang and lang[key]:
                # Keep existing translation (including when value == key for brand names)
                updated[key] = lang[key]
            else:
                # Missing or untranslated - leave empty for translator
                updated[key] = ""

        # Count actually translated (non-empty values)
        translated_count = sum(1 for v in updated.values() if v)
        untranslated_count = len(updated) - translated_count

        # Remove obsolete keys (not in English anymore)
        if args.remove_obsolete:
            removed_count = len(extra)
        else:
            # Keep obsolete translations (might be useful)
            for key in extra:
                updated[key] = lang[key]
            removed_count = 0

        # Write updated file
        header = f"AutoBleem {lang_name} Translation\nFormat: English Text=Translated Text\nUntranslated: {untranslated_count} / Total: {len(english_keys)}"
        write_new_format(lang_file, updated, header)

        # Report
        status_parts = []
        if missing:
            status_parts.append(f"+{len(missing)} added")
            total_added += len(missing)
        if removed_count > 0:
            status_parts.append(f"-{removed_count} removed")
            total_removed += removed_count

        status = ", ".join(status_parts) if status_parts else "no changes"
        pct = (translated_count / len(english_keys) * 100) if english_keys else 0
        print(f"{lang_name:20} {translated_count:3}/{len(english_keys)} ({pct:5.1f}%) translated  [{status}]")

    print(f"\nTotal: +{total_added} added, -{total_removed} removed across all languages")
    return 0


def main():
    parser = argparse.ArgumentParser(
        description='Language file management tools for AutoBleem',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )
    subparsers = parser.add_subparsers(dest='command', help='Command to run')

    # extract command
    p_extract = subparsers.add_parser('extract', help='Extract strings from source code')
    p_extract.add_argument('--src-dir', default='src/code',
                          help='Source directory to scan (default: src/code)')
    p_extract.add_argument('--output', '-o', default='src/resources/lang/English.txt',
                          help='Output file (default: src/resources/lang/English.txt)')

    # convert command
    p_convert = subparsers.add_parser('convert', help='Convert old format to key=value')
    p_convert.add_argument('input', help='Input file (old format)')
    p_convert.add_argument('--output', '-o', help='Output file (default: overwrite input)')

    # validate command
    p_validate = subparsers.add_parser('validate', help='Validate a language file')
    p_validate.add_argument('file', help='Language file to validate')
    p_validate.add_argument('--verbose', '-v', action='store_true',
                           help='Show warnings as well as errors')

    # compare command
    p_compare = subparsers.add_parser('compare', help='Compare language file against English.txt')
    p_compare.add_argument('lang_file', help='Language file to compare')
    p_compare.add_argument('--english-file', '-e', default='src/resources/lang/English.txt',
                          help='English reference file (default: src/resources/lang/English.txt)')

    # update command
    p_update = subparsers.add_parser('update', help='Sync all language files with English.txt keys')
    p_update.add_argument('--lang-dir', '-d', default='src/resources/lang',
                          help='Language directory (default: src/resources/lang)')
    p_update.add_argument('--remove-obsolete', '-r', action='store_true',
                          help='Remove keys not in English.txt (default: keep them)')

    args = parser.parse_args()

    if not args.command:
        parser.print_help()
        return 0

    commands = {
        'extract': cmd_extract,
        'update': cmd_update,
        'convert': cmd_convert,
        'validate': cmd_validate,
        'compare': cmd_compare,
    }

    return commands[args.command](args)


if __name__ == '__main__':
    sys.exit(main())
