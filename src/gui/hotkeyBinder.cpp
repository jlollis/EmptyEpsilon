#include "engine.h"
#include "hotkeyConfig.h"
#include "hotkeyBinder.h"

GuiHotkeyBinder::GuiHotkeyBinder(GuiContainer* owner, string id, string text)
: GuiTextEntry(owner, id, text), has_focus(false)
{
}

void GuiHotkeyBinder::onFocusGained()
{
    SDL_StartTextInput();
    has_focus = true;
}

void GuiHotkeyBinder::onFocusLost()
{
    SDL_StopTextInput();
    has_focus = false;
}

bool GuiHotkeyBinder::onKey(const SDL_Keysym& key, int unicode)
{
    // If the field has focus and any known key is pressed ...
    if (has_focus && key.scancode != SDL_SCANCODE_UNKNOWN)
    {
        // Don't bind hardcoded "back" keys.
        if (key.scancode == SDL_SCANCODE_ESCAPE
            || key.scancode == SDL_SCANCODE_HOME
            || key.scancode == SDL_SCANCODE_F1)
        {
            text = "";
            return true;
        }

        // Get the key's string name and display it.
        string key_name = hotkeys.getStringForKey(key.scancode);

        if (key_name.length() > 0) {
            text = key_name;
            return true;
        }
    }

    return false;
}
