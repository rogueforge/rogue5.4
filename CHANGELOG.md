# Changelog
All notable changes to this project will be documented in this file.

## [Unreleased]

## [5.4.5] - 2008-09-04
### Added
- Some support needed for rog-o-matic
### Changed
- Refactored save state code
### Fixed
- Do not create wanderer on top of mimic
- Potential endless loop in wanderer when no room for monsters
- Monster chases player if player picks up something that is automatically discarded
- Prevent placing a trap on a trap 
- Show_map wizard function showing excess characters in standout mode 
- Quote PASSWD in configure script for --enable-wizardmode
- Long standing nymph/inventory bug fixed
- Other fixes by Peter Shauer, Ed Sirett and Wart

## [5.4.4] - 2007-09-05
### Added
- Port to Mac OS/X
- RPM spec file and desktop integration files contributed by Wart

### Changed
- Include autoconf related files in source tarball
- Clean up autoconf for pacakage maintainers

### Fixed
- Save/restore functionality
- Coredump when c)alling a weapon

## [5.4.3] - 2007-07-23
### Added
- Autoconf based build system

### Fixed
- Bugs related wield/wearing items
- Fedora packaging issues

## [5.4.2] - 2006-01-30
### Added
- Keypad/cursor key support

## [5.4.1] - 2006-01-04
### Added
- New save state code
- Use new framework for making code portable

### Fixed
- Includes some fixes brought forward from rogue 3.6 project

