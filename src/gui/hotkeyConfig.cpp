#include <i18n.h>
#include "hotkeyConfig.h"
#include "preferenceManager.h"
#include "shipTemplate.h"

HotkeyConfig hotkeys;

HotkeyConfig::HotkeyConfig()
{  // this list includes all Hotkeys and their standard configuration
    newCategory("BASIC", "Basic"); // these Items should all have predefined values
    newKey("PAUSE", std::make_tuple("Pause game", "P"));
    newKey("HELP", std::make_tuple("Show in-game help", "F1"));
    newKey("ESCAPE", std::make_tuple("Return to ship options menu", "Escape"));
    newKey("HOME", std::make_tuple("Return to ship options menu", "Home"));  // Remove this item as it does the same as Escape?
    newKey("VOICE_CHAT_ALL", std::make_tuple("Broadcast voice chat to server", "Backspace"));
    newKey("VOICE_CHAT_SHIP", std::make_tuple("Broadcast voice chat to ship", "Tilde"));

    newCategory("GENERAL", "General");
    newKey("NEXT_STATION", std::make_tuple("Switch to next crew station", "Tab"));
    newKey("PREV_STATION", std::make_tuple("Switch to previous crew station", ""));
    newKey("STATION_HELMS", std::make_tuple("Switch to helms station", "F2"));
    newKey("STATION_WEAPONS", std::make_tuple("Switch to weapons station", "F3"));
    newKey("STATION_ENGINEERING", std::make_tuple("Switch to engineering station", "F4"));
    newKey("STATION_SCIENCE", std::make_tuple("Switch to science station", "F5"));
    newKey("STATION_RELAY", std::make_tuple("Switch to relay station", "F6"));

    newCategory("MAIN_SCREEN", "Main Screen");
    newKey("VIEW_FORWARD", std::make_tuple("View forward", "Up"));
    newKey("VIEW_LEFT", std::make_tuple("View left", "Left"));
    newKey("VIEW_RIGHT", std::make_tuple("View right", "Right"));
    newKey("VIEW_BACK", std::make_tuple("View backward", "Down"));
    newKey("VIEW_TARGET", std::make_tuple("Lock view on weapons target", "T"));
    newKey("TACTICAL_RADAR", std::make_tuple("View tactical radar", "Tab"));
    newKey("LONG_RANGE_RADAR", std::make_tuple("View long-range radar", "Q"));
    newKey("FIRST_PERSON", std::make_tuple("Toggle first-person view", "F"));

    // - Single Pilot and Tactical use:
    //   - Helms TURN_LEFT and _RIGHT, DOCK_* and UNDOCK, *_IMPULSE, *_JUMP,
    //     and WARP_*.
    //   - Weapons NEXT_ENEMY_TARGET, NEXT_TARGET, AIM_MISSILE_LEFT and _RIGHT,
    //     *_AIM_LOCK, COMBAT_*, SELECT_MISSILE_*, *_TUBE_*, SHIELD_CAL_*,
    //     and *_SHIELDS.
    // - Tactical also uses:
    //   - Weapons BEAM_FREQUENCY_*, BEAM_SUBSYSTEM_TARGET_*
    // - Operations uses Science hotkeys.

    newCategory("HELMS", "Helms");
    newKey("INC_IMPULSE", std::make_tuple("Increase impulse", "Up"));
    newKey("DEC_IMPULSE", std::make_tuple("Decrease impulse", "Down"));
    newKey("ZERO_IMPULSE", std::make_tuple("Zero impulse", "Space"));
    newKey("MAX_IMPULSE", std::make_tuple("Max impulse", ""));
    newKey("MIN_IMPULSE", std::make_tuple("Max reverse impulse", ""));
    newKey("TURN_LEFT", std::make_tuple("Turn left", "Left"));
    newKey("TURN_RIGHT", std::make_tuple("Turn right", "Right"));
    newKey("WARP_0", std::make_tuple("Warp off", "Num6"));
    newKey("WARP_1", std::make_tuple("Warp 1", "Num7"));
    newKey("WARP_2", std::make_tuple("Warp 2", "Num8"));
    newKey("WARP_3", std::make_tuple("Warp 3", "Num9"));
    newKey("WARP_4", std::make_tuple("Warp 4", "Num0"));
    newKey("DOCK_ACTION", std::make_tuple("Dock request/abort/undock", "D"));
    newKey("DOCK_REQUEST", std::make_tuple("Initiate docking", ""));
    newKey("DOCK_ABORT", std::make_tuple("Abort docking", ""));
    newKey("UNDOCK", std::make_tuple("Undock", "D"));
    newKey("INC_JUMP", std::make_tuple("Increase jump distance", "RBracket"));
    newKey("DEC_JUMP", std::make_tuple("Decrease jump distance", "LBracket"));
    newKey("JUMP", std::make_tuple("Initiate jump", "BackSlash"));
    //newKey("COMBAT_LEFT", "Combat maneuver left");
    //newKey("COMBAT_RIGHT", "Combat maneuver right");
    //newKey("COMBAT_BOOST", "Combat maneuver boost");

    newCategory("WEAPONS", "Weapons");
    newKey("SELECT_MISSILE_TYPE_HOMING", std::make_tuple("Select homing", "Num1"));
    newKey("SELECT_MISSILE_TYPE_NUKE", std::make_tuple("Select nuke", "Num2"));
    newKey("SELECT_MISSILE_TYPE_MINE", std::make_tuple("Select mine", "Num3"));
    newKey("SELECT_MISSILE_TYPE_EMP", std::make_tuple("Select EMP", "Num4"));
    newKey("SELECT_MISSILE_TYPE_HVLI", std::make_tuple("Select HVLI", "Num5"));
    for(int n = 0; n < max_weapon_tubes; n++)
    {
        newKey(std::string("LOAD_TUBE_") + string(n+1), std::make_tuple(std::string("Load tube ") + string(n+1), ""));
    }
    for(int n = 0; n < max_weapon_tubes; n++)
    {
        newKey(std::string("UNLOAD_TUBE_") + string(n+1), std::make_tuple(std::string("Unload tube ") + string(n+1), ""));
    }
    for(int n = 0; n < max_weapon_tubes; n++)
    {
        newKey(std::string("FIRE_TUBE_") + string(n+1), std::make_tuple(std::string("Fire tube ") + string(n+1), ""));
    }
    newKey("NEXT_ENEMY_TARGET", std::make_tuple("Select next hostile target", "C"));
    newKey("NEXT_TARGET", std::make_tuple("Select next target (any)", "Z"));
    newKey("TOGGLE_SHIELDS", std::make_tuple("Toggle shields", "S"));
    newKey("ENABLE_SHIELDS", std::make_tuple("Enable shields", ""));
    newKey("DISABLE_SHIELDS", std::make_tuple("Disable shields", ""));
    newKey("SHIELD_CAL_INC", std::make_tuple("Increase shield frequency target", "Period"));
    newKey("SHIELD_CAL_DEC", std::make_tuple("Decrease shield frequency target", "Comma"));
    newKey("SHIELD_CAL_START", std::make_tuple("Start shield calibration", "Slash"));
    newKey("BEAM_SUBSYSTEM_TARGET_NEXT", std::make_tuple("Next beam subsystem target type", "Quote"));
    newKey("BEAM_SUBSYSTEM_TARGET_PREV", std::make_tuple("Previous beam subsystem target type", "SemiColon"));
    newKey("BEAM_FREQUENCY_INCREASE", std::make_tuple("Increase beam frequency", "M"));
    newKey("BEAM_FREQUENCY_DECREASE", std::make_tuple("Decrease beam frequency", "N"));
    newKey("TOGGLE_AIM_LOCK", std::make_tuple("Toggle missile aim lock", "B"));
    newKey("ENABLE_AIM_LOCK", std::make_tuple("Enable missile aim lock", ""));
    newKey("DISABLE_AIM_LOCK", std::make_tuple("Disable missile aim lock", ""));
    newKey("AIM_MISSILE_LEFT", std::make_tuple("Turn missile aim to the left", "G"));
    newKey("AIM_MISSILE_RIGHT", std::make_tuple("Turn missile aim to the right", "H"));

    newCategory("SCIENCE", "Science");
    newKey("SCAN_OBJECT", std::make_tuple("Scan object", "S"));
    newKey("NEXT_SCANNABLE_OBJECT", std::make_tuple("Select next scannable object", "C"));

    // Engineering functions should not overlap with other stations'.
    newCategory("ENGINEERING", "Engineering");
    newKey("SELECT_REACTOR", std::make_tuple("Select reactor system", "Num1"));
    newKey("SELECT_BEAM_WEAPONS", std::make_tuple("Select beam weapon system", "Num2"));
    newKey("SELECT_MISSILE_SYSTEM", std::make_tuple("Select missile weapon system", "Num3"));
    newKey("SELECT_MANEUVER", std::make_tuple("Select maneuvering system", "Num4"));
    newKey("SELECT_IMPULSE", std::make_tuple("Select impulse system", "Num5"));
    newKey("SELECT_WARP", std::make_tuple("Select warp system", "Num6"));
    newKey("SELECT_JUMP_DRIVE", std::make_tuple("Select jump drive system", "Num7"));
    newKey("SELECT_FRONT_SHIELDS", std::make_tuple("Select front shields system", "Num8"));
    newKey("SELECT_REAR_SHIELDS", std::make_tuple("Select rear shields system", "Num9"));
    newKey("SET_POWER_000", std::make_tuple("Set system power to 0%", ""));
    newKey("SET_POWER_030", std::make_tuple("Set system power to 30%", ""));
    newKey("SET_POWER_050", std::make_tuple("Set system power to 50%", ""));
    newKey("SET_POWER_100", std::make_tuple("Set system power to 100%", "Space"));
    newKey("SET_POWER_150", std::make_tuple("Set system power to 150%", ""));
    newKey("SET_POWER_200", std::make_tuple("Set system power to 200%", ""));
    newKey("SET_POWER_250", std::make_tuple("Set system power to 250%", ""));
    newKey("SET_POWER_300", std::make_tuple("Set system power to 300%", ""));
    newKey("INCREASE_POWER", std::make_tuple("Increase system power", "Up"));
    newKey("DECREASE_POWER", std::make_tuple("Decrease system power", "Down"));
    newKey("INCREASE_COOLANT", std::make_tuple("Increase system coolant", "Right"));
    newKey("DECREASE_COOLANT", std::make_tuple("Decrease system coolant", "Left"));
    newKey("NEXT_REPAIR_CREW", std::make_tuple("Next repair crew", "Q"));
    newKey("REPAIR_CREW_MOVE_UP", std::make_tuple("Crew move up", "W"));
    newKey("REPAIR_CREW_MOVE_DOWN", std::make_tuple("Crew move down", "S"));
    newKey("REPAIR_CREW_MOVE_LEFT", std::make_tuple("Crew move left", "A"));
    newKey("REPAIR_CREW_MOVE_RIGHT", std::make_tuple("Crew move right", "D"));
    newKey("SELF_DESTRUCT_START", std::make_tuple("Start self-destruct", ""));
    newKey("SELF_DESTRUCT_CONFIRM", std::make_tuple("Confirm self-destruct", ""));
    newKey("SELF_DESTRUCT_CANCEL", std::make_tuple("Cancel self-destruct", ""));
}

