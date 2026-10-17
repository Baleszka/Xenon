#include "ui/gui.hpp"
#include "ui/layout.hpp"

$execute 
{
    gui::setupGui();
};

/*
(SUBJECT TO CHANGE)
TODO:
- Make replay engine ✔
- Make recorder engine ✔
- Save macro to file ✔
    - called .xe ✔
    - save & load ✔
- Frame stepper ✔
- Implement my CBF ✔
- Macro editor ✔
    - Advanced macro editor -> float frames instead of integers ✔
    - Needs my CBF ✔
- Show hitboxes ✔
    - Configure what player hitboxes to show ✔
        - Normal
        - Rotated
        - Circle
        - Inner player
    - Configure colors for hitboxes ✔
    - Hitbox trail ✔
- Speedhack ✔
- Change TPS ✔
- Add calc (short for calculator btw) ✔
    - You type in some shit like:
        First frame: 469.1591
        Last frame: 470.6923
        TPS: 240
    - And outputs some shit like:
        Frames available: 1.1533
        Time available: ~3.883ms
        Frame perfect on: ~156.535hz
- Autoclicker ✔
^^^ ALPHA RELEASE ^^^

- Improve hitbox trail performane
- Automatic frame/cbf window counter
    - Input threshold
        How many frames back or forwards to check
    - Input accuracy
        When found the earliest whole integer frame, how many decimal places to check with
- Lock delta (still dont know what ts does sob)
- Proper labels
    - Overlay info in the level instead of in a separate window inside the bot
    - Change what to overlay in settings
- Advanced Autoclicker
    - Input custom sequence
        Hold: 3 frames
        Release: 5 frames
        Swift: bool
        etc.
    - Presets
- Render
    - FFmpeg
    - Resolution
    - Framerate
    - Custom FFmpeg arguments
    - Render with GPU
        - Have a list of codecs to select from
        - Input custom if preferred codec isn't shown
- Highlight upcoming input in macro editor
- Editor hitboxes
- Macro compression
    - Reverse compatibility
    - Convert macro from bot
^^^ BETA RELEASE ^^^

- Mark inputs with with their frame windows
- Automatic dings while rendering
    - Custom sound effects for different frame windows
- Overlay frame window counter while rendering
- Macro database
    - Upload macro to my servers
    - Download other people's macros
    - Slur in macro name detector
- Custom ImGui themes
    - Save & Load custom ImGui themes
    - Download other people's themes from my servers
    - Slur in theme name detector

^^^ FULL RELEASE ^^^
*/

// NOTE: Some parts were made with the help of silicate and gdh source code
// Please go check them out: https://git.silicate.dev/silicate/silicate https://gitlab.com/tobyadd/GDH