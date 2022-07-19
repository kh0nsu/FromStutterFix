#include <Windows.h>
#include <string>
#include <algorithm>

struct Patch {
    DWORD64 relAddr;
    DWORD size;
    char patch[50];
    char orig[50];
};

enum GAME {
    DS1,
    DS3,
    SEKIRO,
    ELDENRING,
    UNKNOWN
};

GAME Game;

typedef DWORD64(__stdcall *DIRECTINPUT8CREATE)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
DIRECTINPUT8CREATE fpDirectInput8Create;
//TODO: fix type mismatch between DWORD64 and HRESULT?
extern "C" __declspec(dllexport)  HRESULT __stdcall DirectInput8Create(
    HINSTANCE hinst,
    DWORD dwVersion,
    REFIID riidltf,
    LPVOID * ppvOut,
    LPUNKNOWN punkOuter
)
{
    return fpDirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}
struct MatchPathSeparator
{
    bool operator()(char ch) const
    {
        return ch == '\\' || ch == '/';
    }
};
std::string
basename(std::string const& pathname)
{
    return std::string(
        std::find_if(pathname.rbegin(), pathname.rend(),
            MatchPathSeparator()).base(),
        pathname.end());
}

GAME DetermineGame() {
    const int fnLenMax = 200;
    char fnPtr[fnLenMax];
    auto fnLen = GetModuleFileNameA(0, fnPtr, fnLenMax);

    auto fileName = basename(std::string(fnPtr, fnLen));
    std::transform(fileName.begin(), fileName.end(), fileName.begin(), ::tolower);
    if (fileName == "darksouls.exe") {
        return GAME::DS1;
    }
    else if (fileName == "darksoulsiii.exe") {
        return GAME::DS3;
    }
    else if (fileName == "sekiro.exe") {
        return GAME::SEKIRO;
    }
    else if (fileName == "eldenring.exe" || fileName == "start_protected_game.exe") {
        return GAME::ELDENRING;
    }
    else {
        return GAME::UNKNOWN;
    }
}

int applyPatches(Patch * patches, int patchCount)
{
    auto baseAddr = GetModuleHandle(NULL);
    int matches = 0;
    for (auto i = 0; i < patchCount; i++) {
        auto patch = patches[i];
        auto addr = (void*)((DWORD64)baseAddr + patch.relAddr);
        auto size = patch.size;

        if (memcmp(addr, patch.orig, size) == 0) {
            DWORD old;
            VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &old);
            memcpy(addr, patch.patch, size);
            VirtualProtect(addr, size, old, &old);
            matches++; //some games require multiple patches, so continue
        }
    }
    return matches;
}

bool ApplyDS1Patches() { //PTDE?
    Patch patches[] = {
        //Latest
        Patch{ 0x8320B0, 7, { static_cast<char>(0xE9) , static_cast<char>(0x27) , static_cast<char>(0x01) , static_cast<char>(0x00) , static_cast<char>(0x00) , static_cast<char>(0x90) , static_cast<char>(0x90) },{ static_cast<char>(0xFF) , static_cast<char>(0x24) , static_cast<char>(0x85) , static_cast<char>(0x24) , static_cast<char>(0x22) , static_cast<char>(0xC3) , static_cast<char>(0x00) } },
        Patch{ 0x8322B3, 2, { static_cast<char>(0x90) , static_cast<char>(0x90) }, { static_cast<char>(0x74) , static_cast<char>(0x0D) } },
        //Debug build
        Patch{ 0x831B30, 7, { static_cast<char>(0xE9) , static_cast<char>(0x27) , static_cast<char>(0x01) , static_cast<char>(0x00) , static_cast<char>(0x00) , static_cast<char>(0x90) , static_cast<char>(0x90) },{ static_cast<char>(0xFF) , static_cast<char>(0x24) , static_cast<char>(0x85) , static_cast<char>(0xA4) , static_cast<char>(0x1C) , static_cast<char>(0xC3) , static_cast<char>(0x00) } },
        Patch{ 0x831D33, 2, { static_cast<char>(0x90) , static_cast<char>(0x90) },{ static_cast<char>(0x74) , static_cast<char>(0x0D) } }
    };
    int patchCount = (sizeof(patches) / sizeof(patches[0]));
    return applyPatches(patches, patchCount) == 2;
}

