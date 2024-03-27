local tests = {}
local testcase = setmetatable({}, {
    __newindex = function(_, k, v)
        if type(k) ~= 'string' then
            error('testcase key must be a string', 2)
        elseif type(v) ~= 'function' then
            error(string.format('testcase.%s value must be a function', k), 2)
        elseif tests[k] then
            error(string.format('testcase.%s already exists', k), 2)
        end
        tests[k] = v
        tests[#tests + 1] = {
            name = k,
            func = v,
        }
    end,
})

local pcall = pcall
local assert = require('assert')
local fileno = require('io.fileno')
local close = require('io.close')

function testcase.close()
    local f = assert(io.tmpfile())

    -- test that close a file
    local ok, err = close(f)
    assert.is_nil(err)
    assert.is_true(ok)

    -- test that return error when close twice
    ok, err = close(f)
    assert.match(err, 'EBADF')
    assert.is_false(ok)

    -- test that close a file descriptor
    f = assert(io.tmpfile())
    local fd = fileno(f)
    ok, err = close(fd)
    assert.is_nil(err)
    assert.is_true(ok)

    -- test that return error when close twice
    ok, err = close(fd)
    assert.match(err, 'EBADF')
    assert.is_false(ok)

    -- test that throw error if argument is not a file or file descriptor
    err = assert.throws(close, 'not a file')
    assert.match(err, 'FILE* expected, got string')
end

function testcase.cannot_close_standard_files()
    -- test that cannot close standard files
    for _, f in ipairs({
        io.stdin,
        io.stdout,
        io.stderr,
    }) do
        local ok, err = close(f)
        assert.match(err, 'cannot close standard file')
        assert.is_false(ok)
    end
end

function testcase.cannot_close_if_file_has_no_close_method()
    -- test that cannot close if file has no close method
    local f = assert(io.tmpfile())
    local mt = getmetatable(f)
    local close_method = mt.close or mt.__close
    mt.__close = nil
    mt.close = nil

    local ok, err = close(f)
    mt.__close = close_method
    mt.close = close_method
    assert.match(err, 'has no close method')
    assert.is_false(ok)

end

local function printf(fmt, ...)
    io.stdout:write(string.format(fmt, ...))
end

printf('run %d tests\n', #tests)
print('===============================================')
local nerr = 0
for _, t in ipairs(tests) do
    printf('- %s ... ', t.name)
    local ok, err = pcall(t.func)
    if ok then
        print('ok')
    else
        print('fail')
        print(err)
        nerr = nerr + 1
    end
end
print('===============================================')

if nerr == 0 then
    printf('all %d tests passed\n\n', #tests)
    os.exit(0)
end
printf('%d tests failed\n\n', nerr)
os.exit(1)

