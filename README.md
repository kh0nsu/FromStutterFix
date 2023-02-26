# FromStutterFix (kinda dead sorry)

This tool will apply a fix for a certain type of stutter in FromSoft games. It will also disable achievements in Elden Ring, to work around a bug where achievements can freeze the game when Steam is offline.

Supported patches:
* DS3 1.15.1
* Sekiro 1.06
* Elden Ring 1.06

If you run a newer patch of Elden Ring, using ertool is the only option. ertool will search for the required addresses so it should work on any patch and likely newer patches too.

## Usage

There is a standalone .exe version which doesn't touch the game files, but needs to be run manually each time. There is also a .dll version which loads automatically, but may be more difficult to use with mods. Both do the same thing.

Note: EAC needs to be disabled for Elden Ring.

### EXE Version

Run the game, then run the program. It will say "flag set" if it worked. Close the program. The fix will last until you restart the game.

### DLL Version

Place the DINPUT8.dll in the game directory. Run the game. After a delay, a chime sound will be played if the fix was applied successfully.

To uninstall, delete the DINPUT8.dll.

Note: This version also includes the logo screen skip, to avoid needing to choose between them or chain-load DLLs.

## How does it work?

The game does...something, relating to Steam Input, which scans over all devices (basically everything listed in Device Manager). This happens any time any device changes, like a microphone being plugged in, a bluetooth headset pairing, or a driver update happening in the background. The game stops rendering frames until this is done, and it can take up to a second on some PCs.

The game has a flag which turns this off. This program sets the flag.

## Okay, but how do I know it's working?

Run the game without the fix, and try plugging/unplugging a USB dongle. If windowed, make sure the game window is focused (the stutter won't happen otherwise). You should see a noticeable stutter. Now apply the fix, then repeat the test. It should no longer occur.

## Does it break anything?

I don't know. I haven't noticed any problems. If you actually use Steam Input, it might stop working. Replugging a controller still works for me.

The achievement-disabling freeze fix will prevent you from getting any achievements, but you probably have them all by now anyway.

## This fixed my unplayable stutters that happened every 10 seconds!

Cool, but something is probably still wrong with your PC. Check if anything looks weird in Device Manager, eg. if it's constantly refreshing, or any warning symbols.

## I still get stutters at certain points in the game

Stutters at loading triggers probably can't be fixed, except by From, or a miracle patch to the rendering pipeline. There may be other sources of stutter also.
