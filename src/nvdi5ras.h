#include <vdi.h>

typedef struct
{
    int red;                                                        /* Rot-IntensitÑt in Promille (0-1000) */
    int green;                                                      /* GrÅn-IntensitÑt in Promille (0-1000) */
    int blue;                                                       /* Blau-IntensitÑt in Promille (0-1000) */
} RGB1000;


/*--------------------------------------------------------------------------------------*/
/* Konstanten fÅr Pixelformate                                                          */
/*--------------------------------------------------------------------------------------*/
#define PX_1COMP    0x01000000L                                     /* Pixel besteht aus einer benutzten Komponente: Farbindex */
#define PX_3COMP    0x03000000L                                     /* Pixel besteht aus drei benutzten Komponenten, z.B. RGB */
#define PX_4COMP    0x04000000L                                     /* Pixel besteht aus vier benutzten Komponenten, z.B. CMYK */

#define PX_REVERSED 0x00800000L                                     /* Pixel wird in umgekehrter Bytreihenfolge ausgegeben */
#define PX_xFIRST   0x00400000L                                     /* unbenutzte Bits liegen vor den benutzen (im Motorola-Format betrachtet) */
#define PX_kFIRST   0x00200000L                                     /* K liegt vor CMY (im Motorola-Format betrachtet) */
#define PX_aFIRST   0x00100000L                                     /* Alphakanal liegen vor den Farbbits (im Motorola-Format betrachtet) */

#define PX_PACKED   0x00020000L                                     /* Bits sind aufeinanderfolgend abgelegt */
#define PX_PLANES   0x00010000L                                     /* Bits sind auf mehrere Ebenen verteilt (Reihenfolge: 0, 1, ..., n) */
#define PX_IPLANES  0x00000000L                                     /* Bits sind auf mehrere Worte verteilt (Reihenfolge: 0, 1, ..., n) */

#define PX_USES1    0x00000100L                                     /* 1 Bit des Pixels wird benutzt */
#define PX_USES2    0x00000200L                                     /* 2 Bit des Pixels werden benutzt */
#define PX_USES3    0x00000300L                                     /* 3 Bit des Pixels werden benutzt */
#define PX_USES4    0x00000400L                                     /* 4 Bit des Pixels werden benutzt */
#define PX_USES8    0x00000800L                                     /* 8 Bit des Pixels werden benutzt */
#define PX_USES15   0x00000f00L                                     /* 15 Bit des Pixels werden benutzt */
#define PX_USES16   0x00001000L                                     /* 16 Bit des Pixels werden benutzt */
#define PX_USES24   0x00001800L                                     /* 24 Bit des Pixels werden benutzt */
#define PX_USES32   0x00002000L                                     /* 32 Bit des Pixels werden benutzt */
#define PX_USES48   0x00003000L                                     /* 48 Bit des Pixels werden benutzt */

#define PX_1BIT     0x00000001L                                     /* Pixel besteht aus 1 Bit */
#define PX_2BIT     0x00000002L                                     /* Pixel besteht aus 2 Bit */
#define PX_3BIT     0x00000003L                                     /* Pixel besteht aus 3 Bit */
#define PX_4BIT     0x00000004L                                     /* Pixel besteht aus 4 Bit */
#define PX_8BIT     0x00000008L                                     /* Pixel besteht aus 8 Bit */
#define PX_16BIT    0x00000010L                                     /* Pixel besteht aus 16 Bit */
#define PX_24BIT    0x00000018L                                     /* Pixel besteht aus 24 Bit */
#define PX_32BIT    0x00000020L                                     /* Pixel besteht aus 32 Bit */
#define PX_48BIT    0x00000030L                                     /* Pixel besteht aus 48 Bit */

#define PX_CMPNTS   0x0f000000L                                     /* Maske fÅr Anzahl der Pixelkomponenten */
#define PX_FLAGS    0x00f00000L                                     /* Maske fÅr diverse Flags */
#define PX_PACKING  0x00030000L                                     /* Maske fÅr Pixelformat */
#define PX_USED     0x00003f00L                                     /* Maske fÅr Anzahl der benutzten Bits */
#define PX_BITS     0x0000003fL                                     /* Maske fÅr Anzahl der Bits pro Pixel */


