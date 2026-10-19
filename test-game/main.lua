-- minimal smoke test for lovelite
local x, y = 120, 160
local speed = 100
local t = 0

function love.load()
  print("love.load ok")
end

function love.update(dt)
  t = t + dt
  if love.keyboard.isDown("left")  then x = x - speed * dt end
  if love.keyboard.isDown("right") then x = x + speed * dt end
  if love.keyboard.isDown("up")    then y = y - speed * dt end
  if love.keyboard.isDown("down")  then y = y + speed * dt end
end

function love.keypressed(key)
  print("pressed: " .. key)
end

function love.draw()
  love.graphics.setColor(0.1, 0.1, 0.3)
  love.graphics.rectangle("fill", 0, 0, 240, 320)
  love.graphics.setColor(1, 0.4, 0.2)
  love.graphics.rectangle("fill", x - 10, y - 10, 20, 20)
  love.graphics.setColor(1, 1, 1)
  love.graphics.circle("line", 120, 160, 50 + math.sin(t * 3) * 10)
  love.graphics.print("lovelite on KaiOS  FPS:" .. love.timer.getFPS(), 4, 4)
end
