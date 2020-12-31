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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>

#include <pspctrl.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <SDL.h>

#include "global.h"
#include "beebwinc.h"
#include "psp_kbd.h"
#include "psp_menu.h"
#include "psp_sdl.h"
#include "psp_danzeff.h"
#include "psp_irkeyb.h"
#include "psp_battery.h"

# define KBD_MIN_ANALOG_TIME      150000
# define KBD_MIN_START_TIME       800000
# define KBD_MAX_EVENT_TIME       500000
# define KBD_MIN_PENDING_TIME     100000
# define KBD_MIN_IR_TIME          100000
# define KBD_MIN_DANZEFF_TIME     150000
# define KBD_MIN_COMMAND_TIME     100000
# define KBD_MIN_BOOT_TIME       1000000
# define KBD_MIN_BATTCHECK_TIME 90000000 

 static SceCtrlData    loc_button_data;
 static unsigned int   loc_last_event_time = 0;
#ifdef USE_PSP_IRKEYB
 static unsigned int   loc_last_irkbd_event_time = 0;
#endif
 static unsigned int   loc_last_analog_time = 0;
 static long           first_time_stamp = -1;
 static char           loc_button_press[ KBD_MAX_BUTTONS ]; 
 static char           loc_button_release[ KBD_MAX_BUTTONS ]; 
 static unsigned int   loc_button_mask[ KBD_MAX_BUTTONS ] =
 {
   PSP_CTRL_UP         , /*  KBD_UP         */
   PSP_CTRL_RIGHT      , /*  KBD_RIGHT      */
   PSP_CTRL_DOWN       , /*  KBD_DOWN       */
   PSP_CTRL_LEFT       , /*  KBD_LEFT       */
   PSP_CTRL_TRIANGLE   , /*  KBD_TRIANGLE   */
   PSP_CTRL_CIRCLE     , /*  KBD_CIRCLE     */
   PSP_CTRL_CROSS      , /*  KBD_CROSS      */
   PSP_CTRL_SQUARE     , /*  KBD_SQUARE     */
   PSP_CTRL_SELECT     , /*  KBD_SELECT     */
   PSP_CTRL_START      , /*  KBD_START      */
   PSP_CTRL_HOME       , /*  KBD_HOME       */
   PSP_CTRL_HOLD       , /*  KBD_HOLD       */
   PSP_CTRL_LTRIGGER   , /*  KBD_LTRIGGER   */
   PSP_CTRL_RTRIGGER   , /*  KBD_RTRIGGER   */
 };

 static char loc_button_name[ KBD_ALL_BUTTONS ][20] =
 {
   "UP",
   "RIGHT",
   "DOWN",
   "LEFT",
   "TRIANGLE",
   "CIRCLE",
   "CROSS",
   "SQUARE",
   "SELECT",
   "START",
   "HOME",
   "HOLD",
   "LTRIGGER",
   "RTRIGGER",
   "JOY_UP",
   "JOY_RIGHT",
   "JOY_DOWN",
   "JOY_LEFT"
 };

 static char loc_button_name_L[ KBD_ALL_BUTTONS ][20] =
 {
   "L_UP",
   "L_RIGHT",
   "L_DOWN",
   "L_LEFT",
   "L_TRIANGLE",
   "L_CIRCLE",
   "L_CROSS",
   "L_SQUARE",
   "L_SELECT",
   "L_START",
   "L_HOME",
   "L_HOLD",
   "L_LTRIGGER",
   "L_RTRIGGER",
   "L_JOY_UP",
   "L_JOY_RIGHT",
   "L_JOY_DOWN",
   "L_JOY_LEFT"
 };
 
  static char loc_button_name_R[ KBD_ALL_BUTTONS ][20] =
 {
   "R_UP",
   "R_RIGHT",
   "R_DOWN",
   "R_LEFT",
   "R_TRIANGLE",
   "R_CIRCLE",
   "R_CROSS",
   "R_SQUARE",
   "R_SELECT",
   "R_START",
   "R_HOME",
   "R_HOLD",
   "R_LTRIGGER",
   "R_RTRIGGER",
   "R_JOY_UP",
   "R_JOY_RIGHT",
   "R_JOY_DOWN",
   "R_JOY_LEFT"
 };
 
  struct bbc_key_trans psp_bbc_key_to_row_col[BBC_MAX_KEY]=
  {
    // BBC            R,C       KEY NAME 
    { BBC_UNDERSCORE, 2,8,   0, "_" },
    { BBC_1,          3,0,   0, "1" },
    { BBC_2,          3,1,   0, "2" },
    { BBC_3,          1,1,   0, "3" },
    { BBC_4,          1,2,   0, "4" },
    { BBC_5,          1,3,   0, "5" },
    { BBC_6,          3,4,   0, "6" },
    { BBC_7,          2,4,   0, "7" },
    { BBC_8,          1,5,   0, "8" },
    { BBC_9,          2,6,   0, "9" },
    { BBC_0,          2,7,   0, "0" },
    { BBC_SEMICOLON,  5,7,   0, ";" },
    { BBC_MINUS    ,  1,7,   0, "-" },
    { BBC_DELETE,     5,9,   0, "DELETE" },
    { BBC_POUND,      2,8,   1, "POUND" },
    { BBC_EXCLAMATN,  3,0,   1, "!" },
    { BBC_DBLQUOTE,   3,1,   1, "\"" },
    { BBC_HASH,       1,1,   1, "#" },
    { BBC_DOLLAR,     1,2,   1, "$" },
    { BBC_PERCENT,    1,3,   1, "%" },
    { BBC_AMPERSAND,  3,4,   1, "&" },
    { BBC_QUOTE,      2,4,   1, "'" },
    { BBC_LEFTPAREN,  1,5,   1, "(" },
    { BBC_RIGHTPAREN, 2,6,   1, ")" },
    { BBC_PLUS,       5,7,   1, "+" },
    { BBC_EQUAL,      1,7,   1, "=" },
    { BBC_TAB,        6,0,   0, "TAB  " },
    { BBC_a,          4,1,   0, "a" },
    { BBC_b,          6,4,   0, "b" },
    { BBC_c,          5,2,   0, "c" },
    { BBC_d,          3,2,   0, "d" },
    { BBC_e,          2,2,   0, "e" },
    { BBC_f,          4,3,   0, "f" },
    { BBC_g,          5,3,   0, "g" },
    { BBC_h,          5,4,   0, "h" },
    { BBC_i,          2,5,   0, "i" },
    { BBC_j,          4,5,   0, "j" },
    { BBC_k,          4,6,   0, "k" },
    { BBC_l,          5,6,   0, "l" },
    { BBC_m,          6,5,   0, "m" },
    { BBC_n,          5,5,   0, "n" },
    { BBC_o,          3,6,   0, "o" },
    { BBC_p,          3,7,   0, "p" },
    { BBC_q,          1,0,   0, "q" },
    { BBC_r,          3,3,   0, "r" },
    { BBC_s,          5,1,   0, "s" },
    { BBC_t,          2,3,   0, "t" },
    { BBC_u,          3,5,   0, "u" },
    { BBC_v,          6,3,   0, "v" },
    { BBC_w,          2,1,   0, "w" },
    { BBC_x,          4,2,   0, "x" },
    { BBC_y,          4,4,   0, "y" },
    { BBC_z,          6,1,   0, "z" },
    { BBC_A,          4,1,   1, "A" },
    { BBC_B,          6,4,   1, "B" },
    { BBC_C,          5,2,   1, "C" },
    { BBC_D,          3,2,   1, "D" },
    { BBC_E,          2,2,   1, "E" },
    { BBC_F,          4,3,   1, "F" },
    { BBC_G,          5,3,   1, "G" },
    { BBC_H,          5,4,   1, "H" },
    { BBC_I,          2,5,   1, "I" },
    { BBC_J,          4,5,   1, "J" },
    { BBC_K,          4,6,   1, "K" },
    { BBC_L,          5,6,   1, "L" },
    { BBC_M,          6,5,   1, "M" },
    { BBC_N,          5,5,   1, "N" },
    { BBC_O,          3,6,   1, "O" },
    { BBC_P,          3,7,   1, "P" },
    { BBC_Q,          1,0,   1, "Q" },
    { BBC_R,          3,3,   1, "R" },
    { BBC_S,          5,1,   1, "S" },
    { BBC_T,          2,3,   1, "T" },
    { BBC_U,          3,5,   1, "U" },
    { BBC_V,          6,3,   1, "V" },
    { BBC_W,          2,1,   1, "W" },
    { BBC_X,          4,2,   1, "X" },
    { BBC_Y,          4,4,   1, "Y" },
    { BBC_Z,          6,1,   1, "Z" },
    { BBC_RETURN,     4,9,   0, "RETURN" },
    { BBC_CONTROL,    0,1,   0, "CONTROL" },
    { BBC_SHIFT,      0,0,   0, "SHIFT" },
    { BBC_CAPSLOCK,   4,0,   0, "CAPSLOCK" },
    { BBC_ESC,        7,0,   0, "ESC" },
    { BBC_SPACE,      6,2,   0, "SPACE" },
    { BBC_LEFT,       1,9,   0, "LEFT" },
    { BBC_UP,         3,9,   0, "UP" },
    { BBC_RIGHT,      7,9,   0, "RIGHT" },
    { BBC_DOWN,       2,9,   0, "DOWN" },
    { BBC_COPY,       6,9,   0, "COPY" },
    { BBC_F0,         2,0,   0, "F0" },
    { BBC_F1,         7,1,   0, "F1" },
    { BBC_F2,         7,2,   0, "F2" },
    { BBC_F3,         7,3,   0, "F3" },
    { BBC_F4,         1,4,   0, "F4" },
    { BBC_F5,         7,4,   0, "F5" },
    { BBC_F6,         7,5,   0, "F6" },
    { BBC_F7,         1,6,   0, "F7" },
    { BBC_F8,         7,6,   0, "F8" },
    { BBC_F9,         7,7,   0, "F9" },
    { BBC_AT,         4,7,   0, "@" },
    { BBC_COLON,      4,8,   0, ":" },
    { BBC_COMMA,      6,6,   0, "," },
    { BBC_PERIOD,     6,7,   0, "." },
    { BBC_SLASH,      6,8,   0, "/" },
    { BBC_ASTERISK,   4,8,   1, "*" },
    { BBC_LESS,       6,6,   1, "<" },
    { BBC_GREATER,    6,7,   1, ">" },
    { BBC_QUESTION,   6,8,   1, "?" },
    { BBC_BREAK,    -2,-2,   0, "BREAK" },

    { BBC_PIPE,       7,8,   1, "|" },
    { BBC_RCBRACE,    5,8,   1, "}" },
    { BBC_RBRACKET,   5,8,   0, "]" },
    { BBC_LBRACKET,   3,8,   0, "[" },
    { BBC_LCBRACE,    3,8,   1, "{" },
    { BBC_TILDA    ,  1,8,   1, "~" },
    { BBC_BACKSLASH,  7,8,   0, "\\" },
    { BBC_POWER,      1,8,   0, "^" }
  };

  static int loc_default_mapping[ KBD_ALL_BUTTONS ] = {
    BBC_UP              , /*  KBD_UP         */
    BBC_RIGHT           , /*  KBD_RIGHT      */
    BBC_DOWN            , /*  KBD_DOWN       */
    BBC_LEFT            , /*  KBD_LEFT       */
    BBC_RETURN          , /*  KBD_TRIANGLE   */
    BBC_BREAK           , /*  KBD_CIRCLE     */
    BBC_SPACE           , /*  KBD_CROSS      */
    BBC_DELETE          , /*  KBD_SQUARE     */
    -1                  , /*  KBD_SELECT     */
    -1                  , /*  KBD_START      */
    -1                  , /*  KBD_HOME       */
    -1                  , /*  KBD_HOLD       */
    KBD_LTRIGGER_MAPPING, /*  KBD_LTRIGGER   */
    KBD_RTRIGGER_MAPPING, /*  KBD_RTRIGGER   */
    BBC_C               , /*  KBD_JOY_UP     */
    BBC_D               , /*  KBD_JOY_RIGHT  */
    BBC_E               , /*  KBD_JOY_DOWN   */
    BBC_F                 /*  KBD_JOY_LEFT   */
  };
  
  static int loc_default_mapping_L[ KBD_ALL_BUTTONS ] = {
    BBC_UP              , /*  KBD_UP         */
    BBC_RIGHT           , /*  KBD_RIGHT      */
    BBC_DOWN            , /*  KBD_DOWN       */
    BBC_LEFT            , /*  KBD_LEFT       */
    BBC_RETURN          , /*  KBD_TRIANGLE   */
    BBC_BREAK           , /*  KBD_CIRCLE     */
    BBC_SPACE           , /*  KBD_CROSS      */
    BBC_DELETE          , /*  KBD_SQUARE     */
    -1                  , /*  KBD_SELECT     */
    -1                  , /*  KBD_START      */
    -1                  , /*  KBD_HOME       */
    -1                  , /*  KBD_HOLD       */
    KBD_LTRIGGER_MAPPING, /*  KBD_LTRIGGER   */
    KBD_RTRIGGER_MAPPING, /*  KBD_RTRIGGER   */
    BBC_G               , /*  KBD_JOY_UP     */
    BBC_H               , /*  KBD_JOY_RIGHT  */
    BBC_I               , /*  KBD_JOY_DOWN   */
    BBC_J                 /*  KBD_JOY_LEFT   */
  };
  
  static int loc_default_mapping_R[ KBD_ALL_BUTTONS ] = {
    BBC_UP              , /*  KBD_UP         */
    BBC_RIGHT           , /*  KBD_RIGHT      */
    BBC_DOWN            , /*  KBD_DOWN       */
    BBC_LEFT            , /*  KBD_LEFT       */
    BBC_RETURN          , /*  KBD_TRIANGLE   */
    BBC_BREAK           , /*  KBD_CIRCLE     */
    BBC_SPACE           , /*  KBD_CROSS      */
    BBC_DELETE          , /*  KBD_SQUARE     */
    -1                  , /*  KBD_SELECT     */
    -1                  , /*  KBD_START      */
    -1                  , /*  KBD_HOME       */
    -1                  , /*  KBD_HOLD       */
    KBD_LTRIGGER_MAPPING, /*  KBD_LTRIGGER   */
    KBD_RTRIGGER_MAPPING, /*  KBD_RTRIGGER   */
    BBC_K               , /*  KBD_JOY_UP     */
    BBC_L               , /*  KBD_JOY_RIGHT  */
    BBC_M               , /*  KBD_JOY_DOWN   */
    BBC_N                 /*  KBD_JOY_LEFT   */
  };