/*--------------------------------------------------------------------------------------*/
/* Pixelformate fÅr ATARI-Grafik                                                        */
/*--------------------------------------------------------------------------------------*/
#define PX_ATARI1   (PX_PACKED + PX_1COMP + PX_USES1 + PX_1BIT)
#define PX_ATARI2   (PX_IPLANES + PX_1COMP + PX_USES2 + PX_2BIT)
#define PX_ATARI4   (PX_IPLANES + PX_1COMP + PX_USES4 + PX_4BIT)
#define PX_ATARI8   (PX_IPLANES + PX_1COMP + PX_USES8 + PX_8BIT)
#define PX_FALCON15 (PX_PACKED + PX_3COMP + PX_USES16 + PX_16BIT)


/*--------------------------------------------------------------------------------------*/
/* Pixelformate fÅr Macintosh                                                           */
/*--------------------------------------------------------------------------------------*/
#define PX_MAC1     (PX_PACKED + PX_1COMP + PX_USES1 + PX_1BIT)
#define PX_MAC4     (PX_PACKED + PX_1COMP + PX_USES4 + PX_4BIT)
#define PX_MAC8     (PX_PACKED + PX_1COMP + PX_USES8 + PX_8BIT)
#define PX_MAC15    (PX_xFIRST + PX_PACKED + PX_3COMP + PX_USES15 + PX_16BIT)
#define PX_MAC24    (PX_PACKED + PX_3COMP + PX_USES24 + PX_24BIT)
#define PX_MAC32    (PX_xFIRST + PX_PACKED + PX_3COMP + PX_USES24 + PX_32BIT)


/*--------------------------------------------------------------------------------------*/
/* Pixelformate fÅr Grafikkarten                                                        */
/*--------------------------------------------------------------------------------------*/
#define PX_VGA1     (PX_PACKED + PX_1COMP + PX_USES1 + PX_1BIT)
#define PX_VGA4     (PX_PLANES + PX_1COMP + PX_USES4 + PX_4BIT)
#define PX_VGA8     (PX_PACKED + PX_1COMP + PX_USES8 + PX_8BIT)
#define PX_VGA15    (PX_REVERSED + PX_xFIRST + PX_PACKED + PX_3COMP + PX_USES15 + PX_16BIT)
#define PX_VGA16    (PX_REVERSED + PX_PACKED + PX_3COMP + PX_USES16 + PX_16BIT)
#define PX_VGA24    (PX_REVERSED + PX_PACKED + PX_3COMP + PX_USES24 + PX_24BIT)
#define PX_VGA32    (PX_REVERSED + PX_xFIRST + PX_PACKED + PX_3COMP + PX_USES24 + PX_32BIT)

#define PX_MATRIX16 (PX_PACKED + PX_3COMP + PX_USES16 + PX_16BIT)

#define PX_NOVA32   (PX_PACKED + PX_3COMP + PX_USES24 + PX_32BIT)

#define PX_PC32     (PX_REVERSED + PX_PACKED + PX_3COMP + PX_USES24 + PX_32BIT)


/*--------------------------------------------------------------------------------------*/
/* Pixelformate fÅr Drucker                                                             */
/*--------------------------------------------------------------------------------------*/
#define PX_PRN1     (PX_PACKED + PX_1COMP + PX_USES1 + PX_1BIT)
#define PX_PRN8     (PX_PACKED + PX_1COMP + PX_USES8 + PX_8BIT)
#define PX_PRN32    (PX_xFIRST + PX_PACKED + PX_3COMP + PX_USES24 + PX_32BIT)


