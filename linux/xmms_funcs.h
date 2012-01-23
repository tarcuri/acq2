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

#ifndef XMMS_FUNC
#define XMMS_FUNC(ret, func, params)
#define UNDEF_XMMS_FUNC
#endif

XMMS_FUNC (void, xmms_remote_play, (gint session))
XMMS_FUNC (void, xmms_remote_pause, (gint session))
XMMS_FUNC (void, xmms_remote_stop, (gint session))
XMMS_FUNC (void, xmms_remote_jump_to_time, (gint session, gint pos))
XMMS_FUNC (gboolean, xmms_remote_is_running, (gint session))
XMMS_FUNC (gboolean, xmms_remote_is_playing, (gint session))
XMMS_FUNC (gboolean, xmms_remote_is_paused, (gint session))
XMMS_FUNC (gboolean, xmms_remote_is_repeat, (gint session))
XMMS_FUNC (gboolean, xmms_remote_is_shuffle, (gint session))
XMMS_FUNC (gint, xmms_remote_get_playlist_pos, (gint session))
XMMS_FUNC (void, xmms_remote_set_playlist_pos, (gint session, gint pos))
XMMS_FUNC (gint, xmms_remote_get_playlist_length, (gint session))
XMMS_FUNC (gchar *, xmms_remote_get_playlist_title, (gint session, gint pos))
XMMS_FUNC (void, xmms_remote_playlist_prev, (gint session))
XMMS_FUNC (void, xmms_remote_playlist_next, (gint session))
XMMS_FUNC (gint, xmms_remote_get_output_time, (gint session))
XMMS_FUNC (gint, xmms_remote_get_playlist_time, (gint session, gint pos))
XMMS_FUNC (gint, xmms_remote_get_main_volume, (gint session))
XMMS_FUNC (void, xmms_remote_set_main_volume, (gint session, gint v))
XMMS_FUNC (void, xmms_remote_toggle_repeat, (gint session))
XMMS_FUNC (void, xmms_remote_toggle_shuffle, (gint session))
XMMS_FUNC (void, g_free, (gpointer))

#ifdef UNDEF_XMMS_FUNC
#undef XMMS_FUNC
#undef UNDEF_XMMS_FUNC
#endif
