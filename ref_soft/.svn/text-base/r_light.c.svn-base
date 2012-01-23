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
// r_light.c

#include "r_local.h"

int	r_dlightframecount;

#define	DLIGHT_CUTOFF	0

/*
=============================================================================

DYNAMIC LIGHTS

=============================================================================
*/

/*
=============
R_MarkLights
=============
*/
void R_MarkLights (dlight_t *light, int bit, mnode_t *node)
{
	mplane_t	*splitplane;
	float		dist;
	msurface_t	*surf;
	int			i;
	
	if (node->contents != CONTENTS_NODE)
		return;

	splitplane = node->plane;

	if (splitplane->type < 3)
		dist = light->origin[splitplane->type] - splitplane->dist;
	else
		dist = DotProduct (light->origin, splitplane->normal) - splitplane->dist;
	
//=====
//PGM
	i = light->intensity;
	if (i < 0)
		i = -i;
//PGM
//=====

	if (dist > i-DLIGHT_CUTOFF)	// PGM (dist > light->intensity)
	{
		R_MarkLights (light, bit, node->children[0]);
		return;
	}
	if (dist < -i+DLIGHT_CUTOFF)	// PGM (dist < -light->intensity)
	{
		R_MarkLights (light, bit, node->children[1]);
		return;
	}
		
// mark the polygons
	surf = r_worldmodel->surfaces + node->firstsurface;
	for (i=0 ; i<node->numsurfaces ; i++, surf++)
	{
		if (surf->dlightframe != r_dlightframecount)
		{
			surf->dlightbits = 0;
			surf->dlightframe = r_dlightframecount;
		}
		surf->dlightbits |= bit;
	}

	R_MarkLights (light, bit, node->children[0]);
	R_MarkLights (light, bit, node->children[1]);
}


/*
=============
R_PushDlights
=============
*/
void R_PushDlights (void)
{
	int	i;
	dlight_t *l;
	vec3_t temp;
	vec3_t old_vpn, old_vup, old_vright;

	r_dlightframecount = r_framecount;

	if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2])
	{
		VectorCopy (vpn, old_vpn);
		VectorCopy (vup, old_vup);
		VectorCopy (vright, old_vright);
		AngleVectors (currententity->angles, vright, vup, vpn);
		VectorInverse (vup);

		for (i=0, l = r_newrefdef.dlights ; i<r_newrefdef.num_dlights ; i++, l++)
		{
			VectorSubtract (l->origin, currententity->origin, temp);
			TransformVector (temp, l->origin);
			R_MarkLights ( l, 1<<i, currentmodel->nodes + currentmodel->firstnode);
			VectorAdd (temp, currententity->origin, l->origin);
		}

		VectorCopy (old_vpn, vpn);
		VectorCopy (old_vup, vup);
		VectorCopy (old_vright, vright);
	} 
	else
	{
		for (i=0, l = r_newrefdef.dlights ; i<r_newrefdef.num_dlights ; i++, l++)
		{
			VectorSubtract (l->origin, currententity->origin, l->origin);
			R_MarkLights ( l, 1<<i, currentmodel->nodes + currentmodel->firstnode);
			VectorAdd (l->origin, currententity->origin, l->origin);
		}
	}
}

void R_PushWorldDlights (model_t *model)
{
	int		i;
	dlight_t	*l;

	r_dlightframecount = r_framecount;
	for (i=0, l = r_newrefdef.dlights ; i<r_newrefdef.num_dlights ; i++, l++)
	{
		R_MarkLights ( l, 1<<i, 
			model->nodes + model->firstnode);
	}
}


/*
=============================================================================

LIGHT SAMPLING

=============================================================================
*/

vec3_t	pointcolor;
mplane_t		*lightplane;		// used as shadow plane
vec3_t			lightspot;