# define KBD_MAX_ENTRIES   116

  int kbd_layout[KBD_MAX_ENTRIES][2] = {
    /* Key            Ascii */
    { BBC_0,          '0' },
    { BBC_1,          '1' },
    { BBC_2,          '2' },
    { BBC_3,          '3' },
    { BBC_4,          '4' },
    { BBC_5,          '5' },
    { BBC_6,          '6' },
    { BBC_7,          '7' },
    { BBC_8,          '8' },
    { BBC_9,          '9' },
    { BBC_A,          'A' },
    { BBC_B,          'B' },
    { BBC_C,          'C' },
    { BBC_D,          'D' },
    { BBC_E,          'E' },
    { BBC_F,          'F' },
    { BBC_G,          'G' },
    { BBC_H,          'H' },
    { BBC_I,          'I' },
    { BBC_J,          'J' },
    { BBC_K,          'K' },
    { BBC_L,          'L' },
    { BBC_M,          'M' },
    { BBC_N,          'N' },
    { BBC_O,          'O' },
    { BBC_P,          'P' },
    { BBC_Q,          'Q' },
    { BBC_R,          'R' },
    { BBC_S,          'S' },
    { BBC_T,          'T' },
    { BBC_U,          'U' },
    { BBC_V,          'V' },
    { BBC_W,          'W' },
    { BBC_X,          'X' },
    { BBC_Y,          'Y' },
    { BBC_Z,          'Z' },
    { BBC_a,          'a' },
    { BBC_b,          'b' },
    { BBC_c,          'c' },
    { BBC_d,          'd' },
    { BBC_e,          'e' },
    { BBC_f,          'f' },
    { BBC_g,          'g' },
    { BBC_h,          'h' },
    { BBC_i,          'i' },
    { BBC_j,          'j' },
    { BBC_k,          'k' },
    { BBC_l,          'l' },
    { BBC_m,          'm' },
    { BBC_n,          'n' },
    { BBC_o,          'o' },
    { BBC_p,          'p' },
    { BBC_q,          'q' },
    { BBC_r,          'r' },
    { BBC_s,          's' },
    { BBC_t,          't' },
    { BBC_u,          'u' },
    { BBC_v,          'v' },
    { BBC_w,          'w' },
    { BBC_x,          'x' },
    { BBC_y,          'y' },
    { BBC_z,          'z' },
    { BBC_DELETE,     DANZEFF_DEL },
    { BBC_SPACE,      ' '         },
    { BBC_F0,         DANZEFF_F0  },
    { BBC_F1,         DANZEFF_F1  },
    { BBC_F2,         DANZEFF_F2  },
    { BBC_F3,         DANZEFF_F3  },
    { BBC_F4,         DANZEFF_F4  },
    { BBC_F5,         DANZEFF_F5  },
    { BBC_F6,         DANZEFF_F6  },
    { BBC_F7,         DANZEFF_F7  },
    { BBC_F8,         DANZEFF_F8  },
    { BBC_F9,         DANZEFF_F9  },
    { BBC_CAPSLOCK,   DANZEFF_CAPSLOCK },
    { BBC_RETURN,     DANZEFF_RETURN   },
    { BBC_SHIFT,      DANZEFF_SHIFT    },
    { BBC_COPY,       DANZEFF_COPY     },
    { BBC_TAB,        DANZEFF_TAB      },
    { BBC_AMPERSAND,  '&' },
    { BBC_ASTERISK,   '*' },
    { BBC_AT,         '@' },
    { BBC_COLON,      ':' },
    { BBC_COMMA,      ',' },
    { BBC_CONTROL,    DANZEFF_CONTROL  },
    { BBC_DOWN,       -1  },
    { BBC_LEFT,       -1  },
    { BBC_RIGHT,      -1  },
    { BBC_UP,         -1  },
    { BBC_DBLQUOTE,   '"' },
    { BBC_QUOTE,      '\'' },
    { BBC_DOLLAR,     '$' },
    { BBC_EQUAL,      '=' },
    { BBC_ESC,        DANZEFF_ESC   },
    { BBC_EXCLAMATN,  '!' },
    { BBC_GREATER,    '>' },
    { BBC_HASH,       '#' },
    { BBC_LEFTPAREN,  '(' },
    { BBC_LESS,       '<' },
    { BBC_MINUS,      '-' },
    { BBC_PERCENT,    '%' },
    { BBC_PERIOD,     '.' },
    { BBC_PLUS,       '+' },
    { BBC_QUESTION,   '?' },
    { BBC_RIGHTPAREN, ')' },
    { BBC_SEMICOLON,  ';' },
    { BBC_SLASH,      '/' },
    { BBC_UNDERSCORE, '_'  },
    { BBC_PIPE,       '|' },
    { BBC_RCBRACE,    '}' },
    { BBC_RBRACKET,   ']' },
    { BBC_LBRACKET,   '[' },
    { BBC_LCBRACE,    '{' },
    { BBC_TILDA    ,  '~' },
    { BBC_BACKSLASH,  '\\' },
    { BBC_POWER,      '^' }
  };

 int psp_kbd_mapping[ KBD_ALL_BUTTONS ];
 int psp_kbd_mapping_L[ KBD_ALL_BUTTONS ];
 int psp_kbd_mapping_R[ KBD_ALL_BUTTONS ];
 int psp_kbd_presses[ KBD_ALL_BUTTONS ];
 int kbd_ltrigger_mapping_active;
 int kbd_rtrigger_mapping_active;
  
 static int danzeff_beeb_key     = 0;
 static int danzeff_beeb_pending = 0;
 static int danzeff_mode        = 0;

