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

#ifdef WITH_XMMS

#include "../client/client.h"
#include <dlfcn.h>
#include <sys/wait.h>
#include <assert.h>
#include <unistd.h>
#include <glib.h>
#include <xmms/xmmsctrl.h>

#define XMMS_SESSION	(xmms_session->integer)

static cvar_t *xmms_dir;
static cvar_t *xmms_session;
static cvar_t *xmms_messages;

#define Sys_MSleep(x) usleep((x) * 1000)

static char *mp3_notrunning_msg = MP3_PLAYERNAME_LEADINGCAP " is not running";

// Define all dynamic XMMS functions...
#define XMMS_FUNC(ret, func, params) \
static ret (*q##func) params;
#include "xmms_funcs.h"
#undef XMMS_FUNC


#define QLIB_FREELIBRARY(lib) (dlclose(lib), lib = NULL)

static void *libxmms_handle = NULL;

static void XMMS_LoadLibrary(void)
{
	if( !(libxmms_handle = dlopen("libxmms.so", RTLD_NOW)) )
	{
		if( !(libxmms_handle = dlopen("libxmms.so.1", RTLD_NOW)) )
		{
			Com_Printf("Could not open 'libxmms.so' or 'libxmms.so.1'\n");
			return;
		}
	}

#define XMMS_FUNC(ret, func, params) \
    if (!(q##func = dlsym(libxmms_handle, #func))) \
    { \
        Com_Printf("Couldn't load XMMS function %s\n", #func); \
        QLIB_FREELIBRARY(libxmms_handle); \
        return; \
    }
#include "xmms_funcs.h"
#undef XMMS_FUNC
}

static void XMMS_FreeLibrary(void) {
	if (libxmms_handle) {
		QLIB_FREELIBRARY(libxmms_handle);
	}
}

qboolean MP3_IsActive(void) {
	return !!libxmms_handle;
}


static qboolean MP3_IsPlayerRunning(void) {
	return (qxmms_remote_is_running((gint) XMMS_SESSION));
}

static int XMMS_pid = 0;
void MP3_Execute_f(void) {
	char path[MAX_OSPATH], *argv[2] = {"xmms", NULL};
	int i, length;

	if (MP3_IsPlayerRunning()) {
		Com_Printf("XMMS is already running\n");
		return;
	}
	Q_strncpyz(path, xmms_dir->string, sizeof(path) - strlen("/xmms"));
	length = strlen(path);
	for (i = 0; i < length; i++) {
		if (path[i] == '\\')
			path[i] = '/';
	}
	if (length && path[length - 1] == '/')
		path[length - 1] = 0;
	strcat(path, "/xmms");

	if (!(XMMS_pid = fork())) {
		execv(path, argv);
		exit(-1);
	}
	if (XMMS_pid == -1) {
		Com_Printf ("Couldn't execute XMMS\n");
		return;
	}
	for (i = 0; i < 6; i++) {
		Sys_MSleep(50);
		if (MP3_IsPlayerRunning()) {
			Com_Printf("XMMS is now running\n");
			return;
		}
	}
	Com_Printf("XMMS (probably) failed to run\n");
}

#define XMMS_COMMAND(Name, Param)						\
    void MP3_##Name##_f(void) {							\
	   if (MP3_IsPlayerRunning()) {						\
		   qxmms_remote_##Param(XMMS_SESSION);			\
	   } else {											\
		   Com_Printf("%s\n", mp3_notrunning_msg);		\
	   }												\
	   return;											\
    }

XMMS_COMMAND(Prev, playlist_prev);
XMMS_COMMAND(Play, play);
XMMS_COMMAND(Pause, pause);
XMMS_COMMAND(Stop, stop);
XMMS_COMMAND(Next, playlist_next);
XMMS_COMMAND(FadeOut, stop);

void MP3_FastForward_f(void) {
	int current;

	if (!MP3_IsPlayerRunning()) {
		Com_Printf("%s\n", mp3_notrunning_msg);
		return;
	}
	current = qxmms_remote_get_output_time(XMMS_SESSION) + 5 * 1000;
	qxmms_remote_jump_to_time(XMMS_SESSION, current);
}

void MP3_Rewind_f(void) {
	int current;

	if (!MP3_IsPlayerRunning()) {
		Com_Printf("%s\n", mp3_notrunning_msg);
		return;
	}
	current = qxmms_remote_get_output_time(XMMS_SESSION) - 5 * 1000;
	current = max(0, current);
	qxmms_remote_jump_to_time(XMMS_SESSION, current);
}

int MP3_GetStatus(void) {
	if (!MP3_IsPlayerRunning())
		return MP3_NOTRUNNING;
	if (qxmms_remote_is_paused(XMMS_SESSION))
		return MP3_PAUSED;
	if (qxmms_remote_is_playing(XMMS_SESSION))
		return MP3_PLAYING;	
	return MP3_STOPPED;
}

static void XMMS_Set_ToggleFn(char *name, void *togglefunc, void *getfunc) {
	int ret, set;
	gboolean (*xmms_togglefunc)(gint) = togglefunc;
	gboolean (*xmms_getfunc)(gint) = getfunc;

	if (!MP3_IsPlayerRunning()) {
		Com_Printf("%s\n", mp3_notrunning_msg);	
		return;
	}
	if (Cmd_Argc() >= 3) {
		Com_Printf("Usage: %s [on|off|toggle]\n", Cmd_Argv(0));
		return;
	}
	ret = xmms_getfunc(XMMS_SESSION);
	if (Cmd_Argc() == 1) {
		Com_Printf("%s is %s\n", name, (ret == 1) ? "on" : "off");
		return;
	}
	if (!Q_stricmp(Cmd_Argv(1), "on")) {
		set = 1;
	} else if (!Q_stricmp(Cmd_Argv(1), "off")) {
		set = 0;
	} else if (!Q_stricmp(Cmd_Argv(1), "toggle")) {
		set = ret ? 0 : 1;
	} else {
		Com_Printf("Usage: %s [on|off|toggle]\n", Cmd_Argv(0));
		return;
	}
	if (set && !ret)
		xmms_togglefunc(XMMS_SESSION);
	else if (!set && ret)
		xmms_togglefunc(XMMS_SESSION);
	Com_Printf("%s set to %s\n", name, set ? "on" : "off");
}

void MP3_Repeat_f(void) {
	XMMS_Set_ToggleFn("Repeat", qxmms_remote_toggle_repeat, qxmms_remote_is_repeat);
}

void MP3_Shuffle_f(void) {
	XMMS_Set_ToggleFn("Shuffle", qxmms_remote_toggle_shuffle, qxmms_remote_is_shuffle);
}

void MP3_SetVolume_f (void) {
	int vol;

	if (!MP3_IsPlayerRunning()) {
		Com_Printf("%s\n", mp3_notrunning_msg);	
		return;
	}

	vol = atoi(Cmd_Argv(1));
	vol = bound(0, vol, 100);
	qxmms_remote_set_main_volume(XMMS_SESSION, vol);
}

void MP3_ToggleRepeat_f(void) {
	if (!MP3_IsPlayerRunning()) {
		Com_Printf("%s\n", mp3_notrunning_msg);	
		return;
	}
	qxmms_remote_toggle_repeat(XMMS_SESSION);
}

void MP3_ToggleShuffle_f(void) {
	if (!MP3_IsPlayerRunning()) {
		Com_Printf("%s\n", mp3_notrunning_msg);	
		return;
	}
	qxmms_remote_toggle_shuffle(XMMS_SESSION);
}

char *MP3_Macro_MP3Info(void) {
	int playlist_pos;
	char *s;
	static char title[MP3_MAXSONGTITLE];

	title[0] = 0;

	if (!MP3_IsPlayerRunning()) {
		Com_Printf("XMMS not running\n");
		return title;
	}
	playlist_pos = qxmms_remote_get_playlist_pos(XMMS_SESSION);
	s = qxmms_remote_get_playlist_title(XMMS_SESSION, playlist_pos);
	if(s) {
		Q_strncpyz(title, s, sizeof(title));
		COM_MakePrintable(title);
		qg_free(s);
	}

	return title;
}

qboolean MP3_GetTrackTime(int *elapsed, int *total) {
	int pos;

	if (!MP3_IsPlayerRunning()) {
		Com_Printf("%s\n", mp3_notrunning_msg);
		return false;
	}

	if(elapsed)
		*elapsed = qxmms_remote_get_output_time(XMMS_SESSION) / 1000.0;

	if(total) {
		pos = qxmms_remote_get_playlist_pos(XMMS_SESSION);
		*total = qxmms_remote_get_playlist_time(XMMS_SESSION, pos) / 1000.0;
	}

	return true;
}

static void MP3_SongTitle_m ( char *buffer, int bufferSize )
{
	char *songtitle;
	int total = 0;

	if (!MP3_IsPlayerRunning())
		return;

	if(!MP3_GetTrackTime(NULL, &total))
		return;

	songtitle = MP3_Macro_MP3Info();
	if (!*songtitle)
		return;

	Com_sprintf(buffer, bufferSize, "%s [%i:%02i]\n", songtitle, total / 60, total % 60);
}

qboolean MP3_GetToggleState(int *shuffle, int *repeat) {
	if (!MP3_IsPlayerRunning()) 
		return false;
	*shuffle = qxmms_remote_is_shuffle(XMMS_SESSION);
	*repeat = qxmms_remote_is_repeat(XMMS_SESSION);
	return true;
}


void MP3_SongInfo_f(void) {
	char *status_string, *title, *s;
	int status, elapsed, total;

	if (!MP3_IsPlayerRunning()) {
		Com_Printf("%s\n", mp3_notrunning_msg);
		return;
	}
	if (Cmd_Argc() != 1) {
		Com_Printf("%s : no arguments expected\n", Cmd_Argv(0));
		return;
	}

	status = MP3_GetStatus();
	if (status == MP3_STOPPED)
		status_string = "Stopped";
	else if (status == MP3_PAUSED)
		status_string = "Paused";
	else
		status_string = "Playing";

	for (s = title = MP3_Macro_MP3Info(); *s; s++)
		*s |= 128;

	if (!MP3_GetTrackTime(&elapsed, &total) || elapsed < 0 || total < 0) {
		Com_Printf(va("%s %s\n", status_string, title));
		return;
	}
	Com_Printf("%s %s [%i:%02i]\n", status_string, title, total / 60, total % 60);
}

char *MP3_Menu_SongTitle(void) {
	static char title[MP3_MAXSONGTITLE], *macrotitle;
	int current;

	if (!MP3_IsPlayerRunning()) {
		Com_Printf("%s\n", mp3_notrunning_msg);
	    Q_strncpyz(title, mp3_notrunning_msg, sizeof(title));
	    return title;
	}
	macrotitle = MP3_Macro_MP3Info();
	MP3_GetPlaylistInfo(&current, NULL);
	if (*macrotitle)
		Q_strncpyz(title, va("%d. %s", current + 1, macrotitle), sizeof(title));
	else
		Q_strncpyz(title, MP3_PLAYERNAME_ALLCAPS, sizeof(title));
	return title;
}

void MP3_GetPlaylistInfo(int *current, int *length) {
	if (!MP3_IsPlayerRunning()) 
		return;
	if (length)
		*length = qxmms_remote_get_playlist_length(XMMS_SESSION);
	if (current)
		*current = qxmms_remote_get_playlist_pos(XMMS_SESSION);
}

void MP3_PrintPlaylist_f(void) {
	int current, length, i;
	char *title, *s;

	if (!MP3_IsPlayerRunning()) {
		Com_Printf("%s\n", mp3_notrunning_msg);
		return;
	}
	MP3_GetPlaylistInfo(&current, &length);
	for (i = 0 ; i < length; i ++) {
		title = qxmms_remote_get_playlist_title(XMMS_SESSION, i);
		if(!title)
			continue;

		COM_MakePrintable(title);
		if (i == current)
			for (s = title; *s; s++)
				*s |= 128;

		Com_Printf("%3d %s\n", i + 1, title);
		qg_free(title);
	}
	return;
}

qboolean MP3_PlayTrack (int num)
{
	int length = -1;

	num -= 1;
	if (num < 0)
		return false;

	MP3_GetPlaylistInfo(NULL, &length);

	if(num > length)
	{
		Com_Printf("XMMS: playlist got only %i tracks\n", length);
        return false;
	}

	qxmms_remote_set_playlist_pos(XMMS_SESSION, num);
	MP3_Play_f();

	return true;
}

void MP3_PlayTrackNum_f(void) {

	if (!MP3_IsPlayerRunning()) {
		Com_Printf("%s\n", mp3_notrunning_msg);
		return;
	}
	if (Cmd_Argc() > 2) {
		Com_Printf("Usage: %s [track #]\n", Cmd_Argv(0));
		return;
	}

	if (Cmd_Argc() == 2)
	{
		MP3_PlayTrack (atoi(Cmd_Argv(1)));
		return;
	}

	MP3_Play_f();
}



int MP3_GetPlaylistSongs(mp3_tracks_t *songList, char *filter)
{
	int current, length, i;
	int playlist_size = 0, tracknum = 0, songCount = 0;
	char *s;

	if (!MP3_IsPlayerRunning())
		return 0;

	MP3_GetPlaylistInfo(&current, &length);
	for (i = 0 ; i < length; i ++) {
		s = qxmms_remote_get_playlist_title(XMMS_SESSION, i);
		if(!s)
			continue;
		if(!Q_stristr(s, filter)) {
			qg_free(s);
			continue;
		}
		qg_free(s);
		songCount++;
	}

	if(!songCount)
		return 0;

	songList->name = Z_TagMalloc (sizeof(char *) * songCount, TAG_MP3LIST);
	songList->num = Z_TagMalloc (sizeof(int) * songCount, TAG_MP3LIST);

	for (i = 0 ; i < length; i ++) {
		s = qxmms_remote_get_playlist_title(XMMS_SESSION, i);

		tracknum++;
		if(!s)
			continue;
		if(!Q_stristr(s, filter)) {
			qg_free(s);
			continue;
		}

		if (strlen(s) >= MP3_MAXSONGTITLE-1)
			s[MP3_MAXSONGTITLE-1] = 0;

		COM_MakePrintable(s);
		songList->num[playlist_size] = tracknum;
		songList->name[playlist_size++] = CopyString(va("%i. %s", tracknum, s), TAG_MP3LIST);
		qg_free(s);

		if(playlist_size >= songCount)
			break;
	}

	return playlist_size;
}

void MP3_Init(void)
{
	XMMS_LoadLibrary();

	if (!MP3_IsActive())
		return;

	Cmd_AddCommand("xmms_prev", MP3_Prev_f);
	Cmd_AddCommand("xmms_play", MP3_PlayTrackNum_f);
	Cmd_AddCommand("xmms_pause", MP3_Pause_f);
	Cmd_AddCommand("xmms_stop", MP3_Stop_f);
	Cmd_AddCommand("xmms_next", MP3_Next_f);
	Cmd_AddCommand("xmms_fforward", MP3_FastForward_f);
	Cmd_AddCommand("xmms_rewind", MP3_Rewind_f);
	Cmd_AddCommand("xmms_fadeout", MP3_FadeOut_f);
	Cmd_AddCommand("xmms_shuffle", MP3_Shuffle_f);
	Cmd_AddCommand("xmms_repeat", MP3_Repeat_f);
	Cmd_AddCommand("xmms_volume", MP3_SetVolume_f );
	Cmd_AddCommand("xmms_playlist", MP3_PrintPlaylist_f);
	Cmd_AddCommand("xmms_songinfo", MP3_SongInfo_f);
	Cmd_AddCommand("xmms_start", MP3_Execute_f);

	xmms_dir = Cvar_Get("xmms_dir", "/usr/local/bin", CVAR_ARCHIVE);
	xmms_session = Cvar_Get("xmms_session", "0", CVAR_ARCHIVE);
	xmms_messages = Cvar_Get("xmms_messages", "0", CVAR_ARCHIVE);

	Cmd_AddMacro( "cursong", MP3_SongTitle_m );
}


void MP3_Shutdown(void) {
	if (XMMS_pid) {
		if (!kill(XMMS_pid, SIGTERM))
			waitpid(XMMS_pid, NULL, 0);
	}

	XMMS_FreeLibrary();
}


void MP3_Frame (void)
{
	char *songtitle;
	int total, track = -1;
	static int curTrack = -1;

	if (!xmms_messages->integer)
		return;

	if(!((int)(cls.realtime>>8)&8))
		return;

	MP3_GetPlaylistInfo (&track, NULL);
	if(track == -1 || track == curTrack)
		return;

	if(!MP3_GetTrackTime(NULL, &total))
		return;

	songtitle = MP3_Macro_MP3Info();
	if (!*songtitle)
		return;

	curTrack = track;

	Com_Printf ("XMMS Title: %s [%i:%02i]\n", songtitle, total / 60, total % 60);
}

#endif

