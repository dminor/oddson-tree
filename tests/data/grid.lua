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

math.randomseed(os.time())

-- uniform in [-scale, scale]
function uniform(scale_x, scale_y) 
    return (math.random() - 0.5)*2.0*scale_x,
        (math.random() - 0.5)*2.0*scale_y
end

if #arg >= 1 then
    pt_count = math.sqrt(tonumber(arg[1]))

    scale= arg[2] or 1.0
    jitter = arg[3] or 0.0

    print(pt_count*pt_count .. ' grid: ' .. scale .. ' ' .. jitter)
    for i=1, pt_count do
        for j=1, pt_count do 
            x, y = uniform(jitter, jitter)

            x = x + i / pt_count * scale
            y = y + j / pt_count * scale
            print(x .. ', ' .. y)
        end
    end 
else 
    print('usage grid.lua <count> <scale> <jitter>')
    return 
end