#ifdef USE_PSP_IRKEYB
 static int irkeyb_beeb_pending = 0;
 static int irkeyb_beeb_key     = 0;
# endif

       char command_keys[ 128 ];
 static int command_mode        = 0;
 static int command_index       = 0;
 static int command_size        = 0;
 static int command_beeb_pending = 0;
 static int command_beeb_key     = 0;

 static int enter_boot_mode = 0;

int
beeb_key_event(int bbc_idx, int press)
{
  int row;
  int col;
  int shift;

  if ((bbc_idx >=          0) && 
      (bbc_idx < BBC_MAX_KEY)) {
    row   = psp_bbc_key_to_row_col[bbc_idx].row;
    col   = psp_bbc_key_to_row_col[bbc_idx].col;
    shift = psp_bbc_key_to_row_col[bbc_idx].shift;

    if (press) {
      if ((bbc_idx != BBC_CAPSLOCK) && 
          (bbc_idx != BBC_SHIFT   )) {
        if (shift) BeebKeyDown(0,0);
      }
      BeebKeyDown(row, col);
    } else {
      BeebKeyUp(row, col);
      if ((bbc_idx != BBC_CAPSLOCK) && 
          (bbc_idx != BBC_SHIFT   )) {
        if (shift) BeebKeyUp(0,0);
      }
    }
  }
  return 0;
}
int 
beeb_kbd_reset()
{
  BeebReleaseAllKeys();
  return 0;
}

