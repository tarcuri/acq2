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
/*
** GLW_IMP.C
**
** This file contains ALL Linux specific stuff having to do with the
** OpenGL refresh.  When a port is being made the following functions
** must be implemented by the port:
**
** GLimp_EndFrame
** GLimp_Init
** GLimp_Shutdown
** GLimp_SwitchFullscreen
**
*/

#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <dlfcn.h>
#ifndef MACOS_X
#include <execinfo.h>
#endif

#include "../ref_gl/gl_local.h"
#include "../client/keys.h"
#include "../linux/glw_linux.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

#ifndef WITHOUT_DGA
#include <X11/extensions/xf86dga.h>
#endif
#include <X11/extensions/xf86vmode.h>

glwstate_t glw_state;

static Display *dpy = NULL;
static int scrnum;
static Window win = 0;
static GLXContext ctx = NULL;
static Atom wmDeleteWindow;

#define KEY_MASK (KeyPressMask | KeyReleaseMask)
#define MOUSE_MASK (ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ButtonMotionMask )
#define X_MASK (KEY_MASK | MOUSE_MASK | VisibilityChangeMask | StructureNotifyMask /*| SubstructureNotifyMask*/ )


/*****************************************************************************/
/* MOUSE                                                                     */
/*****************************************************************************/
#ifdef WITH_EVDEV
extern qboolean mevdev_avail;
extern qboolean evdev_masked;
#endif

int mx, my;
extern int old_mouse_x, old_mouse_y;

extern cvar_t	*in_dgamouse;
static int win_x = 0, win_y = 0;
static int p_mouse_x, p_mouse_y;

extern int vid_minimized;

static XF86VidModeModeInfo **vidmodes;
static int num_vidmodes;
static qboolean vidmode_active = false;
static Window minimized_window;
static int best_fit;

qboolean mouse_active = false;
qboolean dgamouse = false;

static Time myxtime;

static Cursor CreateNullCursor(Display *display, Window root)
{
    Pixmap cursormask; 
    XGCValues xgc;
    GC gc;
    XColor dummycolour;
    Cursor cursor;

    cursormask = XCreatePixmap(display, root, 1, 1, 1/*depth*/);
    xgc.function = GXclear;
    gc =  XCreateGC(display, cursormask, GCFunction, &xgc);
    XFillRectangle(display, cursormask, gc, 0, 0, 1, 1);
    dummycolour.pixel = 0;
    dummycolour.red = 0;
    dummycolour.flags = 04;
    cursor = XCreatePixmapCursor(display, cursormask, cursormask,
          &dummycolour,&dummycolour, 0,0);
    XFreePixmap(display,cursormask);
    XFreeGC(display,gc);
    return cursor;
}

static void install_grabs(void)
{
	XDefineCursor(dpy, win, CreateNullCursor(dpy, win));
	XGrabPointer(dpy, win, True, 0, GrabModeAsync, GrabModeAsync, win, None, CurrentTime);
	//XGrabPointer(dpy, win, False, MOUSE_MASK, GrabModeAsync, GrabModeAsync, win, None, CurrentTime);

	dgamouse = false;
#ifndef WITHOUT_DGA
	if (in_dgamouse->integer)
	{
		int MajorVersion, MinorVersion;

		if (XF86DGAQueryVersion(dpy, &MajorVersion, &MinorVersion)) {
			XF86DGADirectVideo(dpy, DefaultScreen(dpy), XF86DGADirectMouse);
			XWarpPointer(dpy, None, win, 0, 0, 0, 0, 0, 0);
			dgamouse = true;
		} else {
			// unable to query, probalby not supported
			Com_Printf ( "Failed to detect XF86DGA Mouse\n" );
			Cvar_Set( "in_dgamouse", "0" );
		}
	}
#endif
	if(!dgamouse) {
		p_mouse_x = vid.width / 2;
		p_mouse_y = vid.height / 2;
		XWarpPointer(dpy, None, win, 0, 0, 0, 0, p_mouse_x, p_mouse_y);
	}

	XGrabKeyboard(dpy, win, False, GrabModeAsync, GrabModeAsync, CurrentTime);
}

