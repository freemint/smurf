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

/* Ausdruck von Bildern */
/* UnterstÅtzt wird nur der Druck Åber GDOS-Treiber */
/* Farbausdruck nur Åber entsprechende Treiber, d.h ab NVDI 4 */
/* Skalierung muû ebenfalls vom Treiber unterstÅtzt werden */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vdi.h>
#include <tos.h>
#include <ext.h>
#include <math.h>
#include "..\..\..\..\sym_gem.h"
#include "..\..\import.h"
#include "..\..\..\src\smurf_st.h"
#include "..\..\..\src\globdefs.h"
#include "..\..\..\src\smurfine.h"
#include "..\..\..\src\plugin\plugin.h"
#include "gdos.h"
#include "print.h"
#include "wdialog.h"


extern int vq_ext_devinfo(int handle, int device, int *dev_exists, char *name);
extern void v_ext_opnwk(int *work_in, int *handle, int *work_out);
extern void getpix_std_line(char *std, char *buf, int depth, long planelen, int howmany);
extern PLUGIN_FUNCTIONS *smurf_functions;
extern SMURF_VARIABLES *smurf_vars;
extern OBJECT *alerts;

int get_cookie(unsigned long cookie, unsigned long *value);

int scan_devs(void);                            /* sucht nach GDOS-Ausgabedevices */
int print_with_GDOS(void);
int f_d_dither(SMURF_PIC *smurf_pic, char *ziel, unsigned int stripoffset, unsigned int stripheight, char dest_depth);
void set_MFDB(MFDB *srcform, MFDB *dstform, char *srcpic, char BitsPerPixel);
int actualize_DevParam(int gdos_dev, DevParamS *DevParam);
int get_DevInfo(char devID, DevInfoS *DevInfo);

extern SERVICE_FUNCTIONS *services;
extern SMURF_PIC * *smurf_picture;
extern int *active_pic;

int work_out[57];

char dev_anzahl=0;
unsigned int swidth, sheight, dwidth, dheight;

DevInfoS DevInfo[30];
DevParamS DevParam;
OutParamS OutParam;


