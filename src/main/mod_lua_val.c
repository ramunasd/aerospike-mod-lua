#include "mod_lua_val.h"
#include "mod_lua_list.h"
#include "mod_lua_map.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>
#include <stdio.h>

#define LOG(m) \
    printf("%s:%d  -- %s\n",__FILE__,__LINE__, m);

as_val * mod_lua_takeval(lua_State * l, int i) {
    return mod_lua_toval(l, i);
}

as_val * mod_lua_retval(lua_State * l) {
    return mod_lua_toval(l, -1);
}

/**
 * Reads a val from the Lua stack
 *
 * @param l the lua_State to read the val from
 * @param i the position of the val on the stack
 * @returns the val if exists, otherwise NULL.
 */
as_val * mod_lua_toval(lua_State * l, int i) {
    switch( lua_type(l, i) ) {
        case LUA_TNUMBER : {
            return (as_val *) as_integer_new((long) lua_tonumber(l, i));
        }
        case LUA_TBOOLEAN : {
            return (as_val *) as_boolean_new(lua_toboolean(l, i));
        }
        case LUA_TSTRING : {
            return (as_val *) as_string_new(strdup(lua_tostring(l, i)));
        }
        case LUA_TUSERDATA : {
            mod_lua_box * box = (mod_lua_box *) lua_touserdata(l, i);
            if ( box && box->value ) {
                switch( as_val_type(box->value) ) {
                    case AS_BOOLEAN: 
                    case AS_INTEGER: 
                    case AS_STRING: 
                    case AS_LIST:
                    case AS_MAP:
                        switch (box->scope) {
                            case MOD_LUA_SCOPE_LUA:
                                return as_val_ref(box->value);
                            case MOD_LUA_SCOPE_HOST:
                                return box->value;
                        }
                    default:
                        return NULL;
                }
            }
            else {
                return (as_val *) NULL;
            }
        }
        case LUA_TNIL :
        case LUA_TTABLE :
        case LUA_TFUNCTION :
        case LUA_TLIGHTUSERDATA :
        default:
            return (as_val *) NULL;
    }
}


/**
 * Pushes a val onto the Lua stack
 *
 * @param l the lua_State to push the val onto
 * @param v the val to push on to the stack
 * @returns number of values pushed
 */
int mod_lua_pushval(lua_State * l, const as_val * v) {
    switch( as_val_type(v) ) {
        case AS_BOOLEAN: {
            lua_pushboolean(l, as_boolean_tobool((as_boolean *) v) );
            return 1;
        }
        case AS_INTEGER: {
            lua_pushinteger(l, as_integer_toint((as_integer *) v) );
            return 1;
        }
        case AS_STRING: {
            lua_pushstring(l, as_string_tostring((as_string *) v) );
            return 1;   
        }
        case AS_LIST: {
            mod_lua_pushlist(l, (as_list *) as_val_ref((as_val *) v));
            return 1;   
        }
        case AS_MAP: {
            mod_lua_pushmap(l, (as_map *) as_val_ref((as_val *) v));
            return 1;   
        }
        case AS_PAIR: {
            as_pair * p = (as_pair *) lua_newuserdata(l, sizeof(as_pair));
            *p = *((as_pair *)v);
            return 1;   
        }
        default: {
            lua_pushnil(l);
            return 1;
        }
    }
    return 0;
}



mod_lua_box * mod_lua_newbox(lua_State * l, mod_lua_scope scope, void * value, const char * type) {
    mod_lua_box * box = (mod_lua_box *) lua_newuserdata(l, sizeof(mod_lua_box));
    box->scope = scope;
    box->value = value;
    return box;
}

mod_lua_box * mod_lua_pushbox(lua_State * l, mod_lua_scope scope, void * value, const char * type) {
    mod_lua_box * box = (mod_lua_box *) mod_lua_newbox(l, scope, value, type);
    luaL_getmetatable(l, type);
    lua_setmetatable(l, -2);
    return box;
}

mod_lua_box * mod_lua_tobox(lua_State * l, int index, const char * type) {
    mod_lua_box * box = (mod_lua_box *) lua_touserdata(l, index);
    if (box == NULL && type != NULL ) luaL_typerror(l, index, type);
    return box;
}

mod_lua_box * mod_lua_checkbox(lua_State * l, int index, const char * type) {
    luaL_checktype(l, index, LUA_TUSERDATA);
    mod_lua_box * box = (mod_lua_box *) luaL_checkudata(l, index, type);
    if (box == NULL) luaL_typerror(l, index, type);
    return box;
}

int mod_lua_freebox(lua_State * l, int index, const char * type) {
    mod_lua_box * box = mod_lua_checkbox(l, index, type);
    if ( box != NULL && box->scope == MOD_LUA_SCOPE_LUA && box->value != NULL ) {
        as_val_free(box->value);
        box->value = NULL;
    }
    return 0;
}

void * mod_lua_box_value(mod_lua_box * box) {
    return box ? box->value : NULL;
}

