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

/* mpd control by hifii <3 */

#ifdef WITH_MPD

#include "../client/client.h"
#include "libmpdclient.h"

static cvar_t *mpd_port;
static cvar_t *mpd_host;
static cvar_t *mpd_pass;
static cvar_t *mpd_volhop;

static mpd_Connection *conn;
static qboolean mpdConnection = false;


void MPD_Disconnect(void)
{
	if(mpdConnection) {
		mpd_closeConnection(conn);
		mpdConnection = false;
	}
}

/*
=================
MPD_Connect - connect to MPD server
=================
*/
void MPD_Connect(void)
{
	MPD_Disconnect();

	/* Connecting to MPD server */
	conn = mpd_newConnection(mpd_host->string, mpd_port->integer, 10);

	/* Checking if we failed to connect */
	if(conn->error)
	{
		Com_Printf("MPD: %s\n", conn->errorStr);
		mpd_closeConnection(conn);
		return;
	} 
	
	mpdConnection = true;

	// send da password if we're using one
	if(mpd_pass->string[0]) {
		mpd_sendPasswordCommand(conn, mpd_pass->string);
		mpd_finishCommand(conn);
	}
}

static qboolean MP3_Status(void)
{
	if (mpdConnection && !conn->error)
		return true;

	MPD_Connect();
	if (mpdConnection)
		return true;

	return false;
}

/*
=================
MPD_Help - print help to console
=================
*/
void MPD_Help(void)
{
/* max msg width at 320x240 is 37 chars
   (it does new line automagically if it's exactly 37 chars wide) */

	Com_Printf("MPD Control\n");
	Com_Printf("=====================================\n");

	if(!Q_stricmp(Cmd_Argv(1), "cvar"))
	{
		Com_Printf( "mpd_host - hostname or ip to connect\n"
					"           Default: localhost\n"
					"mpd_port - port number to connect\n"
					"           Default: 3000\n"
					"mpd_pass - password for mpd\n"
					"mpd_volhop - how many precent mincvol\n"
					"             and mdecvol will do\n"
					"             Default: 5\n"
					"=====================================\n");
	} else {
		Com_Printf( "mpd_play [song num] - start playback\n"
					"mpd_stop - stop playback\n"
					"mpd_pause - pause playback\n"
					"mpd_status - status info\n"
					"mpd_volume <0-100> - set volume precent\n"
					"mpd_incvol - increase volume\n"
					"mpd_decvol - decrease volume\n"
					"mpd_next - play next song in playlist\n"
					"mpd_prev - play prev song in playlist\n"
					"mpd_playlist - list songs in playlist\n"
					"=====================================\n"
					"Also see mpd_help cvar\n");
	}
}

/*
=================
MPD_Status - show info about currently playing song
=================
*/
void MPD_Status(void)
{
	mpd_Status		*status;
	mpd_InfoEntity	*entity = NULL;
	mpd_Song		*song = 0;
	char			title[MP3_MAXSONGTITLE];


	if(!MP3_Status())
		return;

	mpd_sendStatusCommand(conn);

	if( (status = mpd_getStatus(conn)) == NULL)
	{
		Com_Printf("MPD: %s\n", conn->errorStr);
		mpd_finishCommand(conn);
		return;
	}
	mpd_finishCommand(conn);

	if( status->state == MPD_STATUS_STATE_STOP) {
		/* Playback stopped */
		Com_Printf("MPD: not playing\n");
		mpd_freeStatus(status);
		return;
	}

	if( status->state == MPD_STATUS_STATE_PLAY || MPD_STATUS_STATE_PAUSE )
	{
		mpd_sendCurrentSongCommand(conn);
		while((entity = mpd_getNextInfoEntity(conn)))
		{
			if(entity->type != MPD_INFO_ENTITY_TYPE_SONG)
			{
				mpd_freeInfoEntity(entity);
				continue;
			}

			song = entity->info.song;
			if(song->title == NULL || song->artist == NULL)
				Com_sprintf(title, sizeof(title), "%s [%i:%02i]", (song->title) ? song->title : song->file, status->totalTime / 60, status->totalTime % 60);
			else
				Com_sprintf(title, sizeof(title), "%s - %s [%i:%02i]", song->artist, song->title, status->totalTime / 60, status->totalTime % 60);

			COM_MakePrintable(title);
			Com_Printf("%s\n", title);

			mpd_freeInfoEntity(entity);
			break;
		}
		mpd_finishCommand(conn);
	}

	mpd_freeStatus(status);
}

