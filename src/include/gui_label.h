/* START OF label.h ------------------------------------------------------------
 *
 *	Simple label widget
 *
 *      ---
 *      Written by David Eggleston (2006) <deggleston@users.sourceforge.net>
 *      for the 'BeebEm' Acorn BBC Model B, Integra-B, Model B Plus and
 *      Master 128 emulator.
 *
 *      This file is part of 'Economy GUI' and may be copied only under the
 *      terms of either the GNU General Public License (GPL) or the Artistic
 *      License.
 *
 *      For more details please visit:
 *
 *      http://www.gnu.org/copyleft/gpl.html.
 *      and
 *      http://www.opensource.org/licenses/artistic-license.php
 *      ---
 */


#ifndef _DW_LABEL_H_
#define _DW_LABEL_H_


#if HAVE_CONFIG_H
#       include <config.h>
#endif

#include "include/types.h"
#include "include/sdl.h"

#include "include/gui_window.h"
#include "include/gui_widget.h"

#include <SDL.h>


/* Widget type:
 */
#define EG_Widget_Type_Label 0x5ED5400D


/* Alignment:
 */
#define EG_LABEL_ALIGN_LEFT 	-1
#define EG_LABEL_ALIGN_RIGHT 	1
#define EG_LABEL_ALIGN_CENTER 	0

/* Public functions:
 */
EG_Widget* 	EG_Label_Create(const char *name_ptr, SDL_Color color, int alignment, const char *caption_ptr, SDL_Rect dimension);

EG_BOOL 	EG_Label_SetCaption(EG_Widget *widget_ptr, const char *caption_ptr);
const char* 	EG_Label_GetCaption(EG_Widget *widget_ptr);

int 		EG_Label_GetAlignment(EG_Widget *widget_ptr);
EG_BOOL 	EG_Label_SetAlignment(EG_Widget *widget_ptr, int);

#define EG_Label_Destroy			EG_Widget_CallDestroy

#define EG_Label_SetMyCallback_OnEvent		EG_Widget_SetUserCallback_OnEvent
#define EG_Label_ClearMyCallback_OnEvent	EG_Widget_ClearUserCallback_OnEvent

#define EG_Label_Hide				EG_Window_HideWidget
#define EG_Label_Show				EG_Window_ShowWidget

#define EG_Label_IsVisible			EG_Widget_IsVisible
#define EG_Label_IsEnabled			EG_Widget_IsEnabled

#define EG_Label_SetBackgroundColor		EG_Widget_SetBackgroundColor
#define EG_Label_GetBackgroundColor		EG_Widget_GetBackgroundColor

#define EG_Label_Repaint			EG_Widget_Repaint
#define EG_Label_RepaintLot			EG_Widget_RepaintLot

#define EG_Label_GetID				EG_Widget_GetID
#define EG_Label_GetName			EG_Widget_GetName

/* END OF label.h --------------------------------------------------------------
 */
#endif



