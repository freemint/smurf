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


#define PAGE_DEFAULT    0   /* Voreinstellung benutzen */
#define PAGE_A3         1   /* DIN A3 */
#define PAGE_A4         2   /* DIN A4 */
#define PAGE_A5         3   /* DIN A5 */
#define PAGE_B5         4   /* DIN B5 */
#define PAGE_LETTER     16  /* Letter size */
#define PAGE_HALF       17  /* Half size */
#define PAGE_LEGAL      18  /* Legal size */
#define PAGE_DOUBLE     19  /* Double size */
#define PAGE_BROAD      20  /* Broad sheet size */

#define MONO    1
#define HALF    2
#define COL8    3
#define TC      4


typedef struct
{
   unsigned int nvdi_version;       /* Version im BCD-Format */
   unsigned long nvdi_datum;        /* Datum im BCD-Format */
} NVDI_STR;

/* enthÑlt Infos Åber einen GDOS-Treiber */
typedef struct
{
    char devID;             /* die Treibernummer (21 .. 99) */
    char devName[36];       /* der Treibername */
} DevInfoS;

/* enthÑlt die Infos Åber den ausgewÑhlten GDOS-Treiber */
typedef struct
{
    char devID;             /* die Treibernummer (21 .. 99) */
    int prtwidth;           /* bedruckbare Breite in Pixeln */
    int prtheight;          /* bedruckbare Hîhe in Pixeln */
    int leftBorder;         /* linker Rand */
    int upperBorder;        /* oberer Rand */
    int rightBorder;        /* rechter Rand */
    int lowerBorder;        /* unterer Rand */
    float pixwidth;         /* Pixelbreite in mm */
    float pixheight;        /* Pixelhîhe in mm */
    int horres;             /* horizontale Auflîsung in dpi */
    int verres;             /* horizontale Auflîsung in dpi */
    char depth;             /* Farbtiefe in Bit */
    char can_scale;         /* kann der Treiber skalieren oder nicht? */
    char paperform;         /* Papierformat */
} DevParamS;

/* enthÑlt die Infos Åber */
typedef struct
{
    int zx;                 /* linke obere Ecke der Ausgabe */
    int zy;                 /* linke obere Ecke der Ausgabe */
    int picwidth;           /* effektive Bildbreite */
    int picheight;          /* effektive Bildhîhe */
    char advance;           /* Vorschub nach Seite? */
} OutParamS;
