/*
 Legal Notice: The source code contained in this file has been derived from
 the source code of Encryption for the Masses 2.02a, which is Copyright (c)
 Paul Le Roux and which is covered by the 'License Agreement for Encryption
 for the Masses'. Modifications and additions to that source code contained
 in this file are Copyright (c) TrueCrypt Foundation and are covered by the
 TrueCrypt License 2.3 the full text of which is contained in the file
 License.txt included in TrueCrypt binary and source code distribution
 packages. */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Tcdefs.h"

#include "Crypto.h"
#include "Common/Endian.h"
#include "Format.h"
#include "Fat.h"
#include "Progress.h"
#include "Random.h"

void
GetFatParams (fatparams * ft)
{
	unsigned int fatsecs;
	if(ft->cluster_size == 0)	// 'Default' cluster size
	{
		if (ft->num_sectors * 512LL >= 256*BYTES_PER_GB)
			ft->cluster_size = 128;
		else if (ft->num_sectors * 512LL >= 64*BYTES_PER_GB)
			ft->cluster_size = 64;
		else if (ft->num_sectors * 512LL >= 16*BYTES_PER_GB)
			ft->cluster_size = 32;
		else if (ft->num_sectors * 512LL >= 8*BYTES_PER_GB)
			ft->cluster_size = 16;
		else if (ft->num_sectors * 512LL >= 128*BYTES_PER_MB)
			ft->cluster_size = 8;
		else if (ft->num_sectors * 512LL >= 64*BYTES_PER_MB)
			ft->cluster_size = 4;
		else if (ft->num_sectors * 512LL >= 32*BYTES_PER_MB)
			ft->cluster_size = 2;
		else
			ft->cluster_size = 1;
	}

	// Geometry always set to SECTORS/1/1
	ft->secs_track = 1; 
	ft->heads = 1; 

	ft->dir_entries = 512;
	ft->fats = 2;
	ft->create_time = (unsigned int) time (NULL);
	ft->media = 0xf8;
	ft->sector_size = SECTOR_SIZE;
	ft->hidden = 0;

	ft->size_root_dir = ft->dir_entries * 32;

	// FAT12
	ft->size_fat = 12;
	ft->reserved = 2;
	fatsecs = ft->num_sectors - (ft->size_root_dir + SECTOR_SIZE - 1) / SECTOR_SIZE - ft->reserved;
	ft->cluster_count = (int) (((__int64) fatsecs * SECTOR_SIZE) / (ft->cluster_size * SECTOR_SIZE + 3));
	ft->fat_length = (((ft->cluster_count * 3 + 1) >> 1) + SECTOR_SIZE - 1) / SECTOR_SIZE;

	if (ft->cluster_count >= 4085) // FAT16
	{
		ft->size_fat = 16;
		ft->reserved = 2;
		fatsecs = ft->num_sectors - (ft->size_root_dir + SECTOR_SIZE - 1) / SECTOR_SIZE - ft->reserved;
		ft->cluster_count = (int) (((__int64) fatsecs * SECTOR_SIZE) / (ft->cluster_size * SECTOR_SIZE + 4));
		ft->fat_length = (ft->cluster_count * 2 + SECTOR_SIZE - 1) / SECTOR_SIZE;
	}
	
	if(ft->cluster_count >= 65525) // FAT32
	{
		ft->size_fat = 32;
		ft->reserved = 32;
		fatsecs = ft->num_sectors - ft->reserved;
		ft->size_root_dir = ft->cluster_size * SECTOR_SIZE;
		ft->cluster_count = (int) (((__int64) fatsecs * SECTOR_SIZE) / (ft->cluster_size * SECTOR_SIZE + 8));
		ft->fat_length = (ft->cluster_count * 4 + SECTOR_SIZE - 1) / SECTOR_SIZE;
	}

	if (ft->num_sectors >= 65536 || ft->size_fat == 32)
	{
		ft->sectors = 0;
		ft->total_sect = ft->num_sectors;
	}
	else
	{
		ft->sectors = ft->num_sectors;
		ft->total_sect = 0;
	}
}