/* Gedruckt wird immer das aktive Bild. */
int print_with_GDOS(void)
{
    char *srcpic,
         can_scale;

    int work_in[] = {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2};
    int gdos_handle = 0, pxy[8], clip[4], colindex[2] = {1, 0}, SCALE;
    unsigned int height, stripoffset, stripheight, bh, bl;

    unsigned long mem, w, dy;

    MFDB srcform, dstform;


    /* Daten extrahieren */
    swidth = smurf_picture[*active_pic]->pic_width;
    sheight = smurf_picture[*active_pic]->pic_height;

    /* einloggen */
    work_in[0] = DevParam.devID;                /* Nummer des GDOS-Treibers, siehe ASSIGN.SYS */
    v_opnwk(work_in, &gdos_handle, work_out);
    if(!gdos_handle)
    {
/*      form_alert(1, "[2][Smurf kann den Drucker nicht îffnen][ Abbruch ]"); */
        services->f_alert(alerts[NO_OPEN].TextCast, NULL, NULL, NULL, 1);
        return(-1);
    }

    services->reset_busybox(128, "Druck vorbereiten");

    memset(&srcform, 0x0, sizeof(MFDB));
    memset(&dstform, 0x0, sizeof(MFDB));

    can_scale = DevParam.can_scale;

    if(can_scale)
    {
        SCALE = 0x8000;                     /* fÅr vr?_cpyfm() */
        dwidth = OutParam.picwidth;
        dheight = OutParam.picheight;
    }
    else
    {
        SCALE = 0x0;                        /* fÅr vr?_cpyfm() */
        dwidth = swidth;
        dheight = sheight;
    }

    pxy[0] = 0;                             /* linke obere Ecke des Quellblocks */
    pxy[1] = 0;                             /* linke obere Ecke des Quellblocks */
    pxy[2] = swidth - 1;                    /* rechte untere Ecke des Quellblocks */
    pxy[3] = sheight - 1;                   /* rechte untere Ecke des Quellblocks */
    pxy[4] = OutParam.zx;                   /* linke obere Ecke des Zielblocks */
    pxy[5] = OutParam.zy;                   /* linke obere Ecke des Zielblocks */
    pxy[6] = OutParam.zx + dwidth - 1;      /* rechte untere Ecke des Zielblocks */
    pxy[7] = OutParam.zy + dheight - 1;     /* rechte untere Ecke des Zielblocks */

    clip[0] = pxy[4];                       /* linke obere Ecke des Clipping-Rechtecks */
    clip[1] = pxy[5];                       /* linke obere Ecke des Clipping-Rechtecks */
    clip[2] = pxy[6];                       /* rechte untere Ecke des Clipping-Rechtecks */

    if((mem = Malloc(-1)) < 128000L)
    {
/*      form_alert(1, "[1][Es steht nicht genug Speicher|zum Drucken zur VerfÅgung!][ Oh ]"); */
        services->f_alert(alerts[NO_PRT_MEM].TextCast, NULL, NULL, NULL, 1);
        return(-1);
    }
    stripheight = (unsigned int)((mem - 6535L) / ((((swidth + 15) / 16 * 16) * DevParam.depth) >> 3));
    if(stripheight < 2)
    {
/*      form_alert(1, "[1][Es steht nicht genug Speicher|zum Drucken zur VerfÅgung!][ Oh ]"); */
        services->f_alert(alerts[NO_PRT_MEM].TextCast, NULL, NULL, NULL, 1);
        return(-1);
    }

    w = ((swidth + 15) / 16) * 16;
    if((srcpic = (char *)Malloc(((long)w * (long)stripheight * (long)DevParam.depth) >> 3)) == 0)
    {
/*      form_alert(1, "[1][Nicht genug Speicher fÅr die|Aufbereitung der Bilddaten fÅr|den Druck.][ Oh ]"); */
        services->f_alert(alerts[NO_PREP_MEM].TextCast, NULL, NULL, NULL, 1);
        return(-1);
    }

/*  printf("mem: %lu, stripheight: %u\n", mem, stripheight);*/

    set_MFDB(&srcform, &dstform, srcpic, DevParam.depth);

/*  printf("Start Scheibenausgabe\n");*/
/*
    /* lt. Behnes unsinnig und fÅr Leerseite vor Ausdruck verantwortlich */
    /* Druckstart: Workstation updaten und Puffer lîschen */
    v_updwk(gdos_handle);
    v_clear_disp_list(gdos_handle);
*/
    bh = 128 / (sheight / stripheight);         /* busy-height */
    bl = 0;                                     /* busy-length */

    services->reset_busybox(0, "Seite aufbauen");

    stripoffset = 0;
    height = sheight;
    while(height)
    {
        services->busybox(bl);
        bl += bh;

        if(height < stripheight)
            stripheight = height;

/*      printf("stripheight: %u\n", stripheight);*/

        f_d_dither(smurf_picture[*active_pic], srcpic, stripoffset, stripheight, DevParam.depth);

        dy = stripoffset + stripheight - 1;     /* letzte einzulesende Zeile */
        dy *= dheight;
        if(sheight < dheight)                   /* Quellbild kleiner als das Zielbild? */
            dy += dheight - 1;
        dy /= sheight;

        clip[3] = OutParam.zy + (int)dy;        /* rechte untere Ecke des Clipping-Rechtecks */

        vs_clip(gdos_handle, 1, clip);          /* Clipping-Rechteck setzen */

        srcform.fd_h = stripheight;

        if(DevParam.depth == 1)
            vrt_cpyfm(gdos_handle, MD_REPLACE | SCALE, pxy, &srcform, &dstform, colindex);
        else
            vro_cpyfm(gdos_handle, S_ONLY | SCALE, pxy, &srcform, &dstform);

        pxy[1] -= stripheight;
        pxy[3] -= stripheight;

        clip[1] = clip[3] + 1;                  /* erste auszugebende Zeile fÅr den nÑchsten Durchgang */

        stripoffset += stripheight;

        height -= stripheight;
    }

    services->reset_busybox(128, "Druck ausfÅhren");

    /* ausloggen */
    v_updwk(gdos_handle);
    if(OutParam.advance)
        v_form_adv(gdos_handle);
    v_clswk(gdos_handle);

    services->reset_busybox(128, "OK");

    Mfree(srcpic);

    return(0);
} /* print_with_GDOS */


