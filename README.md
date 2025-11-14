# Turaco Sprite Editor - Windows/SDL2 Port

A modern port of the classic Turaco arcade sprite editor to Windows using SDL2.

## About

Turaco is an arcade graphics editor originally developed for DOS using the Allegro library by Scott Lawrence and Ivan Mackintosh. This editor allows you to load, modify, and save graphics from classic arcade games like Pac-Man, Galaga, Donkey Kong, Commando, and many others.

With Turaco, you can:
- Edit sprites from classic arcade ROM files
- Modify game graphics and give characters custom appearances
- Export modified graphics back to ROM files for use in emulators or real arcade hardware
- Work with authentic arcade game color palettes and sprite formats

This port brings Turaco to modern Windows systems using SDL2, eliminating the need for DOSBox and DOS Protected Mode Interface while maintaining compatibility with the original's functionality.

## Original Turaco

The original Turaco was part of the AGE (Arcade Graphics/Game Editor) project, which was spearheaded by Ivan Mackintosh with contributions from Scott Lawrence and Chris Moore. The DOS version (v1.13) required DOSBox to run on modern systems and utilized game-specific drivers to understand different arcade hardware formats.

## Features

- **Cross-platform graphics library**: Replaced Allegro with SDL2 for better modern OS compatibility
- **Native Windows support**: Runs directly on Windows without emulation
- **Arcade ROM support**: Compatible with original Turaco driver system for various arcade games
- **Sprite editing**: Full palette of editing tools for pixel-perfect arcade graphics
- **Color palette management**: Work with authentic arcade color palettes
- **Multi-page sprite support**: Browse and edit multiple sprite pages
- **Import/Export**: Load from and save to original ROM file formats

## Requirements

### Runtime Requirements
- Windows 7 or later
- SDL2 runtime library (included)

### Build Requirements
- C/C++ compiler (Visual Studio, MinGW, or GCC)
- SDL2 development libraries
- CMake 3.10 or later (optional, for build system)

## Installation

### Binary Release
1. Download the latest release from the [Releases](../../releases) page
2. Extract the archive to your desired location
3. Run `turaco.exe`

## Usage

### Basic Workflow
1. Launch Turaco
2. Select **File → Change Game** to choose an arcade game driver
3. Place the game's ROM files in the appropriate location (as specified by the driver)
4. Browse sprites using the page navigation (>> and <<)
5. Select a sprite and click **Edit** to modify it
6. Use the pixel editing tools to make your changes
7. Click **Back** to save changes to the sprite
8. Select **File → Save Graphics** to write changes back to the ROM files

### Supported Games
The driver system supports numerous classic arcade games including:
- Pac-Man
- Donkey Kong
- Galaga
- Commando
- Many others (see DRIVERS folder for complete list)

### ROM File Locations
Place ROM files in the same directory as the Turaco executable, or in a `roms/` subdirectory. The specific files required depend on the game driver selected.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## Credits

The original Turaco was developed by Scott Lawrence and Ivan Mackintosh. This is an unofficial port with modernized graphics and input handling.
### Original Turaco
- **Scott Lawrence** - Primary developer
- **Ivan Mackintosh** - AGE project founder and contributor
- **Chris Moore** - Contributor

## Resources

- [Original Turaco Information](https://www.umlautllama.com/projects/turaco/)

**Note**: This software is intended for educational and preservation purposes. Users are responsible for ensuring they have legal rights to modify any ROM files they use with this editor.