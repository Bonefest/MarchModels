function loadNameSpace(namespace)
   for key, value in pairs(namespace) do
      if _G[k] ~= nil then
         _G[k] = namespace[k]
      end
   end
end

float3 = require("scripts/internal_vector")

loadNameSpace(math)