/*--------------------------------------------------------------------------------------*/
/* bevorzugte (schnelle) Pixelformate fÅr Bitmaps                                       */
/*--------------------------------------------------------------------------------------*/
#define PX_PREF1    (PX_PACKED + PX_1COMP + PX_USES1 + PX_1BIT)
#define PX_PREF2    (PX_PACKED + PX_1COMP + PX_USES2 + PX_2BIT)
#define PX_PREF4    (PX_PACKED + PX_1COMP + PX_USES4 + PX_4BIT)
#define PX_PREF8    (PX_PACKED + PX_1COMP + PX_USES8 + PX_8BIT)
#define PX_PREF15   (PX_xFIRST + PX_PACKED + PX_3COMP + PX_USES15 + PX_16BIT)
#define PX_PREF32   (PX_xFIRST + PX_PACKED + PX_3COMP + PX_USES24 + PX_32BIT)

/*--------------------------------------------------------------------------------------*/
/* Farbtabellen                                                                         */
/*--------------------------------------------------------------------------------------*/
enum
{
    CSPACE_RGB = 0x0001,
    CSPACE_ARGB = 0x0002,
    CSPACE_CMYK = 0x0004
};

enum
{
    CSPACE_1COMPONENT = 0x0001,
    CSPACE_2COMPONENTS = 0x0002,
    CSPACE_3COMPONENTS = 0x0003,
    CSPACE_4COMPONENTS = 0x0004
};

typedef struct
{
    unsigned int    reserved;
    unsigned int    red;
    unsigned int    green;
    unsigned int    blue;
} COLOR_RGB;

typedef struct
{
    unsigned int    cyan;
    unsigned int    magenta;
    unsigned int    yellow;
    unsigned int    black;
} COLOR_CMYK;

typedef union
{
    COLOR_RGB   rgb;
    COLOR_CMYK  cmyk;
} COLOR_ENTRY;

#define COLOR_TAB_MAGIC 'ctab'

typedef struct                                                      /* Farbtabelle */
{
    long    magic;                                                  /* 'ctab' */
    long    length;
    long    format;                                                 /* Format (0) */
    long    reserved;
    
    long    map_id;                                                 /* Kennung der Farbtabelle */
    long    color_space;                                            /* Farbraum */
    long    flags;                                                  /* interne Flags */
    long    no_colors;                                              /* Anzahl der FarbeintrÑge */

    long    reserved1;
    long    reserved2;
    long    reserved3;
    long    reserved4;

    COLOR_ENTRY colors[];
} COLOR_TAB;

/* vordefinierte Tabelle mit 256 EintrÑgen */
typedef struct                                                      /* Farbtabelle */
{
    long    magic;                                                  /* 'ctab' */
    long    length;
    long    format;                                                 /* Format (0) */
    long    reserved;
    
    long    map_id;                                                 /* Kennung der Farbtabelle */
    long    color_space;                                            /* Farbraum */
    long    flags;                                                  /* interne Flags */
    long    no_colors;                                              /* Anzahl der FarbeintrÑge */

    long    reserved1;
    long    reserved2;
    long    reserved3;
    long    reserved4;

    COLOR_ENTRY colors[256];
} COLOR_TAB256;

typedef void    INVERSE_CTAB;                                       /* der Aufbau ist nicht von Bedeutung */


/*--------------------------------------------------------------------------------------*/
/* Verweise auf Farbtabellen                                                            */
/*--------------------------------------------------------------------------------------*/
typedef COLOR_TAB       *CTAB_PTR;
typedef COLOR_TAB       *CTAB_REF;
typedef INVERSE_CTAB    *ITAB_REF;


/*--------------------------------------------------------------------------------------*/
/*  Bitmaps                                                                             */
/*--------------------------------------------------------------------------------------*/
#define CBITMAP_MAGIC   'cbtm'