bool ApplyDS3Patches() {
    Patch patches[] = {
        //v1.8
        Patch{ 0x0BD6ACF, 20, { static_cast<char>(0x48), static_cast<char>(0x31), static_cast<char>(0xC0), static_cast<char>(0x48), static_cast<char>(0x89), static_cast<char>(0x02), static_cast<char>(0x49), static_cast<char>(0x89), static_cast<char>(0x04), static_cast<char>(0x24), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90) },{ static_cast<char>(0xE8), static_cast<char>(0xAC), static_cast<char>(0xC6), static_cast<char>(0xFB), static_cast<char>(0xFF), static_cast<char>(0x90), static_cast<char>(0x4D), static_cast<char>(0x8B), static_cast<char>(0xC7), static_cast<char>(0x49), static_cast<char>(0x8B), static_cast<char>(0xD4), static_cast<char>(0x48), static_cast<char>(0x8B), static_cast<char>(0xC8), static_cast<char>(0xE8), static_cast<char>(0x9D), static_cast<char>(0xC6), static_cast<char>(0xFB), static_cast<char>(0xFF) } },
        //v1.4
        Patch{ 0x0BBB0CF, 20, { static_cast<char>(0x48), static_cast<char>(0x31), static_cast<char>(0xC0), static_cast<char>(0x48), static_cast<char>(0x89), static_cast<char>(0x02), static_cast<char>(0x49), static_cast<char>(0x89), static_cast<char>(0x04), static_cast<char>(0x24), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90) },{ static_cast<char>(0xE8), static_cast<char>(0x8C), static_cast<char>(0x07), static_cast<char>(0xFC), static_cast<char>(0xFF), static_cast<char>(0x90), static_cast<char>(0x4D), static_cast<char>(0x8B), static_cast<char>(0xC7), static_cast<char>(0x49), static_cast<char>(0x8B), static_cast<char>(0xD4), static_cast<char>(0x48), static_cast<char>(0x8B), static_cast<char>(0xC8), static_cast<char>(0xE8), static_cast<char>(0x7D), static_cast<char>(0x07), static_cast<char>(0xFC), static_cast<char>(0xFF) } },
        //v1.12
        Patch{ 0x0BE7D9F, 20, { static_cast<char>(0x48), static_cast<char>(0x31), static_cast<char>(0xC0), static_cast<char>(0x48), static_cast<char>(0x89), static_cast<char>(0x02), static_cast<char>(0x49), static_cast<char>(0x89), static_cast<char>(0x04), static_cast<char>(0x24), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90) },{ static_cast<char>(0xE8), static_cast<char>(0x0C), static_cast<char>(0xBA), static_cast<char>(0xFB), static_cast<char>(0xFF), static_cast<char>(0x90), static_cast<char>(0x4D), static_cast<char>(0x8B), static_cast<char>(0xC7), static_cast<char>(0x49), static_cast<char>(0x8B), static_cast<char>(0xD4), static_cast<char>(0x48), static_cast<char>(0x8B), static_cast<char>(0xC8), static_cast<char>(0xE8), static_cast<char>(0xFD), static_cast<char>(0xB9), static_cast<char>(0xFB), static_cast<char>(0xFF) } },
        //v1.11
        Patch{ 0x0BE6F8F, 20, { static_cast<char>(0x48), static_cast<char>(0x31), static_cast<char>(0xC0), static_cast<char>(0x48), static_cast<char>(0x89), static_cast<char>(0x02), static_cast<char>(0x49), static_cast<char>(0x89), static_cast<char>(0x04), static_cast<char>(0x24), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90) },{ static_cast<char>(0xE8), static_cast<char>(0x9C), static_cast<char>(0xBD), static_cast<char>(0xFB), static_cast<char>(0xFF), static_cast<char>(0x90), static_cast<char>(0x4D), static_cast<char>(0x8B), static_cast<char>(0xC7), static_cast<char>(0x49), static_cast<char>(0x8B), static_cast<char>(0xD4), static_cast<char>(0x48), static_cast<char>(0x8B), static_cast<char>(0xC8), static_cast<char>(0xE8), static_cast<char>(0x8D), static_cast<char>(0xBD), static_cast<char>(0xFB), static_cast<char>(0xFF) } },
        //v1.10
        Patch{ 0x0BD70FF, 20, { static_cast<char>(0x48), static_cast<char>(0x31), static_cast<char>(0xC0), static_cast<char>(0x48), static_cast<char>(0x89), static_cast<char>(0x02), static_cast<char>(0x49), static_cast<char>(0x89), static_cast<char>(0x04), static_cast<char>(0x24), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90) },{ static_cast<char>(0xE8), static_cast<char>(0xAC), static_cast<char>(0xC6), static_cast<char>(0xFB), static_cast<char>(0xFF), static_cast<char>(0x90), static_cast<char>(0x4D), static_cast<char>(0x8B), static_cast<char>(0xC7), static_cast<char>(0x49), static_cast<char>(0x8B), static_cast<char>(0xD4), static_cast<char>(0x48), static_cast<char>(0x8B), static_cast<char>(0xC8), static_cast<char>(0xE8), static_cast<char>(0x9D), static_cast<char>(0xC6), static_cast<char>(0xFB), static_cast<char>(0xFF) } },
        //v1.9
        Patch{ 0x0BD708F, 20, { static_cast<char>(0x48), static_cast<char>(0x31), static_cast<char>(0xC0), static_cast<char>(0x48), static_cast<char>(0x89), static_cast<char>(0x02), static_cast<char>(0x49), static_cast<char>(0x89), static_cast<char>(0x04), static_cast<char>(0x24), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90) },{ static_cast<char>(0xE8), static_cast<char>(0xAC), static_cast<char>(0xC6), static_cast<char>(0xFB), static_cast<char>(0xFF), static_cast<char>(0x90), static_cast<char>(0x4D), static_cast<char>(0x8B), static_cast<char>(0xC7), static_cast<char>(0x49), static_cast<char>(0x8B), static_cast<char>(0xD4), static_cast<char>(0x48), static_cast<char>(0x8B), static_cast<char>(0xC8), static_cast<char>(0xE8), static_cast<char>(0x9D), static_cast<char>(0xC6), static_cast<char>(0xFB), static_cast<char>(0xFF) } },
        //1.13
        Patch{ 0x0BE993F, 20, { static_cast<char>(0x48), static_cast<char>(0x31), static_cast<char>(0xC0), static_cast<char>(0x48), static_cast<char>(0x89), static_cast<char>(0x02), static_cast<char>(0x49), static_cast<char>(0x89), static_cast<char>(0x04), static_cast<char>(0x24), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90) },{ static_cast<char>(0xE8), static_cast<char>(0x1C), static_cast<char>(0xBA), static_cast<char>(0xFB), static_cast<char>(0xFF), static_cast<char>(0x90), static_cast<char>(0x4D), static_cast<char>(0x8B), static_cast<char>(0xC7), static_cast<char>(0x49), static_cast<char>(0x8B), static_cast<char>(0xD4), static_cast<char>(0x48), static_cast<char>(0x8B), static_cast<char>(0xC8), static_cast<char>(0xE8), static_cast<char>(0x0D), static_cast<char>(0xBA), static_cast<char>(0xFB), static_cast<char>(0xFF) } },
        //1.14
        Patch{ 0x0BE9C0F, 20, { static_cast<char>(0x48), static_cast<char>(0x31), static_cast<char>(0xC0), static_cast<char>(0x48), static_cast<char>(0x89), static_cast<char>(0x02), static_cast<char>(0x49), static_cast<char>(0x89), static_cast<char>(0x04), static_cast<char>(0x24), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90) },{ static_cast<char>(0xE8), static_cast<char>(0x1C), static_cast<char>(0xBA), static_cast<char>(0xFB), static_cast<char>(0xFF), static_cast<char>(0x90), static_cast<char>(0x4D), static_cast<char>(0x8B), static_cast<char>(0xC7), static_cast<char>(0x49), static_cast<char>(0x8B), static_cast<char>(0xD4), static_cast<char>(0x48), static_cast<char>(0x8B), static_cast<char>(0xC8), static_cast<char>(0xE8), static_cast<char>(0x0D), static_cast<char>(0xBA), static_cast<char>(0xFB), static_cast<char>(0xFF) } },
        //1.15
        Patch{ 0x0BE9D0F, 20, { static_cast<char>(0x48), static_cast<char>(0x31), static_cast<char>(0xC0), static_cast<char>(0x48), static_cast<char>(0x89), static_cast<char>(0x02), static_cast<char>(0x49), static_cast<char>(0x89), static_cast<char>(0x04), static_cast<char>(0x24), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90) },{ static_cast<char>(0xE8), static_cast<char>(0x1C), static_cast<char>(0xBA), static_cast<char>(0xFB), static_cast<char>(0xFF), static_cast<char>(0x90), static_cast<char>(0x4D), static_cast<char>(0x8B), static_cast<char>(0xC7), static_cast<char>(0x49), static_cast<char>(0x8B), static_cast<char>(0xD4), static_cast<char>(0x48), static_cast<char>(0x8B), static_cast<char>(0xC8), static_cast<char>(0xE8), static_cast<char>(0x0D), static_cast<char>(0xBA), static_cast<char>(0xFB), static_cast<char>(0xFF) } },
    };
    int patchCount = (sizeof(patches) / sizeof(patches[0]));
    return applyPatches(patches, patchCount) == 1;
}

