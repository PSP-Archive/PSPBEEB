# ifndef BEEB_WIN_C_H
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
# define BEEB_WIN_C_H

#ifdef __cplusplus
extern "C" {
# endif

# define BEEB_RENDER_NORMAL     0
# define BEEB_RENDER_X125       1
# define BEEB_RENDER_X15        2
# define BEEB_RENDER_WIDTH_X2   3
# define BEEB_RENDER_X2         4

# define BEEB_LAST_RENDER       4

# define BEEB_MAX_SAVE_STATE    5

# define    BEEB_MODEL_B        0
# define    BEEB_MODEL_BINT     1
# define    BEEB_MODEL_M128     2

# define BEEB_LAST_MODEL (BEEB_MODEL_M128)
# define BEEB_MAX_MODEL  (BEEB_LAST_MODEL+1)

  typedef struct BBC_save_t {

    SDL_Surface    *surface;
    char            used;
    char            thumb;
    ScePspDateTime  date;

  } BBC_save_t;


  extern void  beeb_emulator_reset();
  extern char *beeb_get_save_name();
  extern BBC_save_t* beeb_get_save_state(int cur_id);

  extern int   beeb_get_snd_enabled();
  extern int   beeb_get_scr_cpu();
  extern int   beeb_get_render_mode();
  extern int   beeb_get_skip_max_frame();
  extern int   beeb_get_display_lr();
  extern int   beeb_get_view_fps();
  extern int   beeb_get_psp_cpu_clock();
  extern int   beeb_get_psp_reverse_analog();
  extern int   beeb_get_psp_screenshot_id();

  extern void   beeb_set_snd_enabled(int value);
  extern void   beeb_set_scr_cpu(int value);
  extern void   beeb_set_render_mode(int value);
  extern void   beeb_set_skip_max_frame(int value);
  extern void   beeb_set_view_fps(int value);
  extern void   beeb_set_display_lr(int value);
  extern void   beeb_set_psp_cpu_clock(int value);
  extern void   beeb_set_psp_reverse_analog(int value);
  extern void   beeb_set_psp_screenshot_id(int value);

  extern int    beeb_state_load(char *filename, int zip_format);
  extern int    beeb_disk_load(char *filename, int disk_id, int zip_format);

  extern int    beeb_load_file_settings(char *FileName);
  extern int    beeb_load_settings(void);
  extern void   beeb_default_settings(void);
  extern int    beeb_save_settings(void);
  extern void   beeb_audio_resume(void);
  extern void   beeb_audio_pause(void);

  extern int    beeb_is_save_used(int slot_id);
  extern ScePspDateTime* beeb_get_save_date(int slot_id);
  extern int    beeb_snapshot_load_slot(int slot_id);
  extern int    beeb_snapshot_save_slot(int slot_id);
  extern int    beeb_snapshot_del_slot(int slot_id);

  extern void  beeb_set_model(int model);
  extern int   beeb_get_model(void);
  extern char* beeb_get_homedir(void);

#ifdef __cplusplus
 }
# endif

# endif
