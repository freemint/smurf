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

; SMURF C-Modul Startupcode
; Version 1.01
; MiNT memory-protection & 040/060 cacheflush-sicher
;
; als Startupcode im Projektfile statt "PCSTART.O" eintragen
;
; 						FÅr Datei-Import-Module
; 
;	
;	main-function im Modul: imp_module_main
;	Struktur: MOD_INFO module_info muû im Modul sein


	.TEXT
	clr.w -(sp)
	trap #1

	.IMPORT imp_module_main				; Hauptfunktion des Moduls holen
	bra imp_module_main					; mainfunction anspringen
	dc.l 'SIMD'							; Magic (TEXT + 8 Bytes)

	.IMPORT module_info					; Modulinformationsstruktur
	dc.l module_info					; Zeiger auf Modulinfo-Struktur (TEXT + 12 Bytes)

	dc.l $0101							; Versionsnummer

	.DATA
	.EXPORT errno

errno: ds.w	1
