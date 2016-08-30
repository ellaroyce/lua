#include <iostream>
#include <string>

extern "C" {
#include <Python.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
}

<<<<<<< HEAD
static PyObject * lua_eval(PyObject* module, PyObject* args)
=======
using namespace std;

void writeVariable(PyObject* dict, lua_State* L); 
PyObject* compute(lua_State* L);
void parseVariables(PyObject* dict, lua_State* L);
static PyObject* lua_eval(PyObject *self, PyObject *args, PyObject *keywords);

#define RETURN_WITH_EXCEPTION(error) do {\
set_exception(error);\
lua_close(L);\
return NULL;\
} while (0)

void set_exception(const char *error) 
{
    PyErr_SetString(PyExc_ValueError, error);
}

string getProgram(PyObject * expr)
{
    string expression(PyUnicode_AsUTF8(expr));
    if (expression.find('\n') == string::npos) 
    {
        expression = "return " + expression;
    }
    return expression;
}

void writeVariable(PyObject* dict, lua_State* L)
{
    if (dict == Py_None) 
    {
        return;
    }

    PyObject *key, *value;
    Py_ssize_t pos = 0;
    while (PyDict_Next(dict, &pos, &key, &value)) 
    {
     if (!PyUnicode_Check(key)) 
     {
         continue; 
     }
     const char* c_key = PyUnicode_AsUTF8(key);

     lua_getglobal(L, c_key);

     PyObject *value;
     if (lua_isboolean(L, -1)) {
        value = PyBool_FromLong(lua_toboolean(L, -1));
    } else if (lua_isnumber(L, -1)) {
        value = PyFloat_FromDouble(lua_tonumber(L, -1));
    } else if (lua_isstring(L, -1)) {
        value = PyUnicode_FromString(lua_tostring(L, -1));
    } else {
        lua_pop(L, 1);
        continue;
    }

    PyDict_SetItem(dict, key, value);
    Py_DECREF(value);
    lua_pop(L, 1);
}
}

void parseVariables(PyObject* dict, lua_State* L) 
>>>>>>> origin/master
{
    PyObject *key, *value;
    Py_ssize_t pos = 0;
    while (PyDict_Next(dict, &pos, &key, &value)) 
    {
        if (!PyUnicode_Check(key)) 
        { 
            continue; 
        }
        const char* c_key = PyUnicode_AsUTF8(key);

        if (PyUnicode_Check(value)) 
        {
            const char* c_value = PyUnicode_AsUTF8(value);
            lua_pushstring(L, c_value);
        } else if (PyLong_Check(value)) 
        {
            long long c_value = PyLong_AsDouble(value);
            lua_pushnumber(L, c_value);
        } else if (PyFloat_Check(value)) 
        {
            double c_value = PyFloat_AsDouble(value);
            lua_pushnumber(L, c_value);
        } else if (PyBool_Check(value)) 
        {
            bool c_value;
            c_value = (value == Py_True);
            lua_pushboolean(L, c_value);
        } else if(value == Py_None) 
        {
            lua_pushnil(L);
        } else 
        {
            continue;
        }

        lua_setglobal(L, c_key);
    }
}



static PyObject * lua_eval(PyObject *self, PyObject *args, PyObject *keywords) 
{
    PyObject *globals = Py_None, *locals = Py_None;
    const char *expr = NULL;
    static char *arglist[] = {"expression", "globals", "locals", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, keywords, "s|OO", arglist, &expr, &globals, &locals))
    {
        return NULL; 
    }
    
    string expression = getProgram(PyTuple_GetItem(args, 0));

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    if (globals == Py_None) 
    {
        globals = PyEval_GetGlobals();
    }

    parseVariables(globals, L);
    parseVariables(locals, L);
    luaL_loadbuffer(L, expression.c_str(), expression.length(), NULL);
    if (lua_pcall(L, 0, 1, 0)) 
    {
        const char* error_message = lua_tostring(L, -1);
        PyObject* exception = PyExc_BaseException;
        PyErr_SetString(exception, error_message);
        return exception;
    }
    writeVariable(globals, L);
    writeVariable(locals, L);

    PyObject* result = compute(L);
    lua_close(L);
    if (result == Py_None) 
    {
        Py_INCREF(Py_None);
    }

    return result;
}

PyObject* compute(lua_State* L) 
{
    PyObject* result;

    if (lua_isnil(L, -1)) {
        Py_INCREF(Py_None);
        Py_RETURN_NONE;
    } 
    else if (lua_isboolean(L, -1)) {
        result = PyBool_FromLong(lua_toboolean(L, -1));
    } 
    else if (lua_isnumber(L, -1)) {
        result = PyFloat_FromDouble(lua_tonumber(L, -1));
    }
    else if (lua_isstring(L, -1)) {
        const char* r = lua_tostring(L, -1);
        result = PyUnicode_FromString(r);
    }
    else {
        RETURN_WITH_EXCEPTION("ERROR");
    }

    return result;
}

PyMODINIT_FUNC PyInit_lua(void)
{
    static PyMethodDef ModuleMethods[] = {
        { 
            "eval", 
            (PyCFunction)lua_eval, 
            METH_VARARGS | METH_KEYWORDS, 
            "Eval For Lua" 
        }, { NULL, NULL, 0, NULL }
    };
    static struct PyModuleDef ModuleDef = {
        PyModuleDef_HEAD_INIT,
        "lua",
        "Eval For Lua",
        -1, ModuleMethods
    };
    PyObject * module = PyModule_Create(&ModuleDef);
    return module;
}
