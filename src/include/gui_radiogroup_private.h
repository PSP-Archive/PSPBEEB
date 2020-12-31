/* START OF radiogroup_private.h -----------------------------------------------
 *
 *	A Radio Button Group widget.
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


#ifndef _DW_RADIOGROUP_PRIVATE_H_
#define _DW_RADIOGROUP_PRIVATE_H_


#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include "include/types.h"

#include "include/gui_widget.h"
#include "include/gui_window.h"

#include <SDL.h>


/* Get EG_RadioGroup struct.
 */

#define EG_RADIOGORUP_GET_STRUCT_PTR(w, v, r)  \
        if (w == NULL ){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget is NULL.", dR); \
                return( r ); \
        } \
        if (EG_Widget_GetType( w ) != EG_Widget_Type_RadioGroup){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is not of type EG_RadioGroup.", dR); \
                return( r ); \
        } \
        if ( (v = (EG_RadioGroup*) EG_Widget_GetPayload( w )) == NULL){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is NULL.", dR); \
                return(r); \
        } \
        EG_ASSERT_MALLOC( v );

#define EG_RADIOGORUP_GET_STRUCT_PTR_VOID(w, v)  \
        if (w == NULL ){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget is NULL.", dR); \
                return; \
        } \
        if (EG_Widget_GetType( w ) != EG_Widget_Type_RadioGroup){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is not of type EG_RadioGroup.", dR); \
                return; \
        } \
        if ( (v = (EG_RadioGroup*) EG_Widget_GetPayload( w )) == NULL){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is NULL.", dR); \
                return; \
        } \
        EG_ASSERT_MALLOC( v );



/* EG_Widget payload:
 */

#define MAX_RADIOGROUP_BUTTONS 256
typedef struct{
        EG_Widget       *button_widget_ptr[MAX_RADIOGROUP_BUTTONS];
        long            count;

	long		selected;
}EG_RadioGroup;
#define EG_AsRadioGroup(n) ((EG_RadioGroup*) n)


/* Private and friend functions:
 */



/* END OF radiogroup_private.h --------------------------------------------------
 */
#endif



