/* START OF button_private.h ---------------------------------------------------
 *
 *      A simple click-able button widget.
 *
 *	---
 *	Written by David Eggleston (2006) <deggleston@users.sourceforge.net>
 *	for the 'BeebEm' Acorn BBC Model B, Integra-B, Model B Plus and
 *	Master 128 emulator.
 *
 *	This file is part of 'Economy GUI' and may be copied only under the 
 *	terms of either the GNU General Public License (GPL) or Dr. David
 *	Alan Gilbert's BeebEm license.
 *
 *      For more details please visit:
 *
 *      http://www.gnu.org/copyleft/gpl.html
 *	---
 */


#ifndef _DW_BUTTON_PRIVATE_H_
#define _DW_BUTTON_PRIVATE_H_


#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include "include/types.h"

#include <SDL.h>

#include "include/gui_window.h"
#include "include/gui_widget.h"

#include "include/gui_button.h"


/* Get EG_Button struct.
 */

#define EG_BUTTON_GET_STRUCT_PTR(w, v, r)  \
        if (w == NULL ){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget is NULL.", dR); \
                return( r ); \
        } \
        if (EG_Widget_GetType( w ) != EG_Widget_Type_Button){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is not of type EG_Button.", dR); \
                return( r ); \
        } \
        if ( (v = (EG_Button*) EG_Widget_GetPayload( w )) == NULL){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is NULL.", dR); \
                return(EG_FALSE); \
        } \
        EG_ASSERT_MALLOC( v );


/* EG_Widget payload:
 */
typedef struct{
        int alignment;
	char caption[MAX_BUTTON_CAPTIONSIZE+1];

	EG_BOOL depressed;
}EG_Button;
#define EG_AsButton(n) ((EG_Button*) n)


/* Private and friend functions:
 */


/* Poof the magic dragon, lived on a shelf.  He had no-one else to play with him,
 * so he pppllayeddd with himself! (Remember that one?)
 */


/* END OF button_private.h ------------------------------------------------------
 */
#endif                                                                           