static std::vector<std::pair<string, SDL_Scancode> > sdl_key_names = {
    {"A", SDL_SCANCODE_A},
    {"B", SDL_SCANCODE_B},
    {"C", SDL_SCANCODE_C},
    {"D", SDL_SCANCODE_D},
    {"E", SDL_SCANCODE_E},
    {"F", SDL_SCANCODE_F},
    {"G", SDL_SCANCODE_G},
    {"H", SDL_SCANCODE_H},
    {"I", SDL_SCANCODE_I},
    {"J", SDL_SCANCODE_J},
    {"K", SDL_SCANCODE_K},
    {"L", SDL_SCANCODE_L},
    {"M", SDL_SCANCODE_M},
    {"N", SDL_SCANCODE_N},
    {"O", SDL_SCANCODE_O},
    {"P", SDL_SCANCODE_P},
    {"Q", SDL_SCANCODE_Q},
    {"R", SDL_SCANCODE_R},
    {"S", SDL_SCANCODE_S},
    {"T", SDL_SCANCODE_T},
    {"U", SDL_SCANCODE_U},
    {"V", SDL_SCANCODE_V},
    {"W", SDL_SCANCODE_W},
    {"X", SDL_SCANCODE_X},
    {"Y", SDL_SCANCODE_Y},
    {"Z", SDL_SCANCODE_Z},
    {"Num0", SDL_SCANCODE_0},
    {"Num1", SDL_SCANCODE_1},
    {"Num2", SDL_SCANCODE_2},
    {"Num3", SDL_SCANCODE_3},
    {"Num4", SDL_SCANCODE_4},
    {"Num5", SDL_SCANCODE_5},
    {"Num6", SDL_SCANCODE_6},
    {"Num7", SDL_SCANCODE_7},
    {"Num8", SDL_SCANCODE_8},
    {"Num9", SDL_SCANCODE_9},
    {"Escape", SDL_SCANCODE_ESCAPE},
    {"LControl", SDL_SCANCODE_LCTRL},
    {"LShift", SDL_SCANCODE_LSHIFT},
    {"LAlt", SDL_SCANCODE_LALT},
    {"LSystem", SDL_SCANCODE_LGUI},
    {"RControl", SDL_SCANCODE_RCTRL},
    {"RShift", SDL_SCANCODE_RSHIFT},
    {"RAlt", SDL_SCANCODE_RALT},
    {"RSystem", SDL_SCANCODE_RGUI},
    {"Menu", SDL_SCANCODE_MENU},
    {"LBracket", SDL_SCANCODE_LEFTBRACKET},
    {"RBracket", SDL_SCANCODE_RIGHTBRACKET},
    {"SemiColon", SDL_SCANCODE_SEMICOLON},
    {"Comma", SDL_SCANCODE_COMMA},
    {"Period", SDL_SCANCODE_PERIOD},
    {"Quote", SDL_SCANCODE_APOSTROPHE},
    {"Slash", SDL_SCANCODE_SLASH},
    {"BackSlash", SDL_SCANCODE_BACKSLASH},
    {"Tilde", SDL_SCANCODE_GRAVE},
    {"Equal", SDL_SCANCODE_EQUALS},
    {"Dash", SDL_SCANCODE_MINUS},
    {"Space", SDL_SCANCODE_SPACE},
    {"Return", SDL_SCANCODE_RETURN},
    {"BackSpace", SDL_SCANCODE_BACKSPACE},
    {"Tab", SDL_SCANCODE_TAB},
    {"PageUp", SDL_SCANCODE_PAGEUP},
    {"PageDown", SDL_SCANCODE_PAGEDOWN},
    {"End", SDL_SCANCODE_END},
    {"Home", SDL_SCANCODE_HOME},
    {"Insert", SDL_SCANCODE_INSERT},
    {"Delete", SDL_SCANCODE_DELETE},
    {"Add", SDL_SCANCODE_KP_PLUS},
    {"Subtract", SDL_SCANCODE_KP_MINUS},
    {"Multiply", SDL_SCANCODE_KP_MULTIPLY},
    {"Divide", SDL_SCANCODE_KP_DIVIDE},
    {"Left", SDL_SCANCODE_LEFT},
    {"Right", SDL_SCANCODE_RIGHT},
    {"Up", SDL_SCANCODE_UP},
    {"Down", SDL_SCANCODE_DOWN},
    {"Numpad0", SDL_SCANCODE_KP_0},
    {"Numpad1", SDL_SCANCODE_KP_1},
    {"Numpad2", SDL_SCANCODE_KP_2},
    {"Numpad3", SDL_SCANCODE_KP_3},
    {"Numpad4", SDL_SCANCODE_KP_4},
    {"Numpad5", SDL_SCANCODE_KP_5},
    {"Numpad6", SDL_SCANCODE_KP_6},
    {"Numpad7", SDL_SCANCODE_KP_7},
    {"Numpad8", SDL_SCANCODE_KP_8},
    {"Numpad9", SDL_SCANCODE_KP_9},
    {"F1", SDL_SCANCODE_F1},
    {"F2", SDL_SCANCODE_F2},
    {"F3", SDL_SCANCODE_F3},
    {"F4", SDL_SCANCODE_F4},
    {"F5", SDL_SCANCODE_F5},
    {"F6", SDL_SCANCODE_F6},
    {"F7", SDL_SCANCODE_F7},
    {"F8", SDL_SCANCODE_F8},
    {"F9", SDL_SCANCODE_F9},
    {"F10", SDL_SCANCODE_F10},
    {"F11", SDL_SCANCODE_F11},
    {"F12", SDL_SCANCODE_F12},
    {"F13", SDL_SCANCODE_F13},
    {"F14", SDL_SCANCODE_F14},
    {"F15", SDL_SCANCODE_F15},
    {"Pause", SDL_SCANCODE_PAUSE},
};

