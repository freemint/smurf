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

/*  Headerfile fÅr Module - SMURF                                   */
/*                  - Strukturdefinitionen -                        */
/*  Modulschnittstelle SCHLUMPFINE V0.6, 20.04.96 Olaf Piesche      */
/*  KOMMUNIKATION groûgeschrieben...                                */

#include <multiaes.h>
#include <vdi.h>
/* has to be absolute because with relative addressing */
/* PURE-C would use the start path of the file that */
/* includes IMPORT.H */
#include "e:\opensmurf\sym_gem.h"

#define CNFVERSION  0x04

/*  OS-defines  */
#define MINT        1
#define MTOS        2
#define NAES        4
#define GENEVA      8
#define MATSCHIG    16
#define WINX        32
#define MAG_PC      64

/*  OSFeatures - Defines */
#define GETINFO     1
#define COLICONS    2
#define BEVT        4
#define MPOPUP      8
#define PEXEC10x    16
#define AES3D       32
#define OSYSVAR     64


/*  Prozessortyp */
#define MC68000 1
#define MC68020 2
#define MC68030 4
#define MC68040 8
#define FPU     16

/*--------- Struktur VECTOR zur Beschreibung eines Vektorobjektes -----*/

/* öber char *next_obj kann in ein Objekt eine Folge aufs nÑchste Objekt */
/* gesetzt werden. Dadurch kînnen komplexere Objekte mit Zeichenreihenfolge */
/* behandelt werden. */

typedef struct
{
    int     *points;            /* Anzahl an Punkten/Polygon */
    int     polygons;           /* Anzahl an Polygonen  */
    long    *x_coord, 
            *y_coord;           /* Zeiger auf Arrays mit den Punktkoordinaten in mm */
    int     *polydef;           /* Zeiger auf Array mit Polygondefinitionen, -1 ist Endkennung fÅr ein Polygon  */
    char    *fillstyle;         /* FÅllstile (nach VDI-Mustern) */  
    char    *linestyle;         /* Linienstile der Polygone, VDI */
    char    *linet;             /* Liniendicke in mm */
    char    *linecol;           /* Linienfarben (wie fÅll) */
    char    *fillcolor;         /* FÅllfarben 3*char ccc mmm yyy, oder 4*char ccc mmm yyy kkk, 24Bittige Farbe  */  
    char    col_format;         /* CMYK oder RGB oder was?  */
    int     resolution;         /* Auflîsung bei gegebenen Koordinaten in dpi */

    char    *next_obj;          /* VECTOR-Zeiger aufs nÑchste Objekt in Reihenfolge (NULL = Ende) */
} VECTOR;


/* Struktur SMURF_PIC - Standard-Bildinformationsstruktur von SMURF */
/* 3 KB */
typedef struct smurfpic
{
    char filename[257];     /* Zeiger auf Dateiname incl. Pfad des Bildes - von Smurf */

    int changed;            /* Bitvektor fÅr BildÑnderungen: 
                               Bit 0: neue NCT wird benîtigt,
                               Bit 1: Redither ist notwendig
                               = 255: Bildstruktur enthÑlt einen Block! */
                                                
    char format_name[41];   /* Zeiger auf Formatname - vom Modul */

    void *pic_data;         /* Zeiger auf rohe Bilddaten */
    void *mask;             /* Zeiger auf 8Bit-Maske, gekocht */

    int pic_width;          /* Breite des Bildes in Pixels, gebraten */
    int pic_height;         /* Hîhe des Bildes in Pixels, gedÅnstet */
    int depth;              /* Farbtiefe in BIT (1-24), 30 min. bei 200C im Backofen... */
    MFDB *screen_pic;       /* Zeiger auf MFDB fÅr Bildschirmdarstellung */
    char *palette;          /* Zeiger auf Palette */
    char bp_pal;            /* Farbtiefe pro Paletteneintrag (BITs) */
    long file_len;          /* DateilÑnge des Bildes */
    char format_type;       /* 0=Pixel Packed, 1=Standardformat */
    char col_format;        /* Farbsystem, in dem das Bild vom Modul zurÅckkommt */
    char infotext[97];      /* Infotext fÅr weitere Bildinformationen */

    int  red[256],          /* Palette, nach der die */
         grn[256],          /* Bildschirmdarstellung */
         blu[256];          /* gedithert wurde. (15Bit-Format) */


    int blockx, blocky, blockwidth, blockheight;    /* Koordinaten fÅr den Blockrahmen */

    struct smurfpic *block;

    int zoom;               /* Zoomfaktor des Bildes */ 
    int image_type;         /* Was ist drin in der Datei? */

    char own_pal;           /* 1: hier herrscht eine eigene Palette */
    unsigned char *local_nct;       /* lokale NCT fÅr die aktuelle Palette (wenn != Syspal!)    */
    int not_in_nct;                 /* Kennung fÅr nicht in der NCT enthaltene Farben           */
    
    struct smurfpic *prev_picture;  /* Zeiger auf vorheriges Bild (SMURF_PIC*) */
    struct smurfpic *next_picture;  /* Zeiger auf nÑxtes Bild (SMURF_PIC*) */
} SMURF_PIC;


