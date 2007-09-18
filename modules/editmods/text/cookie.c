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

#include <tos.h>
#include <stddef.h>

#define FALSE	0
#define TRUE	1


#define Ssystem(mode, arg1, arg2)   gemdos(0x154, (int)mode, (long)arg1, (long)arg2)

/* Konstanten fÅr Ssystem() */
#define OSNAME     	0
#define OSXNAME  		1
#define OSVERSION    	2
#define TOSHEADER    	3
#define OS_BUILD_DATE  4
#define OS_BUILD_TIME  5
#define COMPILE_TYPE 	6

#ifndef FEATURES
	#define FEATURES 		7
#endif

#ifndef GETCOOKIE
	#define GETCOOKIE    	8
#endif

#define SETCOOKIE    	9
#define GET_LVAL 		10
#define GET_WVAL    	11
#define GET_BVAL    	12
#define SET_LVAL		13
#define SET_WVAL    	14
#define SET_BVAL    	15
#define SYS_SLEVEL  	16
#define RUN_LEVEL      17  /* currently disabled, reserved */
#define TSLICE		    18
#define FORCEFASTLOAD  19
#define SYNC_TIME   	20
#define RESERVED_21 	21  /* reserved, use Dcntl instead */
#define CLOCK_UTC    	100

#define FEAT_MP   		1   /* memory protection enabled */
#define FEAT_VM    	2   /* virtual memory manager enabled */
#define GCJARSIZE 0

#ifndef EINVFN#define EINVFN 	-32#endif

/* Erweitert um Ssystem(GETCOOKIE) am 2.5.99 */
int get_cookie(unsigned long cookie, unsigned long *value)
{
	long r, val;

	unsigned long *cookiejar;


	/* Erst den neuen Weg probieren */
	if((r = Ssystem(GETCOOKIE, cookie, &val)) != EINVFN)
	{
		if(r != -1)
		{
			*value = val;
			return(TRUE);
		}
		else
		{
			*value = 0L;
			return(FALSE);
		}
	}
	else
	{
		/* Zeiger auf Cookie Jar holen */
		cookiejar = (unsigned long *)Setexc(0x5a0/4, (void (*)())-1);	

		/* Cookie Jar installiert? */
		if(cookiejar)
		{
			/* Keksdose nach cookie durchsuchen */
			while(*cookiejar && *cookiejar != cookie)
				cookiejar += 2;

			/* wenn cookie gefunden wurde, value auf Cookiewert setzen, */
			/* ansonsten value auf 0L */
			if(*cookiejar == cookie)
			{
				*value = *(cookiejar + 1);
				return(TRUE);
			}
			else
			{
				*value = 0L;
				return(FALSE);
			}
		}
		else
		{
			*value = 0L;
			return(FALSE);
		}
	}
} /* get_cookie */