static void uninstall_grabs(void)
{
	if (dgamouse) {
		dgamouse = false;
#ifndef WITHOUT_DGA
		XF86DGADirectVideo(dpy, DefaultScreen(dpy), 0);
#endif
	}

	XUngrabPointer(dpy, CurrentTime);
	XUngrabKeyboard(dpy, CurrentTime);

// inviso cursor
	XUndefineCursor(dpy, win);
}

static void IN_DeactivateMouse( void ) 
{
	if (!dpy || !win)
		return;

	if (mouse_active) {
		uninstall_grabs();
		mouse_active = false;
		dgamouse = false;
	}
}

static void IN_ActivateMouse( void ) 
{
	if (!dpy || !win)
		return;

	if (!mouse_active) {
		mx = my = 0; // don't spazz
		old_mouse_x = old_mouse_y = 0;
		install_grabs();
		mouse_active = true;
	}
}

void IN_Activate(qboolean active)
{
	if (active)
		IN_ActivateMouse();
	else
		IN_DeactivateMouse ();
}


/*****************************************************************************/
/* KEYBOARD                                                                  */
/*****************************************************************************/

static int XLateKey(XKeyEvent *ev)
{

	int key = 0;
	char buf[64];
	KeySym keysym;

	XLookupString(ev, buf, sizeof(buf), &keysym, 0);

	switch(keysym)
	{
		case XK_KP_Page_Up:	 key = K_KP_PGUP; break;
		case XK_Page_Up:	 key = K_PGUP; break;

		case XK_KP_Page_Down: key = K_KP_PGDN; break;
		case XK_Page_Down:	 key = K_PGDN; break;

		case XK_KP_Home: key = K_KP_HOME; break;
		case XK_Home:	 key = K_HOME; break;

		case XK_KP_End:  key = K_KP_END; break;
		case XK_End:	 key = K_END; break;

		case XK_KP_Left: key = K_KP_LEFTARROW; break;
		case XK_Left:	 key = K_LEFTARROW; break;

		case XK_KP_Right: key = K_KP_RIGHTARROW; break;
		case XK_Right:	key = K_RIGHTARROW;		break;

		case XK_KP_Down: key = K_KP_DOWNARROW; break;
		case XK_Down:	 key = K_DOWNARROW; break;

		case XK_KP_Up:   key = K_KP_UPARROW; break;
		case XK_Up:		 key = K_UPARROW;	 break;

		case XK_Escape: key = K_ESCAPE;		break;

		case XK_KP_Enter: key = K_KP_ENTER;	break;
		case XK_Return: key = K_ENTER;		 break;

		case XK_Tab:		key = K_TAB;			 break;

		case XK_F1:		 key = K_F1;				break;

		case XK_F2:		 key = K_F2;				break;

		case XK_F3:		 key = K_F3;				break;

		case XK_F4:		 key = K_F4;				break;

		case XK_F5:		 key = K_F5;				break;

		case XK_F6:		 key = K_F6;				break;

		case XK_F7:		 key = K_F7;				break;

		case XK_F8:		 key = K_F8;				break;

		case XK_F9:		 key = K_F9;				break;

		case XK_F10:		key = K_F10;			 break;

		case XK_F11:		key = K_F11;			 break;

		case XK_F12:		key = K_F12;			 break;

		case XK_BackSpace: key = K_BACKSPACE; break;

		case XK_KP_Delete: key = K_KP_DEL; break;
		case XK_Delete: key = K_DEL; break;

		case XK_Pause:	key = K_PAUSE;		 break;

		case XK_Shift_L:
		case XK_Shift_R:	key = K_SHIFT;		break;

		case XK_Execute: 
		case XK_Control_L: 
		case XK_Control_R:	key = K_CTRL;		 break;

		case XK_Alt_L:	
		case XK_Meta_L: 
		case XK_Alt_R:	
		case XK_Meta_R: key = K_ALT;			break;

		case XK_KP_Begin: key = K_KP_5;	break;

		case XK_Insert:key = K_INS; break;
		case XK_KP_Insert: key = K_KP_INS; break;

		case XK_KP_Multiply: key = '*'; break;
		case XK_KP_Add:  key = K_KP_PLUS; break;
		case XK_KP_Subtract: key = K_KP_MINUS; break;
		case XK_KP_Divide: key = K_KP_SLASH; break;

		case XK_exclam: key = '1'; break;
		case XK_at: key = '2'; break;
		case XK_numbersign: key = '3'; break;
		case XK_dollar: key = '4'; break;
		case XK_percent: key = '5'; break;
		case XK_asciicircum: key = '6'; break;
		case XK_ampersand: key = '7'; break;
		case XK_asterisk: key = '8'; break;
		case XK_parenleft: key = '9'; break;
		case XK_parenright: key = '0'; break;

		case XK_twosuperior: key = '~'; break;

		case XK_space:
		case XK_KP_Space: key = K_SPACE; break;

		default:
			key = *(unsigned char*)buf;
			if (key >= 'A' && key <= 'Z')
				key = key - 'A' + 'a';
			if (key >= 1 && key <= 26) /* ctrl+alpha */
				key = key + 'a' - 1;
			break;
	} 

	return key;
}