bool ApplySekiroPatches() {
    //seems there's another method to skip the logo in sekiro
    //see https://gist.github.com/fifty-six/ceba4fdb4fe962c732b753c90748ff6b
    //requires identifying the actual patch.

    //generic search AOBs:
    //1st: 488D57304885FF480F44D6488D4D7FE8????????904C8BC3488D5567488BC8E8????????
    //2nd: 488BD7488D4C2420E8????????904C8BC3488D942498000000488BC8E8????????
    Patch patches[] = {
        //1.02
        Patch{ 0x0A78738, 36, { static_cast<char>(0x48), static_cast<char>(0x31), static_cast<char>(0xd2), static_cast<char>(0x48), static_cast<char>(0x89), static_cast<char>(0x55), static_cast<char>(0x67), static_cast<char>(0x48), static_cast<char>(0x89), static_cast<char>(0x55), static_cast<char>(0x7F), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90) },
                              { static_cast<char>(0x48), static_cast<char>(0x8D), static_cast<char>(0x57), static_cast<char>(0x30), static_cast<char>(0x48), static_cast<char>(0x85), static_cast<char>(0xFF), static_cast<char>(0x48), static_cast<char>(0x0F), static_cast<char>(0x44), static_cast<char>(0xD6), static_cast<char>(0x48), static_cast<char>(0x8D), static_cast<char>(0x4D), static_cast<char>(0x7F), static_cast<char>(0xE8), static_cast<char>(0x34), static_cast<char>(0x3A), static_cast<char>(0x37), static_cast<char>(0x00), static_cast<char>(0x90), static_cast<char>(0x4C), static_cast<char>(0x8B), static_cast<char>(0xC3), static_cast<char>(0x48), static_cast<char>(0x8D), static_cast<char>(0x55), static_cast<char>(0x67), static_cast<char>(0x48), static_cast<char>(0x8B), static_cast<char>(0xC8), static_cast<char>(0xE8), static_cast<char>(0x04), static_cast<char>(0xD5), static_cast<char>(0x33), static_cast<char>(0x00) } },
        Patch{ 0x0A7B675, 33, { static_cast<char>(0x48), static_cast<char>(0x31), static_cast<char>(0xd2), static_cast<char>(0x48), static_cast<char>(0x89), static_cast<char>(0x94), static_cast<char>(0x24), static_cast<char>(0x98), static_cast<char>(0x00), static_cast<char>(0x00), static_cast<char>(0x00), static_cast<char>(0x48), static_cast<char>(0x89), static_cast<char>(0x54), static_cast<char>(0x24), static_cast<char>(0x20), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90) },
                              { static_cast<char>(0x48), static_cast<char>(0x8B), static_cast<char>(0xD7), static_cast<char>(0x48), static_cast<char>(0x8D), static_cast<char>(0x4C), static_cast<char>(0x24), static_cast<char>(0x20), static_cast<char>(0xE8), static_cast<char>(0xFE), static_cast<char>(0x0A), static_cast<char>(0x37), static_cast<char>(0x00), static_cast<char>(0x90), static_cast<char>(0x4C), static_cast<char>(0x8B), static_cast<char>(0xC3), static_cast<char>(0x48), static_cast<char>(0x8D), static_cast<char>(0x94), static_cast<char>(0x24), static_cast<char>(0x98), static_cast<char>(0x00), static_cast<char>(0x00), static_cast<char>(0x00), static_cast<char>(0x48), static_cast<char>(0x8B), static_cast<char>(0xC8), static_cast<char>(0xE8), static_cast<char>(0xCA), static_cast<char>(0xA5), static_cast<char>(0x33), static_cast<char>(0x00) } },
        //1.04 not supported currently
        //1.06
        Patch{ 0xA8AB60, 36, { static_cast<char>(0x48), static_cast<char>(0x31), static_cast<char>(0xd2), static_cast<char>(0x48), static_cast<char>(0x89), static_cast<char>(0x55), static_cast<char>(0x67), static_cast<char>(0x48), static_cast<char>(0x89), static_cast<char>(0x55), static_cast<char>(0x7F), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90) },
                             { static_cast<char>(0x48), static_cast<char>(0x8D), static_cast<char>(0x57), static_cast<char>(0x30), static_cast<char>(0x48), static_cast<char>(0x85), static_cast<char>(0xFF), static_cast<char>(0x48), static_cast<char>(0x0F), static_cast<char>(0x44), static_cast<char>(0xD6), static_cast<char>(0x48), static_cast<char>(0x8D), static_cast<char>(0x4D), static_cast<char>(0x7F), static_cast<char>(0xE8), static_cast<char>(0x2c), static_cast<char>(0x0d), static_cast<char>(0x39), static_cast<char>(0x00), static_cast<char>(0x90), static_cast<char>(0x4C), static_cast<char>(0x8B), static_cast<char>(0xC3), static_cast<char>(0x48), static_cast<char>(0x8D), static_cast<char>(0x55), static_cast<char>(0x67), static_cast<char>(0x48), static_cast<char>(0x8B), static_cast<char>(0xC8), static_cast<char>(0xE8), static_cast<char>(0xEC), static_cast<char>(0x24), static_cast<char>(0x35), static_cast<char>(0x00) } },
        Patch{ 0xA8DC75, 33, { static_cast<char>(0x48), static_cast<char>(0x31), static_cast<char>(0xd2), static_cast<char>(0x48), static_cast<char>(0x89), static_cast<char>(0x94), static_cast<char>(0x24), static_cast<char>(0x98), static_cast<char>(0x00), static_cast<char>(0x00), static_cast<char>(0x00), static_cast<char>(0x48), static_cast<char>(0x89), static_cast<char>(0x54), static_cast<char>(0x24), static_cast<char>(0x20), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90), static_cast<char>(0x90) },
                             { static_cast<char>(0x48), static_cast<char>(0x8B), static_cast<char>(0xD7), static_cast<char>(0x48), static_cast<char>(0x8D), static_cast<char>(0x4C), static_cast<char>(0x24), static_cast<char>(0x20), static_cast<char>(0xE8), static_cast<char>(0x1e), static_cast<char>(0xdc), static_cast<char>(0x38), static_cast<char>(0x00), static_cast<char>(0x90), static_cast<char>(0x4C), static_cast<char>(0x8B), static_cast<char>(0xC3), static_cast<char>(0x48), static_cast<char>(0x8D), static_cast<char>(0x94), static_cast<char>(0x24), static_cast<char>(0x98), static_cast<char>(0x00), static_cast<char>(0x00), static_cast<char>(0x00), static_cast<char>(0x48), static_cast<char>(0x8B), static_cast<char>(0xC8), static_cast<char>(0xE8), static_cast<char>(0xDA), static_cast<char>(0xF3), static_cast<char>(0x34), static_cast<char>(0x00) } },

    };
    int patchCount = (sizeof(patches) / sizeof(patches[0]));
    return applyPatches(patches, patchCount) == 2;
}

