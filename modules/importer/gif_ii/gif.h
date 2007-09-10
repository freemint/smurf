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
 * The Initial Developer of the Original Code is
 * Christian Eyrich
 *
 * Contributor(s):
 *
 *
 * ***** END LICENSE BLOCK *****
 */

/*--------- Logical Screen Descriptor -------*/
typedef struct 
{
	unsigned int width,
				 height;
	char packed_fields;
	char background_color;
	char aspect_ratio;

} LSCRDES;

/*--------- Graphic Control Extension -------*/
typedef struct 
{
	char packed_fields;
	unsigned int delay_time;
	char transparent_color;
} GC_EXTENSION;

/*--------- Comment Extension -------*/
typedef struct 
{
	int commentlength;
	char *comment;
} COM_EXTENSION;

/*---------- Image Descriptor -------*/
typedef struct 
{
	char separator;
	unsigned int left_pos, 
				 top_pos;
	unsigned int image_width, 
				 image_height;
	char packed_fields;
} IMAGE_DES;


/*------------ Color Table ----------*/
typedef struct
{
	int depth;
	int size;
	char rgb_values[256L * 3L];
} COLTAB;