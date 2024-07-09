# FromStutterFix

This tool will apply a fix for a certain type of stutter ("device stutter") in FromSoft games. You can trigger this just by plugging in a usb drive.

Supported games:
* DS3
* Sekiro
* Elden Ring

All patches should work.

## Usage

Place dinput8.dll in the game folder. Alternatively, mod loaders for DLL mods can be used.
Note: EAC needs to be disabled for Elden Ring.

Run the game. After a delay, a chime sound will be played if the fix was applied successfully.

## What about the EXE version?

It's obsolete as most people were using the DLL.

## How does it work?

The game gets a message from Windows ("WM_DEVICECHANGE") saying that something changed. The game then will re-scan for any controllers ("EnumDevices"), even if the device is not your controller. The game stops rendering frames until this is done, and it can take up to a second on some PCs. Steam hooks into this process too which probably doesn't help.

This mod will patch the game to not do the scan. As long as the game has seen your controller already, this isn't really a problem, even if you re-plug the controller.

## Okay, but how do I know it's working?

Run the game without the fix, and try plugging/unplugging a USB dongle (any kind). If windowed, make sure the game window is focused (the stutter won't happen otherwise). You should see a noticeable stutter. Now apply the fix, then repeat the test. It should no longer occur.

## This fixed my unplayable stutters that happened every 10 seconds!

Cool, but something is probably still wrong with your PC. Check if anything looks weird in Device Manager, eg. if it's constantly refreshing, or any warning symbols.

## I still get stutters at certain points in the game

Stutters at loading triggers probably can't be fixed, except by From, or a miracle patch to the rendering pipeline. There may be other sources of stutter also.