void HandleEvents(void)
{
	XEvent event;
	qboolean dowarp = false;
	int mwx = vid.width/2;
	int mwy = vid.height/2;

	if (!dpy)
		return;

	while (XPending(dpy)) {

		XNextEvent(dpy, &event);

		switch(event.type) {
		case KeyPress:
			myxtime = event.xkey.time;
		case KeyRelease:
			Key_Event (XLateKey(&event.xkey), event.type == KeyPress, Sys_Milliseconds());
			break;

		case MotionNotify:
			if (mouse_active) {
				#ifdef WITH_EVDEV
				if (mevdev_avail) {
					break;
				}
				#endif
				if (dgamouse) {
					if (in_dgamouse->integer == 2) {
						mx += event.xmotion.x_root * 2;
						my += event.xmotion.y_root * 2;
					} else {
						mx += (event.xmotion.x + win_x) * 2;
						my += (event.xmotion.y + win_y) * 2;
					}
				} 
				else 
				{
					if( !event.xmotion.send_event ) {
						mx += event.xmotion.x - p_mouse_x;
						my += event.xmotion.y - p_mouse_y;

						if( abs(mwx - event.xmotion.x) > mwx / 2 || abs(mwy - event.xmotion.y) > mwy / 2 )
							dowarp = true;
					}
					p_mouse_x = event.xmotion.x;
					p_mouse_y = event.xmotion.y;
				}
			}
			break;


		case ButtonPress:
			myxtime = event.xbutton.time;
		case ButtonRelease:
			#ifdef WITH_EVDEV
			if (mevdev_avail) {
				break;
			}
			#endif
			if (event.xbutton.button == 1) Key_Event(K_MOUSE1, event.type == ButtonPress, Sys_Milliseconds());
			else if (event.xbutton.button == 2) Key_Event(K_MOUSE3, event.type == ButtonPress, Sys_Milliseconds());
			else if (event.xbutton.button == 3) Key_Event(K_MOUSE2, event.type == ButtonPress, Sys_Milliseconds());
			else if (event.xbutton.button == 4) Key_Event(K_MWHEELUP, event.type == ButtonPress, Sys_Milliseconds());
			else if (event.xbutton.button == 5) Key_Event(K_MWHEELDOWN, event.type == ButtonPress, Sys_Milliseconds());
			else if (event.xbutton.button >= 6 && event.xbutton.button <= 9) Key_Event(K_MOUSE4+event.xbutton.button-6, event.type == ButtonPress, Sys_Milliseconds());
			break;

		case CreateNotify :
			win_x = event.xcreatewindow.x;
			win_y = event.xcreatewindow.y;
			break;

		case ConfigureNotify :
			win_x = event.xconfigure.x;
			win_y = event.xconfigure.y;
			break;

		case ClientMessage:
			if (event.xclient.data.l[0] == wmDeleteWindow)
				Cbuf_ExecuteText(EXEC_NOW, "quit");
			break;
	case MapNotify:
		if (event.xmap.window == win)
			IN_ActivateMouse();
		if (!vid_minimized)
			break;
		vid_minimized = 0;
		if (vidmode_active) {
			XDestroyWindow(dpy, minimized_window);
			XMapWindow(dpy, win);
			XF86VidModeSwitchToMode(dpy, scrnum, vidmodes[best_fit]);
			XF86VidModeSetViewPort(dpy, scrnum, 0, 0);
		}
		break;

	case UnmapNotify:
		if (event.xunmap.window != win) break;
		Key_ClearStates();
		IN_DeactivateMouse();
		vid_minimized = 1;
		break;
#ifdef WITH_EVDEV
		case VisibilityNotify:
			switch(event.xvisibility.state) {
			case VisibilityUnobscured:
				evdev_masked = false;
				break;
			case VisibilityFullyObscured:
				evdev_masked = true;
				break;
			}
		break;
#endif
		}
	}
	if (dowarp) {
		/* move the mouse to the window center again */
		p_mouse_x = mwx;
		p_mouse_y = mwy;
		XWarpPointer(dpy,None,win,0,0,0,0, p_mouse_x, p_mouse_y);
	}
}

