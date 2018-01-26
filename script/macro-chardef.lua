local _pf = loadfile("char-def.lua")
local primitives = _pf()



if not(type(characters) == 'table' and type(characters.data) == 'table')  then 
 --print("no data, exit")
 local f 
 f = io.open('macro-chardef.txt','w')
 f:close()
 return 
end

-- identity function for now
local function filter(name,k,v)
 return name
end

local macro = {}
local data  = characters.data
for k,v in pairs(data) do
 if v.contextname then
  -- print(v.contextname,k)
  macro[#macro+1]=filter(v.contextname,k,v)
 end
end 
table.sort(macro)


local f 
f = io.open('macro-chardef.txt','w')
for _,v in pairs(macro) do
  f:write([["\]].. v..[["]].."\n") 
end
f:close()