string HotkeyConfig::getStringForKey(SDL_Scancode key)
{
    for(auto key_name : sdl_key_names)
    {
        if (key_name.second == key)
        {
            return key_name.first;
        }
    }

    return "";
}

void HotkeyConfig::load()
{
    for(HotkeyConfigCategory& cat : categories)
    {
        for(HotkeyConfigItem& item : cat.hotkeys)
        {
            string key_config = PreferencesManager::get(std::string("HOTKEY.") + cat.key + "." + item.key, std::get<1>(item.value));
            item.load(key_config);
            item.value = std::make_tuple(std::get<0>(item.value), key_config);
        }
    }
}

std::vector<HotkeyResult> HotkeyConfig::getHotkey(const SDL_Keysym& key)
{
    std::vector<HotkeyResult> results;
    // SDL will report *all* mod keys - and that includes caps lock and num.
    // However, the game only cares about ctrl/alt/shift/gui (or 'system' in SFML lingo)
    static constexpr uint16_t relevant_mods = KMOD_CTRL | KMOD_ALT | KMOD_GUI | KMOD_SHIFT;
    for(HotkeyConfigCategory& cat : categories)
    {
        for(HotkeyConfigItem& item : cat.hotkeys)
        {
            const auto filtered_mod = key.mod & relevant_mods;
            if (item.hotkey.scancode == key.scancode && item.hotkey.mod == filtered_mod)
            {
                results.emplace_back(cat.key, item.key);
            }
        }
    }
    return results;
}