/* Allgemeine Windowstruktur zur internen Verwaltung von Fenstern */
/*  101 Bytes */
typedef struct
{
    int whandlem;           /* AES-Handle des Windows */
    int module;             /* Modul, dem das Fenster gehîrt */
    int wnum;               /* das wievielte Fenster des Moduls? */
    int wx, wy, ww, wh;     /* X/Y, Breite, Hîhe */ 
    char wtitle[41];        /* Fenstertitel */
    OBJECT *resource_form;  /* Modulresource-Formular */
    SMURF_PIC *picture;     /* Zeigerfeld fÅr Bild/Animation */
    int xoffset, yoffset;   /* Scrolloffsets in Pixels */
    int shaded;             /* Shaded-Flag */
    int pic_xpos, pic_ypos; /* X/Y-Position fÅr 'picture' im Fenster */
    int editob, nextedit, editx;

    int clipwid, cliphgt;   /* Breite und Hîhe des Ausschnittes von *picture */
    int pflag;              /* reines Bildfenster: 1, ansonsten 0 */

    void *prev_window;      /* vorheriges Fenster (WINDOW*) */
    void *next_window;      /* nÑxtes Fenster (WINDOW*) */
    int fullx,fully,fullw,fullh;    /* zum RÅckspeichern der Koordinaten bei WM_FULLED */
} WINDOW;


/* Slider-Struktur zur Verwaltung von Slidern durch f_rslid */
typedef struct
{
    int regler;
    int schiene; 
    OBJECT *rtree;
    int txt_obj;
    long min_val;
    long max_val;
    WINDOW *window;
} SLIDER;


/*---- Struktur MOD_INFO zur öbergabe von pars ans Hauptprogramm:   */
/* Zeiger darauf: bei 8(a0), wobei a0 = Textsegment                 */
/* 225 Bytes */
typedef struct
{
    char *mod_name;                 /* erweiterter Modulname    */
    int  version;                   /* Modulversion alc BCD     */ 
    char *autor;                    /* Autor des Moduls         */
    char *ext[10];                  /* bis zu 10 Extensionen, deren Formate das Modul unterstÅtzt */
    char *slide1;                   /* Funktion Slider 1        */
    char *slide2;                   /* Funktion Slider 2        */
    char *slide3;                   /* Funktion Slider 3        */
    char *slide4;                   /* Funktion Slider 4        */
    char *check1;                   /* Funktion Checkbox 1      */
    char *check2;                   /* Funktion Checkbox 2      */
    char *check3;                   /* Funktion Checkbox 3      */
    char *check4;                   /* Funktion Checkbox 4      */
    char *edit1;                    /* Funktion Ftext 1         */
    char *edit2;                    /* Funktion Ftext 2         */
    char *edit3;                    /* Funktion Ftext 3         */
    char *edit4;                    /* Funktion Ftext 4         */
    long smin1, smax1;              /* min/max-Werte Slider 1   */
    long smin2, smax2;              /* min/max-Werte Slider 2   */
    long smin3, smax3;              /* min/max-Werte Slider 3   */
    long smin4, smax4;              /* min/max-Werte Slider 4   */
    long emin1, emax1;              /* min/max-Werte Ftext 1    */
    long emin2, emax2;              /* min/max-Werte Ftext 2    */
    long emin3, emax3;              /* min/max-Werte Ftext 3    */
    long emin4, emax4;              /* min/max-Werte Ftext 4    */
    long sdef1,sdef2,sdef3,sdef4;   /* Defaultwerte fÅr Slider */
    char cdef1,cdef2,cdef3,cdef4;   /* Defaultwerte fÅr Checkboxes */
    long edef1,edef2,edef3,edef4;   /* Defaultwerte fÅr Edit-Obs */

    char how_many_pix;              /* Wieviele Bilder braucht das EDITModul? */
    char *pic_descr1,               /* Bildbeschreibungen fÅr die einzelnen Bilder */
         *pic_descr2,
         *pic_descr3,
         *pic_descr4,
         *pic_descr5,
         *pic_descr6;
} MOD_INFO;


