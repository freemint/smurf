/*
 * ***** BEGIN LICENSE BLOCK *****
 *
 * The contents of this file are subject to the GNU General Public License
 * Version 2 (the "License"); you may not use this file except in compliance
 * with the GPL. You may obtain a copy of the License at
 * http://www.gnu.org/copyleft/gpl.html or the file GPL.TXT from the program
 * or source code package.
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
 * the specific language governing rights and limitations under the GPL.
 *
 * The Original Code is Therapy Seriouz Software code.
 *
 * The Initial Developer of the Original Code are
 * Olaf Piesche, Christian Eyrich, Dale Russell and Jîrg Dittmer
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

#include <tos.h>
#include <ext.h>
#include <vdi.h>
#include <stdio.h>
#include <stdlib.h>
#include <screen.h>
#include <string.h>
#include <math.h>
#include "..\..\sym_gem.h"
#include "..\..\modules\import.h"
#include "..\smurf_st.h"
#include "..\smurfine.h"
#include "..\smurf_f.h"
#include "smurf.h"
#include "..\globdefs.h"
#include "..\smurfobs.h"
#include "..\ext_obs.h"
#include "..\bindings.h"


void rotozoom(int *img, int *padr, int xstep, int xystep, int pos);
void roto(void);


extern SYSTEM_INFO Sys_info;
extern WINDOW wind_s[25];
extern MFORM	*dummy_ptr;				/* Dummymouse fÅr Maus-Form */

int *Sin, *Cos;
char *image;

int old_timer;
SMURF_PIC rotoimage;
int phi, phi2, phi3;

int xpos[] = {-100, -100,  100,  100, -100, -100, 100,  100};
int ypos[] = {-100,  100,  100, -100, -100,  100, 100, -100};
int zpos[] = {-100, -100, -100, -100,  100,  100, 100,  100};


int x2d[100];
int y2d[100];

int pointNum = 8;
int polyNum = 6;

int polyPointNum[50] = {4, 4, 4, 4, 4, 4};
int polyPoints[50][4];

int offscreenHandle = -1, rotowork_in[40], rotowork_out[40];

int init_roto(void)
{
	double bog;
	long t;


	phi=0;

	graf_mouse(BUSYBEE, dummy_ptr);

	Sin=SMalloc((long)370*2);
	Cos=SMalloc((long)370*2);

	/*---- FÅllen der Sin/Cos-Tabelle ----*/
	for(t=0; t<365; t++)
	{
		bog=(float)t*M_PI/180F;
		Sin[t]=(int)(sin(bog)*256F);
		Cos[t]=(int)(cos(bog)*256F);
	}


	/* Objekt vorbereiten
	 */
	polyPoints[0][0] = 0;
	polyPoints[0][1] = 1;
	polyPoints[0][2] = 2;
	polyPoints[0][3] = 3;
	
	polyPoints[1][0] = 0;
	polyPoints[1][1] = 4;
	polyPoints[1][2] = 5;
	polyPoints[1][3] = 1;

	polyPoints[2][0] = 4;
	polyPoints[2][1] = 7;
	polyPoints[2][2] = 6;
	polyPoints[2][3] = 5;

	polyPoints[3][0] = 3;
	polyPoints[3][1] = 2;
	polyPoints[3][2] = 6;
	polyPoints[3][3] = 7;

	polyPoints[4][0] = 1;
	polyPoints[4][1] = 5;
	polyPoints[4][2] = 6;
	polyPoints[4][3] = 2;

	polyPoints[5][0] = 0;
	polyPoints[5][1] = 3;
	polyPoints[5][2] = 7;
	polyPoints[5][3] = 4;

	rotoimage.pic_data=SMalloc((160L*160L)+1000);
	rotoimage.screen_pic=SMalloc(sizeof(MFDB));
	rotoimage.screen_pic->fd_addr=SMalloc((160L*160L)+1000);

	rotoimage.pic_width=160;
	rotoimage.pic_height=160;
	rotoimage.depth = Sys_info.bitplanes;
	rotoimage.zoom=0;
	
	rotoimage.screen_pic->fd_w=160;
	rotoimage.screen_pic->fd_h=160;
	rotoimage.screen_pic->fd_wdwidth=160/16;
	rotoimage.screen_pic->fd_stand=0;
	rotoimage.screen_pic->fd_nplanes = Sys_info.bitplanes;

	memset(rotowork_in, 0x0, 40);
	rotowork_in[11] = rotoimage.screen_pic->fd_w - 1;
	rotowork_in[12] = rotoimage.screen_pic->fd_h - 1;
	v_opnbm(rotowork_in, rotoimage.screen_pic, &offscreenHandle, rotowork_out);


	wind_s[WIND_INFO].picture=&rotoimage;
	wind_s[WIND_INFO].pic_xpos=wind_s[WIND_INFO].resource_form[ROTOBOX].ob_x;
	wind_s[WIND_INFO].pic_ypos=wind_s[WIND_INFO].resource_form[ROTOBOX].ob_y;
	wind_s[WIND_INFO].xoffset=0;
	wind_s[WIND_INFO].xoffset=0;
	wind_s[WIND_INFO].clipwid=160;
	wind_s[WIND_INFO].cliphgt=160;

	/* Timer umschalten
	 */
	old_timer = Sys_info.Event_Timer;
	Sys_info.Event_Timer = 20;
	
	wind_s[WIND_INFO].resource_form[ROTOBOX].ob_flags &= ~HIDETREE;
	
	Window.redraw(&wind_s[WIND_INFO], NULL, 0, 0);
	graf_mouse(ARROW, dummy_ptr);
	return(0);
}



