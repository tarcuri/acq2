/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#ifndef _GNU_SOURCE
 #define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "../linux/glob.h"

#include "../qcommon/qcommon.h"

#if defined(__FreeBSD__)
#include <machine/param.h>
#endif

//===============================================================================

static byte *membase;
static size_t maxhunksize;
static size_t curhunksize;

#if defined(MACOS_X)
void *Hunk_Begin (int maxsize)
{
	// reserve a huge chunk of memory, but don't commit any yet
	maxhunksize = maxsize;
	curhunksize = 0;
	membase = malloc(maxhunksize);
	if (membase == NULL)
		Sys_Error("Hunk_Begin: unable to virtual allocate %d bytes", maxsize);

	memset( membase, 0, maxhunksize );
	return membase;
}

void *Hunk_Alloc (int size)
{
	byte *buf;

	size = (size+31)&~31; 	// round to cacheline
	if (curhunksize + size > maxhunksize)
		Sys_Error("Hunk_Alloc overflow");
	buf = membase + curhunksize;
	curhunksize += size;
	return buf;
}

int Hunk_End (void)
{
	byte *n;

	n = realloc(membase, curhunksize);
	if (n != membase)
		Sys_Error("Hunk_End:  Could not remap virtual block (%d)", errno);

	return curhunksize;
}
void Hunk_Free (void *base)
{
	if (base)
		free(base);
}
#else //maxosx

#ifdef __FreeBSD__
#define MMAP_ANON MAP_ANON
#else
#define MMAP_ANON MAP_ANONYMOUS
#endif

void *Hunk_Begin (int maxsize)
{
	// reserve a huge chunk of memory, but don't commit any yet
	maxhunksize = maxsize + sizeof(size_t);
	curhunksize = 0;

	membase = mmap(0, maxhunksize, PROT_READ|PROT_WRITE, 
		MAP_PRIVATE|MMAP_ANON, -1, 0);

	if (membase == NULL || membase == (byte *)-1) {
		Sys_Error("Hunk_Begin: unable to virtual allocate %d bytes", maxsize);
	}

	*((size_t *)membase) = curhunksize;

	return membase + sizeof(size_t);
}

void *Hunk_Alloc (int size)
{
	byte *buf;

	// round to cacheline
	size = (size+31)&~31;
	if (curhunksize + size > maxhunksize) {
		Sys_Error("Hunk_Alloc: unable to allocate %d bytes out of %d", size, maxhunksize);
	}
	buf = membase + sizeof(size_t) + curhunksize;
	curhunksize += size;
	return buf;
}

int Hunk_End (void)
{
	byte *n;

#if defined(__FreeBSD__)
	size_t old_size = maxhunksize;
	size_t new_size = curhunksize + sizeof(int);
	void * unmap_base;
	size_t unmap_len;

	new_size = round_page(new_size);
	old_size = round_page(old_size);
	if (new_size > old_size) {
		Sys_Error("Hunk_End: new_size > old_size");
		return;
	}
	else if (new_size < old_size)
	{
		unmap_base = (caddr_t)(membase + new_size);
		unmap_len = old_size - new_size;
		n = munmap(unmap_base, unmap_len) + membase;
	}
#else
	n = (byte *)mremap(membase, maxhunksize, curhunksize + sizeof(size_t), 0);
#endif
	if (n != membase) {
		Sys_Error("Hunk_End: could not remap virtual block: %s", strerror(errno));
	}

	*((size_t *)membase) = curhunksize + sizeof(size_t);
	
	return curhunksize;
}

void Hunk_Free (void *base)
{
	byte *m;

	if (base) {
		m = ((byte *)base) - sizeof(size_t);
		if (munmap(m, *((size_t *)m)))
			Sys_Error("Hunk_Free: munmap failed: %s", strerror(errno));
	}
}
#endif
//===============================================================================


/*
================
Sys_Milliseconds
================
*/
unsigned int curtime;
static unsigned long sys_timeBase = 0;

