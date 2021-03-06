/*
 Copyright (c) TrueCrypt Foundation. All rights reserved.

 Covered by the TrueCrypt License 2.3 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#ifndef KEYFILES_H
#define	KEYFILES_H


#include "Common.h"

#define KEYFILE_POOL_SIZE	64
#define	KEYFILE_MAX_READ_LEN	(1024*1024)

typedef struct KeyFileStruct
{
	char FileName[MAX_PATH];
	struct KeyFileStruct *Next;
} KeyFile;

typedef struct
{
	BOOL EnableKeyFiles;
	KeyFile *FirstKeyFile;
} KeyFilesDlgParam;

KeyFile *KeyFileAdd (KeyFile *firstKeyFile, KeyFile *keyFile);
void KeyFileRemoveAll (KeyFile **firstKeyFile);
KeyFile *KeyFileClone (KeyFile *keyFile);
KeyFile *KeyFileCloneAll (KeyFile *firstKeyFile);
BOOL KeyFilesApply (Password *password, KeyFile *firstKeyFile);

#ifdef _WIN32
BOOL CALLBACK KeyFilesDlgProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
#endif


#endif	/* #ifndef KEYFILES_H */ 
