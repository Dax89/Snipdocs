local lush    = require("lush")
local hsl     = lush.hsl

-- https://github.com/altercation/solarized
-- https://github.com/purpleP/vim-colors-solarized/blob/master/colors/solarized.vim

local PALETTE = {
    b       = {
        base03 = hsl("#002b36"),
        base02 = hsl("#073642"),
        base01 = hsl("#586e75"),
        base00 = hsl("#657b83"),
        base0  = hsl("#839496"),
        base1  = hsl("#93a1a1"),
        base2  = hsl("#eee8d5"),
        base3  = hsl("#fdf6e3"),
    },

    yellow  = hsl("#b58900"),
    orange  = hsl("#cb4b16"),
    red     = hsl("#dc322f"),
    magenta = hsl("#d33682"),
    violet  = hsl("#6c71c4"),
    blue    = hsl("#268bd2"),
    cyan    = hsl("#2aa198"),
    green   = hsl("#859900"),
}

if vim.o.background == "light" then
    PALETTE.base03 = PALETTE.b.base3
    PALETTE.base02 = PALETTE.b.base2
    PALETTE.base01 = PALETTE.b.base1
    PALETTE.base00 = PALETTE.b.base0
    PALETTE.base0  = PALETTE.b.base00
    PALETTE.base1  = PALETTE.b.base01
    PALETTE.base2  = PALETTE.b.base02
    PALETTE.base3  = PALETTE.b.base03
else
    PALETTE.base03 = PALETTE.b.base03
    PALETTE.base02 = PALETTE.b.base02
    PALETTE.base01 = PALETTE.b.base01
    PALETTE.base00 = PALETTE.b.base00
    PALETTE.base0  = PALETTE.b.base0
    PALETTE.base1  = PALETTE.b.base1
    PALETTE.base2  = PALETTE.b.base2
    PALETTE.base3  = PALETTE.b.base3
end

return PALETTE
