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

### Building from Source
```bash
# Clone the repository
git clone https://github.com/yourusername/turaco-sdl2.git
cd turaco-sdl2

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
cmake --build . --config Release

# The executable will be in build/Release/turaco.exe
```

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

## Project Structure

```
turaco-sdl2/
├── src/           # Source code
├── include/       # Header files
├── drivers/       # Arcade game drivers
├── SDL2/          # SDL2 library files
├── docs/          # Documentation
└── README.md      # This file
```

## Porting Notes

### Key Changes from Original
- **Graphics System**: Migrated from Allegro to SDL2 for rendering and input
- **File I/O**: Updated file handling to use modern Windows APIs
- **GUI**: Reimplemented GUI components using SDL2
- **Build System**: Added CMake support for easier compilation

### Compatibility
This port maintains compatibility with:
- Original Turaco driver format
- Original ROM file structures
- Original save file formats

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

### Development Guidelines
- Maintain compatibility with original Turaco driver system
- Follow existing code style
- Test with multiple arcade game drivers
- Update documentation for new features

## License

[Include your license here - check original Turaco licensing]

The original Turaco was developed by Scott Lawrence and Ivan Mackintosh. This is an unofficial port with modernized graphics and input handling.

## Credits

### Original Turaco
- **Scott Lawrence** - Primary developer
- **Ivan Mackintosh** - AGE project founder and contributor
- **Chris Moore** - Contributor

### SDL2 Port
- **[Your Name]** - Windows/SDL2 port

## Resources

- [Original Turaco Information](https://www.umlautllama.com/projects/turaco/)
- [SDL2 Documentation](https://wiki.libsdl.org/)
- [MAME Documentation](https://docs.mamedev.org/) - For arcade hardware specifications

## Known Issues

- [List any known issues specific to your port]

## Changelog

### Version 2.0.0 (SDL2 Port)
- Ported from DOS/Allegro to Windows/SDL2
- Modernized graphics rendering
- Updated input handling
- Improved compatibility with Windows 10/11

### Version 1.13 (Original)
- Final DOS release by Scott Lawrence

## Support

For issues with this port, please open an issue on GitHub.
For questions about the original Turaco or arcade game formats, refer to the resources section.

---

**Note**: This software is intended for educational and preservation purposes. Users are responsible for ensuring they have legal rights to modify any ROM files they use with this editor.