bool ApplyEldenRingPatches() {
    //these patches are so small that there's a high chance to mis-identify the game version as another, but we'll know since the patch count will be wrong.
    Patch patches[] = {
        //1.02.3
        Patch{ 0xAAAF1A, 2, { static_cast<char>(0x90), static_cast<char>(0x90) },
                            { static_cast<char>(0x74), static_cast<char>(0x53) } },
        //1.03.0
        Patch{ 0xAB021D, 2, { static_cast<char>(0x90), static_cast<char>(0x90) },
                            { static_cast<char>(0x74), static_cast<char>(0x53) } },
        //1.03.1
        Patch{ 0xAB022D, 2, { static_cast<char>(0x90), static_cast<char>(0x90) },
                            { static_cast<char>(0x74), static_cast<char>(0x53) } },
        // 1.03.2
        Patch{ 0xAB020D, 2, { static_cast<char>(0x90), static_cast<char>(0x90) },
                            { static_cast<char>(0x74), static_cast<char>(0x53) } },
        // 1.04
        Patch{ 0xA8FB6D, 2, { static_cast<char>(0x90), static_cast<char>(0x90) },
                            { static_cast<char>(0x74), static_cast<char>(0x53) } },
        // 1.04.1
        Patch{ 0xA8FA7D, 2, { static_cast<char>(0x90), static_cast<char>(0x90) },
                            { static_cast<char>(0x74), static_cast<char>(0x53) } },
        // 1.05.0
        Patch{ 0xA9417D, 2, { static_cast<char>(0x90), static_cast<char>(0x90) },
                            { static_cast<char>(0x74), static_cast<char>(0x53) } }
    };
    int patchCount = (sizeof(patches) / sizeof(patches[0]));
    return applyPatches(patches, patchCount) == 1;
}

