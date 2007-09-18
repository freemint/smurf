/* Headerdatei zum Fontselektoraufruf */


/* Smurfstruktur */
typedef struct
{
	int index;
	int ID;
	int size;
	int FX;
} FONT_INFO;


/* Strukturen, Funktionen und Deklarationen */
/* fÅr xFSL-Fontselektoren */
typedef struct
{
	unsigned long xfsl;			/* Magic 'xFSL' */
	unsigned int  revision;		/* Schnittstellen-Revision */
	unsigned long product;		/* Kennung des Fontselektors */
	unsigned int  version;		/* Version des Fontselektors */
    int cdecl(*xfsl_input)(int vdihandle, unsigned int fontflags,
						   const char *headline, int *id, int *size);
	int	xfsl_init;				/* Init-Aufruf */
	int	xfsl_event;				/* Event-Aufruf */
	int	xfsl_exit;				/* Exit-Aufruf */
	int	xfsl_info;				/* Info-Aufruf */
} xFSL;

typedef struct
{
	long size;					/* LÑnge der Struktur, muû vor vqt_xfntinfo() gesetzt werden */
	int format;					/* Fontformat, z.B. 4 fÅr TrueType */
	int id;						/* Font-ID, z.B. 6059 */
	int index;					/* Index */
	char font_name[50];			/* vollstÑndiger Fontname, z.B. "Century 725 Italic BT" */
	char family_name[50];		/* Name der Fontfamilie, z.B. "Century725 BT" */
	char style_name[50];		/* Name des Fontstils, z.B. "Italic" */
	char file_name1[200];		/* Name der 1. Fontdatei, z.B. "C:\FONTS\TT1059M_.TTF" */
	char file_name2[200];		/* Name der optionalen 2. Fontdatei */
	char file_name3[200];		/* Name der optionalen 3. Fontdatei */
	int pt_cnt;					/* Anzahl der Punkthîhen fÅr vst_point(), z.B. 10 */
	int pt_sizes[64];			/* verfÅgbare Punkthîhen,*/
} XFNT_INFO;


/* Strukturen, Funktionen und Deklarationen */
/* fÅr den MagiC-Fontselektor */

/* Beschreibung von <font_flags>: */
#define FNTS_BTMP	1			/* Bitmapfonts anzeigen */
#define FNTS_OUTL	2			/* Vektorfonts anzeigen */
#define FNTS_MONO	4			/* Ñquidistante Fonts anzeigen */
#define FNTS_PROP	8			/* proportionale Fonts anzeigen */

/* Beschreibung von <dialog_flags>: */
#define  FNTS_3D		1		/* Auswahl im 3D-Look anzeigen */

/* Beschreibung von <button_flags>: */
#define FNTS_SNAME		0x01	/* Checkbox fÅr die Namen selektieren */
#define FNTS_SSTYLE		0x02	/* Checkbox fÅr die Stile selektieren */
#define FNTS_SSIZE		0x04	/* Checkbox fÅr die Hîhe selektieren */
#define FNTS_SRATIO		0x08	/* Checkbox fÅr das VerhÑltnis
								   Breite/Hîhe selektieren */

#define FNTS_CHNAME		0x0100	/* Checkbox fÅr die Namen anzeigen */
#define FNTS_CHSTYLE	0x0200	/* Checkbox fÅr die Stile anzeigen */
#define FNTS_CHSIZE		0x0400	/* Checkbox fÅr die Hîhe anzeigen */
#define FNTS_CHRATIO	0x0800	/* Checkbox fÅr das VerhÑltnis
								   Breite/Hîhe anzeigen */
#define FNTS_RATIO		0x1000	/* VerhÑltnis Breite/Hîhe ein-
								   stellbar */
#define FNTS_BSET		0x2000	/* Button "setzen" anwÑhlbar */
#define FNTS_BMARK		0x4000	/* Button "markieren" anwÑhlbar */

#define BUTTON_FLAGS	0

/* Beschreibung von fnts_do-RÅckgabe: */
#define	FNTS_CANCEL		1		/* "Abbruch" wurde angewÑhlt */
#define	FNTS_OK			2		/* "OK" wurde gedrÅckt */
#define	FNTS_SET		3		/* "setzen" wurde angewÑhlt */
#define	FNTS_MARK		4		/* "markieren" wurde betÑtigt */
#define	FNTS_OPT		5		/* der applikationseigene Button wurde ausgewÑhlt */

typedef	void	*FNT_DIALOG;


FNT_DIALOG *fnts_create(int vdi_handle, int no_fonts,
						int font_flags, int dialog_flags,
						char *sample, char *opt_button);

int fnts_delete(FNT_DIALOG *fnt_dialog, int handle);

int fnts_do(FNT_DIALOG *fnt_dialog, long id_in, long pt_in, 
			long ratio_in, long *id, long *pt, long *ratio);