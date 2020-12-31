/*
 *  Copyright (C) 2006 Ludovic Jacomme (ludovic.jacomme@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

# ifndef _KBD_H_
# define _KBD_H_

#ifdef __cplusplus
extern "C" {
# endif

# define PSP_ALL_BUTTON_MASK 0xFFFF

 enum bbc_keys_emum {
   BBC_UNDERSCORE, // _
   BBC_1,          // 1
   BBC_2,          // 2
   BBC_3,          // 3
   BBC_4,          // 4
   BBC_5,          // 5
   BBC_6,          // 6
   BBC_7,          // 7
   BBC_8,          // 8
   BBC_9,          // 9
   BBC_0,          // 0
   BBC_SEMICOLON,  // ;
   BBC_MINUS    ,  // -
   BBC_DELETE,     // DELETE
   BBC_POUND,      // POUND
   BBC_EXCLAMATN,  // !
   BBC_DBLQUOTE,   // "
   BBC_HASH,       // #
   BBC_DOLLAR,     // $
   BBC_PERCENT,    // %
   BBC_AMPERSAND,  // &
   BBC_QUOTE,      // '
   BBC_LEFTPAREN,  // (
   BBC_RIGHTPAREN, // )
   BBC_PLUS,       // +
   BBC_EQUAL,      // =
   BBC_TAB,        // TAB  
   BBC_a,          // a
   BBC_b,          // b
   BBC_c,          // c
   BBC_d,          // d
   BBC_e,          // e
   BBC_f,          // f
   BBC_g,          // g
   BBC_h,          // h
   BBC_i,          // i
   BBC_j,          // j
   BBC_k,          // k
   BBC_l,          // l
   BBC_m,          // m
   BBC_n,          // n
   BBC_o,          // o
   BBC_p,          // p
   BBC_q,          // q
   BBC_r,          // r
   BBC_s,          // s
   BBC_t,          // t
   BBC_u,          // u
   BBC_v,          // v
   BBC_w,          // w
   BBC_x,          // x
   BBC_y,          // y
   BBC_z,          // z
   BBC_A,          // A
   BBC_B,          // B
   BBC_C,          // C
   BBC_D,          // D
   BBC_E,          // E
   BBC_F,          // F
   BBC_G,          // G
   BBC_H,          // H
   BBC_I,          // I
   BBC_J,          // J
   BBC_K,          // K
   BBC_L,          // L
   BBC_M,          // M
   BBC_N,          // N
   BBC_O,          // O
   BBC_P,          // P
   BBC_Q,          // Q
   BBC_R,          // R
   BBC_S,          // S
   BBC_T,          // T
   BBC_U,          // U
   BBC_V,          // V
   BBC_W,          // W
   BBC_X,          // X
   BBC_Y,          // Y
   BBC_Z,          // Z
   BBC_RETURN,     // RETURN
   BBC_CONTROL,    // CONTROL
   BBC_SHIFT,      // SHIFT
   BBC_CAPSLOCK,   // CAPSLOCK
   BBC_ESC,        // ESC
   BBC_SPACE,      // SPACE
   BBC_LEFT,       // LEFT
   BBC_UP,         // UP
   BBC_RIGHT,      // RIGHT
   BBC_DOWN,       // DOWN
   BBC_COPY,       // COPY
   BBC_F0,         // F0
   BBC_F1,         // F1
   BBC_F2,         // F2
   BBC_F3,         // F3
   BBC_F4,         // F4
   BBC_F5,         // F5
   BBC_F6,         // F6
   BBC_F7,         // F7
   BBC_F8,         // F8
   BBC_F9,         // F9
   BBC_AT,         // @
   BBC_COLON,      // :
   BBC_COMMA,      // ,
   BBC_PERIOD,     // .
   BBC_SLASH,      // /
   BBC_ASTERISK,   // *
   BBC_LESS,       // <
   BBC_GREATER,    // >
   BBC_QUESTION,   // ?
   BBC_BREAK,      // BREAK
   BBC_PIPE,       // |
   BBC_RCBRACE,    // }
   BBC_RBRACKET,   // ]
   BBC_LBRACKET,   // [
   BBC_LCBRACE,    // {
   BBC_TILDA,      // ~
   BBC_BACKSLASH,  //  
   BBC_POWER,      // ^

   BBC_MAX_KEY 
 };

 struct bbc_key_trans {
   int  key;
   int  row;
   int  col;
   int  shift;
   char name[10];
 };
  
 extern struct bbc_key_trans psp_bbc_key_to_row_col[BBC_MAX_KEY];

# define KBD_UP           0
# define KBD_RIGHT        1
# define KBD_DOWN         2
# define KBD_LEFT         3
# define KBD_TRIANGLE     4
# define KBD_CIRCLE       5
# define KBD_CROSS        6
# define KBD_SQUARE       7
# define KBD_SELECT       8
# define KBD_START        9
# define KBD_HOME        10
# define KBD_HOLD        11
# define KBD_LTRIGGER    12
# define KBD_RTRIGGER    13

# define KBD_MAX_BUTTONS 14

# define KBD_JOY_UP      14
# define KBD_JOY_RIGHT   15
# define KBD_JOY_DOWN    16
# define KBD_JOY_LEFT    17

# define KBD_ALL_BUTTONS 18

# define KBD_UNASSIGNED         -1

# define KBD_LTRIGGER_MAPPING   -2
# define KBD_RTRIGGER_MAPPING   -3
# define KBD_NORMAL_MAPPING     -1

  extern int psp_screenshot_mode;
  extern int psp_kbd_mapping[ KBD_ALL_BUTTONS ];
  extern int psp_kbd_mapping_L[ KBD_ALL_BUTTONS ];
  extern int psp_kbd_mapping_R[ KBD_ALL_BUTTONS ];
  extern int psp_kbd_presses[ KBD_ALL_BUTTONS ];
  extern int kbd_ltrigger_mapping_active;
  extern int kbd_rtrigger_mapping_active;
  
  extern int  psp_update_keys(void);
  extern void kbd_wait_start(void);
  extern void psp_init_keyboard(void);
  extern void psp_kbd_wait_no_button(void);
  extern int  psp_kbd_is_danzeff_mode(void);
  extern void psp_kbd_display_active_mapping(void);

#ifdef __cplusplus
}
# endif

# endif
