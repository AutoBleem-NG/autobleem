# Changelog

All notable user-facing changes to AutoBleem-NG are documented here.

This project continues development from [AKA-Axanar's AutoBleem](https://github.com/AKA-Axanar/AutoBleem).

## [v1.1.0]

### Added
- CHD disc image format support - compressed game images that save USB space
- Chinese language support (Simplified) - experimental, no bundled font
- Logs written to `USB:/System/Logs/` for easier troubleshooting
- Version and build info displayed in logs to help with bug reports
- Docker-based build system - build ARM binaries without installing toolchain
- User documentation

### Fixed
- Database resource cleanup to prevent memory leaks during long sessions

### Changed
- All 17 language translations reviewed and completed
- Simplified boot scripts for easier maintenance
