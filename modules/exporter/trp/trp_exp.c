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

/* =========================================================*/
/*                  Falcon-TRP-Format Encoder               */
/* =========================================================*/

#include <tos.h>
#include <ext.h>
#include <screen.h>
#include <stdio.h>
#include <string.h>
#include "..\..\..\sym_gem.h"
#include "..\..\import.h"
#include "..\..\..\smurf\smurfine.h"
#include "falcraw.rsh"
#include "falcraw.rh"

#define RLE 1
#define BINARY  2
#define LZW     4

int open_window(GARGAMEL *smurf_struct);



/* Infostruktur fr Hauptmodul */
MOD_INFO    module_info = {"TRP - Therapy Gfx File",
                        0x0010,
                        "Dodger",
                        "DAT", "", "", "", "",
                        "", "", "", "", "",
                        "Slider 1",
                        "Slider 2",
                        "Slider 3",
                        "Slider 4",
                        "Checkbox 1",
                        "Checkbox 2",
                        "Checkbox 3",
                        "Checkbox 4",
                        "Edit 1",
                        "Edit 2",
                        "Edit 3",
                        "Edit 4",
                        0,128,
                        0,128,
                        0,128,
                        0,128,
                        0,10,
                        0,10,
                        0,10,
                        0,10,
                        0,0,0,0,
                        0,0,0,0,
                        0,0,0,0
                        };


MOD_ABILITY  module_ability = {
                        1, 4, 8, 16, 24, 24, 24, 24,
                        FORM_STANDARD,
                        FORM_STANDARD,
                        FORM_PIXELPAK,
                        FORM_PIXELPAK,
                        FORM_PIXELPAK,
                        FORM_PIXELPAK,
                        FORM_PIXELPAK,
                        FORM_PIXELPAK,
                        0
                        };



/*--------------- Type definitions --------*/
typedef struct
{
    char include_header;
    int encoding;
} CONFIG;

typedef struct
{
    char magic[12];
    char Version;
    int width, height;
    int depth;
    int encoding;
    int blocklen;           /* Blockl„nge bei Binary Encoding */
    long palette_offset;
    long data_offset;
} TRP_HEADER;


/*------------ Binary Encoded Block -----------*/
/* die L„nge von buffer ergibt sich aus bitlen*blocklen in bits. */
typedef struct
{
    char bitlen;
    char *buffer;
} BINARY_BLOCK;

typedef struct
{
    char bitlen;
    char num_entries;
    char buffer;
} CODETABLE;

CONFIG dft_opts =
{
    1,
    BINARY
};

TRP_HEADER my_header =
{
    "TRP_GFX_FILE",
    1,
    0, 0,
    16,
    0,
    0,
    0
};



void *binary_encode(char *buffer, char *dest, long len, SMURF_PIC *pic);
void *bencode(char *buffer, int blocksize, int bitlen, char *dest, BINARY_BLOCK *binbloc, CODETABLE *codetab);
int get_Bitlen(char *buffer, int size);



CONFIG  file_config;
BINARY_BLOCK    * *bebs;
CODETABLE       * *ctabs;

WINDOW  mwindow;
OBJECT  *main;

/* -------------------------------------------------*/
/* -------------------------------------------------*/
/*              16 Bit TRP-Format                   */
/* -------------------------------------------------*/
/* -------------------------------------------------*/
EXPORT_PIC *exp_module_main(GARGAMEL *smurf_struct)
{
    EXPORT_PIC *exp_pic;
    char *buffer;
    int *ziel, *ziel2;
    int width, height;
    int message, back;
    int depth;
    char *palette;
    long file_len;
    unsigned int col;

    static int enc;
    
    message =   smurf_struct->module_mode;  


    switch(message)
    {
            /*---------------- Export-Start ----------------*/  
        case MSTART:
        case MEXEC:
                buffer  =   smurf_struct->smurf_pic->pic_data;
                width   =   smurf_struct->smurf_pic->pic_width;
                height  =   smurf_struct->smurf_pic->pic_height;
                depth   =   smurf_struct->smurf_pic->depth;
                palette =   smurf_struct->smurf_pic->palette;
        
                my_header.width=width;
                my_header.height=height;
                my_header.depth=16;
                my_header.encoding=enc;
                my_header.palette_offset=0;
                my_header.data_offset=sizeof(TRP_HEADER);
        
                exp_pic=Malloc(sizeof(EXPORT_PIC));

                if(main[ENC_BINARY].ob_state&SELECTED)      file_config.encoding |= BINARY;
                if(main[INCLUDE_HEADER].ob_state&SELECTED)  file_config.include_header = 1;
            
                if(file_config.include_header) 
                    file_len= ((long)width*(long)height*2L) + sizeof(TRP_HEADER);
                else 
                    file_len= (long)width*(long)height*2L;
                
                ziel=Malloc(file_len);

                if( ! ziel)
                {
                    smurf_struct->module_mode=M_MEMORY; 
                    return(exp_pic);
                }

                /*--- Header & Encoding ---*/
                if(file_config.include_header)
                    ziel2=ziel+sizeof(TRP_HEADER);
                else
                    ziel2=ziel;
                
                if(file_config.encoding&BINARY) 
                    binary_encode(buffer, ziel2, (long)width*(long)height*2L, smurf_struct->smurf_pic);

                exp_pic->pic_data = ziel;
                exp_pic->f_len = file_len;
        
                smurf_struct->module_mode=M_DONEEXIT;
                return(exp_pic);
            
                break;
    
    
    
            /*---------------- Konfigurieren ----------------*/ 
        case MMORE:     back=open_window(smurf_struct);
                        if(back==-1) smurf_struct->module_mode=M_EXIT;  /* keins mehr da? */
                        else smurf_struct->module_mode=M_WAITING;       /* doch? Ich warte... */
                
                        break;
    
    
    
            /*---------------- Konfig canceln ----------------*/    
        case MMORECANC: memcpy(&file_config, &dft_opts, sizeof(CONFIG));
                        smurf_struct->module_mode=M_WAITING;
                
                        break;
    
    
            /*---------------- Buttonevent ----------------*/   
        case MBEVT:     switch(smurf_struct->event_par[0])
                        {
                            case CONF_OK:   smurf_struct->module_mode=M_MOREOK;
                                            break;
                        }
                        break;

        case MTERM:     smurf_struct->module_mode=M_EXIT;
                        break;
    
        default:        
                        smurf_struct->module_mode=M_WAITING;
                        break;
    }

    return(NULL);
}



