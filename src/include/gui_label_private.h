/* START OF label_private.h ----------------------------------------------------
 *
 *	Simple label widget
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


#ifndef _DW_LABEL_PRIVATE_H_
#define _DW_LABEL_PRIVATE_H_

#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include "include/types.h"
#include <SDL.h>

#include "include/gui_label.h"

#include "include/gui_window.h"
#include "include/gui_widget.h"


/* Get EG_Label struct.
 */

#define EG_LABEL_GET_STRUCT_PTR(w, v, r)  \
	if (w == NULL ){ \
		EG_Log(EG_LOG_ERROR, dL"EG_Widget is NULL.", dR); \
		return( r ); \
	} \
	if (EG_Widget_GetType( w ) != EG_Widget_Type_Label){ \
		EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is not of type EG_Label.", dR); \
		return( r ); \
	} \
	if ( (v = (EG_Label*) EG_Widget_GetPayload( w )) == NULL){ \
                EG_Log(EG_LOG_ERROR, dL"EG_Widget payload is NULL.", dR); \
                return(EG_FALSE); \
        } \
        EG_ASSERT_MALLOC( (void*) v );


/* Maximum size a label caption can be (minus the '\0', I add one for that).
 */
#define MAX_LABEL_CAPTIONSIZE         256

/* EG_Widget payload:
 */
typedef struct{
	int alignment;
        char caption[MAX_LABEL_CAPTIONSIZE+1];
}EG_Label;
#define EG_AsLabel(n) ((EG_Label*) n)


/* Private and Friends Functions:
 */

		// If I had any friends they'd be here, but I don't :-( so
		// they're not.


/* END OF label_private.h ------------------------------------------------------
 */
#endif



