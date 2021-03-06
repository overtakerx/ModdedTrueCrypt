/*
 Legal Notice: The source code contained in this file has been derived from
 the source code of Encryption for the Masses 2.02a, which is Copyright (c)
 Paul Le Roux and which is covered by the 'License Agreement for Encryption
 for the Masses'. Modifications and additions to that source code contained
 in this file are Copyright (c) TrueCrypt Foundation and are covered by the
 TrueCrypt License 2.3 the full text of which is contained in the file
 License.txt included in TrueCrypt binary and source code distribution
 packages. */

#include "Common.h"

#ifndef CACHE_SIZE
/* WARNING: Changing this value might not be safe (some items may be hard coded for 4)! Inspection necessary. */
#define CACHE_SIZE 4	
#endif

extern int cacheEmpty;

int VolumeReadHeaderCache (BOOL bCache, char *header, Password *password, PCRYPTO_INFO *retInfo);
void WipeCache (void);
