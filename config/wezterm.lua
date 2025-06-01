local wezterm = require 'wezterm'

local config = wezterm.config_builder()

---------
-- Config start

config.window_background_opacity = 0.75

config.font_size = 10.0

config.enable_wayland = false -- use Xwayland on wayland

config.audible_bell = "Disabled"

config.initial_cols = 120
config.initial_rows = 40

config.font = wezterm.font_with_fallback({
    "Hack",
    "Noto Sans CJK SC",
})

config.keys = {
    {
        key = 'Backspace',
        mods = 'CTRL',
        action = wezterm.action.SendKey {
            key = 'w',
            mods = "CTRL"
        }
    }
}

return config
