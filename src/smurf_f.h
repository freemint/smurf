#ifndef _smurf_f_h_
#define _smurf_f_h_

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

/* **********************************************************************/
/* ----------------------- Fileroutinen ------------------------------- */
/* Alles, was mit Dateizugriffen, Bild laden, etc zu tun hat            */
/* **********************************************************************/

    struct DIRENTRY *build_up_filelist(char *path, char *ext, int pathlen);
    struct DIRENTRY *insert_entry(struct DIRENTRY *ende, char *string);
    void    destroy_filelist(struct DIRENTRY *begin);

/* Dateihandling mit Fileselector */
    int     file_save(char *stext, char *buf, long length);
    void    file_load(char *ltext, char **dateien, int mode);
    int     f_fsbox(char *Path, char *fbtext, char selectart);

/*  File laden */
    char    *fload(char *Path, int header);

/* Bild importieren */
    void    reload_pic(WINDOW *picwindow);
    int     f_import_pic(SMURF_PIC *smurf_picture, char *filename);
    int     f_formhandle(int picture_to_load, int module_ret, char *namename);
    char    *f_do_pcd(char *Path);

    void    save_config(void);
    int     load_config(void);







/* **********************************************************************/
/* ----------------------- Event- und Messagefunktionen --------------- */
/* How to speak AES - Dialog mit dem Betriebssystem u.Ñ.                */
/* **********************************************************************/
void    f_event(void);              /* Allg. Event-Funktion                 */
int     f_handle_message(void);     /* AES-Message-Handler                  */
void    f_pic_event(WINDOW *picwindow, int event_type, int windnum);            /* Auswertung von Events im Bildfenster */
void    picwin_keyboard(int key_scancode, int key_at_event, WINDOW *picwin);

void    check_windclose(int windnum);   /* Auswerten von Windowdialog-AbbrÅchen */
int     f_handle_menuevent(int *message);

void cdecl message_handler(int *msg);   /* Messagehandler fÅr Betrieb des FSEL im Fenster */

void lock_Smurf(void);
void unlock_Smurf(void);


/* **********************************************************************/
/* --------------------- Ditherroutinen u.Ñ. -------------------------- */
/* alles zur Bilddarstellung.                                           */
/* **********************************************************************/

/*---- Dither-Routinen - Maindispatcher ----*/
    int     f_dither(SMURF_PIC *dest, SYSTEM_INFO *sys_info, int pic_changed, GRECT *part, DISPLAY_MODES *displayOpt);

/*---- Histogrammberechnung + Median Cut ----*/
    long *make_histogram(SMURF_PIC *picture);
    void median_cut(long *histogram, SMURF_PIC *picture, SYSTEM_INFO *sysinfo);

/*---- Dit-Routs C Subdispatcher ----*/
    void    f_dither_24_1(SYSTEM_INFO *sysinfo, SMURF_PIC *picture, char *where_to, int mode, char *nct, GRECT *part);
    void    f_transform_to_standard(SMURF_PIC *picture,char *paddr);
    void    f_align_standard(SMURF_PIC *picture,char *paddr, GRECT *part);

/* Anpassen der Palette */
    char    *fix_palette(SMURF_PIC *dest, SYSTEM_INFO *sys_info);

/* Dither-Routinen  -   Assembler */
    void    floyd24Bit_Palette(long *par);      /* 24-Bit " " mit eigener Palette */

/* Nearest-Color-Suchfunktion (Assembler) */
    int seek_nearest_col(long *par, int maxcol);

/* Routinen fÅr 16 und 24Bit-Bildschirmmodus */
    void direct2screen(SMURF_PIC *picture, char *where_to, GRECT *part);

/*--------------------- Transformers -----------------------*/
int     f_convert(SMURF_PIC *picture, MOD_ABILITY *mod_abs, char modcolform, char mode, char automatic);

int     tfm_std_to_std(SMURF_PIC *smurf_pic, char dst_depth, char mode);
int     tfm_std_to_pp(SMURF_PIC *picture, char dst_depth, char mode);
int     tfm_pp_to_std8(SMURF_PIC *picture, char mode);
int     tfm_8_to_16(SMURF_PIC *picture, char mode);
int     tfm_8_to_24(SMURF_PIC *picture, char mode);
int     tfm_16_to_24(SMURF_PIC *picture, char mode);
int     tfm_24_to_16(SMURF_PIC *picture, char mode);
int     tfm_bgr_to_rgb(SMURF_PIC *picture, char mode);
int     tfm_cmy_to_rgb(SMURF_PIC *picture, char mode);
int     tfm_rgb_to_grey(SMURF_PIC *picture, char mode);