int
beeb_get_key_from_ascii(int key_ascii)
{
  int index;
  for (index = 0; index < KBD_MAX_ENTRIES; index++) {
   if (kbd_layout[index][1] == key_ascii) return kbd_layout[index][0];
  }
  return -1;
}

void
psp_kbd_run_command(char *Command)
{
  strncpy(command_keys, Command, 128);
  command_size  = strlen(Command);
  command_index = 0;

  command_beeb_key     = 0;
  command_beeb_pending = 0;
  command_mode         = 1;
}

int
psp_kbd_reset_mapping(void)
{
  memcpy(psp_kbd_mapping, loc_default_mapping, sizeof(loc_default_mapping));
  memcpy(psp_kbd_mapping_L, loc_default_mapping_L, sizeof(loc_default_mapping_L));
  memcpy(psp_kbd_mapping_R, loc_default_mapping_R, sizeof(loc_default_mapping_R));
  return 0;
}

int
psp_kbd_load_mapping(char *kbd_filename)
{
  FILE    *KbdFile;
  int      error = 0;
  
  KbdFile = fopen(kbd_filename, "r");
  error   = 1;

  if (KbdFile != (FILE*)0) {
  psp_kbd_load_mapping_file(KbdFile);
  error = 0;
    fclose(KbdFile);
  }

  kbd_ltrigger_mapping_active = 0;
  kbd_rtrigger_mapping_active = 0;
    
  return error;
}