static char *MPD_SongTitle ( void )
{
	mpd_Status		*status;
	mpd_InfoEntity	*entity = NULL;
	mpd_Song		*song = 0;
	static char title[MP3_MAXSONGTITLE]; 

	title[0] = 0;

	if(!MP3_Status())
		return title;

	mpd_sendCommandListOkBegin(conn);
	mpd_sendStatusCommand(conn);
	mpd_sendCurrentSongCommand(conn);
	mpd_sendCommandListEnd(conn);

	if( (status = mpd_getStatus(conn)) == NULL)
	{
		return title;
	}

	mpd_nextListOkCommand(conn);

	while((entity = mpd_getNextInfoEntity(conn)))
	{
		if(entity->type != MPD_INFO_ENTITY_TYPE_SONG)
		{
			mpd_freeInfoEntity(entity);
			continue;
		}

		song = entity->info.song;
		if(song->title == NULL || song->artist == NULL)
			Com_sprintf(title, sizeof(title), "%s [%i:%02i]", song->file, status->totalTime / 60, status->totalTime % 60);
		else
			Com_sprintf(title, sizeof(title), "%s - %s [%i:%02i]", song->artist, song->title, status->totalTime / 60, status->totalTime % 60);

		COM_MakePrintable(title);

		mpd_freeInfoEntity(entity);
		break;
	}

	mpd_freeStatus(status);
	mpd_finishCommand(conn);

	return title;
}

static void MPD_SongTitle_m ( char *buffer, int bufferSize )
{
	Q_strncpyz ( buffer, MPD_SongTitle(), bufferSize );
}

/*
=================
MPD_Play - start playback
=================
*/
void MPD_Play(void)
{
	if(!MP3_Status())
		return;

	if(Cmd_Argc() == 2)	// User gave song id to play
		mpd_sendPlayIdCommand(conn, atoi(Cmd_Argv(1))-1);
	else
		mpd_sendPlayCommand(conn, -1);

	mpd_finishCommand(conn);

	MPD_Status();
}

/*
=================
MPD_Stop - stop playback
=================
*/
void MPD_Stop(void)
{
	if(!MP3_Status())
		return;

	mpd_sendStopCommand(conn);
	mpd_finishCommand(conn);

	MPD_Status();
}

/*
=================
MPD_Pause - pause playback
=================
*/
void MPD_Pause(void)
{
	if(!MP3_Status())
		return;

	mpd_sendPauseCommand(conn, 1);
	mpd_finishCommand(conn);

	MPD_Status();
}


/*
=================
MPD_Next - play next song
=================
*/
void MPD_Next(void)
{
	if(!MP3_Status())
		return;

	mpd_sendNextCommand(conn);
	mpd_finishCommand(conn);

	MPD_Status();
}

/*
=================
MPD_Prev - play previous song
=================
*/
void MPD_Prev(void)
{
	if(!MP3_Status())
		return;

	mpd_sendPrevCommand(conn);
	mpd_finishCommand(conn);

	MPD_Status();
}

/*
=================
MPD_SetVol - set volume
=================
*/
void MPD_SetVol(void)
{
	mpd_Status		*status;


	if(!MP3_Status())
		return;

	if(Cmd_Argc() < 2)
	{
		mpd_sendStatusCommand(conn);

		if( (status = mpd_getStatus(conn)) == NULL)
		{
			Com_Printf("MPD: %s\n", conn->errorStr);
		} else {
			Com_Printf("MPD: volume %i\%\n", status->volume);
			mpd_freeStatus(status);
		}
		mpd_finishCommand(conn);
	} else {

		mpd_sendCommandListOkBegin(conn);
		mpd_sendSetvolCommand(conn, atoi(Cmd_Argv(1)));
		mpd_sendStatusCommand(conn);
		mpd_sendCommandListEnd(conn);

		mpd_nextListOkCommand(conn);

		if( (status = mpd_getStatus(conn)) == NULL)
		{
			Com_Printf("MPD: %s\n", conn->errorStr);
		} else {
			Com_Printf("MPD: volume %i\%\n", status->volume);
			mpd_freeStatus(status);
		}
		mpd_finishCommand(conn);
	}

}

/*
=================
MPD_IncVol - increase volume
=================
*/
void MPD_IncVol(void)
{
	mpd_Status		*status;

	if(!MP3_Status())
		return;

	/* first get the current volume */
	mpd_sendStatusCommand(conn);

	if( (status = mpd_getStatus(conn)) == NULL)
	{
		Com_Printf("MPD: %s\n", conn->errorStr);
		mpd_finishCommand(conn);
	}
	else
	{
		mpd_finishCommand(conn);
		if(status->volume == MPD_STATUS_NO_VOLUME)
		{
			Com_Printf("MPD: no volume support available\n");
		}
		else
		{
			Com_Printf("MPD: volume %i\%\n", status->volume + mpd_volhop->integer);
			mpd_sendSetvolCommand(conn, status->volume + mpd_volhop->integer);
			mpd_finishCommand(conn);
		}
		mpd_freeStatus(status);
	}

}