void
PutBoot (fatparams * ft, unsigned char *boot)
{
	int cnt = 0;

	boot[cnt++] = 0xeb;	/* boot jump */
	boot[cnt++] = 0x3c;
	boot[cnt++] = 0x90;
	memcpy (boot + cnt, "MSDOS5.0", 8); /* system id */
	cnt += 8;
	*(__int16 *)(boot + cnt) = LE16(ft->sector_size);	/* bytes per sector */
	cnt += 2;
	boot[cnt++] = (__int8) ft->cluster_size;			/* sectors per cluster */
	*(__int16 *)(boot + cnt) = LE16(ft->reserved);		/* reserved sectors */
	cnt += 2;
	boot[cnt++] = (__int8) ft->fats;					/* 2 fats */

	if(ft->size_fat == 32)
	{
		boot[cnt++] = 0x00;
		boot[cnt++] = 0x00;
	}
	else
	{
		*(__int16 *)(boot + cnt) = LE16(ft->dir_entries);	/* 512 root entries */
		cnt += 2;
	}

	*(__int16 *)(boot + cnt) = LE16(ft->sectors);		/* # sectors */
	cnt += 2;
	boot[cnt++] = (__int8) ft->media;					/* media byte */

	if(ft->size_fat == 32)	
	{
		boot[cnt++] = 0x00;
		boot[cnt++] = 0x00;
	}
	else 
	{ 
		*(__int16 *)(boot + cnt) = LE16(ft->fat_length);	/* fat size */
		cnt += 2;
	}

	*(__int16 *)(boot + cnt) = LE16(ft->secs_track);	/* # sectors per track */
	cnt += 2;
	*(__int16 *)(boot + cnt) = LE16(ft->heads);			/* # heads */
	cnt += 2;
	*(__int32 *)(boot + cnt) = LE32(ft->hidden);		/* # hidden sectors */
	cnt += 4;
	*(__int32 *)(boot + cnt) = LE32(ft->total_sect);	/* # huge sectors */
	cnt += 4;

	if(ft->size_fat == 32)
	{
		*(__int32 *)(boot + cnt) = LE32(ft->fat_length); cnt += 4;	/* fat size 32 */
		boot[cnt++] = 0x00;	/* ExtFlags */
		boot[cnt++] = 0x00;
		boot[cnt++] = 0x00;	/* FSVer */
		boot[cnt++] = 0x00;
		boot[cnt++] = 0x02;	/* RootClus */
		boot[cnt++] = 0x00;
		boot[cnt++] = 0x00;
		boot[cnt++] = 0x00;
		boot[cnt++] = 0x01;	/* FSInfo */
		boot[cnt++] = 0x00;
		boot[cnt++] = 0x06;	/* BkBootSec */
		boot[cnt++] = 0x00;
		memset(boot+cnt, 0, 12); cnt+=12;	/* Reserved */
	}

	boot[cnt++] = 0x00;	/* drive number */   // FIXED 80 > 00
	boot[cnt++] = 0x00;	/* reserved */
	boot[cnt++] = 0x29;	/* boot sig */
	*(__int32 *)(boot + cnt) = LE32(ft->create_time);	/* vol id */
	cnt += 4;
	memcpy (boot + cnt, ft->volume_name, 11);	/* vol title */
	cnt += 11;

	switch(ft->size_fat) /* filesystem type */
	{
		case 12: memcpy (boot + cnt, "FAT12   ", 8); break;
		case 16: memcpy (boot + cnt, "FAT16   ", 8); break;
		case 32: memcpy (boot + cnt, "FAT32   ", 8); break;
	}
	cnt += 8;

	memset (boot + cnt, 0, ft->size_fat==32 ? 420:448);	/* boot code */
	cnt += ft->size_fat==32 ? 420:448;
	boot[cnt++] = 0x55;
	boot[cnt++] = 0xaa;	/* boot sig */
}