int
psp_kbd_load_mapping_file(FILE *KbdFile)
{
  char     Buffer[512];
  char    *Scan;
  int      tmp_mapping[KBD_ALL_BUTTONS];
  int      tmp_mapping_L[KBD_ALL_BUTTONS];
  int      tmp_mapping_R[KBD_ALL_BUTTONS];
  int      beeb_key_id = 0;
  int      kbd_id = 0;

  memcpy(tmp_mapping, loc_default_mapping, sizeof(loc_default_mapping));
  memcpy(tmp_mapping_L, loc_default_mapping_L, sizeof(loc_default_mapping_R));
  memcpy(tmp_mapping_R, loc_default_mapping_R, sizeof(loc_default_mapping_R));

  while (fgets(Buffer,512,KbdFile) != (char *)0) {
      
      Scan = strchr(Buffer,'\n');
      if (Scan) *Scan = '\0';
      /* For this #@$% of windows ! */
      Scan = strchr(Buffer,'\r');
      if (Scan) *Scan = '\0';
      if (Buffer[0] == '#') continue;

      Scan = strchr(Buffer,'=');
      if (! Scan) continue;
    
      *Scan = '\0';
      beeb_key_id = atoi(Scan + 1);

      for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++) {
        if (!strcasecmp(Buffer,loc_button_name[kbd_id])) {
          tmp_mapping[kbd_id] = beeb_key_id;
          //break;
        }
      }
      for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++) {
        if (!strcasecmp(Buffer,loc_button_name_L[kbd_id])) {
          tmp_mapping_L[kbd_id] = beeb_key_id;
          //break;
        }
      }
      for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++) {
        if (!strcasecmp(Buffer,loc_button_name_R[kbd_id])) {
          tmp_mapping_R[kbd_id] = beeb_key_id;
          //break;
        }
      }
  }

  memcpy(psp_kbd_mapping, tmp_mapping, sizeof(psp_kbd_mapping));
  memcpy(psp_kbd_mapping_L, tmp_mapping_L, sizeof(psp_kbd_mapping_L));
  memcpy(psp_kbd_mapping_R, tmp_mapping_R, sizeof(psp_kbd_mapping_R));
  
  return 0;
}

int
psp_kbd_save_mapping(char *kbd_filename)
{
  FILE    *KbdFile;
  int      kbd_id = 0;
  int      error = 0;

  KbdFile = fopen(kbd_filename, "w");
  error   = 1;

  if (KbdFile != (FILE*)0) {

    for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++)
    {
      fprintf(KbdFile, "%s=%d\n", loc_button_name[kbd_id], psp_kbd_mapping[kbd_id]);
    }
    for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++)
    {
      fprintf(KbdFile, "%s=%d\n", loc_button_name_L[kbd_id], psp_kbd_mapping_L[kbd_id]);
    }
    for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++)
    {
      fprintf(KbdFile, "%s=%d\n", loc_button_name_R[kbd_id], psp_kbd_mapping_R[kbd_id]);
    }
    error = 0;
    fclose(KbdFile);
  }

  return error;
}

int
psp_kbd_enter_command()
{
  SceCtrlData  c;

  unsigned int command_key = 0;
  int          beeb_key     = 0;

  sceCtrlPeekBufferPositive(&c, 1);

  if (command_beeb_pending) 
  {
    if ((c.TimeStamp - loc_last_event_time) > KBD_MIN_COMMAND_TIME) {
      loc_last_event_time = c.TimeStamp;
      command_beeb_pending = 0;
      beeb_key_event(command_beeb_key, 0);
    }

    return 0;
  }

  if ((c.TimeStamp - loc_last_event_time) > KBD_MIN_COMMAND_TIME) {
    loc_last_event_time = c.TimeStamp;

    if (command_index >= command_size) {

      command_mode  = 0;
      command_index = 0;
      command_size  = 0;

      command_beeb_pending = 0;
      command_beeb_key     = 0;

      return 0;
    }
  
    command_key = command_keys[command_index++];
    beeb_key = beeb_get_key_from_ascii(command_key);

    if (beeb_key != -1) {
      command_beeb_key     = beeb_key;
      command_beeb_pending = 1;
      beeb_key_event(command_beeb_key, 1);
    }

    return 1;
  }

  return 0;
}