/*****************************************************************************/

char *Sys_GetClipboardData(void)
{
	Window sowner;
	Atom type, property;
	unsigned long len, bytes_left, tmp;
	unsigned char *data;
	int format, result;
	char *ret = NULL;

	if (!dpy && !win) 
		return NULL;

	sowner = XGetSelectionOwner(dpy, XA_PRIMARY);
			
	if (sowner != None)
	{
		property = XInternAtom(dpy, "GETCLIPBOARDDATA_PROP", False);

		XConvertSelection(dpy, XA_PRIMARY, XA_STRING, property, win, myxtime); /* myxtime == time of last X event */
		XFlush(dpy);

		XGetWindowProperty(dpy, win, property, 0, 0, False, AnyPropertyType, &type, &format, &len, &bytes_left, &data);
		if (bytes_left > 0) {
			result = XGetWindowProperty(dpy, win, property,
				0, bytes_left, True, AnyPropertyType, &type, &format, &len, &tmp, &data);

			if (result == Success) {
				ret = CopyString(data, TAG_CLIPBOARD);
			}
			XFree(data);
		}
	}
	return ret;
}

/*****************************************************************************/

qboolean GLimp_InitGL (void);

static void signal_handler2(int sig)
{
	void		*array[32];
	size_t		size, i;
	char		**strings;

	printf("Received signal %d, exiting...\n", sig);

	#ifndef MACOS_X
	size = backtrace (array, sizeof(array)/sizeof(void*));
	
	strings = backtrace_symbols (array, size);

	printf("Stack dump (%zd frames):\n", size);

	for (i = 0; i < size; i++)
		printf("%.2zd: %s\n", i, strings[i]);

	free (strings);
	#else
	printf("Note: Backtrace is disabled on OSX, no backtrace available\n");
	#endif

	GLimp_Shutdown();
	_exit(0);
}

static void signal_handler(int sig)
{
	printf("Received signal %d, exiting...\n", sig);
	GLimp_Shutdown();
	_exit(0);
}

static void InitSig(void)
{
	signal(SIGHUP, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGILL, signal_handler);
	signal(SIGTRAP, signal_handler);
	signal(SIGIOT, signal_handler);
	signal(SIGBUS, signal_handler);
	signal(SIGFPE, signal_handler);
	signal(SIGSEGV, signal_handler2);
	signal(SIGTERM, signal_handler);
}

