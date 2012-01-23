#
# Quake2 Makefile for Linux 2.0
#
# Nov '97 by Zoid <zoid@idsoftware.com>
#
# ELF only
#

# Here are your build options:
# (Note: not all options are available for all platforms).
BUILD_GLX=NO		# X11 GLX client
BUILD_X11=NO		# X11 software client
BUILD_GAME=YES		# Build game library
BUILD_EVDEV=NO		# Build evdev mouse input support
BUILD_XMMS=NO		# Buildin xmms remote commands
BUILD_MPD=NO		# Buildin mpd remote commands, cant use with BUILD_XMMS
BUILD_SDLSOUND=YES	# Using sdl for sounds
BUILD_SDL=YES		# Using sdl for keyboard/mouse/windows for both sw/gl
BUILD_HTTP=YES		# Build support for http downloads. You need libcurl.
BUILD_OPENAL=YES	# Build support for openAL sounds
#--------------------------------------------------------

# Check OS type.
PLATFORM=$(shell uname -s|tr A-Z a-z)

ifneq ($(PLATFORM),linux)
ifneq ($(PLATFORM),freebsd)
ifneq ($(PLATFORM),darwin)
  $(error OS $(PLATFORM) is currently not supported)
endif
endif
endif

# Check arch type
ARCH:=$(shell uname -m | sed -e s/i.86/i386/ -e s/sun4u/sparc/ -e s/sparc64/sparc/ -e s/arm.*/arm/ -e s/sa110/arm/ -e s/alpha/axp/)
 
#COMPILATION TOOLS
CC=gcc

#BASE CFLAGS
BASE_CFLAGS= -funsigned-char -pipe
ifneq ($(ARCH),i386)
 BASE_CFLAGS+=-DC_ONLY
endif

ifeq ($(PLATFORM),darwin)
 BASE_CFLAGS += -DMACOS_X -I/Developer/SDKs/MacOSX10.4u.sdk/usr/X11R6/include -I../jpeg -I/Library/Frameworks/SDL.framework/Headers
 # more sane defaults for OSX
 BUILD_SDLSOUND=YES
 BUILD_OPENAL=NO
 BUILD_XMMS=NO
endif

ifeq ($(strip $(BUILD_XMMS)),YES)
 BASE_CFLAGS += -DWITH_XMMS `glib-config --cflags`
else
 ifeq ($(strip $(BUILD_MPD)),YES)
  BASE_CFLAGS += -DWITH_MPD
 endif
endif

ifeq ($(strip $(BUILD_HTTP)),YES)
 BASE_CFLAGS += -DUSE_CURL
endif
 
 ifeq ($(strip $(BUILD_SDL)),YES)
  BASE_CFLAGS += -DUSE_SDL
 endif
 
ifeq ($(strip $(BUILD_EVDEV)),YES)
 BASE_CFLAGS += -DWITH_EVDEV
endif

ifeq ($(strip $(BUILD_OPENAL)),YES)
 BASE_CFLAGS += -DUSE_OPENAL
endif

DEBUG_CFLAGS=$(BASE_CFLAGS) -g -Wall
RELEASE_CFLAGS=$(BASE_CFLAGS) -Wall -O2 -DNDEBUG

#BASE LDFLAGS
ifeq ($(PLATFORM),freebsd)
  LDFLAGS=-lm
else
  ifeq ($(PLATFORM),linux)
    LDFLAGS=-lm -ldl
  endif
endif

ifeq ($(strip $(BUILD_HTTP)),YES)
 LDFLAGS += -lcurl
endif

ifeq ($(PLATFORM),linux)
 SDL_CONFIG=sdl-config
endif
ifeq ($(PLATFORM),freebsd)
 SDL_CONFIG=sdl11-config
endif
ifeq ($(PLATFORM),darwin)
 SDL_CONFIG=/sw/bin/sdl-config
endif

SDLCFLAGS=$(shell $(SDL_CONFIG) --cflags)
SDLLDFLAGS=$(shell $(SDL_CONFIG) --libs)

ifeq ($(strip $(BUILD_SDLSOUND)),YES)
 LDFLAGS += $(SDLLDFLAGS)
