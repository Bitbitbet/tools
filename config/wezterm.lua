local wezterm = require 'wezterm'

local config = wezterm.config_builder()


config.window_background_opacity = 0.75

config.font_size = 10.0

-- Use Xwayland on wayland
config.enable_wayland = false

return config
