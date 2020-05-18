# omf2hex
Convert Apple IIgs OMF Executable into Intel hex file, for C256 Foenix

Until I have a loader for the Foenix that can directly load an OMF file.

This Tool will virtually load the OMF into memory (and do all the fix-ups)
So that straight binary data can just by copied where it's supposed to live
in memory.

Output this data into an Intel HEX file that can be used with the C256 
Uploader, for testing purposes.
