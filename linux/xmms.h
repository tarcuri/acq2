/*

Copyright (C) 2001-2002       A Nourai

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the included (GNU.txt) GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <xmms/xmmsctrl.h>

#define MP3_NOTRUNNING		-1
#define MP3_PLAYING			1
#define MP3_PAUSED			2
#define MP3_STOPPED			3

#define MP3_MAXSONGTITLE	128

#define MP3_PLAYERNAME_ALLCAPS		"XMMS"
#define MP3_PLAYERNAME_LEADINGCAP	"XMMS"
#define MP3_PLAYERNAME_NOCAPS		"xmms"

typedef struct
{
	int *num;
	char **name;

} mp3_tracks_t;

typedef struct
{
	int track;
	int total;
	char name[MP3_MAXSONGTITLE];

} mp3_track_t;

qboolean MP3_IsActive(void);
void MP3_Init(void);
void MP3_Shutdown(void);

void MP3_Next_f(void);
void MP3_FastForward_f(void);
void MP3_Rewind_f(void);
void MP3_Prev_f(void);
void MP3_Play_f(void);
void MP3_Pause_f(void);
void MP3_Stop_f(void);
void MP3_Execute_f(void);
void MP3_ToggleRepeat_f(void);
void MP3_ToggleShuffle_f(void);
void MP3_FadeOut_f(void);

int MP3_GetStatus(void);
qboolean MP3_GetToggleState(int *shuffle, int *repeat);

void MP3_GetPlaylistInfo(int *current, int *length);
int MP3_GetPlaylistSongs(mp3_tracks_t *songList, char *filter);
qboolean MP3_GetTrackTime(int *elapsed, int *total);
qboolean MP3_PlayTrack (int num);
char *MP3_Menu_SongTitle(void);

