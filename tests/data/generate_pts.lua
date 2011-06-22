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

-- normal mean mu, variance sigma
function box_muller(mu, sigma)
    local u = math.random()
    local v = math.random()

    return mu + sigma*math.sqrt(-2.0*math.log(u))*math.cos(2.0*math.pi*v),
        mu + sigma*math.sqrt(-2.0*math.log(u))*math.sin(2.0*math.pi*v)        
end

-- uniform in [-scale, scale]
function uniform(scale) 
    return (math.random() - 0.5)*2.0*scale,
        (math.random() - 0.5)*2.0*scale
end

if #arg >= 2 then
    pt_count = arg[2]

    if arg[1] == '-u' then
        scale = arg[3] or 1.0

        print(pt_count .. ' uniform: ' .. scale)
        for i=1, pt_count do
            x, y = uniform(scale)
            print(x .. ', ' .. y)
        end 
    elseif arg[1] == '-n' then
        mu = arg[3] or 0.0
        sigma = arg[4] or 1.0 

        print(pt_count .. ' mu: ' .. mu .. ' sigma: ' .. sigma)
        for i=1, pt_count do
            x, y = box_muller(mu, sigma)
            print(x .. ', ' .. y)
        end
    else
        print('usage generate_pts.lua -n | -u <count>')
        return
    end 
else 
    print('usage generate_pts.lua -n | -u <count>')
    return 
end
