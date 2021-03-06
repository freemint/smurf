#ifndef _smurfobs_h_
#define _smurfobs_h_

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
 * Olaf Piesche, Christian Eyrich, Dale Russell and J�rg Dittmer
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

/* ----------------------- Smurf-Objekte -----------------------*/
/* Dieses File enth�lt Definitionen, die eine Objektorientierung
    des Smurf-Kerns m�glich machen. Wenn man so will, gibt es
    "Pseudoklassen" f�r jeden Bereich (Module, Dithering, etc.)
    mit Unterklassen, die eine Unterteilung f�r spezielle Aufgaben
    (Modulkommunikation, Modul-GUI) darstellen. Die Unterklassen
    enthalten dann die jeweiligen Routinen.
    Modulstart wird dann z.B. �ber module.comm.start_edit() erledigt.
    Genauso verh�lt es sich mit den Dialogen innerhalb Smurfs, die
    Adressen der Objektb�ume werden �ber eine Struktur angesprochen,
    in der die jeweiligen B�ume von Plugins ersetzt werden.

    Die Objektorientierung ist n�tig, um der Pluginschnittstelle die
    geplante Funktionalit�t zu erm�glichen. Die Pluginschnittstelle
    �bernimmt dann das Ersetzen der Routinen in den Objekten durch
    die ggfs. vom Plugin �bermittelten Routinen, so da� diese
    Smurfweit und auch in den Modulen zur Verf�gung stehen.
    --------------------------------------------------------------*/


/* modComm ---------------------------------------------
    Enth�lt alle Kommunikationsrelevanten Modulfunktionen.
    -----------------------------------------------------*/
typedef struct modComm
{
    int (*startImport)(char *modpath, SMURF_PIC *imp_pic);
    BASPAG *(*startEdit)(char *modpath, BASPAG *edit_basepage, int mode, int mod_id, GARGAMEL *sm_struct);
    EXPORT_PIC * (*startExport)(char *modpath, int message, SMURF_PIC *pic_to_export, BASPAG *exbase, GARGAMEL *sm_struct, int mod_id);
    BASPAG *(*startDither)(int mode, int mod_id, DITHER_DATA *ditherdata);
    void (*checkTerminate)(int mode, int module_number);

    int (*analyzeMessage)(int module_ret, int picture_to_load);
    void (*handleMessage)(GARGAMEL *smurf_struct);

    SMURF_PIC *(*getPic)(int num, int mod_id, MOD_INFO *mod_info, int depth, int form, int col);
    int (*givePics)(MOD_INFO *mod_info, MOD_ABILITY *mod_abs, int module_number);

    int (*inform)(int message, SMURF_PIC *picture);
    void (*transmitAESmsg)(int *msgbuf);
} MODCOMM;


/* modGUI ---------------------------------------------
    Enth�lt alle das GUI betreffenden Modulfunktionen.
    ----------------------------------------------------*/
typedef struct modGUI
{
    int (*openWindow)(WINDOW *module_window);
    int (*handleEvent)(int event_type, WINDOW *mod_window);
    void (*walkTree)(WINDOW *wind, int start);
    void (*initTree)(OBJECT *tree, int index);
    void (*convertIcon)(OBJECT *tree, int index);
    void (*makePreview)(WINDOW *wind);
} MODGUI;


/* moduleObj ---------------------------------------
    globale "Klasse" f�r Module
    -------------------------------------------------*/
typedef struct moduleObj
{
    MODCOMM comm;
    MODGUI gui;

    BASPAG  *bp[21];            /* Basepages der Module */
    GARGAMEL *smStruct[21];     /* �bergabestrukturen der Edit-Module */
} CLASS_MODULE;


/* windowOBJ ----------------------------------------
    allgemeine Windowfunktionen
    --------------------------------------------------*/
typedef struct generalWindow
{
    int (*open) (WINDOW *window);
    void (*redraw) (WINDOW *window, GRECT *mwind, int startob, int flags);
    void (*drawIconified) (WINDOW *window, int *vdiclip);

    int (*rcIntersect) (GRECT *r1, GRECT *r2, GRECT *r3);
    int (*myWindow) (int handle);
    WINDOW *(*myModuleWindow)(int handle);
    
    void (*windowToList) (WINDOW *window);
    void (*removeWindow) (WINDOW *window);
    void (*top) (int handle);
    void (*topNow) (WINDOW *window);
    void (*topHandle) (int handle);
    void (*close) (int handle);

    void (*cursorOn)(WINDOW *window);
    void (*cursorOff)(WINDOW *window);

    /* diese beiden sind Bindings - m�ssen vielleicht woanders hin
     */
    int (*windSet)(int wi_ghandle, int wi_gfield, int wi_gw1, int wi_gw2, int wi_gw3, int wi_gw4);
    int (*windGet)(int wi_ghandle, int wi_gfield, int *wi_gw1, int *wi_gw2, int *wi_gw3, int *wi_gw4);

    OBJECT *titles;
} GENERAL_WINDOW;


typedef struct unDo
{
    void (*saveBuffer)(SMURF_PIC *picture, int picnum);
    void (*swapBuffer)(SMURF_PIC *toPicture, int picNum);
} UNDO;


/* picwindowOBJ ------------------------------------
    Bildfenster betreffender Krempel
    -------------------------------------------------*/
