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
function box_muller(mu_x, sigma_x, mu_y, sigma_y)
    local u = math.random()
    local v = math.random()

    return mu_x + sigma_x*math.sqrt(-2.0*math.log(u))*math.cos(2.0*math.pi*v),
        mu_y + sigma_y*math.sqrt(-2.0*math.log(u))*math.sin(2.0*math.pi*v)
end

-- uniform in [-scale, scale]
function uniform(scale_x, scale_y) 
    return (math.random() - 0.5)*2.0*scale_x,
        (math.random() - 0.5)*2.0*scale_y
end

if #arg >= 2 then
    pt_count = arg[2]

    if arg[1] == '-u' then
        scale_x = arg[3] or 1.0
        scale_y = arg[4] or scale_x 

        print(pt_count .. ' ' .. 2 .. ' uniform: ' .. scale_x .. ' ' .. scale_y)
        for i=1, pt_count do
            x, y = uniform(scale_x, scale_y)
            print(x .. ', ' .. y)
        end 
    elseif arg[1] == '-n' then
        mu_x = arg[3] or 0.0
        sigma_x = arg[4] or 1.0 
        mu_y = arg[5] or mu_x
        sigma_y = arg[6] or sigma_x

        print(pt_count .. ' ' .. 2 .. ' mu: ' .. mu_x .. ' sigma_x: ' .. sigma_x
            .. ' mu_y: ' .. mu_y .. ' sigma_y: ' .. sigma_y)
        for i=1, pt_count do
            x, y = box_muller(mu_x, sigma_x, mu_y, sigma_y)
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
