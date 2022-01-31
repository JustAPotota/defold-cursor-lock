
#include <dmsdk/sdk.h>
#if defined(DM_PLATFORM_HTML5)
#include <emscripten.h>
#endif

#define GLFW_MOUSE_CURSOR 0x00030001
extern "C"
{
    void glfwDisable(int token);
    void glfwEnable(int token);
}

static int Glfw_MouseLock(lua_State* L)
{
    glfwDisable(GLFW_MOUSE_CURSOR);
    return 0;
}

static int Glfw_MouseUnlock(lua_State* L)
{
    glfwEnable(GLFW_MOUSE_CURSOR);
    return 0;
}

#if defined(DM_PLATFORM_HTML5)
typedef void (*DefCursorLock_OnCursorLockChange)(const bool locked);
typedef void (*DefCursorLock_OnCanvasClick)();
extern "C" void DefCursorLock_Init(DefCursorLock_OnCursorLockChange on_change, DefCursorLock_OnCanvasClick on_click);
extern "C" void DefCursorLock_Final();
extern "C" void DefCursorLock_RequestCursorLock();
extern "C" void DefCursorLock_ExitCursorLock();

static dmScript::LuaCallbackInfo *m_OnClick;
static bool m_CursorLocked;

static int Html5_RequestCursorLock(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    DefCursorLock_RequestCursorLock();
    return 0;
}

static int Html5_ExitCursorLock(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    DefCursorLock_ExitCursorLock();
    return 0;
}

static int Html5_CursorLocked(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    lua_pushboolean(L, m_CursorLocked);
    return 1;
}

static void OnCursorLockChange(const bool locked)
{
    m_CursorLocked = locked;
}

static int Html5_OnClick(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    if (lua_isfunction(L, 1)) {
        m_OnClick = dmScript::CreateCallback(L, 1);
    } else {
        dmScript::DestroyCallback(m_OnClick);
        m_OnClick = 0x0;
    }
    return 0;
}

static void OnCanvasClick()
{
    if (!dmScript::IsCallbackValid(m_OnClick)) {
        return;
    }

    lua_State *L = dmScript::GetCallbackLuaContext(m_OnClick);

    if (!dmScript::SetupCallback(m_OnClick)) {
        dmScript::DestroyCallback(m_OnClick);
        m_OnClick = 0x0;
        return;
    }

    dmScript::PCall(L, 1, 0);

    dmScript::TeardownCallback(m_OnClick);
}

#endif

// Functions exposed to Lua
static const luaL_reg Module_methods[] = {
    { "glfw_mouse_lock", Glfw_MouseLock },
    { "glfw_mouse_unlock", Glfw_MouseUnlock },
#if defined(DM_PLATFORM_HTML5)
    { "html5_request_cursor_lock", Html5_RequestCursorLock },
    { "html5_exit_cursor_lock", Html5_ExitCursorLock },
    { "html5_cursor_locked", Html5_CursorLocked },
    { "html5_on_click", Html5_OnClick },
#endif
    /* Sentinel: */
    { NULL, NULL }
};

static void LuaInit(lua_State* L)
{
    int top = lua_gettop(L);

    // Register lua names
    luaL_register(L, "cursor_lock_ext", Module_methods);

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

static dmExtension::Result InitializeExt(dmExtension::Params* params)
{
    LuaInit(params->m_L);

#if defined(DM_PLATFORM_HTML5)
    DefCursorLock_Init(OnCursorLockChange, OnCanvasClick);
#endif

    return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeExt(dmExtension::Params* params)
{
#if defined(DM_PLATFORM_HTML5)
    DefCursorLock_Final();
#endif

    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(cursor_lock, "cursor_lock", 0, 0, InitializeExt, 0, 0, FinalizeExt)