int scan_devs(void)
{
    int i=0;


    if(dev_anzahl > 0)
        return(0);

    services->reset_busybox(128, "Scanne Drucker");

    if(!vq_gdos())
    {
/*      form_alert(1, "[3][Smurf unterstÅtzt nur den Ausdruck | Åber GDOS. Leider ist bei Ihnen | kein GDOS installiert.][ Schade ]"); */
        services->f_alert(alerts[NO_GDOS].TextCast, NULL, NULL, NULL, 1);
        return(-1);
    }

    dev_anzahl = 0;
    for(i = 21; i <= 30; i++)               /* GDOS-Device Drucker-Treiber */
        if(get_DevInfo(i, &DevInfo[dev_anzahl]))
            dev_anzahl++;
    for(i = 81; i <= 90; i++)               /* GDOS-Device FAX-Treiber */
        if(get_DevInfo(i, &DevInfo[dev_anzahl]))
            dev_anzahl++;
    for(i = 91; i <= 100; i++)              /* GDOS-Device IMG-Treiber */
        if(get_DevInfo(i, &DevInfo[dev_anzahl]))
            dev_anzahl++;

    services->reset_busybox(128, "OK");

    if(dev_anzahl == 0)
    {
/*      form_alert(1, "[3][Es wurden keine Druckertreiber gefunden.|Drucken unmîglich.][ Schade ]"); */
        services->f_alert(alerts[NO_DRIVER].TextCast, NULL, NULL, NULL, 1);
        return(-1);
    }
    else
        return(0);
} /* scan_devs */


/* versucht einen Treiber zu îffnen und stellt bei Erfolg Infos in die Dev-Struktur */
int get_DevInfo(char devID, DevInfoS *DevInfo)
{
    char name[128];

    int work_in[] = {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2};
    int gdos_handle = 0, devExist = 0;

    NVDI_STR *nvdi_str = NULL;


    work_in[0] = devID;
    v_opnwk(work_in, &gdos_handle, work_out);
    if(!gdos_handle)
        return(0);

    /* Vorsicht, vq_ext_devinfo existiert nur mit NVDI >= 3.0 */
    get_cookie('NVDI', &(unsigned long)nvdi_str);

    if(nvdi_str && nvdi_str->nvdi_version >= 0x0300)
        vq_ext_devinfo(gdos_handle, devID, &devExist, name);
    else
    {
        memset(name, 0x0, 128);                     /* die Strings von vqt_devinfo() sind nicht nullterminiert */
        vqt_devinfo(gdos_handle, devID, &devExist, name);
    }

    if(devExist)                                    /* nicht nur > 0, denn kann auch -1 sein */
    {
        DevInfo->devID = devID;
        DevInfo->devName[0] = '\0';                 /* damit der cat was zum drancaten hat */
        strncat(DevInfo->devName, name, 35);
/*      printf("Name: %s\n", DevInfo->devName);*/
    }

    v_clswk(gdos_handle);

    return(devExist);
} /* get_DevInfo */


