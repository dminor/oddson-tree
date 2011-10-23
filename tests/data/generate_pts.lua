#!/usr/bin/lua

--[[
Copyright (c) 2011 Daniel Minor 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
--]]

require('random')

math.randomseed(os.time())

function shift(n, a, k)
    result = {}

    local k = k or #a

    for i=n + 1,k do
        result[i - n] = tonumber(arg[i])
    end 

    return result
end

if #arg < 2 then
    print('usage: generate_pts.lua -u|-g|-mog <count> [params]')
    return -1
end

local dist = arg[1]
local n = tonumber(arg[2])
local dim

local sequence
if dist == '-u' then
    local params = shift(2, arg)
    dim = #params / 2
    sequence = random.make_uniform_sequence(unpack(params))
elseif dist == '-g' then
    local params = shift(2, arg)
    dim = #params / 2
    sequence = random.make_gaussian_sequence(unpack(params)) 
elseif dist == '-mog' then
    dim = (#arg - 3)/4
    local k = tonumber(arg[3])
    local uniform_params = shift(3, arg, #arg - (#arg - 3)/4)
    local uniforms = random.make_uniform_sequence(unpack(uniform_params))
    local variances = shift(4 + 2*dim, arg)
    sequence = random.make_mixture_of_gaussians_sequence(k, uniforms, variances)
else
    print('error: unknown distribution')
end

local f = io.stdout 

f:write(n .. ' ' .. dim .. '\n')
for i=1,n do
    local values = {sequence()}
    for k, v in ipairs(values) do 
        f:write(v)
        if k < #values then
            f:write(', ')
        end
    end
    f:write('\n')
end