unsigned int Sys_Milliseconds (void)
{
	struct timeval tp;
	//struct timezone tzp;
	//static unsigned int	secbase = 0;

	gettimeofday(&tp, NULL);
	
	if (!sys_timeBase)
	{
		sys_timeBase = tp.tv_sec;
		return tp.tv_usec/1000;
	}

	curtime = (tp.tv_sec - sys_timeBase)*1000 + tp.tv_usec/1000;
	
	return curtime;
}

#if 0
extern cvar_t *in_subframe;
int Sys_XTimeToSysTime (unsigned long xtime)
{
	int ret, time, test;

	if (!in_subframe->value) // if you don't want to do any event times corrections
		return Sys_Milliseconds();

	// some X servers (like suse 8.1's) report weird event times
	// if the game is loading, resolving DNS, etc. we are also getting old events
	// so we only deal with subframe corrections that look 'normal'
	ret = xtime - (unsigned long)(sys_timeBase*1000);
	time = Sys_Milliseconds();
	test = time - ret;
	//printf("delta: %d\n", test);
	if (test < 0 || test > 30) // in normal conditions I've never seen this go above
		return time;

	return ret;
}
#endif

void Sys_Mkdir (const char *path) {
    mkdir (path, 0777);
}

qboolean Sys_RemoveFile( const char *path ) {
	if( remove( path ) ) {
		return false;
	}
	return true;
}

qboolean Sys_RenameFile( const char *from, const char *to ) {
	if( rename( from, to ) ) {
		return false;
	}
	return true;
}

//============================================

static	char	findbase[MAX_OSPATH];
static	char	findpath[MAX_OSPATH];
static	char	findpattern[MAX_OSPATH];
static	DIR		*fdir;

static qboolean CompareAttributes(const char *path, const char *name,
	unsigned musthave, unsigned canthave )
{
	struct stat st;
	char fn[MAX_OSPATH];

// . and .. never match
	if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
		return false;

	//return true;
	sprintf(fn, "%s/%s", path, name);
	if (stat(fn, &st) == -1)
		return false; // shouldn't happen

	if (st.st_mode & S_IFDIR) {
		if (canthave & SFF_SUBDIR)
			return false;
	}
	else if (musthave & SFF_SUBDIR)
		return false;

	return true;
}

char *Sys_FindFirst (const char *path, unsigned musthave, unsigned canhave)
{
	struct dirent *d;
	char *p;

	if (fdir)
		Sys_Error ("Sys_BeginFind without close");

//	COM_FilePath (path, findbase);
	strcpy(findbase, path);

	if ((p = strrchr(findbase, '/')) != NULL) {
		*p = 0;
		strcpy(findpattern, p + 1);
	} else
		strcpy(findpattern, "*");

	if (strcmp(findpattern, "*.*") == 0)
		strcpy(findpattern, "*");
	
	if ((fdir = opendir(findbase)) == NULL)
		return NULL;
	while ((d = readdir(fdir)) != NULL) {
		if (!*findpattern || glob_match(findpattern, d->d_name)) {
//			if (*findpattern)
//				printf("%s matched %s\n", findpattern, d->d_name);
			if (CompareAttributes(findbase, d->d_name, musthave, canhave)) {
				sprintf (findpath, "%s/%s", findbase, d->d_name);
				return findpath;
			}
		}
	}
	return NULL;
}

char *Sys_FindNext (unsigned musthave, unsigned canhave)
{
	struct dirent *d;

	if (fdir == NULL)
		return NULL;
	while ((d = readdir(fdir)) != NULL) {
		if (!*findpattern || glob_match(findpattern, d->d_name)) {
//			if (*findpattern)
//				printf("%s matched %s\n", findpattern, d->d_name);
			if (CompareAttributes(findbase, d->d_name, musthave, canhave)) {
				sprintf (findpath, "%s/%s", findbase, d->d_name);
				return findpath;
			}
		}
	}
	return NULL;
}

void Sys_FindClose (void)
{
	if (fdir != NULL)
		closedir(fdir);
	fdir = NULL;
}


//============================================