void HotkeyConfig::newCategory(string key, string name)
{
    categories.emplace_back();
    categories.back().key = key;
    categories.back().name = name;
}

void HotkeyConfig::newKey(string key, std::tuple<string, string> value)
{
    categories.back().hotkeys.emplace_back(key, value);
}

std::vector<string> HotkeyConfig::getCategories()
{
    // Initialize return value.
    std::vector<string> ret;

    // Add each category to the return value.
    for(HotkeyConfigCategory& cat : categories)
    {
        ret.push_back(cat.name);
    }

    return ret;
}

std::vector<std::pair<string, string>> HotkeyConfig::listHotkeysByCategory(string hotkey_category)
{
    std::vector<std::pair<string, string>> ret;

    for(HotkeyConfigCategory& cat : categories)
    {
        if (cat.name == hotkey_category)
        {
            for(HotkeyConfigItem& item : cat.hotkeys)
            {
                for(const auto &key_name : sdl_key_names)
                {
                    if (key_name.second == item.hotkey.scancode)
                    {
                        string keyModifier = "";
                        if (item.hotkey.mod & KMOD_SHIFT) {
                            keyModifier = "Shift+";
                        } else if (item.hotkey.mod & KMOD_CTRL) {
                            keyModifier = "Ctrl+";
                        } else if (item.hotkey.mod & KMOD_ALT){
                            keyModifier = "Alt+";
                        }
                        ret.push_back({std::get<0>(item.value), keyModifier + key_name.first});
                    }
                }
            }
        }
    }

    return ret;
}

