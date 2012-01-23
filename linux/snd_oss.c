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
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/wait.h>
#if defined(__FreeBSD__)
#include <sys/soundcard.h>
#else
#include <linux/soundcard.h>
#endif
#include <stdio.h>

#include "../client/client.h"
#include "../client/snd_loc.h"

extern qboolean use_custom_memset;
int audio_fd;
static qboolean snd_inited = false;

extern cvar_t *sndbits;
extern cvar_t *sndspeed;
extern cvar_t *sndchannels;
extern cvar_t *snddevice;

static int tryrates[] = { 44100, 22051, 11025, 8000 };
static const int numRates = (sizeof(tryrates)/sizeof(tryrates[0]));

qboolean SNDDMA_Init_OSS (void)
{

	int rc;
    int fmt;
	int tmp;
    int i;
 	struct audio_buf_info info;
	int caps;
	extern uid_t saved_euid;

	if (snd_inited)
		return true;

// open /dev/dsp, confirm capability to mmap, and get size of dma buffer

	seteuid(saved_euid);
	audio_fd = open(snddevice->string, O_RDWR);
	seteuid(getuid());

	if (audio_fd < 0)
	{
		perror(snddevice->string);
		seteuid(getuid());
		Com_Printf("SNDDMA_Init: Could not open '%s' device.\n", snddevice->string);
		if(strcmp(snddevice->string, "/dev/dsp"))
			Com_Printf("Try to set snddevice to \"/dev/dsp\"\n");

		return false;
	}

    rc = ioctl(audio_fd, SNDCTL_DSP_RESET, 0);
    if (rc < 0)
	{
		perror(snddevice->string);
		Com_Printf("SNDDMA_Init: Could not reset %s.\n", snddevice->string);
		close(audio_fd);
		return false;
	}

	if (ioctl(audio_fd, SNDCTL_DSP_GETCAPS, &caps) == -1)
	{
		perror(snddevice->string);
		Com_Printf("SNDDMA_Init: Sound driver too old.\n");
		close(audio_fd);
		return false;
	}

	if (!(caps & DSP_CAP_TRIGGER) || !(caps & DSP_CAP_MMAP))
	{
		Com_Printf("SNDDMA_Init: Sorry, but your soundcard doesn't support trigger or mmap. (%08x)\n", caps);
		close(audio_fd);
		return false;
	}

    if (ioctl(audio_fd, SNDCTL_DSP_GETOSPACE, &info)==-1)
    {   
        perror("GETOSPACE");
		Com_Printf("SNDDMA_Init: GETOSPACE ioctl failed.\n");
		close(audio_fd);
		return false;
    }
    
// set sample bits & speed

    dma.samplebits = sndbits->integer;
	if (dma.samplebits != 16 && dma.samplebits != 8)
    {
        ioctl(audio_fd, SNDCTL_DSP_GETFMTS, &fmt);
        if (fmt & AFMT_S16_LE)
			dma.samplebits = 16;
        else if (fmt & AFMT_U8)
			dma.samplebits = 8;
		else
			dma.samplebits = 16;
    }

	if (dma.samplebits == 16)
	{
        rc = AFMT_S16_LE;
		rc = ioctl(audio_fd, SNDCTL_DSP_SETFMT, &rc);
		if (rc < 0)
		{
			perror(snddevice->string);
			Com_Printf("SNDDMA_Init: Could not support 16-bit data.  Try 8-bit.\n");
			close(audio_fd);
			return false;
		}
	}
	else if (dma.samplebits == 8)
    {
		rc = AFMT_U8;
		rc = ioctl(audio_fd, SNDCTL_DSP_SETFMT, &rc);
		if (rc < 0)
		{
			perror(snddevice->string);
			Com_Printf("SNDDMA_Init: Could not support 8-bit data.\n");
			close(audio_fd);
			return false;
		}
	}
	else
	{
		perror(snddevice->string);
		Com_Printf("SNDDMA_Init: %d-bit sound not supported.", dma.samplebits);
		close(audio_fd);
		return false;
	}

	dma.speed = sndspeed->integer;
	if (!dma.speed)
	{
		for (i = 0; i < numRates; i++) {
            if (!ioctl(audio_fd, SNDCTL_DSP_SPEED, &tryrates[i])) {
				dma.speed = tryrates[i];
				break;
			}
		}
		if (!dma.speed) {
			perror(snddevice->string);
			Com_Printf("SNDDMA_Init: Could not set %s speed.", snddevice->string);
			close(audio_fd);
			return false;
		}
    }
	else
	{
		rc = ioctl(audio_fd, SNDCTL_DSP_SPEED, &dma.speed);
		if (rc < 0)
		{
			perror(snddevice->string);
			Com_Printf("SNDDMA_Init: Could not set %s speed to %d.", snddevice->string, dma.speed);
			close(audio_fd);
			return false;
		}
	}

	dma.channels = sndchannels->integer;
	if (dma.channels < 1 || dma.channels > 2)
		dma.channels = 2;
	
	tmp = 0;
	if (dma.channels == 2)
		tmp = 1;
	rc = ioctl(audio_fd, SNDCTL_DSP_STEREO, &tmp); //FP: bugs here.
    if (rc < 0)
    {
		perror(snddevice->string);
		Com_Printf("SNDDMA_Init: Could not set %s to stereo=%d.", snddevice->string, dma.channels);
		close(audio_fd);
		return false;
    }

	if (tmp)
		dma.channels = 2;
	else
		dma.channels = 1;


	dma.samples = info.fragstotal * info.fragsize / (dma.samplebits/8);
	dma.submission_chunk = 1;

// memory map the dma buffer

	dma.buffer = (unsigned char *) mmap(NULL, info.fragstotal * info.fragsize, PROT_WRITE|PROT_READ, MAP_FILE|MAP_SHARED, audio_fd, 0);
	if (!dma.buffer || dma.buffer == MAP_FAILED)
	{
		Com_Printf("Could not mmap dma buffer PROT_WRITE|PROT_READ\n");
		Com_Printf("trying mmap PROT_WRITE\n");

		dma.buffer = (unsigned char *) mmap(NULL, info.fragstotal * info.fragsize, PROT_WRITE, MAP_FILE|MAP_SHARED, audio_fd, 0);
		if (!dma.buffer || dma.buffer == MAP_FAILED)
		{
			perror(snddevice->string);
			Com_Printf("SNDDMA_Init: Could not mmap %s.\n", snddevice->string);
			close(audio_fd);
			return false;
		}
		use_custom_memset = true;
	}

// toggle the trigger & start her up

    tmp = 0;
    rc  = ioctl(audio_fd, SNDCTL_DSP_SETTRIGGER, &tmp);
	if (rc < 0)
	{
		perror(snddevice->string);
		Com_Printf("SNDDMA_Init: Could not toggle. (1)\n");
		munmap(dma.buffer, dma.samples * (dma.samplebits/8));
		close(audio_fd);
		return false;
	}
    tmp = PCM_ENABLE_OUTPUT;
    rc = ioctl(audio_fd, SNDCTL_DSP_SETTRIGGER, &tmp);
	if (rc < 0)
	{
		perror(snddevice->string);
		Com_Printf("SNDDMA_Init: Could not toggle. (2)\n");
		munmap(dma.buffer, dma.samples * (dma.samplebits/8));
		close(audio_fd);
		return false;
	}

	dma.samplepos = 0;

	snd_inited = true;
	return true;
}

int SNDDMA_GetDMAPos_OSS (void)
{
	struct count_info count;

	if (!snd_inited)
		return 0;

	if (ioctl(audio_fd, SNDCTL_DSP_GETOPTR, &count)==-1)
	{
		perror(snddevice->string);
		Com_Printf("SNDDMA_GetDMAPos: GETOPTR failed.\n");
		if (dma.buffer) {	
			munmap(dma.buffer, dma.samples * (dma.samplebits/8));
		}
		close(audio_fd);
		snd_inited = false;
		return 0;
	}
	dma.samplepos = count.ptr / (dma.samplebits / 8);

	return dma.samplepos;
}

void SNDDMA_Shutdown_OSS (void)
{
//#if 0
	if (snd_inited)
	{
		if (dma.buffer) {	
			munmap(dma.buffer, dma.samples * (dma.samplebits/8));
		}
		close(audio_fd);
		snd_inited = false;
	}
//#endif
}

/*
==============
SNDDMA_Submit

Send sound to device if buffer isn't really the dma buffer
===============
*/
void SNDDMA_Submit_OSS (void)
{
}

void SNDDMA_BeginPainting_OSS (void)
{
}

