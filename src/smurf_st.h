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

/*---------------- Strukturen -----------------*/

/* Wird spÑter mit Dpathconf() fÅr den Modulpfad selbst bestimmt und Runtime festgelegt. */
/* Falls Dpathconf() nicht existiert, wird 12 angenommen. */
#define NAME_MAX 255        /* maximale LÑnge Dateiname ohne Pfad */

typedef struct DIRENTRY
{
    char *modname;
    struct DIRENTRY *next;
};

/*  Displaymode - Dither- und Palettenmodi fÅr 
    verschiedene Quellbild-Farbtiefen           */
typedef struct
{
    int dither_24, syspal_24;
    int dither_8, syspal_8;
    int dither_4, syspal_4;
    int palette_mode;
} DISPLAY_MODES;


/* Die Namen und Extensions aller Importmodule */
typedef struct
{
    char *imp_mod_list[200];
    char *mod_exts[11][201];
} IMPORT_LIST;


typedef struct
{
    int ascii_code;
    int scan_code;
    char shift;
    char ctrl;
    char alt;
} KINFO;


typedef struct
{
    char cut[3];
    int ltitle;
    int litem;
} CUTTAB;

typedef struct
{
    char    *in_dos;                 /* Adresse der DOS- Semaphore */
    int     *dos_time;               /* Adresse der DOS- Zeit      */
    int     *dos_date;               /* Adresse des DOS- Datums    */
    long    res1;                    /*                            */
    long    res2;                    /*                            */
    long    res3;                    /* ist 0L                     */
    void    *act_pd;                 /* Laufendes Programm         */
    long    res4;                    /*                            */
    int     res5;                    /*                            */
    void    *res6;                   /*                            */
    void    *res7;                   /* interne DOS- Speicherliste */
    void    (*resv_intmem)();        /* DOS- Speicher erweitern    */
    long    (*etv_critic)();         /* etv_critic des GEMDOS      */
    char    *((*err_to_str)(char e)); /* Umrechnung Code->Klartext  */
    long    res8;                    /*                            */
    long    res9;                    /*                            */
    long    res10;                   /*                            */
} DOSVARS;


typedef struct
{
    long magic;                   /* muû $87654321 sein         */
    void *membot;                 /* Ende der AES- Variablen    */
    void *aes_start;              /* Startadresse               */
    long magic2;                  /* ist 'MAGX'                 */
    long date;                    /* Erstelldatum               */
    void (*chgres)(int res, int txt);  /* Auflîsung Ñndern    */
    long (**shel_vector)(void);   /* residentes Desktop         */
    char *aes_bootdrv;            /* von hieraus wurde gebootet */
    int *vdi_device;             /* vom AES benutzter Treiber  */
    void *reservd1;               /* reserviert                 */
    void *reservd2;               /* reserviert                 */
    void *reservd3;               /* reserviert                 */
    int version;                 /* Version ($0201 ist V2.1)   */
    int release;                 /* 0=alpha..3=release         */
} AESVARS;


typedef struct
{
    unsigned long config_status;
    DOSVARS *dosvars;
    AESVARS *aesvars;
} MAGX_COOKIE;

typedef struct
{
    int header_length;
    long data_type;
    long data_length;
    char data_name[65];
    char file_name[257];
} DD_HEADER;


/*------------- Fadenkreuz-Struktur ---------*/ 
typedef struct
{
    char anzahl;            /* Anzahl an Positionsmarkern - max. 6 */
    int xpos[6], ypos[6];   /* Position im Bild */
    int mod_pic[6];         /* Bildnummer (Modul, 1-6) fÅr jeden Marker */
    int smurfpic[6];        /* Smurf-Bildnummer fÅr jeden Marker */
} CROSSHAIR;

/*------------ Blockmodus-Konfiguration ------*/
typedef struct 
{
    int mode;
    long opacity;
    int transparent;    /* Bit 0=s, bit 1=w */
} BLOCKMODE;

/*------------AV_COMM-Struktur, Semaphoren fÅr die Kommunikation
        bei D&D-Operationen (AV_WHAT_IZIT/THAT_IZIT) per AV-Protokoll.
        Damit der Messagehandler weiû, auf was fÅr eine Aktion der 
        WHAT_IZIT gesendet wurde und der THAT_IZIT kommt. */

typedef struct
{
    int type;           /* Typ der gedraggten Daten (0= Bild, 1=Block) */
    int windowhandle;   /* Handle des Smurf-Fensters, aus dem die Daten kommen */
    int keystate;       /* Zustand der Sondertasten beim Auslîsen der Aktion */
} AV_COMM;



typedef struct export_config
{
    int export_mod_num;             /* ID des laufenden Exporters */
    int exp_depth, exp_form, exp_align, exp_colsys;
    int exp_colred, exp_dither;     /* Dither/Palettenkonfiguration fÅr den Export */
    int exp_fix_red[256],           /* Palette fÅr FIXPAL-Palettenmodus */
        exp_fix_green[256],
        exp_fix_blue[256];
} EXPORT_CONFIG;


typedef struct extend_module_config
{
    int id;                 /* vollstÑndige Modul-ID (alle 16 Bits!) */
    int notify_types[10];   /* bis zu 10 Ereignisarten, Åber die das Modul informiert werden will */
    int windhandle;         /* windowhandle fÅr Eventweiterleitung */
} EXT_MODCONF;