int open_window(GARGAMEL *smurf_struct)
{
    int t, back;


    /* Resource Umbauen */
    for (t=0; t<NUM_OBS; t++)
        rsrc_obfix (&rs_object[t], 0);

    main=rs_trindex[EXP_MAIN];      /* Resourcebaum holen */

    mwindow.whandlem=0;                     /* evtl. Handle l”schen */
    mwindow.module = smurf_struct->module_number;
    strcpy(mwindow.wtitle, "16 Bit TRP - Konfiguration");               /* Titel reinkopieren   */
    mwindow.wnum=1;                     /* Fenster nummer 1...  */
    mwindow.wx=-1;                              /* Fenster X-...    */
    mwindow.wy=-1;                              /* ...und Y-Pos     */
    mwindow.ww=main->ob_width;                  /* Fensterbreite    */
    mwindow.wh=main->ob_height;             /* Fensterh”he      */
    mwindow.resource_form=main;             /* Resource         */
    mwindow.picture=NULL;                       /* kein Bild.       */
    mwindow.pflag=0;                            /* kein Bild.       */
    smurf_struct->wind_struct=&mwindow; /* und die Fensterstruktur in die Gargamel */

    back=smurf_struct->services->f_module_window(&mwindow);     /* Gib mir 'n Fenster! */

    return(back);
}


void *binary_encode(char *buffer, char *dest, long len, SMURF_PIC *pic)
{
    int BLOCKSIZE = 32;     /* 32 Bytes Blockgr”že */
    long count=0;
    long blockcount=0;
    int bitlen=0;
    int number_of_blocks=0;
    char *dest2;
    
    number_of_blocks=((long)pic->pic_width*(long)pic->pic_height + (BLOCKSIZE-1) ) /BLOCKSIZE;
    bebs = Malloc(number_of_blocks*4);
    ctabs = Malloc(number_of_blocks*4);

    dest2=dest;

    do
    {
        bitlen = get_Bitlen(buffer+count, BLOCKSIZE);
        
        bebs[blockcount]=Malloc(sizeof(BINARY_BLOCK));
        ctabs[blockcount]=Malloc(sizeof(CODETABLE));

        bencode(buffer+count, BLOCKSIZE, bitlen, dest2, bebs[blockcount], ctabs[blockcount]);

        Mfree(bebs[blockcount]);
        Mfree(ctabs[blockcount]);

        dest2+=(BLOCKSIZE*bitlen)/8;
        count += BLOCKSIZE;
        blockcount++;
    } while(count<len);

    Mfree(bebs);
    Mfree(ctabs);
}





/*--- Gr”žtes Byte im Block suchen ---*/
int get_Bitlen(char *buffer, int size)
{
    int t;
    char byte, oldbyte=0;
    int bit;
    
    for(t=0; t<size; t++)
    {
        byte=*(buffer++);
        if(byte>oldbyte) oldbyte=byte;
    }

    if(byte==0) bit=0;
    else if(byte<=2) bit=1;
    else if(byte<=4) bit=2;
    else if(byte<=8) bit=3;
    else if(byte<=16) bit=4;
    else if(byte<=32) bit=5;
    else if(byte<=64) bit=6;
    else if(byte<=128) bit=7;
    else if(byte<=256) bit=8;
    
    return(bit);
}

/*---- binary encoding von buffer nach dest, blocksize bytes in je bitlen bits ----*/
/* return: codetabelle */
void *bencode(char *buffer, int blocksize, int bitlen, char *dest, BINARY_BLOCK *binbloc, CODETABLE *codetab)
{
    int t, tt;
    char byte;
    char tmp_tab[256];
    char binblk[256];

    printf("Codetable:\n");

    /*--------- Codetable erstellen */
    for(t=0; t<blocksize; t++)
    {
        byte=*(buffer++);
        
        for(tt=0; tt<blocksize; tt++)
        {
            if(tmp_tab[tt]==byte) break;
            else if(tmp_tab[tt]==0) {   tmp_tab[tt]=byte; break;    }
        }
        binblk[t]=tt;
        
        printf("%i: %i\n", t, (int)tmp_tab[tt]);
    }

    printf("Block:\n");

    for(t=0; t<blocksize; t++)
        printf("%i: %i\n", t, (int)binblk[t]);


}