std::vector<std::pair<string, string>> HotkeyConfig::listAllHotkeysByCategory(string hotkey_category)
{
    std::vector<std::pair<string, string>> ret;

    for(HotkeyConfigCategory& cat : categories)
    {
        if (cat.name == hotkey_category)
        {
            for(HotkeyConfigItem& item : cat.hotkeys)
            {
                ret.push_back({std::get<0>(item.value), std::get<1>(item.value)});
            }
        }
    }

    return ret;
}

SDL_Scancode HotkeyConfig::getKeyByHotkey(string hotkey_category, string hotkey_name)
{
    for(HotkeyConfigCategory& cat : categories)
    {
        if (cat.key == hotkey_category)
        {
            for(HotkeyConfigItem& item : cat.hotkeys)
            {
                if (item.key == hotkey_name)
                {
                    return item.hotkey.scancode;
                }
            }
        }
    }

    LOG(WARNING) << "Requested an SFML Key from hotkey " << hotkey_category << ", " << hotkey_name << ", but none was found.";
    return SDL_NUM_SCANCODES;
}

HotkeyConfigItem::HotkeyConfigItem(string key, std::tuple<string, string> value)
{
    this->key = key;
    this->value = value;
    hotkey.scancode = SDL_NUM_SCANCODES;
    hotkey.mod = 0;
}