/* FAT32 FSInfo */
PutFSInfo (unsigned char *sector)
{
	memset (sector, 0, 512);
	sector[3]=0x41; /* LeadSig */
	sector[2]=0x61; 
	sector[1]=0x52; 
	sector[0]=0x52; 
	sector[484+3]=0x61; /* StrucSig */
	sector[484+2]=0x41; 
	sector[484+1]=0x72; 
	sector[484+0]=0x72; 
	sector[488+3]=0xff; /* Free_Count */
	sector[488+2]=0xff;
	sector[488+1]=0xff;
	sector[488+0]=0xff;
	sector[492+3]=0xff; /* Nxt_Free */
	sector[492+2]=0xff;
	sector[492+1]=0xff;
	sector[492+0]=0xff;
	sector[508+3]=0xaa; /* TrailSig */
	sector[508+2]=0x55;
	sector[508+1]=0x00;
	sector[508+0]=0x00;
}


int
FormatFat (unsigned __int64 startSector, fatparams * ft, void * dev, PCRYPTO_INFO cryptoInfo, BOOL quickFormat)
{
	int write_buf_cnt = 0;
	char sector[SECTOR_SIZE], *write_buf;
	unsigned __int64 nSecNo = startSector;
	int x, n;
	int retVal;
	char temporaryKey[DISKKEY_SIZE];

#ifdef _WIN32
	LARGE_INTEGER startOffset;
	LARGE_INTEGER newOffset;

	// Seek to start sector
	startOffset.QuadPart = startSector * SECTOR_SIZE;
	if (!SetFilePointerEx ((HANDLE) dev, startOffset, &newOffset, FILE_BEGIN)
		|| newOffset.QuadPart != startOffset.QuadPart)
	{
		return ERR_VOL_SEEKING;
	}
#endif

	/* Write the data area */

	write_buf = (char *)TCalloc (WRITE_BUF_SIZE);
	memset (sector, 0, sizeof (sector));

	PutBoot (ft, (unsigned char *) sector);
	if (WriteSector (dev, sector, write_buf, &write_buf_cnt, &nSecNo,
		cryptoInfo) == FALSE)
		goto fail;

	/* fat32 boot area */
	if (ft->size_fat == 32)				
	{
		/* fsinfo */
		PutFSInfo((unsigned char *) sector);
		if (WriteSector (dev, sector, write_buf, &write_buf_cnt, &nSecNo,
			cryptoInfo) == FALSE)
			goto fail;

		/* reserved */
		while (nSecNo - startSector < 6)
		{
			memset (sector, 0, sizeof (sector));
			sector[508+3]=0xaa; /* TrailSig */
			sector[508+2]=0x55;
			if (WriteSector (dev, sector, write_buf, &write_buf_cnt, &nSecNo,
				cryptoInfo) == FALSE)
				goto fail;
		}
		
		/* bootsector backup */
		memset (sector, 0, sizeof (sector));
		PutBoot (ft, (unsigned char *) sector);
		if (WriteSector (dev, sector, write_buf, &write_buf_cnt, &nSecNo,
				 cryptoInfo) == FALSE)
			goto fail;

		PutFSInfo((unsigned char *) sector);
		if (WriteSector (dev, sector, write_buf, &write_buf_cnt, &nSecNo,
			cryptoInfo) == FALSE)
			goto fail;
	}

	/* reserved */
	while (nSecNo - startSector < (unsigned int)ft->reserved)
	{
		memset (sector, 0, sizeof (sector));
		if (WriteSector (dev, sector, write_buf, &write_buf_cnt, &nSecNo,
			cryptoInfo) == FALSE)
			goto fail;
	}

	/* write fat */
	for (x = 1; x <= ft->fats; x++)
	{
		for (n = 0; n < ft->fat_length; n++)
		{
			memset (sector, 0, SECTOR_SIZE);

			if (n == 0)
			{
				unsigned char fat_sig[12];
				if (ft->size_fat == 32)
				{
					fat_sig[0] = (unsigned char) ft->media;
					fat_sig[1] = fat_sig[2] = 0xff;
					fat_sig[3] = 0x0f;
					fat_sig[4] = fat_sig[5] = fat_sig[6] = 0xff;
					fat_sig[7] = 0x0f;
					fat_sig[8] = fat_sig[9] = fat_sig[10] = 0xff;
					fat_sig[11] = 0x0f;
					memcpy (sector, fat_sig, 12);
				}				
				else if (ft->size_fat == 16)
				{
					fat_sig[0] = (unsigned char) ft->media;
					fat_sig[1] = 0xff;
					fat_sig[2] = 0xff;
					fat_sig[3] = 0xff;
					memcpy (sector, fat_sig, 4);
				}
				else if (ft->size_fat == 12)
				{
					fat_sig[0] = (unsigned char) ft->media;
					fat_sig[1] = 0xff;
					fat_sig[2] = 0xff;
					fat_sig[3] = 0x00;
					memcpy (sector, fat_sig, 4);
				}
			}

			if (WriteSector (dev, sector, write_buf, &write_buf_cnt, &nSecNo,
				    cryptoInfo) == FALSE)
				goto fail;
		}
	}


	/* write rootdir */
	for (x = 0; x < ft->size_root_dir / SECTOR_SIZE; x++)
	{
		memset (sector, 0, SECTOR_SIZE);
		if (WriteSector (dev, sector, write_buf, &write_buf_cnt, &nSecNo,
				 cryptoInfo) == FALSE)
			goto fail;

	}

	/* Fill the rest of the data area with random data */

	if(!quickFormat)
	{
		/* Generate a random temporary key set to be used for "dummy" encryption that will fill
		the free disk space (data area) with random data.  This is necessary for plausible
		deniability of hidden volumes (and also reduces the amount of predictable plaintext
		within the volume). */

		// Temporary master key
		if (!RandgetBytes (temporaryKey, EAGetKeySize (cryptoInfo->ea), FALSE))
			goto fail;
		// Secondary key (LRW mode)
		if (!RandgetBytes (cryptoInfo->iv, sizeof cryptoInfo->iv, FALSE))		
			goto fail;

		retVal = EAInit (cryptoInfo->ea, temporaryKey, cryptoInfo->ks);
		if (retVal != 0)
		{
			burn (temporaryKey, sizeof(temporaryKey));
			return retVal;
		}
		if (!EAInitMode (cryptoInfo))
		{
			burn (temporaryKey, sizeof(temporaryKey));
			return ERR_MODE_INIT_FAILED;
		}

		x = ft->num_sectors - ft->reserved - ft->size_root_dir / SECTOR_SIZE - ft->fat_length * 2;
		while (x--)
		{
			/* Generate random plaintext. Note that reused plaintext blocks are not a concern
			here since LRW mode is designed to hide patterns. Furthermore, patterns in plaintext
			do occur commonly on media in the "real world", so it might actually be a fatal
			mistake to try to avoid them completely. */

#if RNG_POOL_SIZE > SECTOR_SIZE
#error RNG_POOL_SIZE > SECTOR_SIZE
#endif

#ifdef _WIN32
			if (!RandpeekBytes (sector, RNG_POOL_SIZE)
				|| !RandpeekBytes (sector + RNG_POOL_SIZE, SECTOR_SIZE - RNG_POOL_SIZE))
				goto fail;
#else
			if ((nSecNo & 0x3fff) == 0)
			{
				if (!RandgetBytes (sector, RNG_POOL_SIZE, FALSE)
					|| !RandgetBytes (sector + RNG_POOL_SIZE, SECTOR_SIZE - RNG_POOL_SIZE, FALSE))
					goto fail;
			}
#endif
			// Encrypt the random plaintext and write it to the disk
			if (WriteSector (dev, sector, write_buf, &write_buf_cnt, &nSecNo,
				cryptoInfo) == FALSE)
				goto fail;
		}
		UpdateProgressBar (nSecNo);
	}
	else
		UpdateProgressBar (ft->num_sectors);

	if (write_buf_cnt != 0
#ifdef _WIN32
		&& _lwrite ((HFILE)dev, write_buf, write_buf_cnt) == HFILE_ERROR)
#else
		&& fwrite (write_buf, 1, write_buf_cnt, (FILE *)dev) != (size_t)write_buf_cnt)
#endif
		goto fail;

	TCfree (write_buf);
	burn (temporaryKey, sizeof(temporaryKey));
	return 0;

fail:

	TCfree (write_buf);
	burn (temporaryKey, sizeof(temporaryKey));
	return ERR_OS_ERROR;
}
