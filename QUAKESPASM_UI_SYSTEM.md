# QuakeSpasm UI System Reference

## Drawing Primitives (`Quake/gl_draw.c`, `Quake/draw.h`)

| Function | Purpose |
|---|---|
| `Draw_Character(int x, int y, int num)` | Single 8x8 character |
| `Draw_String(int x, int y, const char *str)` | Text string (8px per char) |
| `Draw_Fill(int x, int y, int w, int h, int c, float alpha)` | Filled rectangle (palette color index + alpha) |
| `Draw_FadeScreen(void)` | Full-screen dark overlay (50% black) |
| `Draw_ConsoleBackground(void)` | Console backdrop |
| `Draw_MakePic(const char *name, int w, int h, byte *data)` | Create texture from raw pixel data in memory |
| `Draw_PicFromWad(const char *name)` | Load from gfx.wad |
| `Draw_CachePic(const char *path)` | Load from .lmp file (e.g. "gfx/conback.lmp") |
| `Draw_Pic(int x, int y, qpic_t *pic)` | Draw loaded image |
| `Draw_TransPicTranslate(int x, int y, qpic_t *pic, int top, int bottom)` | Draw with color translation |
| `Draw_TileClear(int x, int y, int w, int h)` | Tiled background fill |

Use `va()` for formatted strings: `Draw_String(x, y, va("Score: %d", score));`

## Sbar (HUD) Drawing Functions (`Quake/sbar.c`)

| Function | Purpose |
|---|---|
| `Sbar_DrawPic(int x, int y, qpic_t *pic)` | Draw image (auto +24 Y offset) |
| `Sbar_DrawPicAlpha(int x, int y, qpic_t *pic, float alpha)` | With transparency |
| `Sbar_DrawCharacter(int x, int y, int num)` | Single character |
| `Sbar_DrawString(int x, int y, const char *str)` | Text string |
| `Sbar_DrawNum(int x, int y, int num, int digits, int color)` | Large number graphics |
| `Sbar_Changed(void)` | Force HUD redraw next frame |

Note: All Sbar coordinates have an automatic +24 pixel Y offset.

## Menu Drawing Functions (`Quake/menu.c`)

| Function | Purpose |
|---|---|
| `M_Print(int cx, int cy, const char *str)` | Highlighted/brown text (char+128) |
| `M_PrintWhite(int cx, int cy, const char *str)` | Normal white text |
| `M_DrawCharacter(int cx, int line, int num)` | Single character |
| `M_DrawPic(int x, int y, qpic_t *pic)` | Draw menu graphic |
| `M_DrawTransPic(int x, int y, qpic_t *pic)` | Transparent pic |
| `M_DrawTextBox(int x, int y, int width, int lines)` | Ornate text box |
| `M_DrawCheckbox(int x, int y, int on)` | Toggle ("on"/"off") |
| `M_DrawSlider(int x, int y, float range)` | Slider control (0.0-1.0) |

## Canvas System (`Quake/gl_screen.c`)

Set coordinate space with `GL_SetCanvas(canvastype)` before drawing.

| Canvas | Coordinates | Position | Typical Use |
|---|---|---|---|
| `CANVAS_DEFAULT` | glwidth x glheight | Full framebuffer | Raw pixel coords |
| `CANVAS_CONSOLE` | vid.conwidth x vid.conheight | Top-left | Console text, notify |
| `CANVAS_MENU` | 320x200, centered | Center of screen | Menus, centerprint |
| `CANVAS_SBAR` | 320x48 | Bottom of screen | Status bar / HUD |
| `CANVAS_CROSSHAIR` | Origin at center (0,0) | Center | Crosshair |
| `CANVAS_TOPRIGHT` | 320x200 | Top-right corner | FPS, clock, devstats |
| `CANVAS_BOTTOMLEFT` | 320x200 | Bottom-left corner | Custom overlays |
| `CANVAS_BOTTOMRIGHT` | 320x200 | Bottom-right corner | Loading disc |

### Scaling CVars
- `scr_menuscale` - Menu scaling
- `scr_sbarscale` - Status bar scaling
- `scr_sbaralpha` - Status bar transparency (default 0.75)
- `scr_conscale` - Console scaling
- `scr_crosshairscale` - Crosshair scaling

## Screen Render Order (`SCR_UpdateScreen()` in `Quake/gl_screen.c`)

1. 3D World (`V_RenderView`)
2. Switch to 2D (`GL_Set2D`)
3. Tile clear (borders)
4. Dialog / Loading / Intermission overlays
5. Crosshair (`SCR_DrawCrosshair`)
6. Net/Turtle icons (`SCR_DrawNet`, `SCR_DrawTurtle`)
7. Pause icon (`SCR_DrawPause`)
8. Centerprint (`SCR_CheckDrawCenterString`)
9. **HUD / Status Bar (`Sbar_Draw`)** - extend here for custom HUD
10. Dev stats (`SCR_DrawDevStats`)
11. FPS counter (`SCR_DrawFPS`)
12. Clock (`SCR_DrawClock`)
13. Console (`SCR_DrawConsole`)
14. **Menu (`M_Draw`)** - extend here for custom menus
15. Gamma correction
16. Present (`GL_EndRendering`)

## Menu System Architecture (`Quake/menu.c`, `Quake/menu.h`)

State machine using `m_state_e` enum:
```c
enum m_state_e {
    m_none, m_main, m_singleplayer, m_load, m_save,
    m_multiplayer, m_setup, m_net, m_options, m_video,
    m_keys, m_help, m_quit, m_lanconfig, m_gameoptions,
    m_search, m_slist
};
```

### Adding a New Menu Page

1. Add enum value to `m_state_e` in `Quake/menu.h`
2. Implement three functions:
   - `M_Menu_YourMenu_f(void)` — entry/setup
   - `M_YourMenu_Draw(void)` — rendering
   - `M_YourMenu_Key(int key)` — input handling
3. Add cases to `M_Draw()` and `M_Keydown()` switch statements in `Quake/menu.c`
4. Optionally register console command: `Cmd_AddCommand("menu_yourmenu", M_Menu_YourMenu_f);`

## Adding Custom UI Without Asset Files

### Option 1: Extend HUD in `Sbar_Draw()` (Quake/sbar.c)
```c
GL_SetCanvas(CANVAS_TOPRIGHT);
Draw_Fill(10, 10, 150, 30, 0, 0.7f);
Draw_String(15, 15, va("Monsters: %d", cl.stats[STAT_MONSTERS]));
```

### Option 2: Add to screen pipeline in `SCR_UpdateScreen()` (Quake/gl_screen.c)
Insert drawing calls between existing steps.

### Option 3: New menu page in `menu.c`
Use `M_Print()`, `M_DrawSlider()`, `M_DrawCheckbox()`, `M_DrawTextBox()` — all text-based, no assets required.

### Option 4: Create textures programmatically
Use `Draw_MakePic()` to create textures from raw pixel data in memory.

## Character Set
- 8x8 pixel characters from "conchars" texture (128x128 atlas, 16x16 grid)
- Chars 0-127: Normal ASCII
- Chars 128-255: Highlighted/colored text (menus use char+128 for brown text)

## Color System
- 8-bit palette indices (0-255)
- `d_8to24table` converts palette index to RGBA
- Common colors: 0=black, 79=brown/gold, 251-254=various bright colors