/* Dispatcher um ein Bild von beliebigem Format und beliebiger Farbtiefe ins Zielformat */
/* (auf 16 Pixel aligned) und in die vom Treiber erwartete Farbtiefe (s. dest_mode) zu bringen. */
/* MONO = monochromes Bild s/w  (1 Bit Input) */
/* HALF = Halbtonbild, Graustufen, Treiber rastert selbst (32 Bit Input) */
/* COL8 = 8 Farben Bild, feste Palette: Weiû, Schwarz, Rot, GrÅn, Blau, Gelb, Cyan, Magenta (3 Bit Input) */
/* TC = Truecolorbild, Treiber rastert selbst (32 Bit Input) */
/* In BitsPerPixel wird dann auch gleich die tatsÑchliche Farbtiefe zurÅckgegeben */
int f_d_dither(SMURF_PIC *smurf_pic, char *ziel, unsigned int stripoffset, unsigned int stripheight, char dest_depth)
{
    char *buffer, *palette, *pal, *pixbuf,
         v, BitsPerPixel;

    unsigned int *buffer16,
                 x, y, dw, sw, pix;

    unsigned long planelength;

    SMURF_PIC pic;
    SYSTEM_INFO sys_info;


    buffer16 = (unsigned int *)buffer = smurf_pic->pic_data;
    BitsPerPixel = smurf_pic->depth;

    dw = ((swidth + 15) / 16) * 16;

    if(smurf_pic->format_type == FORM_PIXELPAK)
        buffer += ((long)swidth * (long)stripoffset * (long)BitsPerPixel) >> 3;
    else
        buffer += ((long)(swidth + 7) / 8 * (long)stripoffset * (long)BitsPerPixel) >> 3;

    memset(ziel, 0x0, ((long)dw * (long)stripheight * (long)dest_depth) >> 3);

    planelength = (long)(swidth + 7) / 8 * (long)sheight;   /* LÑnge einer Plane in Bytes */

    palette = smurf_pic->palette;

    sw = (swidth + 7) / 8;

    if(dest_depth == 1)                         /* Hier muû eine Ditherroutine rein */
    {
        if(BitsPerPixel == 1)
        {
            v = dw / 8 - sw;

    /*      printf("v: %d, w: %u, w2: %u\n", (int)v, w, w2);*/

            y = 0;
            do
            {
                x = 0;
                do
                {
                    *ziel++ = *buffer++;
                } while(++x < sw);

                ziel += v;
            } while(++y < stripheight);
        }
/*      else
        {
            memcpy(&pic, smurf_pic, sizeof(SMURF_PIC));
            pic.pic_data = buffer;
            pic.pic_height = stripheight;
            pic.screen_pic = NULL;
            memcpy(&sys_info, smurf_vars->Sys_info, sizeof(SYSTEM_INFO));
            sys_info.bitplanes = 1;
            sys_info.Max_col = 1;
            smurf_vars->exp_conf-> = ;

            if(smurf_functions->dither(&pic, &sys_info, 0) != 0)
                return(-2);

            memcpy(ziel, (char *)pic.screen_pic->fd_addr, (long)sw * (long)stripheight);
        } */
    }
    else
    if(dest_depth == 32)
    {
        pixbuf = (char *)Malloc(swidth + 7);

        v = (dw - swidth) * 4;

        y = 0;
        do
        {
            if(BitsPerPixel == 24)
            {
                x = 0;
                do
                {
                    ziel++;
                    *ziel++ = *buffer++;
                    *ziel++ = *buffer++;
                    *ziel++ = *buffer++;
                } while(++x < swidth);
            }
            else
                if(BitsPerPixel == 16)
                {
                    x = 0;
                    do
                    {
                        pix = *buffer16++;

                        ziel++;
                        *ziel++ = (pix & 0xf800) >> 8;
                        *ziel++ = (pix & 0x7e0) >> 3;
                        *ziel++ = (pix & 0x1f) << 3;
                    } while(++x < swidth);
                }
                else
                    if(BitsPerPixel == 8 && smurf_pic->format_type == FORM_PIXELPAK)
                    {
                        x = 0;
                        do
                        {
                            pix = *buffer++;
                            pal = palette + pix + pix + pix;

                            ziel++;
                            *ziel++ = *pal;
                            *ziel++ = *(pal + 1);
                            *ziel++ = *(pal + 2);
                        } while(++x < swidth);
                    }
                    else
                        if(smurf_pic->format_type == FORM_STANDARD)
                        {
                            memset(pixbuf, 0x0, swidth);
                            getpix_std_line(buffer, pixbuf, BitsPerPixel, planelength, swidth);

                            x = 0;
                            do
                            {
                                pix = pixbuf[x];
                                pal = palette + pix + pix + pix;

                                ziel++;
                                *ziel++ = *pal;
                                *ziel++ = *(pal + 1);
                                *ziel++ = *(pal + 2);
                            } while(++x < swidth);

                            buffer += sw;
                        }
            ziel += v;
        } while(++y < stripheight);

        Mfree(pixbuf);
    }

/*  printf("v: %d\n", (char)v);*/

    return(0);
} /* f_d_dither */


/* Setzt MFDBs auf benîtigte Werte */
void set_MFDB(MFDB *srcform, MFDB *dstform, char *srcpic, char BitsPerPixel)
{
    srcform->fd_addr = srcpic;
    srcform->fd_w = swidth;
    srcform->fd_h = sheight;
    srcform->fd_wdwidth = (swidth + 15) / 16;
    srcform->fd_stand = 0;
    srcform->fd_nplanes = BitsPerPixel;

    dstform->fd_addr = 0L;
    /* Mehr braucht nicht */

    return;
} /* set_MFDB */


