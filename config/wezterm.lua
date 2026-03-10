local wezterm = require 'wezterm'

local config = wezterm.config_builder()

---------
-- Config start

config.window_background_opacity = 0.75 -- Transparent background

config.font_size = 10.0

config.enable_wayland = true

config.audible_bell = "Disabled"

config.initial_cols = 120
config.initial_rows = 40

config.font = wezterm.font_with_fallback({
    "Hack",
    "Noto Sans CJK SC",
})

-- Make Ctrl+Backspace act as Ctrl+w
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

-- 3 lines scrolled once using mouse wheel
config.mouse_bindings = {
  {
    event = { Down = { streak = 1, button = { WheelUp = 1 } } },
    action = wezterm.action.ScrollByLine(-3),
  },
  {
    event = { Down = { streak = 1, button = { WheelDown = 1 } } },
    action = wezterm.action.ScrollByLine(3),
  },
}


return config