/*----------------- SERVICE_FUNCTIONS - die Dienstfunktionen */
typedef struct
{
    int (*busybox)(int lft);
    void (*reset_busybox)(int lft, char *string);

    int (*f_module_window)(WINDOW *mod_window);
    void (*f_module_prefs)(MOD_INFO *infostruct, int mod_id);

    int     (*popup)(POP_UP *popup_struct, int mouseflag, int button, OBJECT *poptree);
    void    (*deselect_popup)(WINDOW *wind, int ob1, int ob2);

    int     (*slider)(SLIDER *slider_struct);
    void    (*set_slider)(SLIDER *sliderstruct, long value);

    int     (*listfield)(long *window, int klick_obj, int keyscan, LIST_FIELD *lfstruct);
    void    (*generate_listfield)(int uparrow, int dnarrow, int sliderparent, int sliderobject,
                int listparent,  char *listentries, int num_entries, int max_entries, LIST_FIELD *listfield, int autoloc);

    SMURF_PIC * (*new_pic)(int wid, int hgt, int depth);

    void (*redraw_window)(WINDOW *window, GRECT *mwind, int startob, int flags);

    void (*f_move_preview)(WINDOW *window, SMURF_PIC *orig_pic, int redraw_object);
    void (*copy_preview)(SMURF_PIC *source_pic, SMURF_PIC *module_preview, WINDOW *prev_window);

    void *  (*SMalloc)(long amount);
    int (*SMfree)(void *ptr);
    
    int CPU_type;
    int (*seek_nearest_col)(long *par, int maxcol);
    SMURF_PIC* (*get_pic)(int num, int mod_id, MOD_INFO *mod_info, int depth, int form, int col);

    int (*f_alert)(char *alertstring, char *b1, char *b2, char *b3, char defbt);
    int (*f_fsbox)(char *Path, char *fbtext, char selectart);
    float (*convert_units)(int oldunit, int newunit, float dpi);

    void *(*mconfLoad)(MOD_INFO *modinfo, int mod_id, char *name);
    void (*mconfSave)(MOD_INFO *modinfo, int mod_id, void *confblock, long len, char *name);
} SERVICE_FUNCTIONS;


/*------ Struktur GARGAMEL zur öbergabe an das Modul: */
/*  77 Bytes*/
typedef struct
{
    SMURF_PIC *smurf_pic;                           /* Zeiger auf Bildstruktur, in die das Bild gelegt werden soll - Smurf                          */
    WINDOW *wind_struct;                                /* Zeiger auf Bildstruktur, in die das Bild gelegt werden soll - Smurf                          */
    long slide1, slide2, slide3, slide4;        /* öbergabewerte aus Einstellformular  (nach min/maxwerten aus MOD_INFO-Struktur  -  Smurf      */
    char check1, check2, check3, check4;        /* öbergabewerte aus Einstellformular  (0 oder 1) - Smurf   */
    long edit1, edit2, edit3, edit4;                /* öbergabewerte aus Einstellformular  (nach min/maxwerten aus MOD_INFO-Struktur  -  Smurf      */
    int module_mode;                                    /* Message */

    int event_par[10];                              /* Beim Event betroffenes Objekt */

    int mousex,mousey;                              /* Mauspos beim Event           */
    int module_number;                              /* ID des Modules, von Smurf Åbergeben */
    int klicks;                                         /* Mausklicks beim Event        */
    int picwind_x, picwind_y, picwind_w, picwind_h;     /* Betroffenes Bildfenster - Abmessungen und Pos. */

    SERVICE_FUNCTIONS *services;
} GARGAMEL;


