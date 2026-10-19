-- LOVE glue for Lua.
-- Runs before the game's main.lua.
arg = arg or {}

-- Lua 5.1 / LuaJIT compatibility (LOVE games expect these)
unpack = unpack or table.unpack
loadstring = loadstring or load
math.pow = math.pow or function(a, b) return a ^ b end
table.getn = table.getn or function(t) return #t end
math.randomseed(os.time())

if not setfenv then
function setfenv(f, t)
	f = (type(f) == 'function' and f or debug.getinfo(f + 1, 'f').func)
	local name
	local up = 0
	repeat
	up = up + 1
	name = debug.getupvalue(f, up)
	until name == '_ENV' or name == nil
	if name then
	debug.upvaluejoin(f, up, function() return name end, 1)
	debug.setupvalue(f, up, t)
	end
	return f
end
function getfenv(f)
	f = (type(f) == 'function' and f or debug.getinfo(f + 1, 'f').func)
	local name, val
	local up = 0
	repeat
	up = up + 1
	name, val = debug.getupvalue(f, up)
	until name == '_ENV' or name == nil
	return val
end
end

package.path = '/assets/?.lua;/assets/?/init.lua;./?.lua;./?/init.lua;' .. package.path

-- love.arg / love.system --
love.arg = {
parseGameArguments = function() return {} end,
parseOptions = function() return {} end,
}
love.system = {
getOS = function() return "Web" end,
getPowerInfo = function() return "unknown", nil, nil end,
}

-- love.math --
love.math = {
random = math.random,
randomseed = math.randomseed,
setRandomSeed = math.randomseed,
}

local _random = math.random
math.random = function(m, n)
	if m and n and m > n then m, n = n, m end
	return _random(m, n)
end

-- love.event --
love.event = {}
local queue = {}
function love.event.pump() end
function love.event.push(name, ...)
queue[#queue + 1] = table.pack(name, ...)
end
function love.event.poll()
local i = 0
return function()
	i = i + 1
	local e = queue[i]
	if e then return unpack(e, 1, e.n) end
	queue = {}
	return nil
end
end
function love.event.quit(code)
love.event.push("quit", code)
end

love.handlers = setmetatable({
keypressed = function(key, sc, isrepeat)
	if love.keypressed then return love.keypressed(key, sc, isrepeat) end
end,
keyreleased = function(key, sc)
	if love.keyreleased then return love.keyreleased(key, sc) end
end,
}, { __index = function() return function() end end })

-- love.filesystem extras --
function love.filesystem.lines(name)
local s = assert(love.filesystem.read(name))
local pos, n = 1, #s
return function()
	if pos > n then return nil end
	local e = s:find("\n", pos, true)
	local line
	if e then
	line = s:sub(pos, e - 1)
	pos = e + 1
	else
	line = s:sub(pos)
	pos = n + 1
	end
	return (line:gsub("\r$", ""))
end
end
function love.filesystem.exists(p) return love.filesystem.getInfo(p) ~= nil end
function love.filesystem.createDirectory() return true end
function love.filesystem.setIdentity() end
function love.filesystem.getSaveDirectory() return "/" end

-- love.window / joystick / mouse / keyboard stubs --
love.window = {
setTitle = function() end,
setMode = function() return true end,
getMode = function() return love.graphics.getWidth(), love.graphics.getHeight(), {} end,
setFullscreen = function() return true end,
getDimensions = function() return love.graphics.getWidth(), love.graphics.getHeight() end,
}
love.joystick = { getJoysticks = function() return {} end }
love.mouse = {
isDown = function() return false end,
getPosition = function() return 0, 0 end,
setVisible = function() end,
}
love.keyboard.setKeyRepeat = function() end
love.keyboard.hasKeyRepeat = function() return false end

-- approximate printf (left aligned; align/limit ignored)
function love.graphics.printf(text, x, y, limit, align, ...)
love.graphics.print(text, x, y, ...)
end

-- default love.run (games like the ones using "tick" replace it) --
function love.run()
if love.load then love.load(love.arg.parseGameArguments(arg), arg) end
love.timer.step()
local dt = 0
return function()
	love.event.pump()
	for name, a, b, c, d, e, f in love.event.poll() do
	if name == "quit" then
		if not love.quit or not love.quit() then return a or 0 end
	end
	love.handlers[name](a, b, c, d, e, f)
	end
	dt = love.timer.step()
	if love.update then love.update(dt) end
	if love.graphics.isActive() then
	love.graphics.origin()
	love.graphics.clear(love.graphics.getBackgroundColor())
	if love.draw then love.draw() end
	love.graphics.present()
	end
end
end

-- optional conf.lua --
function love._loadconf()
if love.filesystem.getInfo("conf.lua") then
	dofile("conf.lua")
	if love.conf then
	local t = { window = {}, modules = {}, audio = {}, version = "11.4" }
	love.conf(t)
	end
end
end


-- love.image / love.window
love.image = love.image or {}
love.image.newImageData = function() return {} end
love.window.setIcon = function() end
love.window.showMessageBox = function() end

-- track font and canvas so getFont/getCanvas work
local _setFont, curFont = love.graphics.setFont, nil
function love.graphics.setFont(f) curFont = f; return _setFont(f) end
function love.graphics.getFont() return curFont end

local _setCanvas, curCanvas = love.graphics.setCanvas, nil
function love.graphics.setCanvas(c, ...) curCanvas = c; return _setCanvas(c, ...) end
function love.graphics.getCanvas() return curCanvas end

love.graphics.getScissor = function() return nil end
love.graphics.captureScreenshot = function() end

-- printf with wrapping and alignment (limit is in pre-scale units, like LOVE)
function love.graphics.printf(text, x, y, limit, align, r, sx, sy)
	r, sx = r or 0, sx or 1
	sy = sy or sx
	local font = curFont
	if not font or not limit then
		return love.graphics.print(text, x, y, r, sx, sy)
	end

	local lines = {}
	for para in (tostring(text) .. "\n"):gmatch("(.-)\n") do
		local line = ""
		for word in para:gmatch("%S+") do
			local try = (line == "") and word or (line .. " " .. word)
			if line ~= "" and font:getWidth(try) > limit then
				lines[#lines + 1] = line
				line = word
			else
				line = try
			end
		end
		lines[#lines + 1] = line
	end

	local lh = font:getHeight()
	love.graphics.push()
	love.graphics.translate(x, y)
	if r ~= 0 then love.graphics.rotate(r) end
	love.graphics.scale(sx, sy)
	for i, line in ipairs(lines) do
		local w, ox = font:getWidth(line), 0
		if align == "center" then ox = (limit - w) / 2
		elseif align == "right" then ox = limit - w end
		love.graphics.print(line, ox, (i - 1) * lh)
	end
	love.graphics.pop()
end