void roto(void)
{
	register int pos, t, pxy[5], normvek;
	register long mySin, myCos;
	register int xrot[100], yrot[100], zrot[100];


	/*---- Jetz' gehts los ----*/
	phi+=4;
	if(phi>359) phi=4;
	phi2+=3;
	if(phi2>359) phi2=3;
	phi3+=2;
	if(phi3>359) phi3=2;

	/* rotieren & projezieren
	 */	
	for(t=0; t<pointNum; t++)
	{
		/* y-achse */
		mySin = Sin[phi];
		myCos = Cos[phi];
		xrot[t] = ( ((long)xpos[t]*myCos)-((long)zpos[t]*mySin) ) >>8;
		zrot[t] = ( ((long)xpos[t]*mySin)+((long)zpos[t]*myCos) ) >>8;

		pos = zrot[t];

		/* x-achse */
		mySin = Sin[phi2];
		myCos = Cos[phi2];
		zrot[t] = ( ((long)zrot[t]*myCos)-((long)ypos[t]*mySin) ) >>8;
		yrot[t] = ( ((long)pos*mySin)+((long)ypos[t]*myCos) ) >>8;

		pos = xrot[t];

		/* z-achse */
		mySin = Sin[phi3];
		myCos = Cos[phi3];
		xrot[t] = ( ((long)xrot[t]*myCos)-((long)yrot[t]*mySin) ) >>8;
		yrot[t] = ( ((long)pos*mySin)+((long)yrot[t]*myCos) ) >>8;


		/* projektion
		 */
		x2d[t] = (xrot[t]<<7) / (zrot[t]+300) + 80;
		y2d[t] = (yrot[t]<<7) / (zrot[t]+300) + 80;
	}

	pxy[0]=0; pxy[1]=0;
	pxy[2]=160; pxy[3]=160;
	vsf_interior(offscreenHandle, FIS_SOLID);
	vswr_mode(offscreenHandle, MD_REPLACE);
	vsf_color(offscreenHandle, 1);
	v_bar(offscreenHandle, pxy);

	for(t=0; t<polyNum; t++)
	{
		normvek = (x2d[polyPoints[t][2]]-x2d[polyPoints[t][0]])*(y2d[polyPoints[t][1]]-y2d[polyPoints[t][0]]) - 
				(x2d[polyPoints[t][1]]-x2d[polyPoints[t][0]])*(y2d[polyPoints[t][2]]-y2d[polyPoints[t][0]]);

		if(normvek<0) continue;
	
		pxy[0] = x2d[polyPoints[t][0]];
		pxy[1] = y2d[polyPoints[t][0]];
		pxy[2] = x2d[polyPoints[t][1]];
		pxy[3] = y2d[polyPoints[t][1]];
		pxy[4] = x2d[polyPoints[t][2]];
		pxy[5] = y2d[polyPoints[t][2]];
		pxy[6] = x2d[polyPoints[t][3]];
		pxy[7] = y2d[polyPoints[t][3]];
		vsf_color(offscreenHandle, t+7);
		v_fillarea(offscreenHandle, polyPointNum[t], pxy);
	}
	Window.redraw(&wind_s[WIND_INFO], NULL, 0, DRAWNOTREE|DRAWNOBLOCK|NOBLOCKBOX);

}



void deinit_roto(void)
{
	wind_s[WIND_INFO].picture=0;

	if(rotoimage.pic_data)
		SMfree(rotoimage.pic_data);
	
	if(rotoimage.screen_pic)
	{
		SMfree(rotoimage.screen_pic->fd_addr);
		SMfree(rotoimage.screen_pic);
		rotoimage.pic_data = NULL;
		rotoimage.screen_pic = NULL;
	}

	if(Sin)
	{
		SMfree(Sin);
		SMfree(Cos);
		Sin = Cos = NULL;
	}

	if(offscreenHandle > 0)
	{
		v_clsbm(offscreenHandle);
		offscreenHandle = -1;
	}
	
	Sys_info.Event_Timer = old_timer;
	wind_s[WIND_INFO].resource_form[ROTOBOX].ob_flags |= HIDETREE;
	wind_s[WIND_INFO].resource_form[INFO_YEAH].ob_state &= ~SELECTED;
	Window.redraw(&wind_s[WIND_INFO], NULL, 0, 0);
}