int RecursiveLightPoint (mnode_t *node, vec3_t start, vec3_t end)
{
	float		front, back, frac;
	int			side;
	mplane_t	*plane;
	vec3_t		mid;
	msurface_t	*surf;
	int			ds, dt;
	int			i;
	mtexinfo_t	*tex;
	int			r;

	if (node->contents != CONTENTS_NODE)
		return -1;		// didn't hit anything
	
// calculate mid point

	plane = node->plane;
	if (plane->type < 3)
	{
		front = start[plane->type] - plane->dist;
		back = end[plane->type] - plane->dist;
	}
	else
	{
		front = DotProduct (start, plane->normal) - plane->dist;
		back = DotProduct (end, plane->normal) - plane->dist;
	}

	side = front < 0;
	
	if ( (back < 0) == side)
		return RecursiveLightPoint (node->children[side], start, end);
	
	frac = front / (front-back);
	mid[0] = start[0] + (end[0] - start[0])*frac;
	mid[1] = start[1] + (end[1] - start[1])*frac;
	mid[2] = start[2] + (end[2] - start[2])*frac;

// go down front side	
	r = RecursiveLightPoint (node->children[side], start, mid);
	if (r >= 0)
		return r;		// hit something
		
	if ( (back < 0) == side )
		return -1;		// didn't hit anuthing
		
// check for impact on this node
	VectorCopy (mid, lightspot);
	lightplane = plane;

	surf = r_worldmodel->surfaces + node->firstsurface;
	for (i=0 ; i<node->numsurfaces ; i++, surf++)
	{
		if (surf->flags & (SURF_DRAWTURB|SURF_DRAWSKY)) 
			continue;	// no lightmaps

		tex = surf->texinfo;
		
		ds = DotProduct (mid, tex->vecs[0]) + tex->vecs[0][3] - surf->texturemins[0];
		if (ds < 0 || ds > surf->extents[0])
			continue;

		dt = DotProduct (mid, tex->vecs[1]) + tex->vecs[1][3] - surf->texturemins[1];
		if (dt < 0 || dt > surf->extents[1])
			continue;
		
		if (surf->samples)
		{
			byte	*lightmap;
			float	samp;
			float	*scales;
			int		maps;

			lightmap = surf->samples + (dt>>4) * ((surf->extents[0]>>4)+1) + (ds>>4);
			VectorClear (pointcolor);

			for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ;
					maps++)
			{
				samp = *lightmap * /* 0.5 * */ ONEDIV255;	// adjust for gl scale
				scales = r_newrefdef.lightstyles[surf->styles[maps]].rgb;
				VectorMA (pointcolor, samp, scales, pointcolor);
				lightmap += ((surf->extents[0]>>4)+1) *
						((surf->extents[1]>>4)+1);
			}

			return 1;
		}
		
		return 0;
	}

// go down back side
	return RecursiveLightPoint (node->children[!side], mid, end);
}
/*
===============
R_LightPoint
===============
*/
void R_LightPoint (vec3_t p, vec3_t color)
{
	vec3_t		end;
	float		r;
	int			lnum;
	dlight_t	*dl;
	float		light;
	float		add;
	
	if (!r_worldmodel->lightdata)
	{
		color[0] = color[1] = color[2] = 1.0;
		return;
	}
	
	end[0] = p[0];
	end[1] = p[1];
	end[2] = p[2] - 2048;
	
	r = RecursiveLightPoint (r_worldmodel->nodes, p, end);
	
	if (r == -1)
	{
		VectorClear (color);
	}
	else
	{
		VectorCopy (pointcolor, color);
	}

	//
	// add dynamic lights
	//
	light = 0;
	for (lnum=0 ; lnum<r_newrefdef.num_dlights ; lnum++)
	{
		dl = &r_newrefdef.dlights[lnum];
		add = dl->intensity - Distance (currententity->origin, dl->origin);
		if (add > 0)
		{
			add *= ONEDIV256;
			VectorMA (color, add, dl->color, color);
		}
	}
}

//===================================================================


unsigned		blocklights[1024];	// allow some very large lightmaps