int
psp_kbd_enter_boot_mode()
{
  SceCtrlData  c;

  sceCtrlPeekBufferPositive(&c, 1);

  if (enter_boot_mode == 1) {
    loc_last_event_time = c.TimeStamp;
    BeebKeyDown(0,0);

    enter_boot_mode = 2;
    
    return 0;
  }

  if ((c.TimeStamp - loc_last_event_time) > KBD_MIN_BOOT_TIME) {
    enter_boot_mode = 0;
  }
  return 1;
}

void
psp_kbd_set_boot_mode()
{
  enter_boot_mode = 1;
}

int 
psp_kbd_is_danzeff_mode()
{
  return danzeff_mode;
}

int
psp_kbd_enter_danzeff()
{
  unsigned int danzeff_key = 0;
  int          beeb_key     = 0;
  SceCtrlData  c;

  if (! danzeff_mode) {
    psp_init_keyboard();
    danzeff_mode = 1;
  }

  sceCtrlPeekBufferPositive(&c, 1);

  if (danzeff_beeb_pending) 
  {
    if ((c.TimeStamp - loc_last_event_time) > KBD_MIN_PENDING_TIME) {
      loc_last_event_time = c.TimeStamp;
      danzeff_beeb_pending = 0;
      beeb_key_event(danzeff_beeb_key, 0);
    }

    return 0;
  }

  if ((c.TimeStamp - loc_last_event_time) > KBD_MIN_DANZEFF_TIME) {
    loc_last_event_time = c.TimeStamp;
  
    sceCtrlPeekBufferPositive(&c, 1);
    c.Buttons &= PSP_ALL_BUTTON_MASK;
# ifdef USE_PSP_IRKEYB
    psp_irkeyb_set_psp_key(&c);
# endif
    danzeff_key = danzeff_readInput(c);
  }

  if (danzeff_key == DANZEFF_LEFT) {
    danzeff_key = DANZEFF_DEL;
  } else if (danzeff_key == DANZEFF_DOWN) {
    danzeff_key = DANZEFF_ENTER;
  } else if (danzeff_key == DANZEFF_RIGHT) {
  } else if (danzeff_key == DANZEFF_UP) {
  }

  if (danzeff_key > DANZEFF_START) {
    beeb_key = beeb_get_key_from_ascii(danzeff_key);

    if (beeb_key != -1) {
      danzeff_beeb_key     = beeb_key;
      danzeff_beeb_pending = 1;
      beeb_key_event(danzeff_beeb_key, 1);
    }

    return 1;

  } else if (danzeff_key == DANZEFF_START) {
    danzeff_mode        = 0;
    danzeff_beeb_pending = 0;
    danzeff_beeb_key     = 0;
  } else if (danzeff_key == DANZEFF_SELECT) {
    danzeff_mode        = 0;
    danzeff_beeb_pending = 0;
    danzeff_beeb_key     = 0;
    psp_main_menu();
    //LUDO: psp_init_keyboard();
  }

  return 0;
}

#ifdef USE_PSP_IRKEYB
int
psp_kbd_enter_irkeyb()
{
  int beeb_key   = 0;
  int psp_irkeyb = PSP_IRKEYB_EMPTY;

  SceCtrlData  c;
  sceCtrlPeekBufferPositive(&c, 1);

  if (irkeyb_beeb_pending) 
  {
    if ((c.TimeStamp - loc_last_irkbd_event_time) > KBD_MIN_IR_TIME) {
      loc_last_irkbd_event_time = c.TimeStamp;
      irkeyb_beeb_pending = 0;
      beeb_key_event(irkeyb_beeb_key, 0);
    }

    return 0;
  }

  loc_last_irkbd_event_time = c.TimeStamp;
  
  psp_irkeyb = psp_irkeyb_read_key();
  if (psp_irkeyb != PSP_IRKEYB_EMPTY) {
 
    if (psp_irkeyb == 0x8) {
      beeb_key = BBC_DELETE;
    } else
    if (psp_irkeyb == 0x9) {
      beeb_key = BBC_TAB;
    } else
    if (psp_irkeyb == 0xd) {
      beeb_key = BBC_RETURN;
    } else
    if (psp_irkeyb == 0x1b) {
      beeb_key = BBC_ESC;
    } else
    if (psp_irkeyb == PSP_IRKEYB_UP) {
      beeb_key = BBC_UP;
    } else
    if (psp_irkeyb == PSP_IRKEYB_DOWN) {
      beeb_key = BBC_DOWN;
    } else
    if (psp_irkeyb == PSP_IRKEYB_LEFT) {
      beeb_key = BBC_LEFT;
    } else
    if (psp_irkeyb == PSP_IRKEYB_RIGHT) {
      beeb_key = BBC_RIGHT;
    } else {
      beeb_key = beeb_get_key_from_ascii(psp_irkeyb);
    }
    if (beeb_key != -1) {
      irkeyb_beeb_pending = 1;
      irkeyb_beeb_key = beeb_key;
      beeb_key_event(beeb_key, 1);
 
      return 1;
    }
  }
  
  return 0;
}
# endif

