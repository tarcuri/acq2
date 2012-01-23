/*
	snd_alsa.c

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to:

		Free Software Foundation, Inc.
		59 Temple Place - Suite 330
		Boston, MA  02111-1307, USA

	$Id: snd_alsa.c,v 1.5 2005/01/02 03:29:11 bburns Exp $
*/

#include <alsa/asoundlib.h>
#include <dlfcn.h>
#include "../client/client.h"
#include "../client/snd_loc.h"

// Catch the sizeof functions...
#define snd_pcm_hw_params_sizeof alsa_snd_pcm_hw_params_sizeof

//static qboolean  snd_inited = false; //checked in snd_linux.c

static snd_pcm_t *playback_handle;

static int sample_bytes;
static int buffer_bytes;

extern cvar_t *sndbits;
extern cvar_t *sndspeed;
extern cvar_t *sndchannels;
extern cvar_t *snddevice;

static void	*alsa_handle;

// Define all dynamic ALSA functions...
#define ALSA_FUNC(ret, func, params) \
static ret (*alsa_##func) params;
#include "snd_alsa_funcs.h"
#undef ALSA_FUNC

static void ALSA_FreeLibrary(void)
{
	if (alsa_handle) {
        dlclose (alsa_handle);
        alsa_handle = NULL;
	}

#define ALSA_FUNC(ret, func, params) \
alsa_##func = NULL;
#include "snd_alsa_funcs.h"
#undef ALSA_FUNC
}

static qboolean ALSA_LoadLibrary(void)
{
	if(!(alsa_handle = dlopen("libasound.so.2", RTLD_GLOBAL | RTLD_NOW)) )
	{
		Com_Printf("Could not open 'libasound.so.2'\n");
		return false;
	}

#define ALSA_FUNC(ret, func, params) \
    if (!(alsa_##func = dlsym (alsa_handle, #func))) \
    { \
        Com_Printf("Couldn't load ALSA function %s\n", #func); \
        ALSA_FreeLibrary(); \
        return false; \
    }
#include "snd_alsa_funcs.h"
#undef ALSA_FUNC

	return true;
}

qboolean SNDDMA_Init_ALSA (void)
{
	int err;
	unsigned int rate, rrate = 0;
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_uframes_t   period_size = 1024, buffer_size = 4096;
	char *sdevice;
 
	if(!ALSA_LoadLibrary())
		return false;

	if(strcmp(snddevice->string, "/dev/dsp"))
		sdevice = snddevice->string;
	else
		sdevice = "default";

	err = alsa_snd_pcm_open(&playback_handle, sdevice, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
	if (err < 0) {
		Com_Printf("ALSA: cannot open device %s (%s)\n", sdevice, alsa_snd_strerror(err));
		if(strcmp(sdevice, "default"))
			Com_Printf("Try to set snddevice to \"default\"\n");

		ALSA_FreeLibrary();
		return false;
	}

    // Allocate memory for configuration of ALSA...
	err = alsa_snd_pcm_hw_params_malloc(&hw_params);
	if(err < 0){
		Com_Printf("ALSA: cannot allocate hw params(%s)\n", alsa_snd_strerror(err));
		alsa_snd_pcm_close(playback_handle);
		ALSA_FreeLibrary();
		return false;
	}

	err = alsa_snd_pcm_hw_params_any (playback_handle, hw_params);
	if (err < 0) {
		Com_Printf("ALSA: cannot init hw params (%s)\n", alsa_snd_strerror(err));
		alsa_snd_pcm_hw_params_free(hw_params);
		alsa_snd_pcm_close(playback_handle);
		ALSA_FreeLibrary();
        return false;
	}

	err = alsa_snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		Com_Printf("ALSA: cannot set access (%s)\n", alsa_snd_strerror(err));
		alsa_snd_pcm_hw_params_free(hw_params);
		alsa_snd_pcm_close(playback_handle);
		ALSA_FreeLibrary();
        return false;
	}

	dma.samplebits = sndbits->integer;
	if (dma.samplebits != 8)
	{
		err = alsa_snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_S16);
		if (err < 0) {
			Com_Printf("ALSA: 16 bit sound not supported, trying 8\n");
			dma.samplebits = 8;
		}
		else
			dma.samplebits = 16;
	}

	if (dma.samplebits == 8) {
		err = alsa_snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_U8);
		if (err < 0) {
			Com_Printf("ALSA: cannot set sample format (%s)\n", alsa_snd_strerror(err));
			alsa_snd_pcm_hw_params_free(hw_params);
			alsa_snd_pcm_close(playback_handle);
			ALSA_FreeLibrary();
			return false;
		}
	}

	switch(sndspeed->integer) {
	case 11025:
	case 22050:
	case 44100:
	case 48000:
		rate = sndspeed->integer;
		break;
	default:
		Com_Printf("ALSA: rate %i not supported, trying 44100.\n", sndspeed->integer);
	case 0:
		rate = 44100;
		break;
	}

	rrate = rate;

	err = alsa_snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &rrate, 0);
	if (err < 0) {
		Com_Printf("ALSA: unable to set rate near %d Hz (%s)\n", rate, alsa_snd_strerror(err));
		alsa_snd_pcm_hw_params_free(hw_params);
		alsa_snd_pcm_close(playback_handle);
		ALSA_FreeLibrary();
        return false;
	}
	if (rrate != rate)
		Com_Printf("ALSA: rate %dHz is not available, using %dHz\n", rate, rrate);

	dma.speed = rrate;

	dma.channels = sndchannels->integer;
	if (dma.channels < 1 || dma.channels > 2)
		dma.channels = 2;

	err = alsa_snd_pcm_hw_params_set_channels(playback_handle,hw_params, dma.channels);
	if (err < 0) {
		Com_Printf("ALSA: couldn't set channels to %d (%s).\n", dma.channels, alsa_snd_strerror(err));
		alsa_snd_pcm_hw_params_free(hw_params);
		alsa_snd_pcm_close(playback_handle);
		ALSA_FreeLibrary();
        return false;
	}

	period_size = 1024;
	//period_size = 8 * dma.samplebits * dma.speed / 11025;
    err = alsa_snd_pcm_hw_params_set_period_size_near(playback_handle, hw_params, &period_size, 0);
    if(err < 0)
    {
        Com_Printf("ALSA: unable to set period size near %i (%s)\n", (int)period_size, alsa_snd_strerror(err));
		alsa_snd_pcm_hw_params_free(hw_params);
		alsa_snd_pcm_close(playback_handle);
		ALSA_FreeLibrary();
        return false;
    }

	buffer_size = 2048;
	//buffer_size = period_size * 4;
    err = alsa_snd_pcm_hw_params_set_buffer_size_near(playback_handle, hw_params, &buffer_size);
    if(err < 0)
    {
		Com_Printf("ALSA: unable to set buffer size near %i (%s)\n", (int)buffer_size, alsa_snd_strerror(err));
		alsa_snd_pcm_hw_params_free(hw_params);
		alsa_snd_pcm_close(playback_handle);
		ALSA_FreeLibrary();
        return false;
    }

	err = alsa_snd_pcm_hw_params(playback_handle, hw_params);
	if (err < 0) {
		Com_Printf("ALSA: couldn't set params (%s).\n", alsa_snd_strerror(err));
		alsa_snd_pcm_hw_params_free(hw_params);
		alsa_snd_pcm_close(playback_handle);
		ALSA_FreeLibrary();
        return false;
	}
	alsa_snd_pcm_hw_params_free(hw_params);

	dma.samples = buffer_size * dma.channels;
	dma.submission_chunk = period_size * dma.channels;

	sample_bytes = dma.samplebits / 8;
	buffer_bytes = dma.samples * sample_bytes;
	dma.buffer = malloc(buffer_bytes);
	memset(dma.buffer, 0, buffer_bytes);

	dma.samplepos = 0;

	alsa_snd_pcm_prepare(playback_handle);

	Com_Printf("ALSA: period size %d, buffer size %d\n", (int)period_size, (int)buffer_size);
    Com_Printf("%5d stereo\n", dma.channels - 1);
    Com_Printf("%5d samples\n", dma.samples);
    Com_Printf("%5d samplepos\n", dma.samplepos);
    Com_Printf("%5d samplebits\n", dma.samplebits);
    Com_Printf("%5d submission_chunk\n", dma.submission_chunk);
    Com_Printf("%5d speed\n", dma.speed);

	return true;
}

int SNDDMA_GetDMAPos_ALSA (void)
{
	return dma.samplepos;
}

void SNDDMA_Shutdown_ALSA (void)
{
	alsa_snd_pcm_close(playback_handle);
	ALSA_FreeLibrary();
	if(dma.buffer)
		free(dma.buffer);

	dma.buffer = NULL;
}

/*
  SNDDMA_Submit
Send sound to device if buffer isn't really the dma buffer
*/
void SNDDMA_Submit_ALSA (void)
{
	int s, w;
	snd_pcm_uframes_t nframes;
	void *start;
		
	s = dma.samplepos * sample_bytes;
	start = (void *) &dma.buffer[s];
	
	nframes = dma.submission_chunk / dma.channels;
	
	if((w = alsa_snd_pcm_writei(playback_handle, start, nframes)) < 0){  //write to card
		alsa_snd_pcm_prepare(playback_handle);  //xrun occured
		return;
	}
	
	dma.samplepos += w * dma.channels;  //mark progress
	
	if(dma.samplepos >= dma.samples)
		dma.samplepos = 0;  //wrap buffer
}


void SNDDMA_BeginPainting_ALSA (void)
{    
}

