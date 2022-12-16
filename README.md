# ReconstructD2RIat

x64dbg plugin based on OverwatchDumpFix plugin which reconstruct Diablo 2 Resurrected import address table.
Due to new obfuscation techniques OverwatchDumpFix does not reconstruct the IAT of D2R.
This plugin will do it with the help of OverwatchDumpFix plugin so make sure you have it.

## How to use
1. Compile the project with visual studio.
2. Copy the files to x64dbg plugin folder.
3. Open x64dbg as admin and attach D2R process.
4. Use Scylla to find the virtual address and size of the import address table (advanced search).
5. Enter the command ReconstructD2RIAT 0x<IAT Virtual Address>, 0x<IAT Size> (for example: ReconstructD2RIAT 0x00007ff12345678, 0x1170).
6. Use Scylla reselect D2R process and dump the file.
7. Fix the dump.
8. Rebuild the PE.
  
## Credits
changeofpace - The creator of OverwatchFixDump https://github.com/changeofpace/Overwatch-Dump-Fix