typedef struct  _gcbitmap                                           /* îffentliche Bitmapbeschreibung (mit Versionsheader) */
{
    long            magic;                                          /* Strukturkennung 'cbtm' */
    long            length;                                         /* StrukturlÑnge */
    long            format;                                         /* Strukturformat (0) */
    long            reserved;                                       /* reserviert (0) */

    char            *addr;                                          /* Adresse der Bitmap */
    long            width;                                          /* Breite einer Zeile in Bytes */
    long            bits;                                           /* Bittiefe */
    unsigned long   px_format;                                      /* Pixelformat */

    long            xmin;                                           /* minimale diskrete x-Koordinate der Bitmap */
    long            ymin;                                           /* minimale diskrete y-Koordinate der Bitmap */
    long            xmax;                                           /* maximale diskrete x-Koordinate der Bitmap + 1 */
    long            ymax;                                           /* maximale diskrete y-Koordinate der Bitmap + 1 */

    CTAB_REF        ctab;                                           /* Verweis auf die Farbtabelle oder 0L */
    ITAB_REF        itab;                                           /* Verweis auf die inverse Farbtabelle oder 0L */
    long            reserved0;                                      /* reserviert (0) */
    long            reserved1;                                      /* reserviert (0) */
} GCBITMAP;


/*--------------------------------------------------------------------------------------*/
/* Transfermodi fÅr Bitmaps                                                             */
/*--------------------------------------------------------------------------------------*/
/* Moduskonstanten */
#define T_NOT               4                                       /* Konstante fÅr Invertierung bei logischen Transfermodi */
#define T_COLORIZE          16                                      /* Konstante fÅr EinfÑrbung */

#define T_LOGIC_MODE        0
#define T_DRAW_MODE         32
#define T_ARITH_MODE        64                                      /* Konstante fÅr Arithmetische Transfermodi */
#define T_DITHER_MODE       128                                     /* Konstante fÅrs Dithern */

/* logische Transfermodi */
#define T_LOGIC_COPY        T_LOGIC_MODE+0
#define T_LOGIC_OR          T_LOGIC_MODE+1
#define T_LOGIC_XOR         T_LOGIC_MODE+2
#define T_LOGIC_AND         T_LOGIC_MODE+3
#define T_LOGIC_NOT_COPY    T_LOGIC_MODE+4
#define T_LOGIC_NOT_OR      T_LOGIC_MODE+5
#define T_LOGIC_NOT_XOR     T_LOGIC_MODE+6
#define T_LOGIC_NOT_AND     T_LOGIC_MODE+7

/* Zeichenmodi */
#define T_REPLACE           T_DRAW_MODE+0
#define T_TRANSPARENT       T_DRAW_MODE+1
#define T_HILITE            T_DRAW_MODE+2
#define T_REVERS_TRANSPARENT    T_DRAW_MODE+3

/* arithmetische Transfermodi */
#define T_BLEND             T_ARITH_MODE+0
#define T_ADD               T_ARITH_MODE+1
#define T_ADD_OVER          T_ARITH_MODE+2
#define T_SUB               T_ARITH_MODE+3
#define T_MAX               T_ARITH_MODE+5
#define T_SUB_OVER          T_ARITH_MODE+6
#define T_MIN               T_ARITH_MODE+7


/*--------------------------------------------------------------------------------------*/
/* Strukturen                                                                           */
/*--------------------------------------------------------------------------------------*/

typedef struct                                                      /* Rechteck fÅr 16-Bit-Koordinaten */
{
    int x1;
    int y1;
    int x2;
    int y2;
} RECT16;

typedef struct                                                      /* Rechteck fÅr 32-Bit-Koordinaten */
{
    long    x1;
    long    y1;
    long    x2;
    long    y2;
} RECT32;

typedef struct
{
    long    size;                                       /* LÑnge der Struktur, muû vor vqt_xfntinfo() gesetzt werden */
    int format;                                         /* Fontformat, z.B. 4 fÅr TrueType */
    int id;                                             /* Font-ID, z.B. 6059 */
    int index;                                          /* Index */
    char    font_name[50];                              /* vollstÑndiger Fontname, z.B. "Century 725 Italic BT" */
    char    family_name[50];                            /* Name der Fontfamilie, z.B. "Century725 BT" */
    char    style_name[50];                             /* Name des Fontstils, z.B. "Italic" */
    char    file_name1[200];                            /* Name der 1. Fontdatei, z.B. "C:\FONTS\TT1059M_.TTF" */
    char    file_name2[200];                            /* Name der optionalen 2. Fontdatei */
    char    file_name3[200];                            /* Name der optionalen 3. Fontdatei */
    int pt_cnt;                                         /* Anzahl der Punkthîhen fÅr vst_point(), z.B. 10 */
    int pt_sizes[64];                                   /* verfÅgbare Punkthîhen, z.B. { 8, 9, 10, 11, 12, 14, 18, 24, 36, 48 } */
} XFNT_INFO;


