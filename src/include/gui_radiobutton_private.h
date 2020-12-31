/* START OF radiobutton_private.h ----------------------------------------------
 *
 *	A simple Radio Button (wraps the TickBox widget really).
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


#ifndef _DW_RADIOBUTTON_PRIVATE_H_
#define _DW_RADIOBUTTON_PRIVATE_H_


#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include "include/types.h"

#include "include/gui_tickbox_private.h"

#include "include/gui_window.h"
#include "include/gui_widget.h"

#include <SDL.h>


/* Get EG_TickBox struct.
 */
#define EG_RADIOBUTTON_GET_STRUCT_PTR	EG_TICKBOX_GET_STRUCT_PTR

/* Don't really need this.
 */
#define EG_RadioButton EG_TickBox


/* Private and friend functions:
 */

#define EG_RadioButton_SetGroupParent	EG_TickBox_SetGroupParent
#define EG_RadioButton_GetGroupParent	EG_TickBox_GetGroupParent

#define EG_RadioButton_Tick		EG_TickBox_Tick
#define EG_RadioButton_Untick		EG_TickBox_Untick

/* END OF tickbox_private.h ----------------------------------------------------
 */
#endif                                                                           
