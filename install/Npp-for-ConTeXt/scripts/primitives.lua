local _pf = loadfile("mult-prm.lua")
local primitives = _pf()
local etex = primitives.etex
local tex  = primitives.tex
local luatex = primitives.luatex

table.sort(etex)
table.sort(tex)
table.sort(luatex)

local f 
f = io.open('prm-etex.txt','w')
for _,v in pairs(etex) do
  f:write([["\]].. v..[["]].."\n") 
end
f:close()

f = io.open('prm-tex.txt','w')
for _,v in pairs(tex) do
  f:write([["\]].. v..[["]].."\n") 
end
f:close()

f = io.open('prm-luatex.txt','w')
for _,v in pairs(luatex) do
  f:write([["\]].. v..[["]].."\n") 
end
f:close()