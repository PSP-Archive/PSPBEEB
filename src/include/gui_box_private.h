/* START OF box_private.h ------------------------------------------------------
 *
 *	A simple box widget.
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
 *	For more details please visit:
 *
 *	http://www.gnu.org/copyleft/gpl.html
 *	---
 */


#ifndef _DW_BOX_PRIVATE_H_
#define _DW_BOX_PRIVATE_H_


#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include "include/types.h"

#include <SDL.h>

#include "include/gui_window.h"
#include "include/gui_widget.h"

#include "include/gui_box.h"


/* Get EG_Box struct.
 */

#define EG_BOX_GET_STRUCT_PTR(w, v, r)  \
        if (w == NULL ){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget is NULL.", dR); \
                return( r ); \
        } \
        if (EG_Widget_GetType( w ) != EG_Widget_Type_Box){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is not of type EG_Box.", dR); \
                return( r ); \
        } \
        if ( (v = (EG_Box*) EG_Widget_GetPayload( w )) == NULL){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is NULL.", dR); \
                return(EG_FALSE); \
        } \
        EG_ASSERT_MALLOC( v );


/* EG_Widget payload:
 */
typedef struct{
        int type;
}EG_Box;
#define EG_AsBox(n) ((EG_Box*) n)


/* END OF box_private.h --------------------------------------------------------
 */
#endif                 