/*--------------------------------------------------------------------------------------*/
/* Funktionsdeklarationen                                                               */
/*--------------------------------------------------------------------------------------*/
/* extern int   v_opnprn(int base_handle, PRN_SETTINGS *settings, int *work_out); */
extern void     v_opnbm(int *work_in, MFDB *bitmap, int *handle, int *work_out);
extern int  v_resize_bm(int handle, int width, int height, long byte_width, char *addr);
extern int  v_open_bm(int base_handle, GCBITMAP *bitmap, int color_flags, int unit_flags, int pixel_width, int pixel_height);
extern void     v_clsbm(int handle);
#define v_close_bm( handle )    v_clsbm( handle )
extern void     vq_scrninfo(int handle, int *work_out);
/* extern int   vq_ext_devinfo(int handle, int id, int *exists, char *file_path, char *file_name, char *name); */

extern void _v_bez(int handle, int count, int *xy, char *bezarr, int *extent, int *totpts, int *totmoves);
extern void _v_bez_fill(int handle, int count, int *xy, char *bezarr, int *extent, int *totpts, int *totmoves);

extern void     v_ftext_wide(int handle, int x, int y, unsigned int *string);
extern int  vst_map_mode(int handle, int mode);
extern void     vst_width(int handle, int width, int *char_width, int *char_height, int *cell_width, int *cell_height);
extern void     vst_track_offset(int handle, fix31 offset, int pair_mode, int *tracks, int *pairs);
extern int  vst_name(int handle, int font_format, char *font_name, char *ret_name);

extern unsigned int vqt_char_index(int handle, unsigned int src_index, int src_mode, int dst_mode);
extern int  vqt_ext_name(int handle, int index, char *name, unsigned int *font_format, unsigned int *flags);
extern int  v_get_outline(int handle, unsigned int index, int x_offset, int y_offset, int *pts, char *flags, int max_pts);
extern int  vqt_xfntinfo(int handle, int flags, int id, int index, XFNT_INFO *info);
extern void     vqt_extent_wide(int handle, unsigned int *string, int *extent);
extern void     vqt_real_extent(int handle, int x, int y, char *string, int *extent);
extern void     vqt_real_extent_wide(int handle, int x, int y, unsigned int *string, int *extent);
extern int  vqt_name_and_id(int handle, int font_format, char *font_name, char *ret_name);

/* Vordergrundfarbe kompatibel zu GEM/3-Metafiles setzen */
extern void v_setrgb(int handle, int type, int r, int g, int b);

/* Vordergrundfarbe setzen */
extern int  vst_fg_color(int handle, long color_space, COLOR_ENTRY *fg_color);
extern int  vsf_fg_color(int handle, long color_space, COLOR_ENTRY *fg_color);
extern int  vsl_fg_color(int handle, long color_space, COLOR_ENTRY *fg_color);
extern int  vsm_fg_color(int handle, long color_space, COLOR_ENTRY *fg_color);
extern int  vsr_fg_color(int handle, long color_space, COLOR_ENTRY *fg_color);

/* Hintergrundfarbe setzen */
extern int  vst_bg_color(int handle, long color_space, COLOR_ENTRY *bg_color);
extern int  vsf_bg_color(int handle, long color_space, COLOR_ENTRY *bg_color);
extern int  vsl_bg_color(int handle, long color_space, COLOR_ENTRY *bg_color);
extern int  vsm_bg_color(int handle, long color_space, COLOR_ENTRY *bg_color);
extern int  vsr_bg_color(int handle, long color_space, COLOR_ENTRY *bg_color);