else
 ifeq ($(strip $(BUILD_SDL)),YES)
  LDFLAGS += $(SDLLDFLAGS)
 endif
endif

# game dll
SHLIBEXT=so
GAME_NAME=game$(ARCH).$(SHLIBEXT)

SHLIBCFLAGS=-fPIC
SHLIBLDFLAGS=-shared

ifeq ($(PLATFORM),darwin)
 LDFLAGS += -L../jpeg -framework SDL
 SHLIBCFLAGS+=-fno-common
 SHLIBLDFLAGS=-dynamiclib
 GAME_NAME=game$(ARCH).dylib
endif

LIBDIR=lib

ifeq ($(ARCH),x86_64)
 LIBDIR=lib64
else
ifeq ($(ARCH),ppc64)
 LIBDIR=lib64
else
ifeq ($(ARCH),s390x)
 LIBDIR=lib64
endif
endif
endif

DO_SHLIB_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<

#FOR X11 BUILDS
XCFLAGS=-I/usr/X11R6/include
XLDFLAGS=-L/usr/X11R6/$(LIBDIR) -lX11 -lXext -lz

DO_CC=$(CC) $(CFLAGS) -o $@ -c $<
DO_AS=$(CC) $(CFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<

#FOR GLX BUILDS
GLXCFLAGS=$(XCFLAGS)
ifeq ($(PLATFORM),darwin)
GLXLDFLAGS=$(XLDFLAGS) -ljpeg
else
GLXLDFLAGS=$(XLDFLAGS) -ljpeg -lpng
endif
DO_GL_CC=$(CC) $(CFLAGS) -DGL_QUAKE -o $@ -c $<
DO_GL_AS=$(CC) $(CFLAGS) -DGL_QUAKE -DELF -x assembler-with-cpp -o $@ -c $<

#############################################################################
# SETUP AND BUILD
#############################################################################

MOUNT_DIR=.

BUILD_DEBUG_DIR=debug$(ARCH)
BUILD_RELEASE_DIR=release$(ARCH)
CLIENT_DIR=$(MOUNT_DIR)/client
SERVER_DIR=$(MOUNT_DIR)/server
REF_SOFT_DIR=$(MOUNT_DIR)/ref_soft
REF_GL_DIR=$(MOUNT_DIR)/ref_gl
COMMON_DIR=$(MOUNT_DIR)/qcommon
LINUX_DIR=$(MOUNT_DIR)/linux
GAME_DIR=$(MOUNT_DIR)/game
UI_DIR=$(MOUNT_DIR)/ui


TARGETS=$(BUILDDIR)/cq2

ifeq ($(strip $(BUILD_X11)),YES)
 TARGETS += $(BUILDDIR)/cq2sw
endif

ifeq ($(strip $(BUILD_GAME)),YES)
 TARGETS += $(BUILDDIR)/$(GAME_NAME)
endif

.PHONY : targets debug release makedirs clean clean-debug clean-release clean2

all: release

debug:
	$(MAKE) targets BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

release:
	$(MAKE) targets BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(RELEASE_CFLAGS)"

targets: makedirs $(TARGETS)

makedirs:
	@if [ ! -d $(BUILDDIR) ];then mkdir $(BUILDDIR);fi
	@if [ ! -d $(BUILDDIR)/q2glx ];then mkdir $(BUILDDIR)/q2glx;fi
	@if [ ! -d $(BUILDDIR)/q2swx ];then mkdir $(BUILDDIR)/q2swx;fi
	@if [ ! -d $(BUILDDIR)/game ];then mkdir $(BUILDDIR)/game;fi
	
#############################################################################
# GLX CLIENT
#############################################################################

QUAKE2GLX_OBJS = \
	$(BUILDDIR)/q2glx/cl_cin.o \
	$(BUILDDIR)/q2glx/cl_ents.o \
	$(BUILDDIR)/q2glx/cl_fx.o \
	$(BUILDDIR)/q2glx/cl_input.o \
	$(BUILDDIR)/q2glx/cl_inv.o \
	$(BUILDDIR)/q2glx/cl_main.o \
	$(BUILDDIR)/q2glx/cl_parse.o \
	$(BUILDDIR)/q2glx/cl_pred.o \
	$(BUILDDIR)/q2glx/cl_tent.o \
	$(BUILDDIR)/q2glx/cl_scrn.o \
	$(BUILDDIR)/q2glx/cl_view.o \
	$(BUILDDIR)/q2glx/cl_newfx.o \
	$(BUILDDIR)/q2glx/console.o \
	$(BUILDDIR)/q2glx/keys.o \
	$(BUILDDIR)/q2glx/snd_dma.o \
	$(BUILDDIR)/q2glx/snd_mem.o \
	$(BUILDDIR)/q2glx/snd_mix.o \
	$(BUILDDIR)/q2glx/m_flash.o \
	$(BUILDDIR)/q2glx/cl_loc.o \
	$(BUILDDIR)/q2glx/cl_draw.o \
	$(BUILDDIR)/q2glx/cl_demo.o \
	$(BUILDDIR)/q2glx/cl_http.o \
	\
	$(BUILDDIR)/q2glx/cmd.o \
	$(BUILDDIR)/q2glx/cmodel.o \
	$(BUILDDIR)/q2glx/common.o \
	$(BUILDDIR)/q2glx/crc.o \
	$(BUILDDIR)/q2glx/cvar.o \
	$(BUILDDIR)/q2glx/files.o \
	$(BUILDDIR)/q2glx/md4.o \
	$(BUILDDIR)/q2glx/net.o \
	$(BUILDDIR)/q2glx/net_chan.o \
	$(BUILDDIR)/q2glx/pmove.o \
	$(BUILDDIR)/q2glx/q_shared.o \
	$(BUILDDIR)/q2glx/q_msg.o \
	\
	$(BUILDDIR)/q2glx/sv_ccmds.o \
	$(BUILDDIR)/q2glx/sv_ents.o \
	$(BUILDDIR)/q2glx/sv_game.o \
	$(BUILDDIR)/q2glx/sv_init.o \
	$(BUILDDIR)/q2glx/sv_main.o \
	$(BUILDDIR)/q2glx/sv_send.o \
	$(BUILDDIR)/q2glx/sv_user.o \
	$(BUILDDIR)/q2glx/sv_world.o \
	\
	$(BUILDDIR)/q2glx/q_shlinux.o \
	$(BUILDDIR)/q2glx/vid_so.o \
	$(BUILDDIR)/q2glx/sys_linux.o \
	$(BUILDDIR)/q2glx/glob.o \
	$(BUILDDIR)/q2glx/cd_linux.o \
	\
	$(BUILDDIR)/q2glx/qmenu.o \
	$(BUILDDIR)/q2glx/ui_addressbook.o \
	$(BUILDDIR)/q2glx/ui_atoms.o \
	$(BUILDDIR)/q2glx/ui_controls.o \
	$(BUILDDIR)/q2glx/ui_credits.o \
	$(BUILDDIR)/q2glx/ui_demos.o \
	$(BUILDDIR)/q2glx/ui_dmoptions.o \
	$(BUILDDIR)/q2glx/ui_download.o \
	$(BUILDDIR)/q2glx/ui_game.o \
	$(BUILDDIR)/q2glx/ui_joinserver.o \
	$(BUILDDIR)/q2glx/ui_keys.o \
	$(BUILDDIR)/q2glx/ui_loadsavegame.o \
	$(BUILDDIR)/q2glx/ui_main.o \
	$(BUILDDIR)/q2glx/ui_multiplayer.o \
	$(BUILDDIR)/q2glx/ui_newoptions.o \
	$(BUILDDIR)/q2glx/ui_playerconfig.o \
	$(BUILDDIR)/q2glx/ui_quit.o \
	$(BUILDDIR)/q2glx/ui_startserver.o \
	$(BUILDDIR)/q2glx/ui_video.o \
	$(BUILDDIR)/q2glx/ui_mp3.o \
	\
	$(BUILDDIR)/q2glx/gl_draw.o \
	$(BUILDDIR)/q2glx/gl_image.o \
	$(BUILDDIR)/q2glx/gl_light.o \
	$(BUILDDIR)/q2glx/gl_mesh.o \
	$(BUILDDIR)/q2glx/gl_model.o \
	$(BUILDDIR)/q2glx/gl_rmain.o \
	$(BUILDDIR)/q2glx/gl_rmisc.o \
	$(BUILDDIR)/q2glx/gl_rsurf.o \
	$(BUILDDIR)/q2glx/gl_warp.o \
	$(BUILDDIR)/q2glx/gl_decal.o \
	\
	$(BUILDDIR)/q2glx/qgl_linux.o \
	$(BUILDDIR)/q2glx/rw_linux.o \
	$(BUILDDIR)/q2glx/xmms.o \
	$(BUILDDIR)/q2glx/mpd.o \
	$(BUILDDIR)/q2glx/libmpdclient.o \
	$(BUILDDIR)/q2glx/snd_openal.o \
	\
	$(BUILDDIR)/q2glx/ioapi.o \
	$(BUILDDIR)/q2glx/unzip.o
	
ifeq ($(strip $(BUILD_SDL)),YES)
QUAKE2GLX_OBJS += \
	$(BUILDDIR)/q2glx/rw_sdl.o
else
QUAKE2GLX_OBJS += \
	$(BUILDDIR)/q2glx/gl_glx.o
endif

ifeq ($(strip $(BUILD_SDLSOUND)),YES)
QUAKE2GLX_OBJS += \
	$(BUILDDIR)/q2glx/snd_sdl.o
else
QUAKE2GLX_OBJS += \
	$(BUILDDIR)/q2glx/snd_linux.o \
	$(BUILDDIR)/q2glx/snd_oss.o \
	$(BUILDDIR)/q2glx/snd_alsa.o
endif

ifeq ($(ARCH),i386)
QUAKE2GLX_OBJS += \
	$(BUILDDIR)/q2glx/snd_mixa.o
endif

$(BUILDDIR)/cq2 : $(QUAKE2GLX_OBJS)
	$(CC) -o $@ $(QUAKE2GLX_OBJS) $(LDFLAGS) $(GLXLDFLAGS)

$(BUILDDIR)/q2glx/%.o :    $(CLIENT_DIR)/%.c
	$(DO_GL_CC)

$(BUILDDIR)/q2glx/%.o :    $(GAME_DIR)/%.c
	$(DO_GL_CC)

$(BUILDDIR)/q2glx/%.o :    $(COMMON_DIR)/%.c
	$(DO_GL_CC)

$(BUILDDIR)/q2glx/%.o :    $(SERVER_DIR)/%.c
	$(DO_GL_CC)

$(BUILDDIR)/q2glx/%.o :    $(UI_DIR)/%.c
	$(DO_GL_CC)

$(BUILDDIR)/q2glx/%.o :    $(REF_GL_DIR)/%.c
	$(DO_GL_CC)

$(BUILDDIR)/q2glx/q_shlinux.o :  $(LINUX_DIR)/q_shlinux.c
	$(DO_GL_CC)

$(BUILDDIR)/q2glx/vid_so.o :     $(LINUX_DIR)/vid_so.c
	$(DO_GL_CC)

$(BUILDDIR)/q2glx/sys_linux.o :  $(LINUX_DIR)/sys_linux.c
	$(DO_GL_CC)

$(BUILDDIR)/q2glx/glob.o :       $(LINUX_DIR)/glob.c
	$(DO_GL_CC)

$(BUILDDIR)/q2glx/net_udp.o :    $(LINUX_DIR)/net_udp.c
	$(DO_GL_CC)

$(BUILDDIR)/q2glx/cd_linux.o :   $(LINUX_DIR)/cd_linux.c
	$(DO_GL_CC)

$(BUILDDIR)/q2glx/snd_linux.o :  $(LINUX_DIR)/snd_linux.c
	$(DO_GL_CC)

$(BUILDDIR)/q2glx/snd_mixa.o :   $(LINUX_DIR)/snd_mixa.s
	$(DO_GL_AS)

$(BUILDDIR)/q2glx/rw_linux.o :   $(LINUX_DIR)/rw_linux.c
	$(DO_GL_CC) $(GLXCFLAGS)

$(BUILDDIR)/q2glx/qgl_linux.o :  $(LINUX_DIR)/qgl_linux.c
	$(DO_GL_CC)

$(BUILDDIR)/q2glx/gl_glx.o :     $(LINUX_DIR)/gl_glx.c
	$(DO_GL_CC) $(GLXCFLAGS)
	
$(BUILDDIR)/q2glx/rw_sdl.o :     $(LINUX_DIR)/rw_sdl.c
	$(DO_GL_CC) $(SDLCFLAGS)
	
$(BUILDDIR)/q2glx/snd_oss.o :    $(LINUX_DIR)/snd_oss.c
	$(DO_GL_CC)
	
$(BUILDDIR)/q2glx/snd_alsa.o :   $(LINUX_DIR)/snd_alsa.c
	$(DO_GL_CC)

$(BUILDDIR)/q2glx/snd_sdl.o :    $(LINUX_DIR)/snd_sdl.c
	$(DO_GL_CC) $(SDLCFLAGS)
	
$(BUILDDIR)/q2glx/xmms.o :       $(LINUX_DIR)/xmms.c
	$(DO_GL_CC)
	
$(BUILDDIR)/q2glx/mpd.o :		 $(LINUX_DIR)/mpd.c
	$(DO_GL_CC)

$(BUILDDIR)/q2glx/libmpdclient.o : $(LINUX_DIR)/libmpdclient.c
	$(DO_GL_CC)

$(BUILDDIR)/q2glx/ioapi.o : include/minizip/ioapi.c
	$(DO_GL_CC)

$(BUILDDIR)/q2glx/unzip.o : include/minizip/unzip.c
	$(DO_GL_CC)
	
#############################################################################
# X11 CLIENT
#############################################################################

QUAKE2SWX_OBJS = \
	$(BUILDDIR)/q2swx/cl_cin.o \
	$(BUILDDIR)/q2swx/cl_ents.o \
	$(BUILDDIR)/q2swx/cl_fx.o \
	$(BUILDDIR)/q2swx/cl_input.o \
	$(BUILDDIR)/q2swx/cl_inv.o \
	$(BUILDDIR)/q2swx/cl_main.o \
	$(BUILDDIR)/q2swx/cl_parse.o \
	$(BUILDDIR)/q2swx/cl_pred.o \
	$(BUILDDIR)/q2swx/cl_tent.o \
	$(BUILDDIR)/q2swx/cl_scrn.o \
	$(BUILDDIR)/q2swx/cl_view.o \
	$(BUILDDIR)/q2swx/cl_newfx.o \
	$(BUILDDIR)/q2swx/console.o \
	$(BUILDDIR)/q2swx/keys.o \
	$(BUILDDIR)/q2swx/snd_dma.o \
	$(BUILDDIR)/q2swx/snd_mem.o \
	$(BUILDDIR)/q2swx/snd_mix.o \
	$(BUILDDIR)/q2swx/m_flash.o \
	$(BUILDDIR)/q2swx/cl_loc.o \
	$(BUILDDIR)/q2swx/cl_draw.o \
	$(BUILDDIR)/q2swx/cl_demo.o \
	$(BUILDDIR)/q2swx/cl_http.o \
	\
	$(BUILDDIR)/q2swx/cmd.o \
	$(BUILDDIR)/q2swx/cmodel.o \
	$(BUILDDIR)/q2swx/common.o \
	$(BUILDDIR)/q2swx/crc.o \
	$(BUILDDIR)/q2swx/cvar.o \
	$(BUILDDIR)/q2swx/files.o \
	$(BUILDDIR)/q2swx/md4.o \
	$(BUILDDIR)/q2swx/net.o \
	$(BUILDDIR)/q2swx/net_chan.o \
	$(BUILDDIR)/q2swx/pmove.o \
	$(BUILDDIR)/q2swx/q_shared.o \
	$(BUILDDIR)/q2swx/q_msg.o \
	\
	$(BUILDDIR)/q2swx/sv_ccmds.o \
	$(BUILDDIR)/q2swx/sv_ents.o \
	$(BUILDDIR)/q2swx/sv_game.o \
	$(BUILDDIR)/q2swx/sv_init.o \
	$(BUILDDIR)/q2swx/sv_main.o \
	$(BUILDDIR)/q2swx/sv_send.o \
	$(BUILDDIR)/q2swx/sv_user.o \
	$(BUILDDIR)/q2swx/sv_world.o \
	\
	$(BUILDDIR)/q2swx/q_shlinux.o \
	$(BUILDDIR)/q2swx/vid_so.o \
	$(BUILDDIR)/q2swx/sys_linux.o \
	$(BUILDDIR)/q2swx/glob.o \
	$(BUILDDIR)/q2swx/cd_linux.o \
	\
	$(BUILDDIR)/q2swx/qmenu.o \
	$(BUILDDIR)/q2swx/ui_addressbook.o \
	$(BUILDDIR)/q2swx/ui_atoms.o \
	$(BUILDDIR)/q2swx/ui_controls.o \
	$(BUILDDIR)/q2swx/ui_credits.o \
	$(BUILDDIR)/q2swx/ui_demos.o \
	$(BUILDDIR)/q2swx/ui_dmoptions.o \
	$(BUILDDIR)/q2swx/ui_download.o \
	$(BUILDDIR)/q2swx/ui_game.o \
	$(BUILDDIR)/q2swx/ui_joinserver.o \
	$(BUILDDIR)/q2swx/ui_keys.o \
	$(BUILDDIR)/q2swx/ui_loadsavegame.o \
	$(BUILDDIR)/q2swx/ui_main.o \
	$(BUILDDIR)/q2swx/ui_multiplayer.o \
	$(BUILDDIR)/q2swx/ui_newoptions.o \
	$(BUILDDIR)/q2swx/ui_playerconfig.o \
	$(BUILDDIR)/q2swx/ui_quit.o \
	$(BUILDDIR)/q2swx/ui_startserver.o \
	$(BUILDDIR)/q2swx/ui_video.o \
	$(BUILDDIR)/q2swx/ui_mp3.o \
	\
	$(BUILDDIR)/q2swx/r_aclip.o \
	$(BUILDDIR)/q2swx/r_alias.o \
	$(BUILDDIR)/q2swx/r_bsp.o \
	$(BUILDDIR)/q2swx/r_draw.o \
	$(BUILDDIR)/q2swx/r_edge.o \
	$(BUILDDIR)/q2swx/r_image.o \
	$(BUILDDIR)/q2swx/r_light.o \
	$(BUILDDIR)/q2swx/r_main.o \
	$(BUILDDIR)/q2swx/r_misc.o \
	$(BUILDDIR)/q2swx/r_model.o \
	$(BUILDDIR)/q2swx/r_part.o \
	$(BUILDDIR)/q2swx/r_poly.o \
	$(BUILDDIR)/q2swx/r_polyse.o \
	$(BUILDDIR)/q2swx/r_rast.o \
	$(BUILDDIR)/q2swx/r_scan.o \
	$(BUILDDIR)/q2swx/r_sprite.o \
	$(BUILDDIR)/q2swx/r_surf.o \
	\
	$(BUILDDIR)/q2swx/rw_linux.o \
	$(BUILDDIR)/q2swx/xmms.o \
	$(BUILDDIR)/q2swx/mpd.o \
	$(BUILDDIR)/q2swx/libmpdclient.o \
	$(BUILDDIR)/q2swx/snd_openal.o \
	\
	$(BUILDDIR)/q2swx/ioapi.o \
	$(BUILDDIR)/q2swx/unzip.o
	
ifeq ($(strip $(BUILD_SDL)),YES)
QUAKE2SWX_OBJS += \
	$(BUILDDIR)/q2swx/rw_sdl.o
else
QUAKE2SWX_OBJS += \
	$(BUILDDIR)/q2swx/rw_x11.o
endif

ifeq ($(strip $(BUILD_SDLSOUND)),YES)
QUAKE2SWX_OBJS += \
	$(BUILDDIR)/q2swx/snd_sdl.o
else
QUAKE2SWX_OBJS += \
	$(BUILDDIR)/q2swx/snd_linux.o \
	$(BUILDDIR)/q2swx/snd_oss.o \
	$(BUILDDIR)/q2swx/snd_alsa.o
endif

ifeq ($(ARCH),i386)
QUAKE2SWX_OBJS += \
	$(BUILDDIR)/q2swx/snd_mixa.o \
	$(BUILDDIR)/q2swx/r_aclipa.o \
	$(BUILDDIR)/q2swx/r_draw16.o \
	$(BUILDDIR)/q2swx/r_drawa.o \
	$(BUILDDIR)/q2swx/r_edgea.o \
	$(BUILDDIR)/q2swx/r_scana.o \
	$(BUILDDIR)/q2swx/r_spr8.o \
	$(BUILDDIR)/q2swx/r_surf8.o \
	$(BUILDDIR)/q2swx/math.o \
	$(BUILDDIR)/q2swx/d_polysa.o \
	$(BUILDDIR)/q2swx/r_varsa.o \
	$(BUILDDIR)/q2swx/sys_dosa.o
endif


$(BUILDDIR)/cq2sw :  $(QUAKE2SWX_OBJS)
	$(CC) -o $@ $(QUAKE2SWX_OBJS) $(LDFLAGS) $(XLDFLAGS)


$(BUILDDIR)/q2swx/%.o :    $(CLIENT_DIR)/%.c
	$(DO_CC)

$(BUILDDIR)/q2swx/%.o :    $(GAME_DIR)/%.c
	$(DO_CC)

$(BUILDDIR)/q2swx/%.o :    $(COMMON_DIR)/%.c
	$(DO_CC)

$(BUILDDIR)/q2swx/%.o :    $(SERVER_DIR)/%.c
	$(DO_CC)

$(BUILDDIR)/q2swx/%.o :    $(UI_DIR)/%.c
	$(DO_CC)

$(BUILDDIR)/q2swx/%.o :    $(REF_SOFT_DIR)/%.c
	$(DO_CC)

$(BUILDDIR)/q2swx/q_shlinux.o :  $(LINUX_DIR)/q_shlinux.c
	$(DO_CC)

$(BUILDDIR)/q2swx/vid_so.o :     $(LINUX_DIR)/vid_so.c
	$(DO_CC)

$(BUILDDIR)/q2swx/sys_linux.o :  $(LINUX_DIR)/sys_linux.c
	$(DO_CC)

$(BUILDDIR)/q2swx/glob.o :       $(LINUX_DIR)/glob.c
	$(DO_CC)

$(BUILDDIR)/q2swx/net_udp.o :    $(LINUX_DIR)/net_udp.c
	$(DO_CC)

$(BUILDDIR)/q2swx/cd_linux.o :   $(LINUX_DIR)/cd_linux.c
	$(DO_CC)

$(BUILDDIR)/q2swx/snd_linux.o :  $(LINUX_DIR)/snd_linux.c
	$(DO_CC)

$(BUILDDIR)/q2swx/rw_linux.o :   $(LINUX_DIR)/rw_linux.c
	$(DO_CC) $(XCFLAGS)

$(BUILDDIR)/q2swx/rw_x11.o :     $(LINUX_DIR)/rw_x11.c
	$(DO_CC) $(XCFLAGS)
	
$(BUILDDIR)/q2swx/rw_sdl.o :     $(LINUX_DIR)/rw_sdl.c
	$(DO_CC) $(SDLCFLAGS)

$(BUILDDIR)/q2swx/snd_oss.o :    $(LINUX_DIR)/snd_oss.c
	$(DO_CC)
		
$(BUILDDIR)/q2swx/snd_alsa.o :   $(LINUX_DIR)/snd_alsa.c
	$(DO_CC)

$(BUILDDIR)/q2swx/snd_sdl.o :    $(LINUX_DIR)/snd_sdl.c
	$(DO_CC) $(SDLCFLAGS)
	
$(BUILDDIR)/q2swx/xmms.o :       $(LINUX_DIR)/xmms.c
	$(DO_CC)

$(BUILDDIR)/q2swx/mpd.o :        $(LINUX_DIR)/mpd.c
	$(DO_CC)

$(BUILDDIR)/q2swx/libmpdclient.o : $(LINUX_DIR)/libmpdclient.c
	$(DO_CC)

$(BUILDDIR)/q2swx/%.o :          $(LINUX_DIR)/%.s
	$(DO_AS)

$(BUILDDIR)/q2swx/ioapi.o : include/minizip/ioapi.c
	$(DO_CC)

$(BUILDDIR)/q2swx/unzip.o : include/minizip/unzip.c
	$(DO_CC)
	
#############################################################################
# GAME
#############################################################################

GAME_OBJS = \
	$(BUILDDIR)/game/q_shared.o \
	$(BUILDDIR)/game/g_ai.o \
	$(BUILDDIR)/game/p_client.o \
	$(BUILDDIR)/game/g_cmds.o \
	$(BUILDDIR)/game/g_svcmds.o \
	$(BUILDDIR)/game/g_chase.o \
	$(BUILDDIR)/game/g_combat.o \
	$(BUILDDIR)/game/g_func.o \
	$(BUILDDIR)/game/g_items.o \
	$(BUILDDIR)/game/g_main.o \
	$(BUILDDIR)/game/g_misc.o \
	$(BUILDDIR)/game/g_monster.o \
	$(BUILDDIR)/game/g_phys.o \
	$(BUILDDIR)/game/g_save.o \
	$(BUILDDIR)/game/g_spawn.o \
	$(BUILDDIR)/game/g_target.o \
	$(BUILDDIR)/game/g_trigger.o \
	$(BUILDDIR)/game/g_turret.o \
	$(BUILDDIR)/game/g_utils.o \
	$(BUILDDIR)/game/g_weapon.o \
	$(BUILDDIR)/game/m_actor.o \
	$(BUILDDIR)/game/m_berserk.o \
	$(BUILDDIR)/game/m_boss2.o \
	$(BUILDDIR)/game/m_boss3.o \
	$(BUILDDIR)/game/m_boss31.o \
	$(BUILDDIR)/game/m_boss32.o \
	$(BUILDDIR)/game/m_brain.o \
	$(BUILDDIR)/game/m_chick.o \
	$(BUILDDIR)/game/m_flipper.o \
	$(BUILDDIR)/game/m_float.o \
	$(BUILDDIR)/game/m_flyer.o \
	$(BUILDDIR)/game/m_gladiator.o \
	$(BUILDDIR)/game/m_gunner.o \
	$(BUILDDIR)/game/m_hover.o \
	$(BUILDDIR)/game/m_infantry.o \
	$(BUILDDIR)/game/m_insane.o \
	$(BUILDDIR)/game/m_medic.o \
	$(BUILDDIR)/game/m_move.o \
	$(BUILDDIR)/game/m_mutant.o \
	$(BUILDDIR)/game/m_parasite.o \
	$(BUILDDIR)/game/m_soldier.o \
	$(BUILDDIR)/game/m_supertank.o \
	$(BUILDDIR)/game/m_tank.o \
	$(BUILDDIR)/game/p_hud.o \
	$(BUILDDIR)/game/p_trail.o \
	$(BUILDDIR)/game/p_view.o \
	$(BUILDDIR)/game/p_weapon.o \
	$(BUILDDIR)/game/m_flash.o

$(BUILDDIR)/$(GAME_NAME) : $(GAME_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(GAME_OBJS)

$(BUILDDIR)/game/%.o :    $(GAME_DIR)/%.c
	$(DO_SHLIB_CC)

#############################################################################
# MISC
#############################################################################

clean: clean-debug clean-release

clean-debug:
	$(MAKE) clean2 BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean-release:
	$(MAKE) clean2 BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean2:
	rm -f $(QUAKE2GLX_OBJS)	$(QUAKE2SWX_OBJS) $(GAME_OBJS)

distclean:
	@-rm -rf $(BUILD_DEBUG_DIR) $(BUILD_RELEASE_DIR)
	@-rm -f `find . \( -not -type d \) -and \
		\( -name '*~' \) -type f -print`
