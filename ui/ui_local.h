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
#ifndef __QMENU_H__
#define __QMENU_H__

#include "../client/client.h"

extern int Developer_searchpath( void );

#define MAXMENUITEMS	64

#define MTYPE_SLIDER		0
#define MTYPE_LIST			1
#define MTYPE_ACTION		2
#define MTYPE_SPINCONTROL	3
#define MTYPE_SEPARATOR  	4
#define MTYPE_FIELD			5
#define MTYPE_BITMAP		6

#define MLIST_SPACING	10
#define MLIST_BSIZE 3
#define MLIST_SSIZE 16

#define	K_TAB			9
#define	K_ENTER			13
#define	K_ESCAPE		27
#define	K_SPACE			32

// normal keys should be passed as lowercased ascii

#define	K_BACKSPACE		127
#define	K_UPARROW		128
#define	K_DOWNARROW		129
#define	K_LEFTARROW		130
#define	K_RIGHTARROW	131

#define QMF_LEFT_JUSTIFY	0x00000001
#define QMF_GRAYED			0x00000002
#define QMF_NUMBERSONLY		0x00000004

#define	MAX_SAVEGAMES	15

#define menu_in_sound		"misc/menu1.wav"
#define menu_move_sound		"misc/menu2.wav"
#define menu_out_sound		"misc/menu3.wav"

typedef struct _tag_menuframework
{
	int x, y;
	int	cursor;

	int	nitems;
	void *items[64];

	const char *statusbar;

	void (*draw)( struct _tag_menuframework *self );
	const char *(*key)( struct _tag_menuframework *self, int k );
	void (*cursordraw)( struct _tag_menuframework *m );
	
} menuframework_s;

typedef struct
{
	int type;
	const char *name;
	int x, y;
	int width;
	int height;
	menuframework_s *parent;
	int cursor_offset;
	int	localdata[4];
	unsigned flags;

	const char *statusbar;

	void (*callback)( void *self );
	void (*statusbarfunc)( void *self );
	void (*ownerdraw)( void *self );
	void (*cursordraw)( void *self );
} menucommon_s;

typedef struct
{
	menucommon_s generic;

	char		buffer[80];
	int			cursor;
	int			length;
	int			visible_length;
	int			visible_offset;
} menufield_s;

typedef struct 
{
	menucommon_s generic;

	float minvalue;
	float maxvalue;
	float curvalue;

	float range;
} menuslider_s;

typedef struct
{
	menucommon_s generic;

	int prestep;
	int curvalue;
	int count;
	const char **itemnames;

	int	width;
	int height;
	int lastClick;
	int maxItems;
} menulist_s;

typedef struct
{
	menucommon_s generic;
} menuaction_s;

typedef struct
{
	menucommon_s generic;
} menuseparator_s;

typedef struct {
	menucommon_s generic;
} menubitmap_s;

void M_Banner( char *name );
void M_PushMenu ( menuframework_s *menu );
void M_ForceMenuOff (void);
void M_PopMenu( void );
const char *Default_MenuKey( menuframework_s *m, int key );
void M_DrawCharacter( int cx, int cy, int num );
void M_Print( int cx, int cy, char *str );
void M_PrintWhite( int cx, int cy, char *str );
void M_DrawPic( int x, int y, char *pic );
void M_DrawCursor( int x, int y, int f );
void M_DrawTextBox( int x, int y, int width, int lines );
int Menu_HitTest( menuframework_s *menu, int x, int y );

qboolean Field_Key( menufield_s *field, int key );
qboolean List_Key ( menulist_s *l, int key);

void	Menu_AddItem( menuframework_s *menu, void *item );
void	Menu_AdjustCursor( menuframework_s *menu, int dir );
void	Menu_Center( menuframework_s *menu );
void	Menu_Draw( menuframework_s *menu );
void	*Menu_ItemAtCursor( menuframework_s *m );
qboolean Menu_SelectItem( menuframework_s *s );
void	Menu_SetStatusBar( menuframework_s *s, const char *string );
void	Menu_SlideItem( menuframework_s *s, int dir );
//int		Menu_TallySlots( menuframework_s *menu );

void MenuList_Init( menulist_s *l );

void	 Menu_DrawString( int, int, const char * );
void	 Menu_DrawStringDark( int, int, const char * );
void	 Menu_DrawStringR2L( int, int, const char * );
void	 Menu_DrawStringR2LDark( int, int, const char * );

void M_Menu_Main_f (void);
	void M_Menu_Game_f (void);
		void M_Menu_LoadGame_f (void);
		void M_Menu_SaveGame_f (void);
		void M_Menu_PlayerConfig_f (void);
			void M_Menu_DownloadOptions_f (void);
		void M_Menu_Credits_f( void );
	void M_Menu_Multiplayer_f( void );
		void M_Menu_JoinServer_f (void);
			void M_Menu_AddressBook_f( void );
		void M_Menu_StartServer_f (void);
			void M_Menu_DMOptions_f (void);
	void M_Menu_Video_f (void);
	void M_Menu_Options_f (void);
		void M_Menu_NewOptions_f (void);
		void M_Menu_Demos_f( void );
		void M_Menu_Keys_f (void);
	void M_Menu_Quit_f (void);
#if defined(_WIN32) || defined(WITH_XMMS)
	void M_Menu_MP3_f( void );
#endif
	void M_Menu_Credits( void );
#endif