void SetupD8Proxy() {
    char syspath[320];
    GetSystemDirectoryA(syspath, 320);
    strcat_s(syspath, "\\dinput8.dll");
    auto hMod = LoadLibraryA(syspath);
    fpDirectInput8Create = (DIRECTINPUT8CREATE)GetProcAddress(hMod, "DirectInput8Create");
}

DWORD WINAPI doPatching(LPVOID lpParam)
{
    bool good = false;
    int i;
    for(i=0;i<60;i++)
    {
        if (Game == GAME::DS3) {
            good = ApplyDS3Patches();
        }
        else if (Game == GAME::SEKIRO) {
            good = ApplySekiroPatches();
        }
        else if (Game == GAME::DS1) {
            good = ApplyDS1Patches();
        }
        else if (Game == GAME::ELDENRING) {
            good = ApplyEldenRingPatches();
        }
        if (good) { break; }
        Sleep(500); //wait up to 30 seconds total (slow PCs?). this really only applies to sekiro where steam needs to deobfuscate the .exe. other games should work right away.
    }
    if (!good) {
        MessageBoxA(0, "Patching failed. You may be running an unsupported version.", "", 0);
        return 1;
    }
#if _DEBUG
    else
    {
        char buf[64];
        sprintf(buf, "Patch success, i %d", i);
        MessageBoxA(0, buf, "", 0);
    }
#endif

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    HANDLE res = 0;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        Game = DetermineGame();
        if (Game == GAME::UNKNOWN) {
            MessageBoxA(0, "Unable to determine game. Valid EXEs are darksouls.exe, darksoulsiii.exe, sekiro.exe, elden_ring.exe and start_protected_game.exe", "", 0);
            break;
        }

        SetupD8Proxy();
        
        res = CreateThread(NULL, 0, doPatching, NULL, 0, NULL);
        if (res == NULL)
        {
            MessageBoxA(0, "Could not start patching thread.", "", 0);
        }

        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
