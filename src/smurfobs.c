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
 * Olaf Piesche, Christian Eyrich, Dale Russell and J”rg Dittmer
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

#include <tos.h>
#include "..\sym_gem.h"
#include "..\mod_devl\import.h"
#include "smurf_st.h"
#include "smurfine.h"
#include "smurf_f.h"
#include "globdefs.h"

#include "smurfobs.h"
#include "ext_rsc.h"


CLASS_MODULE module;

GENERAL_WINDOW	Window;
IMAGEWINDOW	imageWindow;
DIALOGS Dialog;
COMMUNICATION Comm;

void init_moduleObs(void)
{
	extern void init_modtree(OBJECT *tree, int index);
	extern void convert_icon(OBJECT *tree, int index);
	extern void walk_module_tree(WINDOW *wind, int start);
	extern void make_modpreview(WINDOW *wind);


	/*
	 * Modulkommunikation
	 */
	module.comm.startImport = start_imp_module;
	module.comm.startEdit = start_edit_module;
	module.comm.startExport = start_exp_module;
	module.comm.startDither = start_dither_module;
	module.comm.checkTerminate = check_and_terminate;
	module.comm.analyzeMessage = analyze_message;
	module.comm.handleMessage = f_handle_modmessage;
	module.comm.getPic = get_pic;
	module.comm.givePics = f_give_pics;
	module.comm.inform = inform_modules;
	module.comm.transmitAESmsg = AESmsg_to_modules;

	/*
	 * Modulspezifisches GUI
	 */
	module.gui.openWindow = f_open_module_window;
	module.gui.handleEvent = handle_modevent;
	module.gui.walkTree = walk_module_tree;
	module.gui.initTree = init_modtree;
	module.gui.convertIcon = convert_icon;
	module.gui.makePreview = make_modpreview;

	return;
} /* init_moduleObs */