/* Vordergrundfarbe erfragen */
extern long vqt_fg_color(int handle, COLOR_ENTRY *fg_color);
extern long vqf_fg_color(int handle, COLOR_ENTRY *fg_color);
extern long vql_fg_color(int handle, COLOR_ENTRY *fg_color);
extern long vqm_fg_color(int handle, COLOR_ENTRY *fg_color);
extern long vqr_fg_color(int handle, COLOR_ENTRY *fg_color);

/* Hintergrundfarbe erfragen */
extern long vqt_bg_color(int handle, COLOR_ENTRY *bg_color);
extern long vqf_bg_color(int handle, COLOR_ENTRY *bg_color);
extern long vql_bg_color(int handle, COLOR_ENTRY *bg_color);
extern long vqm_bg_color(int handle, COLOR_ENTRY *bg_color);
extern long vqr_bg_color(int handle, COLOR_ENTRY *bg_color);

/* diverse Modi parametrisieren */
extern int  vs_hilite_color(int handle, long color_space, COLOR_ENTRY *hilite_color);
extern int  vs_min_color(int handle, long color_space, COLOR_ENTRY *min_color);
extern int  vs_max_color(int handle, long color_space, COLOR_ENTRY *max_color);
extern int  vs_weight_color(int handle, long color_space, COLOR_ENTRY *weight_color);

extern long vq_hilite_color(int handle, COLOR_ENTRY *hilite_color);
extern long vq_min_color(int handle, COLOR_ENTRY *min_color);
extern long vq_max_color(int handle, COLOR_ENTRY *max_color);
extern long vq_weight_color(int handle, COLOR_ENTRY *weight_color);

/* Farben erfragen */
extern unsigned long    v_color2value(int handle, long color_space, COLOR_ENTRY *color);
extern long v_value2color(int handle, unsigned long value, COLOR_ENTRY *color);
extern long v_color2nearest(int handle, long color_space, COLOR_ENTRY *color, COLOR_ENTRY *nearest_color);
extern long vq_px_format(int handle, unsigned long *px_format);

/* Farbtabellen */
extern int  vs_ctab(int handle, COLOR_TAB *ctab);
extern int  vs_ctab_entry(int handle, int index, long color_space, COLOR_ENTRY *color);
extern int  vs_dflt_ctab(int handle);

extern int  vq_ctab(int handle, long ctab_length, COLOR_TAB *ctab);
extern long vq_ctab_entry(int handle, int index, COLOR_ENTRY *color);
extern long vq_ctab_id(int handle);
extern int  v_ctab_idx2vdi(int handle, int index);
extern int  v_ctab_vdi2idx(int handle, int vdi_index);
extern unsigned long    v_ctab_idx2value(int handle, int index);
extern long v_get_ctab_id(int handle);
extern int  vq_dflt_ctab(int handle, long ctab_length, COLOR_TAB *ctab);
extern COLOR_TAB    *v_create_ctab(int handle, long color_space, unsigned long px_format);
extern int  v_delete_ctab(int handle, COLOR_TAB *ctab);

/* inverse Farbtabellen */
extern ITAB_REF v_create_itab(int handle, COLOR_TAB *ctab, int bits);
extern int  v_delete_itab(int handle, ITAB_REF itab);

/* Bitmapausgabe */
extern void vr_transfer_bits(int handle, GCBITMAP *src_bm, GCBITMAP *dst_bm, int *src_rect, int *dst_rect, int mode);

/* Drucker */
extern int  v_orient(int handle, int orientation);
extern int  v_copies(int vdi_handle, int copies);
extern long vq_prn_scaling(int handle);
extern int  vq_margins(int handle, int *top_margin, int *bottom_margin, int *left_margin, int *right_margin, int *hdpi, int *vdpi);
extern int  vs_document_info(int handle, int type, void *s, int wchar);