/*
** GLimp_SetMode
*/
rserr_t GLimp_SetMode( int *pwidth, int *pheight, int mode, qboolean fullscreen )
{
	int width, height;
	int attrib[] = {
		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE, 1,
		GLX_STENCIL_SIZE, 1,
		None
	};
	int attrib_nostencil[] = {
		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE, 1,
		None
	};

	Window root;
	XVisualInfo *visinfo;
	XSetWindowAttributes attr;
	XSizeHints *sizehints;
	unsigned long mask;
	int i;


	Com_Printf ( "Initializing OpenGL display\n");

	if (fullscreen)
		Com_Printf ("...setting fullscreen mode %d:", mode );
	else
		Com_Printf ("...setting mode %d:", mode );

	if ( !R_GetModeInfo( &width, &height, mode ) )
	{
		Com_Printf ( " invalid mode\n" );
		return rserr_invalid_mode;
	}

	Com_Printf ( " %d %d\n", width, height );

	// destroy the existing window
	GLimp_Shutdown ();

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "Error couldn't open the X display\n");
		return rserr_invalid_mode;
	}

	scrnum = DefaultScreen(dpy);
	root = RootWindow(dpy, scrnum);

	visinfo = qglXChooseVisual(dpy, scrnum, attrib);
	if (!visinfo) {
		fprintf(stderr, "W: couldn't get an RGBA, DOUBLEBUFFER, DEPTH, STENCIL visual\n");
		visinfo = qglXChooseVisual(dpy, scrnum, attrib_nostencil);
		if (!visinfo) {
			fprintf(stderr, "E: couldn't get an RGBA, DOUBLEBUFFER, DEPTH visual\n");
			return rserr_invalid_mode;
		}
	}

	/* do some pantsness */
	gl_state.stencil = false;
	if ( qglXGetConfig )
	{
		int red_bits, blue_bits, green_bits, depth_bits, alpha_bits;
		int stencil_bits;

		qglXGetConfig(dpy, visinfo, GLX_RED_SIZE, &red_bits);
		qglXGetConfig(dpy, visinfo, GLX_BLUE_SIZE, &blue_bits);
		qglXGetConfig(dpy, visinfo, GLX_GREEN_SIZE, &green_bits);
		qglXGetConfig(dpy, visinfo, GLX_DEPTH_SIZE, &depth_bits);
		qglXGetConfig(dpy, visinfo, GLX_ALPHA_SIZE, &alpha_bits);
		if (!qglXGetConfig(dpy, visinfo, GLX_STENCIL_SIZE, &stencil_bits)) {
			if (stencil_bits >= 1) {
				gl_state.stencil = true;
			}
		}
		else
			stencil_bits = 0;
		
		Com_Printf ( "Red(%dbits) Blue(%dbits) Green(%dbits)\n", red_bits, blue_bits, green_bits);
		Com_Printf ( "Depth(%dbits) Alpha(%dbits) Stencil(%dbits)\n", depth_bits, alpha_bits, stencil_bits);
	}

	vidmode_active = false;
	if (fullscreen)
	{
		int MajorVersion = 0, MinorVersion = 0;

		// Get video mode list
		if (!XF86VidModeQueryVersion(dpy, &MajorVersion, &MinorVersion))
		{ 
			Com_Printf ("..XFree86-VidMode Extension not available, going windowed mode\n");
		}
		else
		{
			int best_dist, dist, x, y;

			Com_Printf ( "Using XFree86-VidModeExtension Version %d.%d\n", MajorVersion, MinorVersion);
		
			num_vidmodes = 0;
			XF86VidModeGetAllModeLines(dpy, scrnum, &num_vidmodes, &vidmodes);

			best_dist = 9999999;
			best_fit = -1;

			for (i = 0; i < num_vidmodes; i++) {
				if (width > vidmodes[i]->hdisplay ||
					height > vidmodes[i]->vdisplay)
					continue;

				x = width - vidmodes[i]->hdisplay;
				y = height - vidmodes[i]->vdisplay;
				dist = (x * x) + (y * y);
				if (dist < best_dist) {
					best_dist = dist;
					best_fit = i;
				}
			}

			if (best_fit > -1) {
				width = vidmodes[best_fit]->hdisplay;
				height = vidmodes[best_fit]->vdisplay;

				// change to the mode
				XF86VidModeSwitchToMode(dpy, scrnum, vidmodes[best_fit]);
				vidmode_active = true;

				// Move the viewport to top left
				XF86VidModeSetViewPort(dpy, scrnum, 0, 0);
			}
			else
			{
				Com_Printf("XFree86-VidModeExtension: No acceptable modes found, going windowed mode\n");
				if(num_vidmodes)
					XFree(vidmodes);
			}
		}
	}

	/* window attributes */
	attr.background_pixel = 0;
	attr.border_pixel = 0;
	attr.colormap = XCreateColormap(dpy, root, visinfo->visual, AllocNone);
	attr.event_mask = X_MASK;
	if (vidmode_active) {
		mask = CWBackPixel | CWColormap | CWSaveUnder | CWBackingStore | 
			CWEventMask | CWOverrideRedirect;
		attr.override_redirect = True;
		attr.backing_store = NotUseful;
		attr.save_under = False;
	} else
		mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

	win = XCreateWindow(dpy, root, 0, 0, width, height,
						0, visinfo->depth, InputOutput,
						visinfo->visual, mask, &attr);


	sizehints = XAllocSizeHints();
	if (sizehints) {
		sizehints->min_width = sizehints->max_width = width;
		sizehints->min_height = sizehints->max_height = height;
		sizehints->flags = PMinSize | PMaxSize;

		XSetWMNormalHints (dpy, win, sizehints);
		XFree (sizehints);
	}

	XStoreName(dpy, win, APPLICATION);
	XSetIconName(dpy, win, APPLICATION);

	wmDeleteWindow = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(dpy, win, &wmDeleteWindow, 1);

	XMapWindow(dpy, win);

	if (vidmode_active) {
		XMoveWindow(dpy, win, 0, 0);
		XRaiseWindow(dpy, win);
	}

	XFlush(dpy);

	ctx = qglXCreateContext(dpy, visinfo, NULL, True);

	qglXMakeCurrent(dpy, win, ctx);

	*pwidth = width;
	*pheight = height;

	// let the sound and input subsystems know about the new window
	VID_NewWindow (width, height);

	if (fullscreen && !vidmode_active)
		return rserr_invalid_fullscreen;

	return rserr_ok;
}

