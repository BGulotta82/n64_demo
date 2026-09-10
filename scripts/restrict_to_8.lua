-- Round to nearest increment of 8
local function round8(val)
    return math.max(0, math.min(255, math.floor((val / 8) + 0.5) * 8))
end

-- Get current foreground color
local color = app.fgColor

-- Restrict RGB values
local r = round8(color.red)
local g = round8(color.green)
local b = round8(color.blue)

-- Apply the restricted color back to the palette
app.fgColor = Color{ r=r, g=g, b=b, a=color.alpha }
