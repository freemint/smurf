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
 * The Initial Developer of the Original Code is
 * Christian Eyrich
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

/* =========================================================*/
/*              Portable Bitmap Utilities Formats           */
/* Version 0.1  --  irgendwann 1996                         */
/*    24 Bit, binÑre Version                                */
/* Version 0.2  --  01.02.97                                */
/*    1, 8, 24 Bit ASCII-Versionen, Vorbereitung fÅr Block- */
/*    laderoutinen (bisher wurde einfach alle gememcpyt)    */
/*    und flexiblere Headerinterpretation (ermîglicht erst  */
/*    Bilder ohne Maxvalangabe und Bilder mit Grîûenfeldern */
/*    != drei Stellen. Kommentare werden nun auch verdaut.  */
/* Version 0.3  --  20.02.97                                */
/*    Wie zu erwarten war, war in der ASCII-Version noch    */
/*    ein Fehler, konnte ich ohne Bilder natÅrlich nicht    */
/*    merken.                                               */
/* Version 0.4  --  xx.xx.97                                */
/*    YCbCr-Bilder (z.B. von HPCDTOPPM) werden gelesen und  */
/*    verstanden.                                           */
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "..\import.h"
#include "..\..\src\smurfine.h"

void *(*SMalloc)(long amount);
int (*SMfree)(void *ptr);

unsigned int getval(void);

/* Infostruktur fÅr Hauptmodul */
MOD_INFO module_info = {"PPM-Importer",
                        0x0030,
                        "Dale Russell, Christian Eyrich",
                        "PPM", "PBM", "PGM", "PNM", "",
                        "", "", "", "", "",
                        "Slider 1",
                        "Slider 2",
                        "Slider 3",
                        "Slider 4",
                        "Checkbox 1",
                        "Checkbox 2",
                        "Checkbox 3",
                        "Checkbox 4",
                        "Edit 1",
                        "Edit 2",
                        "Edit 3",
                        "Edit 4",
                        0,128,
                        0,128,
                        0,128,
                        0,128,
                        0,10,
                        0,10,
                        0,10,
                        0,10,
                        0,0,0,0,
                        0,0,0,0,
                        0,0,0,0,
                        0
                        };


char *buffer;

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*          PBMPlus Picture Format (PPM)            */
/*      1, 8, 24 Bit, unkomprimiert,                */
/*      Speicherformen binÑr und ASCII              */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
int imp_module_main(GARGAMEL *smurf_struct)
{
    char *obuffer, *pal, *ziel, *oziel,
         type, typeASC[4], BitsPerPixel, scaletab[256], j;
    char dummy[3], impmessag[17];

    unsigned int i, x, y, width, height, maxval;

    unsigned long w, memwidth;


/* wie schnell sind wir? */
/*  init_timer(); */
    SMalloc = smurf_struct->services->SMalloc;
    SMfree = smurf_struct->services->SMfree;

    buffer = smurf_struct->smurf_pic->pic_data;
    obuffer = buffer;

    if(*buffer != 0x50 || *(buffer + 1) < 0x31 || *(buffer + 1) > 0x36) 
        return(M_INVALID);
    else
    {
        type = *(buffer + 1) - '0';

        switch((int)type)
        {
            case 1:
            case 4: BitsPerPixel = 1;
                    strcpy(typeASC, "PBM");
                    break;
            case 2:
            case 5: BitsPerPixel = 8;
                    strcpy(typeASC, "PGM");
                    break;
            case 3:
            case 6: BitsPerPixel = 24;
                    strcpy(typeASC, "PPM");
                    break;
        }

        buffer += 2;

        width = getval();
        height = getval();

        if(type != 1 && type != 4)
            maxval = getval();
        else
            maxval = 255;

        i = 0;
        do
        {
            scaletab[i] = i * 255 / maxval;
        } while(++i < 256); 

        strncpy(smurf_struct->smurf_pic->format_name, "PBMPlus' ", 21);
        strcat(smurf_struct->smurf_pic->format_name, typeASC);
        smurf_struct->smurf_pic->pic_width = width;
        smurf_struct->smurf_pic->pic_height = height;
        smurf_struct->smurf_pic->depth = BitsPerPixel;

        strcpy(impmessag, "PPM-Image ");
        strcat(impmessag, itoa(BitsPerPixel, dummy, 10));
        strcat(impmessag, " Bit");
        smurf_struct->services->reset_busybox(128, impmessag);  

        if(BitsPerPixel == 1)
        {
            w = (unsigned long)(width + 7) / 8;
            memwidth = (unsigned long)(width + 7) / 8 * 8;
        }
        else
        {
            if(BitsPerPixel == 24)
                w = (unsigned long)width * 3;
            else
                w = (unsigned long)width;
            memwidth = (unsigned long)width;
        }

        if((ziel = SMalloc((memwidth * (long)height * BitsPerPixel) >> 3)) == 0)
            return(M_MEMORY);
        else
        {
            oziel = ziel;
            memset(ziel, 0x0, (memwidth * (long)height * BitsPerPixel) >> 3);

            if(type <= 3)
            /* ASCII */
            {
                if(BitsPerPixel == 1)
                {
                    y = 0;
                    do
                    {
                        x = 0;
                        do
                        {
                            j = 0;
                            do
                            {
                                *ziel |= (scaletab[getval()] << (7 - j));
                                x++;
                            } while(++j < 8 && x < width);
                            ziel++;
                        } while(x < width);
                    } while(++y < height);
                }
                else
                {
                    y = 0;
                    do
                    {
                        x = 0;
                        do
                        {
                                *ziel++ = scaletab[getval()];
                        } while(++x < w);
                    } while(++y < height);
                }
            }
            else
            /* binÑr */
            {
                y = 0;
                do
                {
                    x = 0;
                    do
                    {
                        *ziel++ = scaletab[*buffer++];
                    } while(++x < w);
                } while(++y < height);
                
            }           

            buffer = obuffer;
            ziel = oziel;

            smurf_struct->smurf_pic->pic_data = ziel;

            SMfree(buffer);

            pal = smurf_struct->smurf_pic->palette;
            if(BitsPerPixel == 1)
            {
                pal[0] = 255;
                pal[1] = 255;
                pal[2] = 255;
                pal[3] = 0;
                pal[4] = 0;
                pal[5] = 0;

                smurf_struct->smurf_pic->format_type = FORM_STANDARD;
            }
            else
            {
                if(BitsPerPixel == 8)
                {
                    for(i = 0; i < 256; i++)
                    {
                        *pal++ = i;
                        *pal++ = i;
                        *pal++ = i;
                    }   
                }

                smurf_struct->smurf_pic->format_type = FORM_PIXELPAK;
            }

            if(BitsPerPixel == 8)
                smurf_struct->smurf_pic->col_format = GREY;
            else
                smurf_struct->smurf_pic->col_format = RGB;
        } /* Malloc */
    } /* Erkennung */

/* wie schnell waren wir? */
/*  printf("%lu", get_timer);
    getch(); */

    return(M_PICDONE);
}


unsigned int getval(void)
{
    char ch;

    unsigned int i;


    do
    {
        ch = *buffer++;
    } while(ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');

    if(ch == '#')
    {
        do
        {
            ch = *buffer++;
        } while(ch != '\n' && ch != '\r');
        ch = *buffer++;
    }

    i = 0;
    do
    {
        i = i * 10 + ch - '0';
        ch = *buffer++;
    } while(ch >= '0' && ch <= '9');

    return i;
}