void
psp_kbd_display_active_mapping()
{
  if (kbd_ltrigger_mapping_active) {
    psp_sdl_fill_rectangle(0, 0, 10, 3, psp_sdl_rgb(0x0, 0x0, 0xff), 0);
  } else {
    psp_sdl_fill_rectangle(0, 0, 10, 3, 0x0, 0);
  }

  if (kbd_rtrigger_mapping_active) {
    psp_sdl_fill_rectangle(470, 0, 10, 3, psp_sdl_rgb(0x0, 0x0, 0xff), 0);
  } else {
    psp_sdl_fill_rectangle(470, 0, 10, 3, 0x0, 0);
  }
}

int
beeb_decode_key(int psp_b, int button_pressed)
{
  int wake = 0;
  int reverse_analog = beeb_get_psp_reverse_analog();

  if (reverse_analog) {
    if ((psp_b >= KBD_JOY_UP  ) &&
        (psp_b <= KBD_JOY_LEFT)) {
      psp_b = psp_b - KBD_JOY_UP + KBD_UP;
    } else
    if ((psp_b >= KBD_UP  ) &&
        (psp_b <= KBD_LEFT)) {
      psp_b = psp_b - KBD_UP + KBD_JOY_UP;
    }
  }

  if (psp_b == KBD_START) {
     if (button_pressed) psp_kbd_enter_danzeff();
  } else
  if (psp_b == KBD_SELECT) {
    if (button_pressed) {
      psp_main_menu();
      //LUDO: psp_init_keyboard();
    }
  } else {
 
    if (psp_kbd_mapping[psp_b] >= 0) {
      wake = 1;
      if (button_pressed) {
        // Determine which buton to press first (ie which mapping is currently active)
        if (kbd_ltrigger_mapping_active) {
          // Use ltrigger mapping
          psp_kbd_presses[psp_b] = psp_kbd_mapping_L[psp_b];
          beeb_key_event(psp_kbd_presses[psp_b], button_pressed);
        } else
        if (kbd_rtrigger_mapping_active) {
          // Use rtrigger mapping
          psp_kbd_presses[psp_b] = psp_kbd_mapping_R[psp_b];
          beeb_key_event(psp_kbd_presses[psp_b], button_pressed);
        } else {
          // Use standard mapping
          psp_kbd_presses[psp_b] = psp_kbd_mapping[psp_b];
          beeb_key_event(psp_kbd_presses[psp_b], button_pressed);
        }
      } else {
          // Determine which button to release (ie what was pressed before)
          beeb_key_event(psp_kbd_presses[psp_b], button_pressed);
      }

    } else {
      if (psp_kbd_mapping[psp_b] == KBD_LTRIGGER_MAPPING) {
        kbd_ltrigger_mapping_active = button_pressed;
        kbd_rtrigger_mapping_active = 0;
      } else
      if (psp_kbd_mapping[psp_b] == KBD_RTRIGGER_MAPPING) {
        kbd_rtrigger_mapping_active = button_pressed;
        kbd_ltrigger_mapping_active = 0;
      }
    }
  }
  return 0;
}

# define ANALOG_THRESHOLD 60

void 
kbd_get_analog_direction(int Analog_x, int Analog_y, int *x, int *y)
{
  int DeltaX = 255;
  int DeltaY = 255;
  int DirX   = 0;
  int DirY   = 0;

  *x = 0;
  *y = 0;

  if (Analog_x <=        ANALOG_THRESHOLD)  { DeltaX = Analog_x; DirX = -1; }
  else 
  if (Analog_x >= (255 - ANALOG_THRESHOLD)) { DeltaX = 255 - Analog_x; DirX = 1; }

  if (Analog_y <=        ANALOG_THRESHOLD)  { DeltaY = Analog_y; DirY = -1; }
  else 
  if (Analog_y >= (255 - ANALOG_THRESHOLD)) { DeltaY = 255 - Analog_y; DirY = 1; }

  *x = DirX;
  *y = DirY;
}

static int 
kbd_reset_button_status(void)
{
  int b = 0;
  /* Reset Button status */
  for (b = 0; b < KBD_MAX_BUTTONS; b++) {
    loc_button_press[b]   = 0;
    loc_button_release[b] = 0;
  }
  psp_init_keyboard();
  return 0;
}