void init_GuiObs(void)
{
	extern void draw_iconified(WINDOW *window, int *vdiclip);
	extern void insert_picwinzoom(WINDOW *window);
	extern void scrollWindowRT(WINDOW *window, int xamount, int yamount);
	extern void show_picman_wh(SMURF_PIC *pic);
	extern void f_resort_piclist(void);
	extern void picman_windowmove(void);
	extern int pm_autoscroll(int mx, int my);
	extern void draw_picmanboxes(void);
	extern void remove_block(WINDOW *picwindow);
	extern void fulldisable_busybox(void);

	extern void emod_info_on(int mod_index);
	extern void init_exmod_info(int mod_index);
	extern void exmod_info_off(void);
	extern WINDOW wind_s[25];

	extern void f_options(void);
	extern void f_block_popup(WINDOW *picwindow);
	extern int init_dialog(int DialogNumber, int DialogOK);
	extern void close_dialog(int windnum);

	extern void saveUndoBuffer(SMURF_PIC *picture, int picnum);
	void swapUndoBuffer(SMURF_PIC *toPicture, int picNum);


	/*
	 * genereller Fensterkrempel
	 */
	Window.open = f_open_window;
	Window.redraw = f_redraw_window;
	Window.drawIconified = draw_iconified;
	Window.rcIntersect = rc_intersect;
	Window.myWindow = my_window;
	Window.myModuleWindow = my_module_window;
	Window.windowToList = window_to_list;
	Window.removeWindow = remove_window;
	Window.top = top_window;
	Window.topNow = top_window_now;
	Window.topHandle = top_windowhandle;
	Window.close = close_window;

	Window.cursorOn = cursor_on;
	Window.cursorOff = cursor_off;
	Window.windSet = SM_wind_set;
	Window.windGet = SM_wind_get;

	/*
	 * Bildfensterspezifischer Krempel
	 */
	imageWindow.drawBlock = draw_block;
	imageWindow.insertZoom = insert_picwinzoom;
	imageWindow.drawCrosshair = f_draw_crosshair;
	imageWindow.drawBlockbox = f_draw_blockbox;
	imageWindow.findCrosshair = find_crosshair;
	imageWindow.arrowWindow = f_arrow_window;
	imageWindow.slideWindow = f_slide_window;
	imageWindow.setSliders = f_setsliders;
	imageWindow.scrollRT = scrollWindowRT;
	imageWindow.clipPicwin = clip_picw2screen;
	imageWindow.toggleAsterisk = toggle_asterisk;
	imageWindow.displayCoords = f_display_coords;
	imageWindow.nullCoords = set_nullcoord;
	imageWindow.removeBlock = remove_block;
	imageWindow.blockPop = f_block_popup;
	imageWindow.undo.saveBuffer = saveUndoBuffer;
	imageWindow.undo.swapBuffer = swapUndoBuffer;

	/*
	 * und jetzt der dialogspezifische Krempel ...
	 * das ist ziemlich viel.
	 */
	Dialog.init = init_dialog;
	Dialog.close = close_dialog;

	/*
	 * der Bildmanager
	 */
	Dialog.picMan.handlePicman = f_picman;
	Dialog.picMan.insertPic = insert_to_picman;
	Dialog.picMan.showWH = show_picman_wh;
	Dialog.picMan.cleanupList = f_resort_piclist;
	Dialog.picMan.navigateWindow = picman_windowmove;
	Dialog.picMan.autoScroll = pm_autoscroll;
	Dialog.picMan.makeThumbnail = make_picman_thumbnail;
	Dialog.picMan.drawPicmanBoxes = draw_picmanboxes;
	Dialog.picMan.window = &wind_s[WIND_PICMAN];

	/*
	 * die Busybox
	 */
	Dialog.busy.draw = draw_busybox;
	Dialog.busy.reset = reset_busybox;
	Dialog.busy.ok = ok_busybox;
	Dialog.busy.dispRAM = actualize_ram;
	Dialog.busy.disable = disable_busybox;
	Dialog.busy.enable = enable_busybox;
	Dialog.busy.fullDisable = fulldisable_busybox;
	Dialog.busy.noEvents = 0;
	Dialog.busy.resetClock = 0;
	Dialog.busy.disabled = 0;

	/*
	 * Editmodulliste
	 */
	Dialog.emodList.handleList = f_edit_pop;
	Dialog.emodList.infoOn = emod_info_on;
	Dialog.emodList.infoOff = emod_info_off;

	/*
	 * Exportmodulliste
	 */
	Dialog.expmodList.handleList = f_export_pic;
	Dialog.expmodList.infoOn = init_exmod_info;
	Dialog.expmodList.infoOff = exmod_info_off;

	/*
	 * Optionsdialog
	 */
	Dialog.smurfOpt.options = f_options;	
	Dialog.smurfOpt.setEnvconfig = f_set_envconfig;	

	/*
	 * Displayoptions
	 */
	Dialog.dispOpt.displayOptions = f_display_opt;
	Dialog.dispOpt.updateWindow = f_update_dwindow;
	

	/*
	 * die Window-Alerts
	 */
	Dialog.winAlert.openAlert = f_alert;
	Dialog.winAlert.closeAlert = close_alert;

	return;
} /* init_GuiObs */


void init_MiscObs(void)
{
	extern int init_OLGA(void);
	extern int deinit_OLGA(void);
	extern int rename_OLGA(char *oldname, char *newname);
	extern int update_OLGA(char *filename);
	extern int dd_getheader(DD_HEADER *dd_header, int pipe_handle);
	extern int dd_sendheader(int pipe_handle, long ext, long size, char *data_name, char *file_name);
	extern void bubble_gem(int windownum, int xpos, int ypos, int modulemode);
	

	/*
	 * Kommunikationsklasse
	 */
	Comm.initOLGA = init_OLGA;
	Comm.deinitOLGA = deinit_OLGA;
	Comm.updateOLGA = update_OLGA;
	Comm.renameOLGA = rename_OLGA;

	Comm.getDragdrop = get_dragdrop;
	Comm.sendDragdrop = send_dragdrop;
	Comm.ddGetheader = dd_getheader;
	Comm.ddSendheader = dd_sendheader;

	Comm.updateDhst = update_dhst;
	Comm.bubbleGem = bubble_gem;

	Comm.getAvserv = get_avserv;
	Comm.initAVPROTO = init_AVPROTO;

	Comm.sendAESMsg = send_AESMessage;
	
	return;
} /* init_MiscObs */