void    transform_pic(void);        /* "Bild Wandeln" - Dialog */

/*-------- Pixel Packed -> Standardformat - Routinen    ----------------*/
int     setpix_standard(char *buf16, char *dest, int depth, long planelen, int howmany);
int     setpix_pp(char *buf16, char *dest, int depth, long planelen, int howmany);
int     setpix_standard_16(char *buf16, char *dest, int depth, long planelen, int howmany);
void    get_standard_pix(void *st_pic, void *buf16, int planes, long planelen);



/* **********************************************************************/
/* --------------------------- Bildanpassung -------------------------- */
/* CMYK nach YCBCR oder RGB, Intel nach Motorola, usw.                  */
/* **********************************************************************/
SMURF_PIC   *f_generate_newpic(int wid, int hgt, int depth);


/* **********************************************************************/
/* ------------------------ SMURF-Systemfunktionen -------------------- */
/* Was, Bin ich, wo bin ich, wieviele Farben hab' ich?                  */
/* **********************************************************************/
void    f_scan_edit(void);                  /* Edit Modules scannen */
void    f_scan_export(void);                /* Edit Modules scannen */
void    f_scan_dither(void);                /* Dither Modules scannen */
int     f_init_system(void);                /* Systeminitialisierung - die Mutterfunktion */
void    f_init_palette(void);               /* Palette auslesen     */
char    *f_init_table(void);                /* NC-Tabelle init      */
void    f_init_bintable(OBJECT *rsc);       /* BinÑrwert-Table init */

void    *SMalloc(long amount);
int     SMfree(void *ptr);

void    actualize_menu(void);       /* MenÅzeile aktualisieren */

int     handle_keyboardevent(WINDOW *wind_s, int scancode, int *sel_object);

void    init_smurfrsc(char *rscpath);
void    fix_rsc(void);              /* RSC-Anpassung an OS, Farbtiefe, AES, etc... */
void    f_init_popups(void);        /* Alle Popups initialisieren */
void    f_init_sliders(void);   
void    f_init_menu(void);          /* MenÅzeile her! */
void    f_exit_menu(void);          /* MenÅzeile weg! */

char    set_menu_key(OBJECT *menu);
char    get_menu_key(OBJECT *menu, KINFO *ki, int *title, int *item);
int     scan_2_ascii(int scan, int state);

int     CallDialog(int topwin);     /* Dialogdispatcher */
int     UDO_or_not(WINDOW *wind, int klickobj);

int     appl_xgetinfo (int type, int *out1, int *out2, int *out3, int *out4);

void    f_handle_radios(OBJECT *tree, int klickobj, int windnum);
void    change_object(WINDOW *window, int objct, int status, int redraw);

void    f_analyze_system(void);

void    f_resort_piclist(void);

void    f_activate_pic(int windnum);

int     do_MBEVT(int module_number, WINDOW *mod_win, int mode);

void    check_and_terminate(int mode, int module_number);

void    get_module_structures(char* *textseg_begin, MOD_INFO* *mod_info, MOD_ABILITY* *mod_abs, long *mod_magic, int mod_num);

void    init_AVPROTO(void);
void    get_avserv(void);

void    close_alert(void);

int     init_dialog(int DialogNumber, int DialogOK);
void    close_dialog(int windnum);

void    insert_to_picman(int pic_to_insert);
void    make_picman_thumbnail(int picture_num);

void    f_pic_changed(WINDOW *window, int onoff);

void    f_move_preview(WINDOW *window, SMURF_PIC *orig_pic, int redraw_object);
void    copy_preview(SMURF_PIC *source_pic, SMURF_PIC *module_preview, WINDOW *prev_window);

void    make_smurf_pic(int pic_to_make, int wid, int hgt, int depth, char *picdata);
void    make_pic_window(int pic_to_make, int wid, int hgt, char *name);


void    f_scan_import(void);                                    /* Importmodule scannen */
int     load_import_list(void);                                 /* Extensionsliste laden */
int     seek_module(SMURF_PIC *picture, char *extension);       /* Modul aus Extensionsliste suchen */

MOD_INFO *ready_modpics_popup(WINDOW *mwindow);     /* Modul-Bildreihenfolge-Popup vorbereiten */

int blend_demopic(SMURF_PIC *picture);


void clip_picw2screen(WINDOW *picw);

