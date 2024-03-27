# lua-io-close

[![test](https://github.com/mah0x211/lua-io-close/actions/workflows/test.yml/badge.svg)](https://github.com/mah0x211/lua-io-close/actions/workflows/test.yml)
[![codecov](https://codecov.io/gh/mah0x211/lua-io-close/branch/master/graph/badge.svg)](https://codecov.io/gh/mah0x211/lua-io-close)

Close the file or file descriptor.


## Installation

```sh
luarocks install io-close
```


## Error Handling

the following functions return the `error` object created by https://github.com/mah0x211/lua-errno module.


## ok, err = close( file )

close the file or file descriptor.

**Parameters**

- `file:file*|integer`: a file handle or a file descriptor.

**Returns**

- `ok:boolean`: `true` if the file is closed successfully.
- `err:any`: error object.


## Usage

```lua
local fileno = require('io.fileno')
local close = require('io.close')

local f = assert(io.tmpfile())
-- close file
local ok, err = close(f)
print(ok, err)
-- true nil

-- close file again
ok, err = close(f)
print(ok, err)
-- false ./example.lua:11: in main chunk: [EBADF:9][close] Bad file descriptor

-- file:close() method usually throws an error if the file is already closed
ok, err = pcall(function()
    f:close()
end)
print(ok, err)
-- false ./example.lua:17: attempt to use a closed file

-- close file descriptor
f = assert(io.tmpfile())
local fd = fileno(f)
ok, err = close(fd)
print(ok, err)
-- true nil

-- close file descriptor again
ok, err = close(fd)
print(ok, err)
-- false ./example.lua:30: in main chunk: [EBADF:9][close] Bad file descriptor
```