void VID_Minimize_f(void) {

	if (vidmode_active) {
		XSetWindowAttributes attr;
		Window root = RootWindow(dpy, scrnum);
		unsigned long mask;

		attr.background_pixel = 0;
		attr.border_pixel = 0;
		attr.event_mask = X_MASK;
		mask = CWBackPixel | CWBorderPixel | CWEventMask;
		
		minimized_window = XCreateWindow(dpy, root, 0, 0, 1, 1,0, CopyFromParent, InputOutput, CopyFromParent, mask, &attr);
		XMapWindow(dpy, minimized_window);
		XIconifyWindow(dpy, minimized_window, scrnum);

		XStoreName(dpy, minimized_window, APPLICATION);
		XSetIconName(dpy, minimized_window, APPLICATION);

		XUnmapWindow(dpy, win);
		XF86VidModeSwitchToMode(dpy, scrnum, vidmodes[0]);
	} else
		XIconifyWindow(dpy, win, scrnum);
}

/*
** GLimp_Shutdown
**
** This routine does all OS specific shutdown procedures for the OpenGL
** subsystem.  Under OpenGL this means NULLing out the current DC and
** HGLRC, deleting the rendering context, and releasing the DC acquired
** for the window.  The state structure is also nulled out.
**
*/
void GLimp_Shutdown( void )
{
	IN_DeactivateMouse();
	mouse_active = false;
	dgamouse = false;
	win_x = win_y = 0;
	vid_minimized = 0;

	if (dpy) {
		if (ctx)
			qglXDestroyContext(dpy, ctx);
		if (win)
			XDestroyWindow(dpy, win);
		if (vidmode_active) {
			XF86VidModeSwitchToMode(dpy, scrnum, vidmodes[0]);
			XFree(vidmodes);
		}

		XCloseDisplay(dpy);
	}
	vidmode_active = false;
	dpy = NULL;
	win = 0;
	ctx = NULL;
}

/*
** GLimp_Init
**
** This routine is responsible for initializing the OS specific portions
** of OpenGL.  
*/
int GLimp_Init( void *hinstance, void *wndproc )
{
	static qboolean firstTime = true;

	InitSig();

	if(firstTime) {
		Cmd_AddCommand ("vid_minimize", VID_Minimize_f);
		firstTime = false;
	}

	if ( glw_state.OpenGLLib)
		return true;

	
	return false;
}

/*
** GLimp_BeginFrame
*/
void GLimp_BeginFrame( float camera_seperation )
{
}

/*
** GLimp_EndFrame
** 
** Responsible for doing a swapbuffers and possibly for other stuff
** as yet to be determined.  Probably better not to make this a GLimp
** function and instead do a call to GLimp_SwapBuffers.
*/
void GLimp_EndFrame (void)
{
	qglFlush();
	qglXSwapBuffers(dpy, win);
}

/*
** GLimp_AppActivate
*/
void GLimp_AppActivate( qboolean active )
{
}

