#define	PDLG_CANCEL	1						/* "Abbruch" wurde angewÑhlt */
#define	PDLG_OK		2

#define	PDLG_3D		1						/* Dialog im 3D-Stil anzeigen */

#define	PDLG_PREFS	0						/* Einstelldialog anzeigen */
#define	PDLG_PRINT	1						/* Druckdialog anzeigen */

typedef struct
{
	long	magic;							/* 'pset' */
	long	length;							/* StrukturlÑnge */
	long	format;							/* Strukturtyp */
	long	reserved;

	long	page_flags;						/* Flags, u.a. gerade Seiten, ungerade Seiten */
	int		first_page;						/* erste zu druckende Seite */
	int		last_page;						/* letzte zu druckende Seite */
	int		no_copies;						/* Anzahl der Kopien */
	int		orientation;					/* Drehung */
	long	scale;							/* Skalierung: 0x10000L entspricht 100% */

	int		driver_id;						/* VDI-GerÑtenummer */
	int		driver_type;					/* Typ des eingestellten Treibers */
	long	driver_mode;					/* Flags, u.a. fÅr Hintergrunddruck */
	long	reserved1;
	long	reserved2;
	
	long	printer_id;						/* Druckernummer */
	long	mode_id;						/* Modusnummer */
	int		mode_hdpi;						/* horizontale Auflîsung in dpi */
	int		mode_vdpi;						/* vertikale Auflîsung in dpi */
	long	quality_id;						/* Druckmodus (hardwÑremÑûige QualitÑt, z.B. Microweave oder Econofast) */

	long	color_mode;						/* Farbmodus */
	long	plane_flags;					/* Flags fÅr auszugebende Farbebenen (z.B. nur cyan) */
	long	dither_mode;					/* Rasterverfahren */
	long	dither_value;					/* Parameter fÅr das Rasterverfahren */

	long	size_id;						/* Papierformat */
	long	type_id;						/* Papiertyp (normal, glossy) */
	long	input_id;						/* Papiereinzug */
	long	output_id;						/* Papierauswurf */

	long	contrast;						/* Kontrast: 0x10000L entspricht Normaleinstellung */
	long	brightness;						/* Helligkeit: 0x1000L entspricht Normaleinstellung */
	long	reserved3;
	long	reserved4;

	long	reserved5;
	long	reserved6;
	long	reserved7;
	long	reserved8;

	char	device[128];					/* Dateiname fÅr den Ausdruck */

	struct
	{
		char	inside[120];
	} mac_settings;
} PRN_SETTINGS;

typedef	void *PRN_DIALOG;

typedef struct																/* Ereignisstruktur fÅr EVNT_multi(), Fensterdialoge, etc. */
{
	int	mwhich;
	int	mx;
	int	my;
	int	mbutton;
	int	kstate;
	int	key;
	int	mclicks;
	int	reserved[9];
	int	msg[16];
} EVNT;


PRN_DIALOG *pdlg_create(int dialog_flags);
int pdlg_delete(PRN_DIALOG *prn_dialg);
int pdlg_open(PRN_DIALOG *prn_dialog, PRN_SETTINGS *settings,
			  char *document_name, int option_flags, int x, int y);
int pdlg_close(PRN_DIALOG *prn_dialog, int *x, int *y);
int pdlg_update(PRN_DIALOG *prn_dialog, char *name);
PRN_SETTINGS *pdlg_new_settings(PRN_DIALOG *prn_dialog);
int pdlg_free_settings(PRN_SETTINGS *settings);
int pdlg_evnt(PRN_DIALOG *prn_dialog, PRN_SETTINGS *settings,
			   EVNT *events, int *button);
void EVNT_multi(int evtypes, int nclicks, int bmask, int bstate,
							MOBLK *m1, MOBLK *m2, unsigned long ms,
							EVNT *event);