/* fÅllt die Treiberinfostruktur */
int actualize_DevParam(int gdos_dev, DevParamS *DevParam)
{
    int work_in[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0};
    int gdos_handle;

    float res;

/*  printf("gdos_dev: %d\n", gdos_dev); */
    /* DevParamS-Struktur initialisieren */
    work_in[0] = gdos_dev;
    work_in[11] = DevParam->paperform;      /* Papierformat setzen */

/*  printf("Drucker îffnen\n"); */
    memset(DevParam, 0x0, sizeof(DevParamS));
    v_ext_opnwk(work_in, &gdos_handle, work_out);
    if(!gdos_handle)
    {
/*      form_alert(1, "[2][Smurf kann den Drucker nicht îffnen.][ Abbruch ]"); */
        services->f_alert(alerts[NO_OPEN].TextCast, NULL, NULL, NULL, 1);
        return(-1);
    }
/*  else
        printf("Drucker offen\n"); */

    DevParam->devID = gdos_dev;                 /* GDOS-GerÑte-ID */
    vq_extnd(gdos_handle, 0, work_out);         /* Modus v_opnwk */
    DevParam->prtwidth = work_out[0];           /* bedruckbare Breite in Pixeln */
    DevParam->prtheight = work_out[1];          /* bedruckbare Hîhe in Pixeln */
    DevParam->pixwidth = work_out[3];           /* Pixelgrîûe in Êm */
    DevParam->pixheight = work_out[4];          /* Pixelgrîûe in Êm */

    vq_extnd(gdos_handle, 1, work_out);         /* Modus vq_extnd */
    DevParam->depth = work_out[4];              /* Farbtiefe in Bit */
    DevParam->can_scale = work_out[30]&0x01;    /* Bit 0 gesetzt - kann der Treiber skalieren? */
    DevParam->paperform = work_out[44];         /* Papierformat */
    if(work_out[20] != 0)
    {
        /* work_out[20]: 1=1/10, 2=1/100, 3=1/1000 Êm */
        res = pow10(work_out[20]);
        DevParam->pixwidth = work_out[21] / res;    /* Pixelbreite in Êm */
        DevParam->pixheight = work_out[22] / res;   /* Pixelhîhe in Êm */
        DevParam->horres = work_out[23];        /* horizontale Auflîsung in dpi */
        DevParam->verres = work_out[24];        /* vertikale Auflîsung in dpi */
        DevParam->leftBorder = work_out[40];
        DevParam->upperBorder = work_out[41];
        DevParam->rightBorder = work_out[42];
        DevParam->lowerBorder = work_out[43];
    }
    else
    {
        DevParam->leftBorder = 0;
        DevParam->upperBorder = 0;
        DevParam->rightBorder = 0;
        DevParam->lowerBorder = 0;
    }

    DevParam->pixwidth /= 1000;                 /* aus Êm mm machen */
    DevParam->pixwidth /= 1000;                 /* aus Êm mm machen */

/*  printf("Drucker schlieûen\n"); */
    v_clswk(gdos_handle);

    return(0);
} /* actualize_DevParam */


#define FALSE   0
#define TRUE    1

/* Funktion um Cookie auf Anwesenheit zu testen */
/* Christian Eyrich irgendwann im 20. Jahrhundert */
int get_cookie(unsigned long cookie, unsigned long *value)
{
    int cookiecnt = 0;

    unsigned long oldstack = 0;
    unsigned long *cookiejar;

    
    /* in Supervisormodus schalten */
    if(!Super((void *)1L))
        oldstack = Super(0L);

    /* Zeiger auf Cookie Jar holen */
    cookiejar = (unsigned long *)*((unsigned int **)0x5a0L);

    /* Supervisormodus verlassen */
    if(oldstack)
        Super((void *)oldstack);

    /* Cookie Jar installiert? */
    if(cookiejar == NULL)
    {
        *value = 0L;
        return(FALSE);
    }
    
    /* Keksdose nach cookie durchsuchen */
    while(*cookiejar != 0L && *cookiejar != cookie)
    {
        cookiejar += 2;
        cookiecnt++;
    }

    /* wenn cookie gefunden wurde, value auf Cookiewert setzen, */
    /* ansonsten value auf 0L */
    if(*cookiejar == cookie)
    {
        *value = *(cookiejar + 1);
        return(TRUE);
    }
    else
    {
        *value = 0L;
        return(FALSE);
    }
} /* get_cookie */
