/* ------------------------------------------------------------------------- */
/* ----- Selectric Structure ----------------------------------------------- */
/* ------------------------------------------------------------------------- */

typedef struct
{
	unsigned long	id;				/* Selectric ID (`SLCT') */
	unsigned int	version;		/* Version (BCD-Format) */
	struct
	{
		unsigned           : 7;		/* reserviert */
		unsigned todaytime : 1;		/* aktuelle Dateien mit Uhrzeit */
		unsigned pthsav    : 1;		/* TOS-Pfade sichern */
		unsigned stdest    : 1;		/* Im Zielpfad bleiben */
		unsigned           : 1;		/* reserviert */
		unsigned numsrt    : 1;		/* numerisches Sortieren */
		unsigned lower     : 1;		/* Kleinbuchstaben benutzen */
		unsigned dclick    : 1;		/* Ordner mit Doppelklick */
		unsigned hidden    : 1;		/* versteckte Dateien */
		unsigned onoff     : 1;		/* Selectric¿ AN/AUS */
	} config;
	int		sort;					/* Sortiermodus (neg. = rev.) */
	int		num_ext;				/* Anzahl Extensions */
	char	*(*ext)[];				/* Standard-Extensions */
	int		num_paths;				/* Anzahl Pfade */
	char	*(*paths)[];			/* Standard-Pfade */
	int		comm;					/* communication word */
	int		in_count;				/* input counter */
	void	*in_ptr;				/* input pointer */
	int		out_count;				/* output counter */
	void	*out_ptr;				/* output pointer */
	int		cdecl (*get_first)(DTA *dta, int attrib);
	int 	cdecl (*get_next)(DTA *dta);
	int		cdecl (*release_dir)(void);
	} SLCT_STR;

/* ------------------------------------------------------------------------- */