/* SYSTEM-Struktur fÅr Systeminformationen */
/* Muû beim Start von Smurf gefÅllt werden (globale variable) */
/*  1857 Bytes */
typedef struct
{
    int app_id;
    char bitplanes;         /* aktuelle Planetiefe */
    unsigned int Max_col;   /* maximale Farben momentan */
    int screen_width;       /* Bildschirmbreite */
    int screen_height;      /* Bildschirmhîhe */
    int vdi_handle;
    char *nc_table;         /* Standard-Nearest-Color-Table. Muû von Smurf anfangs geladen / erzeugt werden! */
    char *plane_table;      /* Standard-Binary-Palette-Table. Muû von Smurf anfangs erzeugt werden! */

    int *red, *grn, *blu;   /* Zeiger auf Systempalette - mÅssen von Smurf anfangs gefÅllt werden! */


    char standard_path[256];    /* Smurf-Startpfad */
    char home_path[256];        /* Zeiger auf $HOME-Verzeichnis - kann auf Standard_path zeigen! */

    int DSP;                /* DSP-existant-Flag */

    int pal_red[257],       /* Systempalette (die echte im VDI-Format!)*/
        pal_green[257], 
        pal_blue[257];
    
/* Variablen fÅr die Konfiguration */
    int center_dialog;      
    int window_alert;       
    int profi_mode;     
    int busybox_oc;
    int harddisk_swap;
    int immed_prevs;
    int realtime_dither;
    int dither24;
    int dither8;
    int dither4;
    int pal24,pal8,pal4;
    int environment;
    int busybox_icon;
    int outcol;
    int AES_version;
    int OS;
    int OSFeatures;
    int dialog_xposition[25];
    int dialog_yposition[25];
    char dialog_opened[25];

    int ENV_avserver;
    int olgaman_ID;

    int Event_Timer;
    
    int PCD_Defsize;
    int PreviewDither;
    int PreviewMoveDither;
    int PicmanDither;
    int AutoconvDither, AutoconvPalmode;

    int AES_bgcolor;
    int keyevents;
    int defaultExporter;
    char *scrp_path;
} SYSTEM_INFO;


/* ---------------- Definition der Modul-FÑhigkeiten -------------- */
/* In dieser Struktur legen Edit- und Exportmodule Werte ab, die    */
/*  bestimmen, in welchen Farbtiefen, Datenformaten, Farbsystemen,  */
/*  etc. das Bild verarbeitet werden kann.                          */
/*  Eine Struktur dieses Typs muû in jedem Edit- und Exportmodul    */
/*  global definiert sein und wird vom Startupcode in den Modul-    */
/*  header eingehÑngt.                                              */
/*  --------------------------------------------------------------  */
/*  74 Bytes */
typedef struct
{
/*----------Die ersten 3 Parameter werden von Smurf zur automatischen   */
/*          Transformierung benutzt.                                    */
    /* Farbtiefen, die vom Modul unterstÅtzt werden: */
    char depth1, depth2, depth3, depth4, depth5, depth6, depth7, depth8;
    /* Dazugehîrige Datenformate (FORM_PIXELPAK/FORM_STANDARD/FORM_BOTH) */
    char form1, form2, form3, form4, form5, form6, form7, form8;

    char ext_flag;      /* Ext-Flag: Bit 0=Preview, 1=MMORE, 2=Infotext mîglich */
} MOD_ABILITY;


/*--------------- RÅckgabestruktur vom Exportmodul ---------------- */
/*  Diese Struktur muû vom Exportmodul nach Kodierung des Bildes an */
/*  Smurf zurÅckgegeben werden. Dieser speichert den fertigen Block */
/*  dann ab.                                                        */
/* ---------------------------------------------------------------- */
/*  8 Bytes */
typedef struct
{
    void *pic_data;     /* Daten des zu speichernden Blockes    */
    long f_len;         /* LÑnge des Blockes in Bytes           */
} EXPORT_PIC;


int     f_rslid(SLIDER *slider_struct);
void    setslider(SLIDER *sliderstruct, long value);
void    f_txtinsert(int num, OBJECT *tree, int txt_obj, WINDOW *ws);
