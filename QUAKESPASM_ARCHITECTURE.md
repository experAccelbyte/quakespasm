# QuakeSpasm Architecture Guide

This document provides a comprehensive overview of the QuakeSpasm codebase for developers integrating new features or SDKs.

## Project Overview

QuakeSpasm is a modern, cross-platform Quake engine source port based on FitzQuake. It's written in C and uses SDL for cross-platform graphics/input/audio. Current version is 0.97.0.

---

## Source Code Structure

All engine source is in the `Quake/` directory, organized by subsystem:

```
Quake/
├── Core Engine
│   ├── host.c          # Main loop, initialization, frame timing
│   ├── common.c        # Utilities, memory, file system, argument parsing
│   ├── zone.c          # Custom memory allocator (zone/hunk system)
│   ├── cmd.c           # Command system
│   ├── cvar.c          # Console variable system
│   └── quakedef.h      # Primary header with version and limits
│
├── Client (cl_*.c)
│   ├── cl_main.c       # Client main loop, connection handling
│   ├── cl_input.c      # Input processing and command building
│   ├── cl_parse.c      # Server message parsing
│   ├── cl_demo.c       # Demo recording/playback
│   ├── console.c       # In-game console
│   ├── keys.c          # Keyboard input handling
│   └── menu.c          # Menu system (main menu, options, etc.)
│
├── Server (sv_*.c)
│   ├── sv_main.c       # Server loop, client management
│   ├── sv_phys.c       # Physics simulation
│   ├── sv_move.c       # Entity movement
│   └── sv_user.c       # User command processing
│
├── QuakeC VM (pr_*.c)
│   ├── pr_exec.c       # Bytecode VM execution
│   ├── pr_edict.c      # Entity (edict) management
│   └── pr_cmds.c       # Built-in QuakeC functions
│
├── Rendering (gl_*.c, r_*.c)
│   ├── gl_rmain.c      # Main rendering loop
│   ├── gl_model.c      # BSP/MDL/SPR model loading (~3400 lines)
│   ├── gl_texmgr.c     # Texture management
│   ├── gl_vidsdl.c     # SDL video initialization
│   ├── gl_draw.c       # 2D drawing (HUD, console)
│   ├── gl_screen.c     # Screen rendering, calls M_Draw()
│   ├── r_world.c       # World/brush rendering
│   └── r_alias.c       # Alias model (MDL) rendering
│
├── Sound (snd_*.c)
│   ├── snd_dma.c       # DMA sound mixing core
│   ├── snd_mix.c       # Sample mixing
│   ├── snd_sdl.c       # SDL audio driver
│   ├── bgmusic.c       # Background music playback
│   └── snd_codec.c     # Pluggable codec system
│
├── Networking (net_*.c)
│   ├── net_main.c      # Network abstraction layer
│   ├── net_dgrm.c      # Datagram protocol (reliable over UDP)
│   └── net_udp.c       # UDP socket implementation (Unix)
│   └── net_wins.c      # WinSock implementation (Windows)
│
└── Platform Layer
    ├── main_sdl.c      # Entry point
    ├── in_sdl.c        # Input handling
    ├── sys_sdl_unix.c  # Unix-specific system code
    └── sys_sdl_win.c   # Windows-specific system code
```

---

## Main Loop and Initialization Flow

### Startup Sequence

```
main() [main_sdl.c]
  └── Host_Init() [host.c:780-890]
        ├── Memory_Init()           # Initialize zone/hunk memory
        ├── Cbuf_Init()             # Command buffer
        ├── Cmd_Init()              # Command system
        ├── Cvar_Init()             # Console variables
        ├── COM_Init()              # Common utilities, file system
        ├── Key_Init()              # Keyboard bindings
        ├── Con_Init()              # Console
        ├── PR_Init()               # QuakeC VM
        ├── SV_Init()               # Server
        ├── CL_Init()               # Client (if not dedicated)
        ├── M_Init()                # Menu system  <-- Good place for SDK init
        ├── VID_Init()              # Video/OpenGL
        ├── S_Init()                # Sound
        └── ... other subsystems
```

### Main Frame Loop

```
Host_Frame() [host.c:720-775]
  └── _Host_Frame()
        ├── Sys_SendKeyEvents()     # Process input
        ├── Host_GetConsoleCommands()
        ├── Cbuf_Execute()          # Execute buffered commands
        ├── CL_SendMove()           # Send client input to server
        ├── SV_Frame()              # Server frame (if hosting)
        ├── CL_Frame()              # Client frame
        ├── SCR_UpdateScreen()      # Render frame
        │     └── M_Draw()          # Draw menu if active
        └── host_framecount++       <-- Good place for per-frame updates
```

