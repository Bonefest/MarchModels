function loadNameSpace(namespace)
   for key, value in pairs(namespace) do
      if _G[k] ~= nil then
         _G[k] = namespace[k]
      end
   end
end

loadNameSpace(math)

