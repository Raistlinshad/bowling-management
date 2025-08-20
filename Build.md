# Building the Bowling Management System

## Prerequisites

### Required Software
- **Qt 6.2 or later** (with Widgets, Network, and SQL modules)
- **C++17 compatible compiler**
- **CMake 3.16+** or **qmake** (comes with Qt)

### Qt Installation
Download Qt from: https://www.qt.io/download

Make sure to install these components:
- Qt 6.x (latest version)
- Qt Creator IDE (recommended)
- CMake support
- Debugging tools

## Building with Qt Creator (Recommended)

1. **Open Qt Creator**
2. **Open Project**: File → Open File or Project
3. **Select**: Choose either `CMakeLists.txt` or `BowlingManagement.pro`
4. **Configure**: Select your Qt kit and build configuration
5. **Build**: Press Ctrl+B or click the Build button
6. **Run**: Press Ctrl+R or click the Run button

## Building with Command Line

### Option 1: Using CMake

```bash
# Create build directory
mkdir build
cd build

# Configure the project
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt6

# Build the project
cmake --build .

# Run the application
./BowlingManagement  # Linux/macOS
# or
BowlingManagement.exe  # Windows
```

### Option 2: Using qmake

```bash
# Generate Makefile
qmake BowlingManagement.pro

# Build the project
make  # Linux/macOS
# or
nmake  # Windows with MSVC
# or
mingw32-make  # Windows with MinGW

# Run the application
./BowlingManagement  # Linux/macOS
# or
BowlingManagement.exe  # Windows
```

## Project Structure

```
BowlingManagement/
├── main.cpp                    # Application entry point
├── MainWindow.h/.cpp          # Main application window
├── Actions.h/.cpp             # Action handlers
├── DatabaseManager.h/.cpp     # Database operations
├── EventBus.h/.cpp            # Event system
├── LaneServer.h/.cpp          # Lane communication
├── LaneWidget.h/.cpp          # Lane display widgets
├── QuickStartDialog.h/.cpp    # Quick start menu
├── QuickGameDialog.h/.cpp     # Quick game setup
├── BowlerInfoDialog.h/.cpp    # Bowler management
├── TeamManagementDialog.h/.cpp # Team management
├── LeagueManagementDialog.h/.cpp # League management
├── CMakeLists.txt             # CMake build file
├── BowlingManagement.pro      # qmake build file
└── BUILD.md                   # This file
```

## Troubleshooting

### Common Issues

#### "Qt not found"
- Make sure Qt is properly installed
- Set CMAKE_PREFIX_PATH to your Qt installation directory
- Example: `-DCMAKE_PREFIX_PATH=C:\Qt\6.5.0\msvc2019_64`

#### "MOC errors"
- Clean and rebuild the project
- Make sure all Qt header files have proper include guards
- Verify Q_OBJECT macro is present in class declarations

#### "SQLite driver not available"
- Qt should include SQLite by default
- If issues persist, install Qt with SQL modules explicitly

#### "Cannot find qmake"
- Add Qt's bin directory to your PATH
- Example: `C:\Qt\6.5.0\msvc2019_64\bin`

### Platform-Specific Notes

#### Windows
- Use Visual Studio 2019/2022 or MinGW
- Make sure to use the correct Qt kit for your compiler

#### macOS
- Xcode command line tools required: `xcode-select --install`
- Qt should be installed via official installer or Homebrew

#### Linux
- Install Qt via package manager or official installer
- Ubuntu/Debian: `sudo apt install qt6-base-dev qt6-tools-dev`
- Fedora: `sudo dnf install qt6-qtbase-devel qt6-qttools-devel`

## Running the Application

Once built successfully:

1. **First Run**: The application will create a SQLite database automatically
2. **Database Location**: 
   - Windows: `%APPDATA%/BowlingManagement/bowling.db`
   - macOS: `~/Library/Application Support/BowlingManagement/bowling.db`
   - Linux: `~/.local/share/BowlingManagement/bowling.db`
3. **Add Data**: Start by adding bowlers through the "Bowler Info" menu
4. **Create Teams**: Use the "Teams" menu to create and manage teams
5. **Setup Leagues**: Use the "Leagues" menu for league management

## Development

### Adding New Features
1. Create new dialog header/source files
2. Add to CMakeLists.txt or .pro file
3. Include in Actions.h/.cpp for menu integration
4. Update DatabaseManager if new data structures needed

### Database Changes
- Modify DatabaseManager::createTables() for schema changes
- Add new data structures to DatabaseManager.h
- Implement CRUD operations for new entities

### Testing
- Build and test on your target platform
- Test database operations with sample data
- Verify UI responsiveness and error handling