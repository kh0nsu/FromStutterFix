#include <Windows.h>
#include <iostream>

volatile bool gameIsAlive = false; //this is hacky but it's non-essential

typedef HRESULT(__stdcall *DIRECTINPUT8CREATE)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN);
DIRECTINPUT8CREATE fpDirectInput8Create;
extern "C" __declspec(dllexport)  HRESULT __stdcall DirectInput8Create(
    HINSTANCE hinst,
    DWORD dwVersion,
    REFIID riidltf,
    LPVOID * ppvOut,
    LPUNKNOWN punkOuter
)
{
    gameIsAlive = true;
    return fpDirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}

void SetupD8Proxy() {
    char syspath[320];
    GetSystemDirectoryA(syspath, 320);
    strcat_s(syspath, "\\dinput8.dll");
    auto hMod = LoadLibraryA(syspath);
    if (0 == hMod)
    {
        MessageBoxA(0, "Could not setup DINPUT8 proxy.", "", 0);
        return;
    }
    fpDirectInput8Create = (DIRECTINPUT8CREATE)GetProcAddress(hMod, "DirectInput8Create");
}

BYTE aob[] = { 0x48, 0x88, 0x88, 0x88, 0x80, 0xB9, 0x88, 0x88, 0x00, 0x00, 0x00, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x00, 0x00, 0x88, 0x8B, 0x88, 0x88, 0x44, 0x24, 0x20, 0x01 };
BYTE mask[] = { 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00 };
int patternlength = 37;
BYTE patch[] = { 0x48, 0x85, 0xC9, 0x90, 0x90, 0x90, 0x90 };
int patchOffset = 4;
int patchLength = 7;

DWORD WINAPI doPatching(LPVOID lpParam)
{
    //we need to wait for the game to fully load/deobfuscate and scan controllers
    //if loaded as dinput8, we'll know this because the create hook was hit
    //otherwise we'll just wait up to 15 seconds
    //TODO: consider checking if the game window has opened instead. this will happen after loading but before the scan
    //alternatively, we could hook EnumDevices itself, but steam also hooks this and it seems more likely to crash the game
    for (int i = 0; i < 15; i++)
    {
        if (gameIsAlive) { break; }
        Sleep(1000);
    }
    Sleep(1500); //wait a bit longer as the scan may not have run immediately, though it's safe to patch if the scan is already underway.

    bool success = false;

    auto hModule = GetModuleHandle(NULL);
    if (!hModule) { return 1; }
    auto pDosHeader = (PIMAGE_DOS_HEADER)hModule;
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) { return 1; }
    auto pNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + pDosHeader->e_lfanew);
    if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE) { return 1; }
    auto pSectionHeader = IMAGE_FIRST_SECTION(pNtHeaders);

    //int patchCount = 0;
    for (auto i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++, pSectionHeader++)
    {
        if (strncmp((char*)pSectionHeader->Name, ".text", IMAGE_SIZEOF_SHORT_NAME) == 0)
        {
            BYTE* sectionStart = (BYTE*)hModule + pSectionHeader->VirtualAddress;
            int sectionSize = (int)pSectionHeader->Misc.VirtualSize;

            for (int j = 0; j < sectionSize - patternlength + 1; ++j)
            {
                bool found = true;
                for (int k = 0; k < patternlength; ++k)
                {
                    if (!mask[k] && sectionStart[j + k] != aob[k])
                    {
                        found = false;
                        break;
                    }
                }
                if (found)
                {
                    auto patchAddr = sectionStart + j + patchOffset;

                    DWORD oldFlags;
                    VirtualProtect(patchAddr, patchLength, PAGE_EXECUTE_READWRITE, &oldFlags);
                    memcpy(patchAddr, patch, patchLength);
                    VirtualProtect(patchAddr, patchLength, oldFlags, &oldFlags);

                    success = true;
                    
                    //patchCount++;
                    //if (patchCount >= 2) { break; } //AC6 can be patched in two places, but this is untested
                    break; //for now just break on first match
                }
            }
        }
    }

    if (!success)
    {
        MessageBoxA(0, "Stutter fix failed. You may be running an unsupported version.", "", 0);
        return 1;
    }

    PlaySound(TEXT("SystemStart"), NULL, SND_SYNC);
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
