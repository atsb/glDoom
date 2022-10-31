/*
 ddp.h
 Doom Data Pak header file

 All integer data stored in the DDP files is stored
 as in little-endian (Intel) format.

 This is the header of the DDP (Doom Data Pak)file
 It contains an idcode which is used to determine
 if the file is the main DDP file or a patch DDP file.
 The version number is a 4 digit version which must
 be all numeric and zero filled. Version 1.0 would be
 0100 and version 3.52 would be 0352.

 Regardless of which operating system is used, sub-directories
 are indicated by the use of the '/' (slash) character rather
 than the '\' (slant) character.  DOS utilities may, however
 have a problem with the '/' character so attention should be
 given to that when doing DOS portable DDP code.

 Pathnames can be any length up to 255. A sub-directory name is
*/

#include "doomtypes.h"

// D_DIRMAX is the maximum length of a directory name
#define D_DIRMAX 256

typedef struct
   {
    char     id[4];
    char     version[4];
    dword    offset;
    dword    entries;
   }dirdata_t;

typedef struct
   {
    dword    offset;
    dword    size;
    char     dirname[D_DIRMAX];
   }dirent_t;

/*
*/