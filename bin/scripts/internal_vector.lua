local float3 = {}
float3.__index = float3

local function new(x, y, z)
   return setmetatable({x = x or 0, y = y or 0, z = z or z}, float3)
end

function float3.__add(v1, v2)
   return new(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z)
end

function float3.__sub(v1, v2)
   return new(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z)
end

function float3.__mul(v1, v2)
   if type(v2) == 'number' then
      return new(v1.x * v2, v1.y * v2, v1.z * v2)
   elseif getmetatable(v2) == float3 then
      return new(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z)
   end
end

local function length2(v)
   assert(getmetatable(v) == float3)
   return v.x * v.x + v.y * v.y + v.z * v.z
end

local function length(v)
   assert(getmetatable(v) == float3)
   return math.sqrt(length2(v))
end

local function dot(v1, v2)
   assert(getmetatable(v1) == float3 and getmetatable(v2) == float3)
   return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z
end

local module = {}
module.new = new
module.length2 = length2
module.length = length
module.dot = dot
return setmetatable(module, {__call = function(_, ...) return new(...) end})