void cursor_off(WINDOW *window);
void cursor_on(WINDOW *window);
void f_handle_editklicks(WINDOW *window, int object);

void free_preview(void);

void make_thumbnail(SMURF_PIC *original_pic, SMURF_PIC *thumbnail, int dither);

void mr_message(void);
void protocol_message(int message, int id, char *modname);

void    exmod_info_off(void);
void    emod_info_off(void);

int init_roto(void);
void roto(void);
void deinit_roto(void);

void gdps_main(void);


/* **********************************************************************/
/* ------------------- SMURF-Kommunikationsfunktionen ----------------- */
/* Bitte reden Sie mit mir, ich bin so alleine.                         */
/* **********************************************************************/
int get_dragdrop(WINDOW *window_to_handle, int *messagebuf);
int send_dragdrop(SMURF_PIC *picture, int dest_whandle, int mx, int my);
void send_AESMessage(int dest_id, int msg, ...);                /* AES-Message an anderen Prozess schicken */
void update_dhst(char *path);


/* **********************************************************************/
/* ---------------------------- Modulfunktionen ----------------------- */
/* Some of them want to use you, some of them want to get used by you...*/
/* **********************************************************************/
void    f_module_prefs(MOD_INFO *infostruct, int mod_id);   /* Modulformular aufrufen */
void    f_mpref_change(void);                               /* Eingabe im Modulformular */
void    f_export_pic(void);                                 /* Bild exportieren */

int     analyze_message(int module_ret, int picture_to_load);   /* Analyse eines Modul-Returns */
int     handle_modevent(int event_type, WINDOW *mod_window);    /* Handling eines Events im Modulfenster */
void    f_handle_modmessage(GARGAMEL *smurf_struct);

int     f_give_pics(MOD_INFO *mod_info, MOD_ABILITY *mod_abs, int module_number);       /* mehrere Bilder ans Modul Åbergeben */

int     give_free_module(void);     /* Ermittelt Strukturindex fÅr freies Modul und gibt diesen zurÅck */

SMURF_PIC *get_pic(int num, int mod_id, MOD_INFO *mod_info, int depth, int form, int col);

int inform_modules(int message, SMURF_PIC *picture);    /* Informiert alle Module und Plugins Åber message */
void AESmsg_to_modules(int *msgbuf);
long get_modmagic(BASPAG *basepage);

void *mconfLoad(MOD_INFO *modinfo, int mod_id, char *name);
void mconfSave(MOD_INFO *modinfo, int mod_id, void *confblock, long len, char *name);
void transmitConfig(BASPAG *modbase, GARGAMEL *smurf_struct);
void memorize_emodConfig(BASPAG *modbase, GARGAMEL *smurf_struct);
void memorize_expmodConfig(BASPAG *modbase, GARGAMEL *smurf_struct, char save);
void *load_from_modconf(MOD_INFO *modinfo, char *name, int *num, long type);


/* **********************************************************************/
/* --------------------------- Windowfunktionen ----------------------- */
/* WINDOWSø sucks!                                                      */
/* **********************************************************************/
void    f_treewalk(OBJECT *tree,int start);                 /* Userdefined Objects eintragen */
void    f_arrow_window(int mode, WINDOW *window, int amount);   /* Pfeilbedienung im Bildfenster */
void    f_slide_window(int pos, WINDOW *wind, int mode);    /* Bildfenster sliden */
void    f_setsliders(WINDOW *wind);                         /* Slider setzen */
int     f_alert(char *alertstring, char *b1, char *b2, char *b3, int defbt);   /* Fenster-Alertfunktion */
int     f_open_window(WINDOW *wind);                        /* Fenster îffnen */
void    top_window(int handle);
void    top_window_now(WINDOW *window);
void    top_windowhandle(int handle);
void    close_window(int handle);
int     f_open_module_window(WINDOW *module_window);        /* Modulfenster îffnen */
int     my_window(int handle);                              /* Fenster identifizieren */
WINDOW  *my_module_window(int handle);                      /* Modulfenster identif. */
void    f_redraw_window(WINDOW *window, GRECT *mwind, int startob, int flags);      /* Fenster Redraw */
int     rc_intersect( GRECT *r1, GRECT *r2, GRECT *r3);     /* Zum Redraw. */
void    f_info(void);

void    f_draw_vectorgraphic(WINDOW *window);

void    toggle_asterisk(WINDOW *picwin, int onoff);

void    disable_busybox(void);
void    enable_busybox(void);

void window_to_list(WINDOW *window);        /* Fenster in die Liste hÑngen */
void remove_window(WINDOW *window);