---

## Menu System Architecture

### Key Files
- `Quake/menu.c` - All menu logic (~2700 lines)
- `Quake/menu.h` - Menu state enums and declarations

### Menu States

```c
// menu.h
enum m_state_e {
    m_none,           // No menu active
    m_main,           // Main menu
    m_singleplayer,
    m_load, m_save,
    m_multiplayer,
    m_setup, m_net, m_lanconfig, m_gameoptions, m_search, m_slist,
    m_options, m_keys, m_video,
    m_help,
    m_quit
};

// Global state
extern enum m_state_e m_state;      // Current menu
extern keydest_t key_dest;          // key_game, key_console, key_menu
```

### Menu Entry Point

```c
// menu.c:244-255
void M_Menu_Main_f (void)
{
    if (key_dest != key_menu)
    {
        m_save_demonum = cls.demonum;
        cls.demonum = -1;
    }
    IN_Deactivate(modestate == MS_WINDOWED);
    key_dest = key_menu;
    m_state = m_main;
    m_entersound = true;
    // <-- SDK login trigger can go here
}
```

### Menu Drawing Pipeline

```
SCR_UpdateScreen() [gl_screen.c]
  └── M_Draw() [menu.c:2569-2674]
        ├── if (m_state == m_none) return;
        └── switch (m_state)
              case m_main: M_Main_Draw(); break;
              case m_options: M_Options_Draw(); break;
              ...
```

### Menu Input Handling

```
Key_Event() [keys.c]
  └── M_Keydown(key) [menu.c:2677-2748]
        └── switch (m_state)
              case m_main: M_Main_Key(key); break;
              ...
```

---

## Console Variable System (CVars)

### Declaring CVars

```c
// In a .c file
cvar_t my_cvar = {"my_cvar", "default_value", CVAR_ARCHIVE};

// Flags:
// CVAR_ARCHIVE  - Save to config.cfg
// CVAR_NOTIFY   - Notify players when changed
// CVAR_SERVERINFO - Send to server info
```

### Registering CVars

```c
// In an Init function
Cvar_RegisterVariable(&my_cvar);
```

### Accessing CVars

```c
// Get value
float val = my_cvar.value;
const char* str = my_cvar.string;

// Set value
Cvar_Set("my_cvar", "new_value");
Cvar_SetValue("my_cvar", 42.0f);
```

---

## Command System

### Registering Commands

```c
// In an Init function
Cmd_AddCommand("mycommand", MyCommand_f);

// Command handler
void MyCommand_f(void) {
    int argc = Cmd_Argc();
    const char* arg1 = Cmd_Argv(1);
    Con_Printf("Command executed with %d args\n", argc);
}
```

---

## C/C++ Interop Pattern

QuakeSpasm uses the standard `extern "C"` pattern for C/C++ interoperability:

```c
// In a .h file
#ifdef __cplusplus
extern "C" {
#endif

void MyCFunction(void);
int AnotherFunction(const char* str);

#ifdef __cplusplus
}
#endif
```

Example from `zone.h`:
```c
#ifdef __cplusplus
extern "C" {
#endif
void Z_Free (void *ptr);
void *Z_Malloc (int size);
void *Z_Realloc (void *ptr, int size);
char *Z_Strdup (const char *s);
#ifdef __cplusplus
}
#endif
```

---

## Build System (CMake)

### Project Configuration

```cmake
# CMakeLists.txt line 21
project(QuakeSpasm VERSION 0.97.0 LANGUAGES C)

# To add C++ support:
project(QuakeSpasm VERSION 0.97.0 LANGUAGES C CXX)
```

### Adding Source Files

```cmake
# Common sources list (line 79-171)
set(COMMON_SOURCES
    ${QUAKE_DIR}/host.c
    ${QUAKE_DIR}/menu.c
    ...
)

# Add new sources
list(APPEND COMMON_SOURCES ${QUAKE_DIR}/MyNewFile.cpp)
```

### Conditional Compilation

```cmake
# Define preprocessor macro
target_compile_definitions(quakespasm PRIVATE MY_FEATURE)

# Conditional block
if(USE_MY_FEATURE)
    list(APPEND COMMON_SOURCES ${QUAKE_DIR}/my_feature.cpp)
    target_compile_definitions(quakespasm PRIVATE USE_MY_FEATURE)
endif()
```

