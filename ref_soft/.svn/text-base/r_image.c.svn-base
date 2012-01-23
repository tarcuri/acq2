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

#include "r_local.h"


#define	MAX_RIMAGES	1024
static image_t	r_images[MAX_RIMAGES];
static int		numr_images;

#define IMAGES_HASH_SIZE	64
static image_t	*images_hash[IMAGES_HASH_SIZE];

/*
===============
R_ImageList_f
===============
*/
void	R_ImageList_f (void)
{
	int		i;
	image_t	*image;
	int		texels;

	Com_Printf ("------------------\n");
	texels = 0;

	for (i=0, image=r_images ; i<numr_images ; i++, image++)
	{
		if (image->registration_sequence <= 0)
			continue;
		texels += image->width*image->height;
		switch (image->type)
		{
		case it_skin:
			Com_Printf ("M");
			break;
		case it_sprite:
			Com_Printf ("S");
			break;
		case it_wall:
			Com_Printf ("W");
			break;
		case it_pic:
			Com_Printf ("P");
			break;
		default:
			Com_Printf (" ");
			break;
		}

		Com_Printf (" %3i %3i : %s%s\n",
			image->width, image->height, image->name);
	}
	Com_Printf ("Total texel count: %i\n", texels);
}


/*
=================================================================

PCX LOADING

=================================================================
*/

/*
==============
LoadPCX
==============
*/
void LoadPCX (const char *filename, byte **pic, byte **palette, int *width, int *height)
{
	byte	*raw;
	pcx_t	*pcx;
	int		x, y;
	int		len;
	int		dataByte, runLength;
	byte	*out, *pix;

	*pic = NULL;

	//
	// load the file
	//
	len = FS_LoadFile (filename, (void **)&raw);
	if (!raw)
		return;

	//
	// parse the PCX file
	//
	pcx = (pcx_t *)raw;

    pcx->xmin = LittleShort(pcx->xmin);
    pcx->ymin = LittleShort(pcx->ymin);
    pcx->xmax = LittleShort(pcx->xmax);
    pcx->ymax = LittleShort(pcx->ymax);
    pcx->hres = LittleShort(pcx->hres);
    pcx->vres = LittleShort(pcx->vres);
    pcx->bytes_per_line = LittleShort(pcx->bytes_per_line);
    pcx->palette_type = LittleShort(pcx->palette_type);

	raw = &pcx->data;

	if (pcx->manufacturer != 0x0a
		|| pcx->version != 5
		|| pcx->encoding != 1
		|| pcx->bits_per_pixel != 8
		|| pcx->xmax >= 640
		|| pcx->ymax >= 480)
	{
		Com_Printf ("Bad pcx file %s\n", filename);
		FS_FreeFile ((void *)pcx);
		return;
	}

	out = malloc ( (pcx->ymax+1) * (pcx->xmax+1) );

	*pic = out;

	pix = out;

	if (palette)
	{
		*palette = malloc(768);
		memcpy (*palette, (byte *)pcx + len - 768, 768);
	}

	if (width)
		*width = pcx->xmax+1;
	if (height)
		*height = pcx->ymax+1;

	for (y=0 ; y<=pcx->ymax ; y++, pix += pcx->xmax+1)
	{
		for (x=0 ; x<=pcx->xmax ; )
		{
			dataByte = *raw++;

			if((dataByte & 0xC0) == 0xC0)
			{
				runLength = dataByte & 0x3F;
				dataByte = *raw++;
			}
			else
				runLength = 1;

			while(runLength-- > 0)
				pix[x++] = dataByte;
		}

	}

	if ( raw - (byte *)pcx > len)
	{
		Com_DPrintf ( "LoadPCX: file %s was malformed", filename);
		free (*pic);
		*pic = NULL;
	}

	FS_FreeFile ((void *)pcx);
}


//=======================================================

image_t *R_FindFreeImage (void)
{
	image_t		*image;
	int			i;

	// find a free image_t
	for (i=0, image=r_images ; i<numr_images ; i++,image++)
	{
		if (!image->registration_sequence)
			break;
	}
	if (i == numr_images)
	{
		if (numr_images == MAX_RIMAGES)
			Com_Error (ERR_DROP, "MAX_RIMAGES");
		numr_images++;
	}
	image = &r_images[i];

	return image;
}

/*
================
GL_LoadPic

================
*/
image_t *GL_LoadPic (const char *name, byte *pic, int width, int height, imagetype_t type)
{
	image_t		*image;
	int			i, c, b;

	image = R_FindFreeImage ();

	Q_strncpyz (image->name, name, sizeof(image->name));
	image->registration_sequence = registration_sequence;

	image->width = width;
	image->height = height;
	image->type = type;

	c = width*height;
	image->pixels[0] = malloc (c);
	image->transparent = false;
	for (i=0 ; i<c ; i++)
	{
		b = pic[i];
		if (b == 255)
			image->transparent = true;
		image->pixels[0][i] = b;
	}

	return image;
}

