#include "BugFixes.h"
#include "CommandLine.h"
#include "NamePlates.h"
#include "Misc.h"
#include "Hooks.h"
#include "Inventory.h"
#include "UnitAPI.h"
#include <Windows.h>
#include <Detours/detours.h>

// <--- NUEVO: Inicio del bloque de código para probar SetScale ----
// 1. Definimos la función original usando la dirección que nos dio el creador.
static auto* SetScale_original = (void(*)())0x004040F0;

// 2. Creamos nuestra función "hook". Por ahora, solo hará que el juego crashee.
// ¡Si el juego se cierra al llamar a :SetScale(), significa que el hook FUNCIONÓ!
static void SetScale_hook()
{
    // Forzamos un crash para saber que hemos llegado aquí.
    *(int*)0 = 0;
}
// <--- NUEVO: Fin del bloque de código ----


static int lua_debugbreak(lua_State* L)
{
    if (IsDebuggerPresent())
        DebugBreak();
    return 0;
}

static int lua_openawesomewotlk(lua_State* L)
{
    lua_pushnumber(L, 1.f);
    lua_setglobal(L, "AwesomeWotlk");
#ifdef _DEBUG
    lua_pushcfunction(L, lua_debugbreak);
    lua_setglobal(L, "debugbreak");
#endif
    return 0;
}

static void OnAttach()
{
//#ifdef _DEBUG
//    system("pause");
//    FreeConsole();
//#endif

    // Invalid function pointer hack
    *(DWORD*)0x00D415B8 = 1;
    *(DWORD*)0x00D415BC = 0x7FFFFFFF;

    *(DWORD*)0x00B6AF54 = 1; // TOSAccepted = 1
    *(DWORD*)0x00B6AF5C = 1; // EULAAccepted = 1

    // Initialize modules
    DetourTransactionBegin();

    // <--- NUEVO: Adjuntamos nuestro hook para SetScale ----
    DetourAttach(&(LPVOID&)SetScale_original, SetScale_hook);
    // <--- NUEVO: Fin de la línea a añadir ----

    Hooks::initialize();
    BugFixes::initialize();
    CommandLine::initialize();
    Inventory::initialize();
    NamePlates::initialize();
    Misc::initialize();
    UnitAPI::initialize();
    DetourTransactionCommit();

    // Register base
    Hooks::FrameXML::registerLuaLib(lua_openawesomewotlk);
}

int __stdcall DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
        OnAttach();
    return 1;
}