/*
===============
R_AddDynamicLights
===============
*/
void R_AddDynamicLights (void)
{
	msurface_t *surf;
	int			lnum;
	int			sd, td;
	float		dist, rad, minlight;
	vec3_t		impact, local, dlorigin;
	vec3_t		temp;
	int			s, t;
	int			smax, tmax;
	mtexinfo_t	*tex;
	dlight_t	*dl;
	qboolean	negativeLight;	//PGM
	qboolean	rotated = false;
	vec3_t		old_vpn, old_vup, old_vright;

	surf = r_drawsurf.surf;
	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	tex = surf->texinfo;

	if (currententity->angles[0] || currententity->angles[1] || currententity->angles[2])
	{
		rotated = true;
		VectorCopy (vpn, old_vpn);
		VectorCopy (vup, old_vup);
		VectorCopy (vright, old_vright);
		AngleVectors (currententity->angles, vright, vup, vpn);
		VectorInverse (vup);
	}

	for (lnum=0 ; lnum<r_newrefdef.num_dlights ; lnum++)
	{
		if ( !(surf->dlightbits & (1<<lnum) ) )
			continue;		// not lit by this light

		dl = &r_newrefdef.dlights[lnum];
		rad = dl->intensity;

//=====
//PGM
		negativeLight = false;
		if (rad < 0)
		{
			negativeLight = true;
			rad = -rad;
		}
//PGM
//=====

		VectorSubtract (dl->origin, currententity->origin, dlorigin);

		if (rotated)
		{
			VectorCopy (dlorigin, temp);
			TransformVector (temp, dlorigin);
		}

		if (surf->plane->type < 3)
			dist = dlorigin[surf->plane->type] - surf->plane->dist;
		else
			dist = DotProduct (dlorigin, surf->plane->normal) - surf->plane->dist;

		rad -= (float)fabs(dist);
		minlight = DLIGHT_CUTOFF;		// dl->minlight;
		if (rad < minlight)
			continue;
		minlight = rad - minlight;

		if (surf->plane->type < 3)
		{
			VectorCopy (dlorigin, impact);
			impact[surf->plane->type] -= dist;
		} 
		else 
		{
			VectorMA (dlorigin, -dist, surf->plane->normal, impact);
		}

		local[0] = DotProduct (impact, tex->vecs[0]) + tex->vecs[0][3];
		local[1] = DotProduct (impact, tex->vecs[1]) + tex->vecs[1][3];

		local[0] -= surf->texturemins[0];
		local[1] -= surf->texturemins[1];
		
		for (t = 0 ; t<tmax ; t++)
		{
			td = local[1] - t*16;
			if (td < 0)
				td = -td;
			for (s=0 ; s<smax ; s++)
			{
				sd = local[0] - s*16;
				if (sd < 0)
					sd = -sd;
				if (sd > td)
					dist = sd + (td>>1);
				else
					dist = td + (sd>>1);
//====
//PGM
				if (!negativeLight)
				{
					if (dist < minlight)
						blocklights[t*smax + s] += (rad - dist)*256;
				}
				else
				{
					if (dist < minlight)
						blocklights[t*smax + s] -= (rad - dist)*256;
					if (blocklights[t*smax + s] < minlight)
						blocklights[t*smax + s] = minlight;
				}
//PGM
//====
			}
		}
	}

	if (rotated)
	{
		VectorCopy (old_vpn, vpn);
		VectorCopy (old_vup, vup);
		VectorCopy (old_vright, vright);
	}
}

/*
===============
R_BuildLightMap

Combine and scale multiple lightmaps into the 8.8 format in blocklights
===============
*/
void R_BuildLightMap (void)
{
	int			smax, tmax;
	int			t;
	int			i, size;
	byte		*lightmap;
	unsigned	scale;
	int			maps;
	msurface_t	*surf;

	surf = r_drawsurf.surf;

	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;
	size = smax*tmax;

	if (r_fullbright->integer || !r_worldmodel->lightdata)
	{
		for (i=0 ; i<size ; i++)
			blocklights[i] = 0;
		return;
	}

// clear to no light
	for (i=0 ; i<size ; i++)
		blocklights[i] = 0;


// add all the lightmaps
	lightmap = surf->samples;
	if (lightmap)
		for (maps = 0 ; maps < MAXLIGHTMAPS && surf->styles[maps] != 255 ;
			 maps++)
		{
			scale = r_drawsurf.lightadj[maps];	// 8.8 fraction		
			for (i=0 ; i<size ; i++)
				blocklights[i] += lightmap[i] * scale;
			lightmap += size;	// skip to next lightmap
		}

// add all the dynamic lights
	if (surf->dlightframe == r_framecount)
		R_AddDynamicLights ();

// bound, invert, and shift
	for (i=0 ; i<size ; i++)
	{
		t = (int)blocklights[i];
		if (t < 0)
			t = 0;
		t = (255*256 - t) >> (8 - VID_CBITS);

		if (t < (1 << 6))
			t = (1 << 6);

		blocklights[i] = t;
	}
}