/*
================
R_LoadWal
================
*/
image_t *R_LoadWal (const char *name)
{
	miptex_t	*mt;
	int			ofs;
	image_t		*image;
	int			size;

	FS_LoadFile (name, (void **)&mt);
	if (!mt)
	{
		Com_Printf ("R_LoadWal: can't load %s\n", name);
		return NULL;
	}

	image = R_FindFreeImage ();
	strcpy (image->name, name);
	image->width = LittleLong (mt->width);
	image->height = LittleLong (mt->height);
	image->type = it_wall;
	image->registration_sequence = registration_sequence;

	size = image->width*image->height * (256+64+16+4)/256;
	image->pixels[0] = malloc (size);
	image->pixels[1] = image->pixels[0] + image->width*image->height;
	image->pixels[2] = image->pixels[1] + image->width*image->height/4;
	image->pixels[3] = image->pixels[2] + image->width*image->height/16;

	ofs = LittleLong (mt->offsets[0]);
	memcpy ( image->pixels[0], (byte *)mt + ofs, size);

	FS_FreeFile ((void *)mt);

	return image;
}

/*
===============
R_FindImage

Finds or loads the given image
===============
*/
image_t	*R_FindImage (const char *name, imagetype_t type)
{
	image_t	*image;
	int		i, len = 0;
	byte	*pic, *palette;
	int		width, height;
	char	pathname[MAX_QPATH];
	unsigned int hash;

	if (!name || !name[0])
		return NULL;	// Com_Error (ERR_DROP, "R_FindImage: NULL name");

	for( i = ( name[0] == '/' || name[0] == '\\' ); name[i] && (len < sizeof(pathname)-5); i++ ) {
		if( name[i] == '\\' ) 
			pathname[len++] = '/';
		else
			pathname[len++] = name[i];
	}

	if (len<5)
		return NULL;	// Com_Error (ERR_DROP, "R_FindImage: bad name: %s", name);

	pathname[len] = 0;

	hash = Com_HashKey(pathname, IMAGES_HASH_SIZE);
	// look for it
	for (image = images_hash[hash]; image; image = image->hashNext)
	{
		if (!strcmp(pathname, image->name))
		{
			image->registration_sequence = registration_sequence;
			return image;
		}
	}

	// load the pic from disk
	pic = NULL;
	palette = NULL;

	if (!strcmp(pathname+len-4, ".pcx"))
	{
		LoadPCX (pathname, &pic, &palette, &width, &height);
		if (!pic)
			return NULL;	// Com_Error (ERR_DROP, "R_FindImage: can't load %s", name);
		image = GL_LoadPic (pathname, pic, width, height, type);
	}
	else if (!strcmp(pathname+len-4, ".wal"))
	{
		image = R_LoadWal (pathname);
		if(!image)
			return r_notexture_mip;
	}
	else
		return NULL;	// Com_Error (ERR_DROP, "R_FindImage: bad extension on: %s", name);

	if (pic)
		free(pic);
	if (palette)
		free(palette);

	image->hashNext = images_hash[hash];
	images_hash[hash] = image;

	return image;
}



/*
===============
R_RegisterSkin
===============
*/
struct image_s *R_RegisterSkin (const char *name)
{
	return R_FindImage (name, it_skin);
}


/*
================
R_FreeUnusedImages

Any image that was not touched on this registration sequence
will be freed.
================
*/
void R_FreeUnusedImages (void)
{
	int		i;
	image_t	*image, *entry, **back;
	unsigned int hash;

	for (i=0, image=r_images ; i<numr_images ; i++, image++)
	{
		if (image->registration_sequence == registration_sequence)
		{
			Com_PageInMemory ((byte *)image->pixels[0], image->width*image->height);
			continue;		// used this sequence
		}
		if (!image->registration_sequence)
			continue;		// free texture
		if (image->type == it_pic)
			continue;		// don't free pics

		hash = Com_HashKey(image->name, IMAGES_HASH_SIZE);
		// delete it from hash table
		for( back=&images_hash[hash], entry=images_hash[hash]; entry; back=&entry->hashNext, entry=entry->hashNext ) {
			if( entry == image ) {
				*back = entry->hashNext;
				break;
			}
		}
		//if( !entry )
		//	Com_Error (ERR_FATAL, "R_FreeUnusedImages: %s not found in hash array", image->name );

		// free it
		free (image->pixels[0]);	// the other mip levels just follow
		memset (image, 0, sizeof(*image));
	}
}



/*
===============
R_InitImages
===============
*/
void	R_InitImages (void)
{
	registration_sequence = 1;
}

/*
===============
R_ShutdownImages
===============
*/
void	R_ShutdownImages (void)
{
	int		i;
	image_t	*image;

	for (i=0, image=r_images ; i<numr_images ; i++, image++)
	{
		if (!image->registration_sequence)
			continue;		// free texture
		// free it
		free (image->pixels[0]);	// the other mip levels just follow
	}
	numr_images = 0;
	memset(r_images, 0, sizeof(r_images));
	memset( images_hash, 0, sizeof(images_hash) );
}