/*
=================
MPD_DecVol - decrease volume
=================
*/
void MPD_DecVol(void)
{
	mpd_Status		*status;

	if(!MP3_Status())
		return;

	/* first get the current volume */
	mpd_sendStatusCommand(conn);

	if( (status = mpd_getStatus(conn)) == NULL)
	{
		Com_Printf("MPD: %s\n", conn->errorStr);
		mpd_finishCommand(conn);
	}
	else
	{
		mpd_finishCommand(conn);
		if(status->volume == MPD_STATUS_NO_VOLUME)
		{
			Com_Printf("MPD: no volume support available\n");
		}
		else
		{
			Com_Printf("MPD: volume %i\%\n", status->volume - mpd_volhop->integer);
			mpd_sendSetvolCommand(conn, status->volume - mpd_volhop->integer);
			mpd_finishCommand(conn);
		}
		mpd_freeStatus(status);
	}

}

/*
=================
MPD_Playlist - print playlist items
=================
*/
void MPD_Playlist(void)
{
	mpd_InfoEntity	*entity;
	mpd_Song		*song;
	char			title[MP3_MAXSONGTITLE];

	if(!MP3_Status())
		return;

	mpd_sendPlaylistInfoCommand(conn, -1);
	while((entity = mpd_getNextInfoEntity(conn)))
	{
		if(entity->type != MPD_INFO_ENTITY_TYPE_SONG)
		{
			mpd_freeInfoEntity(entity);
			continue;
		}

		song = entity->info.song;
		if(song->title == NULL || song->artist == NULL)
			Com_sprintf(title, sizeof(title), "%02i. %s", song->pos+1, song->file);
		else
			Com_sprintf(title, sizeof(title), "%02i. %s - %s", song->pos+1, song->artist, song->title);
	
		COM_MakePrintable(title);
		Com_Printf("%s\n", title);

		mpd_freeInfoEntity(entity);
	}
	mpd_finishCommand(conn);
}

void OnChange_MPDAddress(cvar_t *self, const char *oldValue)
{
	Com_Printf("MPD: Using host \"%s\" and port \"%i\" for connections.\n", mpd_host->string, mpd_port->integer);
	Com_Printf("MPD: Testing server version...");

	MPD_Connect();
	if (mpdConnection)
		Com_Printf(" %i.%i.%i!!\n", conn->version[0], conn->version[1], conn->version[2]);
	else
		Com_Printf(" failed! Is the server running?\n");
}

/*
=================
MP3_Init - init function to add commands and default cvars
=================
*/
void MP3_Init(void)
{

	Com_Printf("Music Player Daemon control initializing...\n");

	Cmd_AddCommand ("mpd_help", MPD_Help);

	Cmd_AddCommand ("mpd_status", MPD_Status);
	Cmd_AddCommand ("mpd_play", MPD_Play);
	Cmd_AddCommand ("mpd_stop", MPD_Stop);
	Cmd_AddCommand ("mpd_pause", MPD_Pause);
	Cmd_AddCommand ("mpd_next", MPD_Next);
	Cmd_AddCommand ("mpd_prev", MPD_Prev);
	Cmd_AddCommand ("mpd_volume", MPD_SetVol);
	Cmd_AddCommand ("mpd_incvol", MPD_IncVol);
	Cmd_AddCommand ("mpd_decvol", MPD_DecVol);
	Cmd_AddCommand ("mpd_playlist", MPD_Playlist);

	Cmd_AddMacro( "cursong", MPD_SongTitle_m );

	/* Set the default settings for MPD cvars if they are not given in autoexec.cfg */
	mpd_port = Cvar_Get("mpd_port", "6600", 0);
	mpd_host = Cvar_Get("mpd_host", "localhost", 0);
	mpd_pass = Cvar_Get("mpd_pass", "", 0);
	mpd_volhop = Cvar_Get("mpd_volhop", "5", 0);
	mpd_port->OnChange = OnChange_MPDAddress;
	mpd_host->OnChange = OnChange_MPDAddress;

	Com_Printf("MPD: Using host \"%s\" and port \"%i\" for connections.\n", mpd_host->string, mpd_port->integer);
	Com_Printf("MPD: Testing server version...");

	MPD_Connect();
	if (mpdConnection)
		Com_Printf(" %i.%i.%i!!\n", conn->version[0], conn->version[1], conn->version[2]);
	else
		Com_Printf(" failed! Is the server running?\n");

}

void MP3_Shutdown(void)
{
	MPD_Disconnect();
}
#endif
