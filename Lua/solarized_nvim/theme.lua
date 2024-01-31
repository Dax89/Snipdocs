local lush = require("lush")
local C = require("config.solarized.palette")

---@diagnostic disable: undefined-global
local colorscheme = lush(function(f)
    local sym = f.sym

    return {
        --- @@@ General @@@
        Comment { fg = C.base01, italic = true },
        Constant { fg = C.cyan },
        Error { fg = C.red, bold = true },
        Identifier { fg = C.blue },
        Normal { fg = C.base0, bg = C.base03 },
        PreProc { fg = C.orange },
        Special { fg = C.red },
        Statement { fg = C.green },
        String { fg = C.violet },
        Todo { fg = C.magenta, bold = true },
        Type { fg = C.yellow },
        Underlined { fg = C.violet, underline = true },
        EndOfBuffer { Normal },

        --- @@@ Extended @@@
        Conceal { fg = C.blue },
        Cursor { fg = C.base03, bg = C.base0 },
        CursorColumn { bg = C.base02 },
        CursorLine { sp = C.base1, bg = C.base02 },
        CursorLineNr { fg = C.base01, bg = C.base03, sp = C.base1 },
        DiffAdd { fg = C.green, reverse = true },
        DiffChange { fg = C.blue, reverse = true },
        DiffDelete { fg = C.red, reverse = true },
        DiffText { fg = C.green, reverse = true },
        Directory { fg = C.blue },
        ErrorMsg { fg = C.red },
        FoldColumn { fg = C.base0, bg = C.base02 },
        Folded { fg = C.base0, bg = C.base02, sp = C.base03, underline = true, bold = true },
        IncSearch { fg = C.orange, reverse = true },
        LineNr { fg = C.base01, bg = C.base02 },
        MatchParen { fg = C.orange, bold = true },
        ModeMsg { fg = C.blue },
        MoreMsg { fg = C.blue },
        NonText { fg = C.base00, bold = true },
        Pmenu { fg = C.base1, bg = C.base02 },
        PmenuSbar { fg = C.base0, bg = C.base2 },
        PmenuSel { fg = C.base03, bg = C.base00 },
        PmenuThumb { fg = C.base03, bg = C.base0 },
        Question { fg = C.cyan, bold = true },
        QuickFixLine { bg = C.base02 },
        Search { fg = C.base03, bg = C.green },
        SignColumn { fg = C.base0, bg = C.base02 },
        SpecialKey { fg = C.base00, bg = C.base02, bold = true },
        SpellBad { sp = C.red, undercurl = true },
        SpellCap { sp = C.violet, undercurl = true },
        SpellLocal { sp = C.yellow, undercurl = true },
        SpellRare { sp = C.cyan, undercurl = true },
        StatusLine { fg = C.base01, bg = C.base1 },
        StatusLineNC { fg = C.base01, bg = C.base02 },
        TabLine { fg = C.base0, bg = C.base02, sp = C.base0, underline = true },
        TabLineFill { fg = C.base0, bg = C.base02, sp = C.base0, underline = true },
        TabLineSel { fg = C.base2, bg = C.base01, sp = C.base0, underline = true },
        Title { fg = C.orange, bold = true },
        VertSplit { fg = C.base1, bg = C.base03 },
        Visual { fg = C.base03, bg = C.base01 },
        VisualNOS { bg = C.base02, reverse = true },
        WarningMsg { fg = C.yellow },
        WildMenu { fg = C.base02, bg = C.base2 },
        lCursor { Cursor },

        --- @@@ Diagnostic @@@
        DiagnosticDeprecated { fg = C.base00, strikethrough = true },
        DiagnosticHint { fg = C.base1 },
        DiagnosticInfo { fg = C.blue },
        DiagnosticOk { fg = C.green },
        DiagnosticWarn { fg = C.orange },
        DiagnosticError { ErrorMsg },

        --- @@@ TreeSitter @@@
        sym("@variable") { fg = Normal.fg },
        sym("@variable.builtin") { Identifier },

        --- @@@ LSP @@@
        sym("@lsp.type.variable") { fg = C.magenta },
        sym("@lsp.mod.readonly") { Constant },

        --- @@@ nvim-telescope/telescope.nvim @@@
        TelescopeSelection { CursorLine },

        --- @@@ nvimdev/dashboard-nvim @@@
        DashboardHeader { fg = C.blue },
        DashboardFooter { fg = C.cyan },
    }
end)

return colorscheme
