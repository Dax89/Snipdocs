local M = { }

function M.private_stash()
    return setmetatable({}, {__mode = "k"})
end

function M.class(base)
    if base and type(base) ~= "table" then
        error("Base class type must be a table, got '" .. type(base) .. "'")
    end

    local MT = {
        __index = base or nil,
        __call = function(self, ...)
            local inst = setmetatable({class = self}, {__index = self})
            if type(inst.init) == "function" then inst:init(...) end
            return inst
        end
    }

    return setmetatable({
        super = base,
        instanceof = function(self, c)
            if not c then error("Invalid Class Type") end
            if type(c) ~= "table" then error("Expected 'table', got '" .. type(c) .. "'") end

            if self.class == c then return true end
            if not self.super then return false end
            if self.super == c then return true end

            return self.super.instanceof and self.super:instanceof(c) or false
        end
    }, MT)
end

return M