### AccelByte SDK Integration (Already Configured)

```cmake
# CMakeLists.txt lines 296-306
if(USE_ACCELBYTE_GAMESDK)
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/thirdparties/accelbyte-gamesdk")
    find_package(accelbyte-gamesdk REQUIRED)
    target_link_libraries(quakespasm PRIVATE
        accelbyte-gamesdk::user
        accelbyte-gamesdk::lobby
        accelbyte-gamesdk::settings
    )
    target_compile_definitions(quakespasm PRIVATE USE_ACCELBYTE_GAMESDK)
endif()
```

---

## Key Constants and Limits

Defined in `quakedef.h`:

```c
#define MAX_EDICTS      32000   // Entity limit
#define MAX_MODELS      2048
#define MAX_SOUNDS      2048
#define MAX_MSGLEN      64000   // Network message size
#define QUAKESPASM_VERSION  0.97.0
```

---

## Extension Points for SDK Integration

### 1. Initialization (`host.c`)

Best place: After `M_Init()` in `Host_Init()` (line 864)
```c
M_Init();
// Add SDK initialization here
#ifdef USE_MY_SDK
    MySDK_Init();
#endif
```

### 2. Per-Frame Update (`host.c`)

Best place: End of `_Host_Frame()` before `host_framecount++`
```c
// Add per-frame SDK update
#ifdef USE_MY_SDK
    MySDK_Update();
#endif
host_framecount++;
```

### 3. Shutdown (`host.c`)

Best place: `Host_Shutdown()`
```c
void Host_Shutdown(void) {
    #ifdef USE_MY_SDK
        MySDK_Shutdown();
    #endif
    // ... existing cleanup
}
```

### 4. Menu Entry (`menu.c`)

Best place: `M_Menu_Main_f()` for triggering on menu open
```c
void M_Menu_Main_f(void) {
    #ifdef USE_MY_SDK
    static qboolean sdk_triggered = false;
    if (!sdk_triggered) {
        sdk_triggered = true;
        MySDK_OnMenuOpen();
    }
    #endif
    // ... existing code
}
```

---

## Console Output

```c
// Print to console
Con_Printf("Message: %s\n", str);

// Print warning
Con_Warning("Warning: %s\n", str);

// Print debug (only in debug builds)
Con_DPrintf("Debug: %s\n", str);

// Safe printf (handles null pointers)
Con_SafePrintf("Safe: %s\n", str);
```

---

## Platform-Specific Code

### Windows vs Unix

```c
#ifdef _WIN32
    // Windows-specific code
    #include <windows.h>
#else
    // Unix/Linux/macOS code
    #include <unistd.h>
#endif
```

### Platform Files

- `sys_sdl_win.c` - Windows system functions
- `sys_sdl_unix.c` - Unix system functions
- `net_wins.c` / `net_wipx.c` - Windows networking
- `net_udp.c` / `net_bsd.c` - Unix networking

---

## Time Functions

```c
// Get current time in seconds (double precision)
double Sys_DoubleTime(void);

// Game time (affected by slowmo, pause)
extern double realtime;

// Host time tracking
extern double host_frametime;
```

---

## Memory Management

```c
// Zone memory (general purpose, can be freed)
void* ptr = Z_Malloc(size);
Z_Free(ptr);

// Hunk memory (level data, freed on map change)
void* ptr = Hunk_Alloc(size);

// Temp memory (per-frame, auto-freed)
void* ptr = Hunk_TempAlloc(size);
```

---

## File System

```c
// Load file into memory
byte* data = COM_LoadFile(filename, NULL);
// ... use data
// Memory is from Hunk, freed on level change

// Check if file exists
if (COM_FileExists(filename)) { ... }

// Get full path
char path[MAX_OSPATH];
COM_FOpenFile(filename, &file);
```

---

## Quick Reference: Adding a New Feature

1. **Create header file** with `extern "C"` guards
2. **Create implementation file** (`.c` or `.cpp`)
3. **Add to CMakeLists.txt** source list
4. **Register cvars** in an Init function
5. **Register commands** in an Init function
6. **Hook into lifecycle**:
   - Init in `Host_Init()`
   - Update in `_Host_Frame()`
   - Shutdown in `Host_Shutdown()`
7. **Use `#ifdef` guards** for optional features