int find_crosshair(WINDOW *window);         /* findet eine Fadenkreuzstruktur zu einem Bildfenster */


/* **********************************************************************/
/*                              Blockfunktionen                         */
/*  Alles was mit den Blîcken zu tun hat, oder:                         */
/*      "Um den nÑchsten Block und dann links abbiegen..."              */
/* **********************************************************************/

void    f_draw_blockbox(WINDOW *window);
void    draw_block(WINDOW *window, GRECT *picbox);

void    block_freistellen(WINDOW *pwindow);
void    block_type_in(void);
void    blockmode(void);

int encode_block(SMURF_PIC *picture, EXPORT_PIC **pic_to_save);
int save_block(EXPORT_PIC *pic_to_save, char *path);

int block2clip(SMURF_PIC *picture, int mode, char *path);
void clip2block(SMURF_PIC *picture, char *data, int mx, int my);

void    f_draw_crosshair(WINDOW *window);       /* Positionierungskreuz in einem Bild zeichnen */

SMURF_PIC *previewBlock(SMURF_PIC *picture, GRECT *blockpart);

/* **********************************************************************/
/* ------------------------------ Statusbox --------------------------- */
/* "Picard an Maschinenraum! Statusbericht!"                            */
/* **********************************************************************/
void    ok_busybox(void);                       /* Busybox auf 128/"OK" setzen */
void    reset_busybox(int lft,  char *txt);     /* Busybox setzen und Text einfÅgen */
int     draw_busybox(int lft);                  /* Busybox setzen */
void    actualize_ram(void);                    /* RAM anzeigen */


/* **********************************************************************/
/* ------------------------- Bedienungsfunktionen --------------------- */
/* Alles, was dem Benutzer das Leben einfach und dem Programmierer      */
/*  schwer macht...                                                     */
/* **********************************************************************/
void    f_display_opt(void);            /* Display-Optionen */

void    f_options(void);                /* SMURF - Optionen */
void    f_edit_pop(void);               /* Edit-Modul-Fenster */

int     f_loadpic(char *pic, char *picpath);
int     f_save_pic(MOD_ABILITY *export_mabs);
void    f_set_syspal(void);             /* Systempalette setzen */
void    f_set_picpal(SMURF_PIC *pic);   /* Bildpalette setzen */
void    f_deselect_popup(WINDOW *wind, int ob1, int ob2);

void    f_pic_info(void);               /* Bildinfo anzeigen */
void    f_newpic(int scancode);

void    f_export_formular(void);

void    f_display_coords(WINDOW *pic_window, int mx, int my, char blockflag);
void    set_nullcoord(WINDOW *picwindow);

void    f_picman(void);

void    f_set_envconfig(void);          /* Environment-Konfiguration Åbernehmen */

void    f_update_dwindow(int mode, int redraw);


/* **********************************************************************/
/* ---------------------------- Toolfunktionen ------------------------ */
/* **********************************************************************/
char    *quote_arg(char *s);
char    *unquote_arg(char *s);
char    *strargvtok(char *s);
char    *mystrtok(char *s, char c);
int     strsrchl(char *s, char c);
int     strsrchr(char *s, char c);
void    get_tmp_dir(char *tmpdir);
int     get_cookie(unsigned long cookie, unsigned long *value);
void    BCD2string(char *string, int bcd);
char    *strrpbrk(char *s1beg, char *s1, char *s2);
char    *shorten_name(char *string, char newlen);

long    get_maxnamelen(char *path);

void f_drag_object ( WINDOW *wind, int objct, int *dex, int *dey, int call, int (*call_me)(int mx, int my) );

int get_selected_object(OBJECT *tree, int first_enty, int last_entry);
void call_stguide(int topwin_handle);
void make_singular_display(DISPLAY_MODES *old, int Dither, int Pal);
void restore_display(DISPLAY_MODES *old);

/* **********************************************************************/
/* ---------------------- Bindings neuer Funktionen ------------------- */
/* Denn kein Betriebssystem ist vollkommen.                             */
/* **********************************************************************/
int cdecl fsel_boxinput(char *path, char *name, int *button, char *label, void *callback);
int SM_wind_set(int wi_ghandle, int wi_gfield, int wi_gw1,
                int wi_gw2, int wi_gw3, int wi_gw4);
int SM_wind_get(int wi_ghandle, int wi_gfield, int *wi_gw1,
                int *wi_gw2, int *wi_gw3, int *wi_gw4);

#endif