int
kbd_scan_keyboard(void)
{
  SceCtrlData c;
  long        delta_stamp;
  int         event;
  int         b;

  int new_Lx;
  int new_Ly;
  int old_Lx;
  int old_Ly;

  event = 0;
  sceCtrlPeekBufferPositive( &c, 1 );
  c.Buttons &= PSP_ALL_BUTTON_MASK;

# ifdef USE_PSP_IRKEYB
  psp_irkeyb_set_psp_key(&c);
# endif

  if ((c.Buttons & (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) ==
      (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) {
    /* Exit ! */
    psp_sdl_exit(0);
  }

  delta_stamp = c.TimeStamp - first_time_stamp;
  if ((delta_stamp < 0) || (delta_stamp > KBD_MIN_BATTCHECK_TIME)) {
    first_time_stamp = c.TimeStamp;
    if (psp_is_low_battery()) {
      psp_main_menu();
      psp_init_keyboard();
      return 0;
    }
  }

  /* Check Analog Device */
  kbd_get_analog_direction(loc_button_data.Lx,loc_button_data.Ly,&old_Lx,&old_Ly);
  kbd_get_analog_direction( c.Lx, c.Ly, &new_Lx, &new_Ly);

  /* Analog device has moved */
  if (new_Lx > 0) {
    if (old_Lx <  0) beeb_decode_key(KBD_JOY_LEFT , 0);
    if (old_Lx <= 0) beeb_decode_key(KBD_JOY_RIGHT, 1);
  } else 
  if (new_Lx < 0) {
    if (old_Lx >  0) beeb_decode_key(KBD_JOY_RIGHT, 0);
    if (old_Lx >= 0) beeb_decode_key(KBD_JOY_LEFT , 1);
  } else {
    if (old_Lx >  0) beeb_decode_key(KBD_JOY_RIGHT , 0);
    else
    if (old_Lx <  0) beeb_decode_key(KBD_JOY_LEFT, 0);
  }

  if (new_Ly > 0) {
    if (old_Ly <  0) beeb_decode_key(KBD_JOY_UP , 0);
    if (old_Ly <= 0) beeb_decode_key(KBD_JOY_DOWN, 1);
  } else 
  if (new_Ly < 0) {
    if (old_Ly >  0) beeb_decode_key(KBD_JOY_DOWN, 0);
    if (old_Ly >= 0) beeb_decode_key(KBD_JOY_UP , 1);
  } else {
    if (old_Ly >  0) beeb_decode_key(KBD_JOY_DOWN , 0);
    else
    if (old_Ly <  0) beeb_decode_key(KBD_JOY_UP, 0);
  }
  
  for (b = 0; b < KBD_MAX_BUTTONS; b++) 
  {
    if (c.Buttons & loc_button_mask[b]) {
      if (!(loc_button_data.Buttons & loc_button_mask[b])) {
        loc_button_press[b] = 1;
        event = 1;
      }
    } else {
      if (loc_button_data.Buttons & loc_button_mask[b]) {
        loc_button_release[b] = 1;
        event = 1;
      }
    }
  }
  memcpy(&loc_button_data,&c,sizeof(SceCtrlData));

  return event;
}

void
kbd_wait_start(void)
{
  while (1)
  {
    SceCtrlData c;
    sceCtrlReadBufferPositive(&c, 1);
    c.Buttons &= PSP_ALL_BUTTON_MASK;
    if (c.Buttons & PSP_CTRL_START) break;
  }
  psp_kbd_wait_no_button();
}

void
psp_init_keyboard(void)
{
  beeb_kbd_reset();
  kbd_ltrigger_mapping_active = 0;
  kbd_rtrigger_mapping_active = 0;
}

void
psp_kbd_wait_no_button(void)
{
  SceCtrlData c;

  do {
   sceCtrlPeekBufferPositive(&c, 1);
   c.Buttons &= PSP_ALL_BUTTON_MASK;

  } while (c.Buttons != 0);
} 

void
psp_kbd_wait_button(void)
{
  SceCtrlData c;

  do {
   sceCtrlReadBufferPositive(&c, 1);
  } while (c.Buttons == 0);
} 

int
psp_update_keys(void)
{
  int         b;

  static char first_time = 1;
  static int release_pending = 0;

  if (first_time) {

    memcpy(psp_kbd_mapping, loc_default_mapping, sizeof(loc_default_mapping));
    memcpy(psp_kbd_mapping_L, loc_default_mapping_L, sizeof(loc_default_mapping_L));
    memcpy(psp_kbd_mapping_R, loc_default_mapping_R, sizeof(loc_default_mapping_R));

    beeb_kbd_load();

    SceCtrlData c;
    sceCtrlPeekBufferPositive(&c, 1);
    c.Buttons &= PSP_ALL_BUTTON_MASK;

    if (first_time_stamp == -1) first_time_stamp = c.TimeStamp;
    if ((! c.Buttons) && ((c.TimeStamp - first_time_stamp) < KBD_MIN_START_TIME)) return 0;

    first_time      = 0;
    release_pending = 0;

    for (b = 0; b < KBD_MAX_BUTTONS; b++) {
      loc_button_release[b] = 0;
      loc_button_press[b] = 0;
    }
    sceCtrlPeekBufferPositive(&loc_button_data, 1);
    loc_button_data.Buttons &= PSP_ALL_BUTTON_MASK;

    psp_main_menu();
    psp_init_keyboard();

    return 0;
  }

  if (enter_boot_mode) {
    return psp_kbd_enter_boot_mode();
  }

  if (command_mode) {
    return psp_kbd_enter_command();
  }

  if (danzeff_mode) {
    return psp_kbd_enter_danzeff();
  }

# ifdef USE_PSP_IRKEYB
  if (psp_kbd_enter_irkeyb()) {
    return 1;
  }
# endif

  if (release_pending)
  {
    release_pending = 0;
    for (b = 0; b < KBD_MAX_BUTTONS; b++) {
      if (loc_button_release[b]) {
        loc_button_release[b] = 0;
        loc_button_press[b] = 0;
        beeb_decode_key(b, 0);
      }
    }
  }

  kbd_scan_keyboard();

  /* check press event */
  for (b = 0; b < KBD_MAX_BUTTONS; b++) {
    if (loc_button_press[b]) {
      loc_button_press[b] = 0;
      release_pending     = 0;
      beeb_decode_key(b, 1);
    }
  }
  /* check release event */
  for (b = 0; b < KBD_MAX_BUTTONS; b++) {
    if (loc_button_release[b]) {
      release_pending = 1;
      break;
    } 
  }

  return 0;
}