typedef struct picWindow
{
    void (*drawBlock) (WINDOW *window, GRECT *picbox);
    void (*insertZoom) (WINDOW *window);
    void (*drawCrosshair) (WINDOW *window);
    void (*drawBlockbox) (WINDOW *window);
    int (*findCrosshair)(WINDOW *window);
    void (*arrowWindow) (int mode, WINDOW *wind,int amount);
    void (*slideWindow) (int pos, WINDOW *wind, int mode);
    void (*setSliders) (WINDOW *wind);
    void (*scrollRT)(WINDOW *window, int xamount, int yamount);

    void (*clipPicwin) (WINDOW *picw);
    void (*toggleAsterisk) (WINDOW *picwin, int onoff);
    void (*displayCoords)(WINDOW *pic_window, int mx, int my, char blockflag);
    void (*nullCoords)(WINDOW *picwindow);
    void (*removeBlock)(WINDOW *picwindow);
    void (*blockPop)(WINDOW *picwindow);

    UNDO undo;

    int topPicwin;
} IMAGEWINDOW;


/* picManager --------------------------------------
    Dialoghandling f�r PicManager
    -------------------------------------------------*/
typedef struct picManager
{
    void (*handlePicman)(void);
    void (*insertPic)(int pic_to_insert);
    void (*showWH)(SMURF_PIC *pic);
    void (*cleanupList)(void);
    void (*navigateWindow)(void);
    int (*autoScroll)(int mx, int my);
    void (*makeThumbnail)(int picture_num);
    void (*drawPicmanBoxes) (void);

    OBJECT *tree;
    WINDOW *window;
    SMURF_PIC thumbnail;
    LIST_FIELD  pictureList;
    int picmanList[25];
    int selectedPic;
} PICTURE_MANAGER;


/* windowAlert -------------------------------------
    Handler f�r die Windowalerts
    -------------------------------------------------*/
typedef struct windowAlert
{
    int (*openAlert)(char *alertstring, char *b1, char *b2, char *b3, int defbt);
    void (*closeAlert)(void);

    OBJECT  *alerts;
    int isTop, winHandle;
} WIN_ALERT;


/* busyBox ---------------------------------
    Komplette Busybox mit allem was dazugeh�rt
    -----------------------------------------*/
typedef struct busyBox
{
    int (*draw)(int lft);
    void (*reset)(int lft, char *txt);
    void (*ok)(void);
    void (*dispRAM)(void);

    void (*disable)(void);
    void (*enable)(void);
    void (*fullDisable)(void);

    int noEvents;
    long resetClock;
    char disabled;
    OBJECT *busyTree;
} BUSYBOX;


/* editmodList ---------------------------------
    Editmodul-Listfenster mit allem dazugeh�rigen Schnickschnack
    ---------------------------------------------*/
typedef struct editmodList
{
    void (*handleList)(void);
    void (*infoOff)(void);
    void (*infoOn)(int mod_index);

    char *modNames[100];            /* Namen f�r bis zu 100 Edit-Module */
    LIST_FIELD  modList;
    OBJECT *tree;
    OBJECT *infoTree;
    int anzahl;
} EDITMOD_LIST;


/* exportList ----------------------------------
    Exportmodul-Listfenster
    ---------------------------------------------*/
typedef struct exportList
{
    void (*handleList)(void);
    void (*infoOn)(int mod_index);
    void (*infoOff)(void);

    char *modNames[100];            /* Namen f�r bis zu 100 Edit-Module */
    LIST_FIELD  modList;
    OBJECT *tree;
    OBJECT *infoTree;
    int anzahl;
} EXPMOD_LIST;


/* smurfOptions ---------------------------------
    Optionsdialog
    ----------------------------------------------*/
typedef struct smurfOptions
{
    void (*options)(void);
    void (*setEnvconfig)(void);

    OBJECT *tree;   
} OPTIONS_DIALOG;


/* displayOptions ----------------------------------
    Displayoptions-Dialog
    -------------------------------------------------*/
typedef struct displayOptions
{
    void (*displayOptions)(void);
    void (*updateWindow)(int mode, int redraw);

    OBJECT *tree;
} DISPOPTS_DIALOG;


/* internalGuiOBJ -----------------------------------
    Smurf-interne GUI-Funktionen
    enth�lt z.B. die Strukturen aller einzelnen Dialoge
    --------------------------------------------------*/
typedef struct dialogHandlers
{
    PICTURE_MANAGER picMan;
    BUSYBOX busy;
    WIN_ALERT winAlert;
    EDITMOD_LIST emodList;
    EXPMOD_LIST expmodList;
    OPTIONS_DIALOG smurfOpt;
    DISPOPTS_DIALOG dispOpt;

    int topDialog;

    int (*init)(int DialogNumber, int DialogOK);
    void (*close)(int windnum);
} DIALOGS;


typedef struct commHandlers
{
    int (*initOLGA)(void);
    int (*deinitOLGA)(void);
    int (*updateOLGA)(char *filename);
    int (*renameOLGA)(char *oldname, char *newname);
    
    int (*getDragdrop)(WINDOW *window_to_handle, int *messagebuf);
    int (*sendDragdrop)(SMURF_PIC *picture, int dest_whandle, int mx, int my);
    int (*ddGetheader)(DD_HEADER *dd_header, int pipe_handle);
    int (*ddSendheader)(int pipe_handle, long ext, long size, char *data_name, char *file_name);
    
    void (*updateDhst)(char *path);

    void (*getAvserv)(void);
    void (*initAVPROTO)(void);

    int (*initbubbleGem)(void);
    int (*deinitbubbleGem)(void);
    void (*bubbleGem)(int windownum, int xpos, int ypos, int modulemode);


    void (*sendAESMsg)(int dest_id, int msg, ...);

    AV_COMM avComm;
} COMMUNICATION;


/*
 * die dazugeh�rigen Funktionen zum Initialisieren der Strukturen
 */
void init_moduleObs(void);          /* Modulobjekte initialisieren */
void init_GuiObs(void);             /* GUI-Objekte initialisieren */
void init_MiscObs(void);            /* allgemeine Objekte initialisieren */

#endif