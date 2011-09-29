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

module(..., package.seeall) 

-- Return a closure that generates a sequence of n dimensional normally
-- distributed numbers with the specified mean and variance
function make_gaussian_sequence(...)
    local n = #arg / 2 
    local params = arg
    local next

    function box_muller()
        local u = math.random()
        local v = math.random()

        return math.sqrt(-2.0*math.log(u))*math.cos(2.0*math.pi*v),
            math.sqrt(-2.0*math.log(u))*math.sin(2.0*math.pi*v)
    end

    return function()
        local i
        local result = {}

        for i=1,n do
            if (next) then
                table.insert(result, params[i*2 -1 ] + params[i*2]*next);
                next = nil
            else
                local current, next = box_muller()
                table.insert(result, params[i*2 -1 ] + params[i*2]*current)
            end
        end

        return unpack(result)
    end
end


-- Return a closure that generates a sequence of n dimensional uniformly 
-- distributed numbers in the range [-scale, scale] 
function make_uniform_sequence(...)
    local n = #arg / 2 
    local params = arg

    return function()
        local i
        local result = {}

        for i=1,n do
            local min = params[i*2 - 1]
            local max = params[i*2]
            table.insert(result, min + math.random()*(max - min))
        end

        return unpack(result) 
    end
end 

-- Return n random points from a mixture of k gaussian distributions,
-- with means drawn from the uniforms and with the specified variances
function make_mixture_of_gaussians_sequence(k, uniforms, variances)

    local zip = function(xs, ys)
        local result = {}

        local min
        if #xs < #ys then
            min = #xs
        else
            min = #ys
        end

        local i
        for i=1,min do
            result[i*2-1] = xs[i]
            result[i*2] = ys[i] 
        end

        i = min
        while i < #xs do
            result[i] = xs[i]
            i = i + 1
        end

        while i < #ys do
            result[i] = ys[i]
            i = i + 1
        end 

        return result
    end

    local sequences = {}
    for i=1,k do 
        local means = {uniforms()} 
        local params = zip(means, variances) 
        table.insert(sequences, make_gaussian_sequence(unpack(params)))
    end

    local next_sequence = 1

    return function() 
        local result = {sequences[next_sequence]()}
        next_sequence = next_sequence + 1
        if next_sequence > k then next_sequence = 1 end

        return unpack(result)
    end 
end