void HotkeyConfigItem::load(string key_config)
{
    for(const string& config : key_config.split(";"))
    {
        if (config == "[alt]")
            hotkey.mod |= KMOD_ALT;
        else if (config == "[control]")
            hotkey.mod |= KMOD_CTRL;
        else if (config == "[shift]")
            hotkey.mod |= KMOD_CTRL;
        else if (config == "[system]")
            hotkey.mod |= KMOD_GUI;
        else
        {
            for(const auto &key_name : sdl_key_names)
            {
                if (key_name.first.lower() == config.lower())
                {
                    hotkey.scancode = key_name.second;
                    break;
                }
            }
        }
    }
}

bool HotkeyConfig::setHotkey(std::string work_cat, std::pair<string,string> key, string new_value)
{
    // test if new_value is part of the sfml_list
    for (std::pair<string, SDL_Scancode> sdl_key : sdl_key_names)
    {
        if ((sdl_key.first.lower() == new_value.lower()) || new_value == "")
        {
            for (HotkeyConfigCategory &cat : categories)
            {
                if (cat.name == work_cat)
                {
                    for (HotkeyConfigItem &item : cat.hotkeys)
                    {
                        if (key.first == std::get<0>(item.value))
                        {
                            item.load(new_value);
                            item.value = std::make_tuple(std::get<0>(item.value), new_value);

                            PreferencesManager::set(std::string("HOTKEY.") + cat.key + "." + item.key, std::get<1>(item.value));

                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}
