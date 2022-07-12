# FromStutterFix

This program will apply a fix for a certain type of stutter in FromSoft games.

## Usage

Run the game, then run the program. It will say "flag set" if it worked. Close the program. The fix will last until you restart the game.

EAC needs to be disabled for Elden Ring.

## How does it work?

The game does...something, relating to Steam Input, which scans over all devices (basically everything listed in Device Manager). This happens any time any device changes, like a microphone being plugged in, a bluetooth headset pairing, or a driver update happening in the background. The game stops rendering frames until this is done, and it can take up to a second on some PCs.

The game has a flag which turns this off. This program sets the flag.

## Okay, but how do I know it's working?

Run the game without the fix, and try plugging/unplugging a USB dongle. If windowed, make sure the game window is focused (the stutter won't happen otherwise). You should see a noticeable stutter. Now apply the fix, then repeat the test. It should no longer occur.

## Does it break anything?

I don't know. I haven't noticed any problems. If you actually use Steam Input, it might stop working. Replugging a controller still works for me.

## This fixed my unplayable stutters that happened every 10 seconds!

Cool, but something is probably still wrong with your PC. Check if anything looks weird in Device Manager, eg. if it's constantly refreshing, or any warning symbols.

## I still get stutters at certain points in the game

Stutters at loading triggers probably can't be fixed, except by From, or a miracle patch to the rendering pipeline. There may be other sources of stutter also.

## Can you make it a DLL that loads automatically?

I don't know how. If someone wants to do that, that would be great.
