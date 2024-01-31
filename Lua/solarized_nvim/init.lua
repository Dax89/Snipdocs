local M = {}

function M.setup_terminal_colors()
    local palette = require("config.solarized.palette")
    vim.g.terminal_color_0 = palette.base03.hex
    vim.g.terminal_color_1 = palette.red.hex
    vim.g.terminal_color_2 = palette.green.hex
    vim.g.terminal_color_3 = palette.yellow.hex
    vim.g.terminal_color_4 = palette.blue.hex
    vim.g.terminal_color_5 = palette.magenta.hex
    vim.g.terminal_color_6 = palette.cyan.hex
    vim.g.terminal_color_7 = palette.base2.hex
    vim.g.terminal_color_8 = palette.base02.hex
    vim.g.terminal_color_9 = palette.orange.hex
    vim.g.terminal_color_10 = palette.base01.hex
    vim.g.terminal_color_11 = palette.base00.hex
    vim.g.terminal_color_12 = palette.base0.hex
    vim.g.terminal_color_13 = palette.violet.hex
    vim.g.terminal_color_14 = palette.base1.hex
    vim.g.terminal_color_15 = palette.base3.hex
end

function M.setup(config)
    M.config = vim.F.if_nil(config, {})
    vim.g.colors_name = "solarized"
    M.setup_terminal_colors()

    local colorscheme = require("config.solarized.theme")
    -- TODO: Customize Highlight Groups
    return colorscheme
end

return M
