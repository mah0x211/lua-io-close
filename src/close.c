/**
 *  Copyright (C) 2024 Masatoshi Fukunaga
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */
#include <unistd.h>
// lua
#include <lauxhlib.h>
#include <lua_errno.h>

static int closefp_lua(lua_State *L)
{
    const char *errmsg = NULL;
    int nret           = 0;

    // call file:close method on the metatable
    lua_getmetatable(L, 1);
    lua_pushstring(L, "close");
    lua_rawget(L, -2);
    if (lua_isfunction(L, -1)) {
        lua_insert(L, 1);
    } else {
        lua_pop(L, 1);
        lua_pushstring(L, "__close");
        lua_rawget(L, -2);
        if (!lua_isfunction(L, -1)) {
            lua_pushboolean(L, 0);
            lua_errno_new_with_message(L, EBADF, "close",
                                       "file object has no close method");
            return 2;
        }
        lua_insert(L, 1);
    }
    lua_settop(L, 2);
    errno = 0;
    switch (lua_pcall(L, 1, LUA_MULTRET, 0)) {
    case 0:
        nret = lua_gettop(L);
        nret = (nret > 3) ? 3 : nret;
        switch (nret) {
        case 3:
            errno = lua_tointeger(L, 3);
        case 2:
            errmsg = lua_tostring(L, 2);
        case 1:
            if (!lua_isboolean(L, 1)) {
                lua_pushboolean(L, 0);
            }
            break;

        default:
            lua_pushboolean(L, 1);
        }
        lua_settop(L, 1);
        break;

    default:
        errmsg = lua_tostring(L, -1);
        lua_pushboolean(L, 0);
        if (!errno) {
            errno = EBADF;
        }
    }

    if (errmsg) {
        lua_errno_new_with_message(L, errno, "close", errmsg);
        return 2;
    }
    return 1;
}

static int close_lua(lua_State *L)
{
    int fd   = -1;
    FILE *fp = NULL;

    if (lauxh_isint(L, 1)) {
        fd = lauxh_checkint(L, 1);
    } else {
        fp = lauxh_checkfile(L, 1);
        // if fp is NULL, then set fd to -1 to indicate invalid file descriptor
        fd = (fp) ? fileno(fp) : -1;
    }

    if (fd < 0) {
        lua_pushboolean(L, 0);
        lua_errno_new(L, EBADF, "close");
        return 2;
    } else if (fd <= 2) {
        lua_pushboolean(L, 0);
        lua_errno_new_with_message(L, EBADF, "close",
                                   "cannot close standard file descriptor");
        return 2;
    }

    lua_settop(L, 1);
    if (fp) {
        return closefp_lua(L);
    }

RETRY:
    if (close(fd) != 0) {
        if (errno == EINTR) {
            goto RETRY;
        }
        lua_pushboolean(L, 0);
        lua_errno_new(L, errno, "close");
        return 2;
    }
    lua_pushboolean(L, 1);
    return 1;
}

LUALIB_API int luaopen_io_close(lua_State *L)
{
    lua_errno_loadlib(L);
    lua_pushcfunction(L, close_lua);
    return 1;
}
