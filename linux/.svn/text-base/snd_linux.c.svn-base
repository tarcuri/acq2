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
#include "../client/client.h"
#include "../client/snd_loc.h"

#define SND_NONE	0
#define SND_ALSA	1
#define SND_OSS		2

qboolean SNDDMA_Init_ALSA(void);
qboolean SNDDMA_Init_OSS(void);
int SNDDMA_GetDMAPos_ALSA(void);
int SNDDMA_GetDMAPos_OSS(void);
void SNDDMA_Shutdown_ALSA(void);
void SNDDMA_Shutdown_OSS(void);
void SNDDMA_Submit_ALSA(void);

static int snd_inited = SND_NONE;

cvar_t *sndalsa;
cvar_t *sndbits;
cvar_t *sndspeed;
cvar_t *sndchannels;
cvar_t *snddevice;

qboolean use_custom_memset = false;

void Snd_Memset (void* dest, const int val, const size_t count)
{
	int *pDest;
	int i, iterate;

	if (!use_custom_memset)
	{
		memset(dest,val,count);
		return;
	}
	iterate = count / sizeof(int);
	pDest = (int*)dest;
	for(i=0; i<iterate; i++)
	{
		pDest[i] = val;
	}
}

qboolean SNDDMA_Init(void)
{
	qboolean retval = false;

	if (snd_inited)
		return true;

	if (!snddevice)
	{
		sndbits = Cvar_Get("sndbits", "16", CVAR_ARCHIVE|CVAR_LATCHED);
		sndspeed = Cvar_Get("sndspeed", "0", CVAR_ARCHIVE|CVAR_LATCHED);
		sndchannels = Cvar_Get("sndchannels", "2", CVAR_ARCHIVE|CVAR_LATCHED);
		sndalsa = Cvar_Get("sndalsa", "1", CVAR_ARCHIVE|CVAR_LATCHED);
		snddevice = Cvar_Get("snddevice", "/dev/dsp", CVAR_ARCHIVE|CVAR_LATCHED);
	}

	if(sndalsa->integer)
	{
		Com_Printf("Attempting to initialise ALSA sound.\n");
		retval = SNDDMA_Init_ALSA();
		if(retval)
		{
			snd_inited = SND_ALSA;
		}
		else
		{
			Com_Printf("Falling back to OSS sound.\n");
			retval = SNDDMA_Init_OSS();
			if(retval)
				snd_inited = SND_OSS;
		}
	}
	else
	{
		Com_Printf("Attempting to initialize OSS sound.\n");
		retval = SNDDMA_Init_OSS();
		if(retval)
			snd_inited = SND_OSS;
	}

	return retval;
}

int SNDDMA_GetDMAPos(void)
{
	if (snd_inited == SND_ALSA)
		return SNDDMA_GetDMAPos_ALSA();
	else if(snd_inited == SND_OSS)
		return SNDDMA_GetDMAPos_OSS();
	else
		return 0;
}

void SNDDMA_Shutdown(void)
{
	if (snd_inited == SND_ALSA)
		SNDDMA_Shutdown_ALSA();
	else if (snd_inited == SND_OSS)
		SNDDMA_Shutdown_OSS();

	snd_inited = SND_NONE;
	use_custom_memset = false;
}

/*
==============
SNDDMA_Submit

Send sound to device if buffer isn't really the dma buffer
===============
*/
void SNDDMA_Submit(void)
{
	if (snd_inited == SND_ALSA)
		SNDDMA_Submit_ALSA();
#if 0 //oss doesnt use this
	else if(snd_inited == SND_OSS)
		SNDDMA_Submit_OSS();
#endif
}

void SNDDMA_BeginPainting (void)
{
#if 0 //oss or alsa doesnt use this
	if (snd_inited == SND_ALSA)
		SNDDMA_BeginPainting_ALSA();
	else if(snd_inited == SND_OSS)
		SNDDMA_BeginPainting_OSS();
#endif
}

