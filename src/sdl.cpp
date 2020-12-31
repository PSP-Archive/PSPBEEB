/****************************************************************************/
/*              Beebem - (c) David Alan Gilbert 1994                        */
/*              ------------------------------------                        */
/* This program may be distributed freely within the following restrictions:*/
/*                                                                          */
/* 1) You may not charge for this program or for any part of it.            */
/* 2) This copyright message must be distributed with all copies.           */
/* 3) This program must be distributed complete with source code.  Binary   */
/*    only distribution is not permitted.                                   */
/* 4) The author offers no warrenties, or guarentees etc. - you use it at   */
/*    your own risk.  If it messes something up or destroys your computer   */
/*    thats YOUR problem.                                                   */
/* 5) You may use small sections of code from this program in your own      */
/*    applications - but you must acknowledge its use.  If you plan to use  */
/*    large sections then please ask the author.                            */
/*                                                                          */
/* If you do not agree with any of the above then please do not use this    */
/* program.                                                                 */
/* Please report any problems to the author at beebem@treblig.org           */
/****************************************************************************/

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <unistd.h>
#include "include/sdl.h"

#include "global.h"
#include "include/line.h"
#include "include/log.h"
#include "include/types.h"

// Remove this once command line stuff fixed
#include "include/main.h"

#include <string.h>

#include "psp_sdl.h"
#include "psp_kbd.h"
#include "psp_danzeff.h"
#include "beebwinc.h"

static EG_BOOL EG_DrawString_Initialise(void);
static void EG_DrawString_Free(void);

 ushort video_to_screen_color[8];

/* If set will force the X server to change the colormap to suit BeebEm, if
 * unset will try and find the closest matching colors from the existing
 * colormap and use them instead.  You should probably never need this.
 */

/* Emulate a CRT display (odd scanlines are always dark. Will become an
 * option later on).
 */
//#define EMULATE_CRT

/* If this is defined then the sound code will dump samples (causing distortion)
 * whenever the buffer becomes too large (i.e.: over 5 lots of samples or some
 * such).  If I don't do this then the sound effects in games will happen
 * longer and longer after the event.
 *
 * This is coursed by other processes slowing down BeebEm and the timing of the
 * emulator becoming wrong.  BeebEm will try to compensate, by creating new
 * sound data for the missing time that's then dumped into my sound buffer.
 *
 * It should all work nicely, the emulator core will catchup to the current time
 * and I'll get lots of new sound to play, but the timings off somewhere
 * so this missing sound is converted from a catchup into a latency problem.
 * Unfortunely the more and more interruptions BeebEm has to handle the greater
 * the sound latency problem becomes..  So for I'm going to dump samples when
 * the latency is too great..  If you'd rather have nicer sound (with the
 * latency problem) then remove the definition below.
 */
#define WANT_LOW_LATENCY_SOUND

// This code is nasty :-( but I'm desperate for sound.
// *** it was worth it! YEAH! MELVYN WRIGHT'S MUSIC MACHINE :-) ***
//
// In order to handle sound without studying the BeebEm sound code in too much
// detail, I'll (for now) just let it dump it's samples into this bloody huge
// buffer instead..  The buffer is wrapped so once 1024*100 bytes are used
// we index from 0 again.
#define SOUND_BUFFER_SIZE (1024*100)
Uint8 SDLSoundBuffer[SOUND_BUFFER_SIZE];

// Offset within sound buffer for adding sound to the buffer:
unsigned long SDLSoundBufferOffset_IN;

// Offset within sound buffer for reading sound from the buffer:
unsigned long SDLSoundBufferOffset_OUT;

// Number of bytes left in the buffer.  I.e.: the amount we've yet to read
// (or dump if sound latency becomes too high):
unsigned long SDLSoundBufferBytesHave;

/* Rendering scale 1 = 100%, 0.5 = 50%
 */
static float scale = 1;

// Number of samples we want SDL to work with each time.  The smaller the
// number the lower the latency (I think):
#define REQUESTED_NUMBER_OF_SAMPLES 1024
//#define REQUESTED_NUMBER_OF_SAMPLES 128

// The actual number of samples per time SDL will want (SDL returns this after
// we submit our request for a sound stream):
int samples;

// Sigh, look what I've reduced myself to..  The sound support here is truly
// shocking.. Please feel free to rewrite it for me..
void InitializeSoundBuffer(void)
{
  SDLSoundBufferOffset_IN =0;
  SDLSoundBufferOffset_OUT = 0;
  SDLSoundBufferBytesHave = 0;

  int i;
  for(i=0; i< SOUND_BUFFER_SIZE; i++)
    SDLSoundBuffer[i] = (Uint8) 0;  
}

// When the user switches to the menu, use it as an excuse to flush the buffer.
// Don't like it?  Please feel free to rewrite it for me..  I HATE SOUND..
void FlushSoundBuffer(void)
{
  InitializeSoundBuffer();
}

// The BeebEm emulator core (the Windows code) calls this when it wants to
// play some samples.  We place those samples in our bloody huge buffer 
// instead.
// Please feel free to rewrite the sound support for me..
void AddBytesToSDLSoundBuffer(void *p, int len)
{
  int i;
  Uint8 *pp;
  pp = (Uint8*) p;

  //printf("ADDED %d BYTES\n %d %d %d", len, (int) pp[0], (int) pp[1], (int) pp[2]);

  for(i=0; i<len; i++){
    SDLSoundBuffer[SDLSoundBufferOffset_IN] = *(pp++);

    SDLSoundBufferOffset_IN++;
    if (SDLSoundBufferOffset_IN >= SOUND_BUFFER_SIZE)
      SDLSoundBufferOffset_IN = (unsigned long) 0;
  }
  SDLSoundBufferBytesHave+=len;
}

// Returns a sequential buffer containing the next
// block of sound for SDL to play.
//
// Due to the wrap effect of the bloody huge buffer above, I need to copy
// the samples SDL will one into a sequencial list..  Remember YOU can rewrite
// the sound support if you want!
Uint8 SequentialSDLSoundBuffer[10*1024]; // I'm assuming SDL wont want 10k..

// Get pointer to above array.. Remember YOU can rewrite the sound support if
// you want!
Uint8* GetSoundBufferPtr(void)
{
  return SequentialSDLSoundBuffer;
}

// Removes 'len' bytes from the bloody huge wrapped sound buffer and places it
// into the sequencial list above..  If there are less than 'len' samples
// available, it copies what's available into the sequencial buffer and returns
// the actual amount it managed to copy.  So you need to check the return
// value.
// Remember YOU can rewrite the sound support if you want!
int GetBytesFromSDLSoundBuffer(int len)
{
  static unsigned int uiCatchedUpTimes = 0;
  int i;
  Uint8 *p;

  // check for under sampling (len > SDLSoundBufferBytesHave)
  // (this needs to be communicated back to the caller as the return
  //  value).
  if ( (unsigned long) len > SDLSoundBufferBytesHave)
    len = SDLSoundBufferBytesHave;

  p = SequentialSDLSoundBuffer;

  for(i=0; i<len; i++){
    *(p++) = SDLSoundBuffer[SDLSoundBufferOffset_OUT];
    
    SDLSoundBufferOffset_OUT++;
    if (SDLSoundBufferOffset_OUT >= SOUND_BUFFER_SIZE)
                        SDLSoundBufferOffset_OUT = (unsigned long) 0;
  }

  SDLSoundBufferBytesHave-=len;

#ifdef WANT_LOW_LATENCY_SOUND
  // I really need to catchup if SDLSoundBufferBytesHave becomes too large
  // otherwise the sound will lag more and more behind the action..
  //
  // Things shouldn't be getting out of sync as everything is timed to
  // the systems clock..  But it is..
  //
  // Ok, so, if we have more than five blocks that SDL will want soon left
  // in the sound buffer, lets catch-up by dumping some sound.
  // Hopefully when some other task slows down beebem the sound will
  // crackle but action sound effects will remain in sync.
  //
  // I'll make this optional in the final version so users can either
  // have good but high latency sound, or maybe crap but low latency
  // sound..
  //
  // I could also make sure that if the user switches to the Hatari GUI
  // menu I use it as an excuse to dump everything in the sound buffer.
  if (SDLSoundBufferBytesHave > ( (unsigned long) samples * 5)){
    pINFO(dL"Dumping some sound samples, catched up %u times so far..", dR, ++uiCatchedUpTimes);

    // we dump everything apart from two blocks overwise we're
    // always living on a knife edge.. (i.e.: we're max'ed out
    // all the time and have no spare resources to fall back on.
    while (SDLSoundBufferBytesHave > ( (unsigned long)samples * 2)){
      SDLSoundBufferOffset_OUT++;
      if (SDLSoundBufferOffset_OUT >= SOUND_BUFFER_SIZE)
        SDLSoundBufferOffset_OUT = (unsigned long) 0;
      SDLSoundBufferBytesHave--;
    }
  }
#endif

  return len;
}

// Guess what this does. Remember YOU can rewrite the sound support if you want!
unsigned long HowManyBytesLeftInSDLSoundBuffer(void)
{
  return SDLSoundBufferBytesHave;
}

/*
#define SOUNDFILESIZE 9135855
#define SOUNDFILENAME "./pugwash.dbg"
unsigned char audiobuf[SOUNDFILESIZE];

void loadsound(void)
{
  FILE *f;
  f = fopen(SOUNDFILENAME, "rb");
  if (f==NULL){
          printf("Can't open file\n");
          exit(1);
  }else{
          fread(audiobuf, 1, SOUNDFILESIZE, f);
  }
  fclose(f);
}

//Uint8 *audio_chunk;
//Uint32 audio_len;
//Uint8 *audio_pos;

*/

SDL_Surface *video_output = NULL;
extern SDL_Surface *back_surface;
extern SDL_Surface *back2_surface;

SDL_AudioSpec wanted;

void fill_audio(void *udata, Uint8 *stream, int len)
{
  Uint8 *p;

  void *tmp_udata;
  tmp_udata = udata;

  /* Only play if we have data left */
  if (HowManyBytesLeftInSDLSoundBuffer() == 0)
    return;
//  if ( audio_len == 0 )
//    return;

  // We can only play upto one sample.  I'll fix this later..
//  len = ( len > samples ? samples : len );

//  // Add debuging sound to sound buffer.
//  AddBytesToSDLSoundBuffer(audio_pos, len);
//  audio_pos += len;
//  audio_len -= len;



  
  if (len > (int) HowManyBytesLeftInSDLSoundBuffer() )
    len = HowManyBytesLeftInSDLSoundBuffer();

  p = GetSoundBufferPtr();
  len = GetBytesFromSDLSoundBuffer(len);
  SDL_MixAudio(stream, p, len, SDL_MIX_MAXVOLUME);
}

int 
InitializeSDLSound(int soundfrequency)
{
  samples = REQUESTED_NUMBER_OF_SAMPLES;

  // Offset within sound buffer.
  SDLSoundBufferOffset_IN = 0;
  SDLSoundBufferOffset_OUT = 0;
  SDLSoundBufferBytesHave = 0;

  wanted.freq = soundfrequency;
  wanted.format = AUDIO_U8;
  wanted.channels = 1;
  wanted.samples = samples;      //1024;
  wanted.callback = fill_audio;
  wanted.userdata = NULL;

  /* Open the audio device, forcing the desired format */
  if ( SDL_OpenAudio(&wanted, NULL) < 0 ) {
    fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
    return(0);
  }

//  loadsound();
//      audio_chunk = (Uint8*) &audiobuf;
//      audio_pos = audio_chunk;
//      audio_len = SOUNDFILESIZE;

  InitializeSoundBuffer();

  return(1);
}

void 
FreeSDLSound(void)
{
  SDL_CloseAudio();
}

/* You don't need this.. I hope! ;-)
 */
void SetPalette(void)
{
  SDL_Color colors[8];

  /* Force X Server to set the first eight colors of the hardware
   * colormap to the BBC's colors
   */
  colors[0].r = 0x00; colors[0].g = 0x00; colors[0].b = 0x00; // Black
  colors[1].r = 0xff; colors[1].g = 0x00; colors[1].b = 0x00; // Red
  colors[2].r = 0x00; colors[2].g = 0xff; colors[2].b = 0x00; // Green
  colors[3].r = 0xff; colors[3].g = 0xff; colors[3].b = 0x00; // Yellow
  colors[4].r = 0x00; colors[4].g = 0x00; colors[4].b = 0xff; // Blue
  colors[5].r = 0xff; colors[5].g = 0x00; colors[5].b = 0xff; // Magenta
  colors[6].r = 0x00; colors[6].g = 0xff; colors[6].b = 0xff; // Cyan
  colors[7].r = 0xff; colors[7].g = 0xff; colors[7].b = 0xff; // White
  
  /* Set BeebEm Emulator core bitmaps palette 
   */
  SDL_SetColors(video_output, colors, 0, 8);

  /* Force X Servers palette to change
   */
  SDL_SetColors(back_surface, colors, 0, 8);
  SDL_SetColors(blit_surface, colors, 0, 8);

  for (int i = 0; i < 8; i++) {
    video_to_screen_color[i] = SDL_MapRGB(back_surface->format, colors[i].r, colors[i].g, colors[i].b);
  }
}
/* Called by BeebWin::Initialise() to set the palette colors for BBC colors
 * depending on FORCE_COLORMAP this is either 0 to 7 (as per Windows version),
 * or SDL will search for the BBC colors and set the BeebEm emulator cores
 * color value to the closest matching color.  This is really what you'll want
 * for X Windows.
 */
void 
SetBeebEmEmulatorCoresPalette(unsigned int *cols)
{
  for(int i=0;i<12;i++)
    *(cols++) = (unsigned char) i;

  SetPalette();
}


void 
doCleanUp (void)
{
  SDL_CloseAudio();
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
  SDL_Quit();
}

bool 
InitialiseSDL(int argc, char *argv[])
{
  int tmp_argc;
  char **tmp_argv;

  tmp_argv=argv;
  tmp_argc = argc;

  /* [R001]: Real SDL_Init please.. Lamer..
   * Initialize SDL and handle failures.
   */     
  if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_TIMER|SDL_INIT_NOPARACHUTE|SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "Unable to initialise SDL: %s\n" , SDL_GetError());
    return false;
  }       

# if 0 //LUDO:
  icon = SDL_LoadBMP(DATA_DIR"/resources/icon.bmp");
  if (icon != NULL){
    SDL_SetColorKey(icon, SDL_SRCCOLORKEY, SDL_MapRGB(icon->format
     , 0xff, 0x0, 0xff));
    SDL_WM_SetIcon(icon, NULL);
  }
# endif

  /* [R002]: Don't call SDL_Quit, call your own function that
   *         cleanly exits BeebEm and then calls SDL_Quit.
   *
   * Cleanup SDL when exiting.
   */
  atexit(doCleanUp);

  /* Initialize SDL applications window.
   * NOTE: Both window and video_output surfaces are fixed to 8bit
   * depth at the moment.  I'll work on fixing it later.. 
   */
  if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "Unable to init sub system %s\n" , SDL_GetError());
    return false;
  }

#ifndef LINUX_MODE
  back_surface=SDL_SetVideoMode(SDL_WINDOW_WIDTH,SDL_WINDOW_HEIGHT, 16, 
                              SDL_ANYFORMAT|SDL_DOUBLEBUF|SDL_HWSURFACE|SDL_HWPALETTE);
# else
  back_surface=SDL_SetVideoMode(PSP_SDL_SCREEN_WIDTH,PSP_SDL_SCREEN_HEIGHT, 16 , 
                                SDL_DOUBLEBUF|SDL_HWSURFACE);
# endif

  if ( !back_surface) {
    fprintf(stderr, "Unable to set video mode: %s\n" , SDL_GetError());
    return false;
  }
  SDL_ShowCursor(SDL_DISABLE);

# ifndef LINUX_MODE
  psp_sdl_gu_init();
# endif

  blit_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 
     BEEB_WIDTH, BEEB_HEIGHT,
     back_surface->format->BitsPerPixel,
     back_surface->format->Rmask,
     back_surface->format->Gmask,
     back_surface->format->Bmask, 0);

  // HACK
# ifndef LINUX_MODE
  blit_surface->pixels = (void *)0x44088000;
# endif

  /* Create an area the BeebEm emulator core (the Windows code)
   * can draw on.  It's hardwired to an 800x600 8bit byte per pixel
   * bitmap.
   */
  if ( (video_output = SDL_CreateRGBSurface(SDL_SWSURFACE, 
               BEEBEM_VIDEO_CORE_SCREEN_WIDTH, 
               BEEBEM_VIDEO_CORE_SCREEN_HEIGHT, 8, 0, 0, 0, 0) ) == NULL)
  {
    fprintf(stderr, "Unable to create a bitmap buffer: %s\n" , SDL_GetError());
    return false;
  }

  /* Give our new surface the same palette as the physical application
   * window
   */
# if 0 //LUDO: FOR_TEST
  if (back_surface->format->palette->ncolors > 256)
  {
    fprintf(stderr, "Trying to set 8bit bitmaps palette but have too many colors!  Bug alert.. Quiting\n");
    return false;
  }
  SDL_SetColors(video_output, back_surface->format->palette->colors, 0 , back_surface->format->palette->ncolors);
# endif

  psp_sdl_display_splash();

  /* Danzeff Keyboard */
  danzeff_load();
  danzeff_set_screen(back_surface);

  /* Create surface for save state */
  int Index = 0;
  for (Index = 0; Index < BEEB_MAX_SAVE_STATE; Index++) {
    beeb_get_save_state(Index)->surface = 
       SDL_CreateRGBSurface(SDL_SWSURFACE, 
                            SNAP_WIDTH, SNAP_HEIGHT,
                            back_surface->format->BitsPerPixel,
                            back_surface->format->Rmask,
                            back_surface->format->Gmask,
                            back_surface->format->Bmask, 0);
  }


  InitializeSDLSound(44100);    // Fix hardwiring later..

  SDL_Delay(1000);        // Give sound some time to init

  SDL_PauseAudio(0);

  return true;
}

void 
UninitialiseSDL(void)
{
  FreeSDLSound();  
  SDL_ShowCursor(SDL_ENABLE);
  SDL_FreeSurface(video_output);
}


/* A delay function. BeebEm will 'sleep' for uiTicks milliseconds.
 */
void 
SaferSleep(unsigned int uiTicks)
{
  // Now maybe it's me going insane, but I think the code below is far
  // safer than SDL_Delay on it's own..
  if (uiTicks<1){
    pERROR(dL"Asked to wait for 0 milliseconds.. Assuming this is bogus!", dR);
    return;
  }

  SDL_Delay(uiTicks);
  return;
}

static inline void
loc_RenderTeletextModeNormal_blit(int line)
{
  if (line <0 || line > 511) return;

  ushort *screen_vram = (ushort *)blit_surface->pixels;
  uchar  *video_vram  = (uchar  *)video_output->pixels;
  
  video_vram  += 36 + ((line) * BEEBEM_VIDEO_CORE_SCREEN_WIDTH);
  /* Width 800 -> 400 so we have 40 pixels left on each side */
  screen_vram += (line/2) * blit_surface->w;
  
  for (int x = 0; x < BEEBEM_VIDEO_CORE_SCREEN_WIDTH; x += 2)
  {
    int xd = x >> 1;
    int color = video_to_screen_color[video_vram[x] & 0x7];
    screen_vram[xd] = color;
  }
}

static inline void
loc_RenderTeletextModeNormal_gu()
{
  SDL_Rect srcRect;
  SDL_Rect dstRect;

  srcRect.x = 0;
  srcRect.y = 0;
  srcRect.w = 400;
  srcRect.h = 255;
  dstRect.x = 40;
  dstRect.y = 6;
  dstRect.w = 400;
  dstRect.h = 255;

  psp_sdl_gu_stretch(&srcRect, &dstRect);
}

static inline void
loc_RenderGraphicModeNormal_blit(int line, int xoffset)
{
  int window_y;

  window_y = (line -32);

  if (window_y < 0 || window_y > 255) return;

  ushort *screen_vram = (ushort *)blit_surface->pixels;
  uchar  *video_vram  = (uchar  *)video_output->pixels;

  video_vram  += line * BEEBEM_VIDEO_CORE_SCREEN_WIDTH;
  /* Width 800 -> 400 so we have 40 pixels left on each side */
  screen_vram += (xoffset/2) + (window_y * blit_surface->w);

  for (int x = 0; x < (BEEBEM_VIDEO_CORE_SCREEN_WIDTH - xoffset); x += 2)
  {
    int xd = x >> 1;
    int color = video_to_screen_color[video_vram[x] & 0x7];
    screen_vram[xd] = color;
  }
}

static inline void
loc_RenderGraphicModeNormal_gu(int xoffset)
{
  SDL_Rect srcRect;
  SDL_Rect dstRect;

  srcRect.x = 0;
  srcRect.y = 0;
  srcRect.w = 400;
  srcRect.h = 255;
  dstRect.x = 40;
  dstRect.y = 0;
  dstRect.w = 400;
  dstRect.h = 255;

  psp_sdl_gu_stretch(&srcRect, &dstRect);
}

static inline void
loc_RenderTeletextModeX125_gu()
{
  SDL_Rect srcRect;
  SDL_Rect dstRect;

  srcRect.x = 0;
  srcRect.y = 0;
  srcRect.w = 400;
  srcRect.h = 255;
  dstRect.x = 0;
  dstRect.y = 0;
  dstRect.w = 480;
  dstRect.h = 270;

  psp_sdl_gu_stretch(&srcRect, &dstRect);
}

static inline void
loc_RenderGraphicModeX125_gu(int xoffset)
{
  SDL_Rect srcRect;
  SDL_Rect dstRect;

  srcRect.x = 0;
  srcRect.y = 0;
  srcRect.w = 400 - (xoffset/2);
  srcRect.h = 255;
  dstRect.x = 0;
  dstRect.y = 0;
  dstRect.w = 480;
  dstRect.h = 270;

  psp_sdl_gu_stretch(&srcRect, &dstRect);
}

static inline void
loc_RenderTeletextModeX15_gu()
{
  SDL_Rect srcRect;
  SDL_Rect dstRect;

  srcRect.x = 4;
  srcRect.y = 0;
  srcRect.w = 240;
  srcRect.h = 255;
  dstRect.x = 0;
  dstRect.y = 0;
  dstRect.w = 480;
  dstRect.h = 270;

  psp_sdl_gu_stretch(&srcRect, &dstRect);
}


static inline void
loc_RenderGraphicModeX15_gu(int xoffset)
{
  SDL_Rect srcRect;
  SDL_Rect dstRect;

  srcRect.x = 7 + (xoffset/2); ///????
  srcRect.y = 0;
  srcRect.w = 240;
  srcRect.h = 255;
  dstRect.x = 0;
  dstRect.y = 0;
  dstRect.w = 480;
  dstRect.h = 270;

  psp_sdl_gu_stretch(&srcRect, &dstRect);
}


static inline void
loc_RenderTeletextModeWidthX2_gu()
{
  SDL_Rect srcRect;
  SDL_Rect dstRect;

  srcRect.x = 0;
  srcRect.y = 0;
  srcRect.w = 288;
  srcRect.h = 135;
  dstRect.x = 0;
  dstRect.y = 0;
  dstRect.w = 480;
  dstRect.h = 270;

  psp_sdl_gu_stretch(&srcRect, &dstRect);
}


static inline void
loc_RenderGraphicModeWidthX2_gu(int xoffset)
{
  SDL_Rect srcRect;
  SDL_Rect dstRect;

  srcRect.x = 7 + (xoffset/2); ///????
  srcRect.y = 0;
  srcRect.w = 240;
  srcRect.h = 255;
  dstRect.x = 0;
  dstRect.y = 0;
  dstRect.w = 480;
  dstRect.h = 270;

  psp_sdl_gu_stretch(&srcRect, &dstRect);
}

static inline void
loc_RenderTeletextModeX2_gu()
{
  SDL_Rect srcRect;
  SDL_Rect dstRect;

  srcRect.x = 18;
  srcRect.y = 0;
  srcRect.w = 288;
  srcRect.h = 68;
  dstRect.x = 0;
  dstRect.y = 0;
  dstRect.w = 480;
  dstRect.h = 270;

  psp_sdl_gu_stretch(&srcRect, &dstRect);
}

static inline void
loc_RenderGraphicModeX2_gu(int xoffset)
{
  SDL_Rect srcRect;
  SDL_Rect dstRect;

  srcRect.x = 15 + (xoffset/2); ///????
  srcRect.y = 32;
  srcRect.w = 240;
  srcRect.h = 136;
  dstRect.x = 0;
  dstRect.y = 0;
  dstRect.w = 480;
  dstRect.h = 270;

  psp_sdl_gu_stretch(&srcRect, &dstRect);
}

//-> [I005]: Castle Quests title text disappers off the bottom of the screen
//       I'm missing some offset somewhere..
void 
RenderLine(int line, int isTeletext, int xoffset)
{
  static int last_isTeletext = 1, last_xoffset = 0;

  // *** this could really be nasty with split gfx res stuff.  Fuck it..
  if (last_isTeletext != isTeletext || last_xoffset != xoffset){
    psp_sdl_clear_blit(0);
    last_isTeletext = isTeletext;
    last_xoffset = xoffset;
  }

  // Make sure we're trying to draw within a sane part of the bitmap
  if (video_output != NULL && back_surface != NULL && line>=0 && line<512){
    if (isTeletext) {
      loc_RenderTeletextModeNormal_blit(line);
    } else {
      loc_RenderGraphicModeNormal_blit(line, xoffset);
    }
  }
}

void
RenderScreen(int isTeletext, int xoffset)
{
  // Render a teletext line
  if (isTeletext) {
    if (mainWin->m_render_mode == BEEB_RENDER_NORMAL  ) loc_RenderTeletextModeNormal_gu();
    else 
    if (mainWin->m_render_mode == BEEB_RENDER_WIDTH_X2) loc_RenderTeletextModeWidthX2_gu();
    else
    if (mainWin->m_render_mode == BEEB_RENDER_X2      ) loc_RenderTeletextModeX2_gu();
    else
    if (mainWin->m_render_mode == BEEB_RENDER_X15     ) loc_RenderTeletextModeX15_gu();
    else
    if (mainWin->m_render_mode == BEEB_RENDER_X125    ) loc_RenderTeletextModeX125_gu();
  }
  // render a graphics mode line.
  else 
  {
    if (mainWin->m_render_mode == BEEB_RENDER_NORMAL  ) loc_RenderGraphicModeNormal_gu( xoffset);
    else 
    if (mainWin->m_render_mode == BEEB_RENDER_WIDTH_X2) loc_RenderGraphicModeWidthX2_gu( xoffset);
    else
    if (mainWin->m_render_mode == BEEB_RENDER_X2      ) loc_RenderGraphicModeX2_gu( xoffset);
    else
    if (mainWin->m_render_mode == BEEB_RENDER_X15     ) loc_RenderGraphicModeX15_gu( xoffset);
    else
    if (mainWin->m_render_mode == BEEB_RENDER_X125    ) loc_RenderGraphicModeX125_gu( xoffset);
  }
}

unsigned char* 
GetSDLScreenLinePtr(int line)
{
# if 0 //LUDO:
  if ((video_output == NULL) || (line < 0) || (line >= BEEBEM_VIDEO_CORE_SCREEN_HEIGHT)) return NULL;
# endif
  return  (unsigned char *)video_output->pixels + line * video_output->pitch;
}


/* Initialize GUI. Must be called before any other EG function call.
 */
EG_BOOL 
EG_Initialize(void)
{  
//  /* Initialize event handler:
//   */
//  if (EG_Event_InitializeQueue() == EG_FALSE)
//    return(EG_FALSE);
//
//  /* Load label fonts:
//   */

  EG_Draw_FlushEventQueue();

  if (EG_DrawString_Initialise() == EG_FALSE)
    return(EG_FALSE);


  return(EG_TRUE);
}

void 
EG_Quit(void)
{
//  EG_Event_FreeQueue();
  EG_DrawString_Free();
}


/* Rendering scale (1 = 100%, 0.5 = 50%)
 */
void 
EG_Draw_SetToLowResolution(void)
{
  scale=0.5;
}

void 
EG_Draw_SetToHighResolution(void)
{
  scale=1.0;
}

/* Get scale (divide coordinates by this value to calculate the actual size
 * of anything physically drawn on the surface).
 */
float 
EG_Draw_GetScale(void)
{
  return(scale);
}


Uint32 EG_Draw_GetCurrentTime(void)
{
  return(SDL_GetTicks());
}

Uint32 
EG_Draw_GetTimePassed(Uint32 time_start)
{
  Uint32 time_end = SDL_GetTicks();

        if (time_end < time_start)
                return(time_end + ( ((Uint32) 0xffffffff) - time_start) );
        else
                return(time_end - time_start);
}

void 
EG_Draw_FlushEventQueue(void)
{
  SDL_Event event;
  while ( SDL_PollEvent(&event) ){}
}


/* Wrapper for SDL surface update
 *
 * Should call a user callback so program knows the GUI has updated something
 */
void 
EG_Draw_UpdateSurface(SDL_Surface *surface, Sint32 x, Sint32 y, Sint32 w , Sint32 h)
{
  SDL_UpdateRect(surface, x, y, w, h);
}



/* Text functions */
static SDL_Surface *label_low = NULL, *label_high = NULL;

static EG_BOOL 
EG_DrawString_Initialise(void)
{       char largefont[1024];
        char smallfont[1024];

        char cpath[512];
        getcwd(cpath,sizeof(cpath));

        strcpy(largefont, cpath);
        if (cpath[strlen(cpath)] != '/')
                        strcat(largefont, "/");

        strcpy(smallfont, largefont);

        strcat(largefont, "resources/font10x16.bmp");
        strcat(smallfont, "resources/font5x8.bmp");

        label_low = SDL_LoadBMP(smallfont);
        label_high = SDL_LoadBMP(largefont);

        if (label_low == NULL || label_high == NULL){
                printf("Failed to load font..\n");
                return(EG_FALSE);
        }

        // Set color key
  SDL_SetColorKey(label_low, SDL_SRCCOLORKEY
   , SDL_MapRGB(label_low->format, 255,255,255));
  SDL_SetColorKey(label_high, SDL_SRCCOLORKEY
   , SDL_MapRGB(label_high->format, 255,255,255));

  return(EG_TRUE);
}

static void 
EG_DrawString_Free(void)
{
        SDL_FreeSurface(label_low);
        SDL_FreeSurface(label_high);
}

// 0 = center, -1 = left. 1 = right
void 
EG_Draw_String(SDL_Surface *surface, SDL_Color *color, EG_BOOL bold, 
               SDL_Rect *area_ptr, int justify, char *string)
{
  SDL_Rect drawing_area; // SDL_DW_Draw_CalcDrawingArea(surface, &area);

        int i, w, h, x, y, len;

  drawing_area = EG_Draw_CalcDrawingArea(surface, area_ptr);

  w = (int) (10 * EG_Draw_GetScale() );
  h = (int) (16 * EG_Draw_GetScale() );

  drawing_area.x = (int) (drawing_area.x * EG_Draw_GetScale() );
  drawing_area.y = (int) (drawing_area.y * EG_Draw_GetScale() );
  drawing_area.w = (int) (drawing_area.w * EG_Draw_GetScale() );
  drawing_area.h = (int) (drawing_area.h * EG_Draw_GetScale() );

        // Calc. number of letters we can actually draw.
        len = strlen(string);

        //SDL_FillRect(surface, &drawing_area, SDL_MapRGB(surface->format, 255
        // , 255, 255));

        x = drawing_area.x;
        y = drawing_area.y + ((drawing_area.h - h)/2);

        // Don't bother rendering anything if area width is less than one
  // character
        if ( w > drawing_area.w)
                return;

        // We will need to clip it:
        if ( (len * w) > drawing_area.w){
                len = (drawing_area.w / w) -1;
                x += (drawing_area.w % w) /2;

                for(i=0; i<len;i++){
                        EG_Draw_Char(surface, color, bold, x, y, string[i]);
                                x += w;
                }
                EG_Draw_Char(surface, color, bold, x, y, (char) 8);

        }else{
        // We don't need to clip it. But may need to center it:
                if (justify == 0)
                        x += (drawing_area.w - len * w) /2;
                else if (justify == 1)
                        x += (drawing_area.w - len * w);

                for(i=0; i<len;i++){
                        EG_Draw_Char(surface, color, bold, x, y, string[i]);
                                x += w;
                }
        }
        EG_Draw_UpdateSurface(surface, drawing_area.x, drawing_area.y
   , drawing_area.w, drawing_area.h);
}

void 
EG_Draw_Char(SDL_Surface *surface, SDL_Color *color, EG_BOOL bold, Uint16 x, Uint16 y , char c)
{
        SDL_Rect src, dst;
        SDL_Surface *source_surface;

  SDL_Color *tmp;    // Dump compiler warning
  tmp = color;

  src.x = c & 15;
  src.y = ((unsigned char) c) >> 4;

        if (EG_Draw_GetScale() >= 1){
                source_surface = label_high;
                src.w = 10; src.h = 16;
        }else{
                source_surface = label_low;
                src.w = 5; src.h = 8;
        }
        src.x *= src.w; src.y *= src.h;


        dst.x = x; dst.y = y;

        SDL_BlitSurface(source_surface, &src, surface, &dst);
  if (bold == EG_TRUE){
    dst.x++;
    SDL_BlitSurface(source_surface, &src, surface, &dst);
  }
}

/* If surface == NULL, returns an SDL_Rect of 0,0,0,0.  If update == NULL,
 * returns whole area of surface.  Otherwise returns update.
 */
SDL_Rect 
EG_Draw_CalcDrawingArea(SDL_Surface *surface, SDL_Rect *update)
{
        SDL_Rect area = {0,0,0,0};

        if (surface == NULL)
                return(area);

        if (update == NULL){
                area.w = surface->w;
                area.h = surface->h;
                area.x = 0;
                area.y = 0;
        }else{
                area = *(update);
        }

        return(area);
}

void 
EG_Draw_Box(SDL_Surface *surface, SDL_Rect *area, SDL_Color *color)
{       
  SDL_Rect drawing_area = EG_Draw_CalcDrawingArea(surface, area);
      
        if (surface == NULL)
                return;
        
  drawing_area.x = (int) (drawing_area.x * EG_Draw_GetScale() );
  drawing_area.y = (int) (drawing_area.y * EG_Draw_GetScale() );
  drawing_area.w = (int) (drawing_area.w * EG_Draw_GetScale() );
  drawing_area.h = (int) (drawing_area.h * EG_Draw_GetScale() );

        SDL_FillRect(surface, &drawing_area, SDL_MapRGB(surface->format
   , color->r, color->g, color->b));

  EG_Draw_UpdateSurface(surface, drawing_area.x, drawing_area.y
   , drawing_area.w, drawing_area.h);
}


void 
EG_Draw_TabBorder(SDL_Surface *surface, SDL_Rect *area, SDL_Color *color , int type)
{
  SDL_Rect drawing_area = EG_Draw_CalcDrawingArea(surface, area);
        SDL_Rect line = {0,0,0,0};
        Uint32 bright_col = 0, dull_col = 0;

        if (surface == NULL)
                return;

        drawing_area.x = (int) (drawing_area.x * EG_Draw_GetScale() );
        drawing_area.y = (int) (drawing_area.y * EG_Draw_GetScale() );
        drawing_area.w = (int) (drawing_area.w * EG_Draw_GetScale() );
        drawing_area.h = (int) (drawing_area.h * EG_Draw_GetScale() );

  switch (type){

    case EG_Draw_Border_Normal:
      bright_col = SDL_MapRGB(surface->format
       , (Uint8) color->r
       , (Uint8) color->g
       , (Uint8) color->b );
      
      dull_col = bright_col;  
    break;

    case EG_Draw_Border_BoxHigh:
      bright_col = SDL_MapRGB(surface->format
       , (int) (1.3333*color->r >255.0 ? 255.0 : 1.3333*color->r)
       , (int) (1.3333*color->g >255.0 ? 255.0 : 1.3333*color->g)
       , (int) (1.3333*color->b >255.0 ? 255.0 : 1.3333*color->b) );
  
      dull_col = SDL_MapRGB(surface->format
       , (int) (color->r * 0.6666)
       , (int) (color->g * 0.6666)
       , (int) (color->b * 0.6666) );
    break;

    case EG_Draw_Border_BoxLow:
      dull_col = SDL_MapRGB(surface->format
       , (int) (1.3333*color->r >255.0 ? 255.0 : 1.3333*color->r)
       , (int) (1.3333*color->g >255.0 ? 255.0 : 1.3333*color->g)
       , (int) (1.3333*color->b >255.0 ? 255.0 : 1.3333*color->b) );
  
      bright_col = SDL_MapRGB(surface->format
       , (int) (color->r * 0.6666)
       , (int) (color->g * 0.6666)
       , (int) (color->b * 0.6666) );
    break;

    case EG_Draw_Border_Focused:

                        dull_col = SDL_MapRGB(surface->format
                         , (int) (color->r * 0.4)
                         , (int) (color->g * 0.4)
                         , (int) (color->b * 0.4) );

      bright_col = dull_col;
    break;

  }

        // Top line:
        line.x = drawing_area.x  +1  ; line.y = drawing_area.y; 
  line.w = drawing_area.w  -2  ; line.h = 1;
        SDL_FillRect(surface, &line, bright_col);

        // Bottom line:
        line.x = drawing_area.x  +1  ; line.y = drawing_area.y + drawing_area.h-1;
  line.w = drawing_area.w  -2  ; 
  line.h = 1;
 //       SDL_FillRect(surface, &line, dull_col);

        // Left line:
        line.x=drawing_area.x; line.y=drawing_area.y  +1  ; line.h=drawing_area.h  -2;
  line.w = 1;
    line.h+=1;
        SDL_FillRect(surface, &line, bright_col);
  line.h-=1;

        // Right line:
        line.x=drawing_area.x+drawing_area.w-1; line.y=drawing_area.y  +1  ;
  line.w=1; line.h=drawing_area.h  -2  ;

//  line.h++;
        SDL_FillRect(surface, &line, dull_col);
//  line.h--;

  // Update: (if I was smart, I'd only draw the actual lines..)
        EG_Draw_UpdateSurface(surface, drawing_area.x, drawing_area.y
         , drawing_area.w, drawing_area.h+2);
}




void 
EG_Draw_Border(SDL_Surface *surface, SDL_Rect *area, SDL_Color *color , int type)
{
  SDL_Rect drawing_area = EG_Draw_CalcDrawingArea(surface, area);
        SDL_Rect line = {0,0,0,0};
        Uint32 bright_col = 0, dull_col = 0;

        if (surface == NULL)
                return;

        drawing_area.x = (int) ( drawing_area.x * EG_Draw_GetScale() );
        drawing_area.y = (int) ( drawing_area.y * EG_Draw_GetScale() );
        drawing_area.w = (int) ( drawing_area.w * EG_Draw_GetScale() );
        drawing_area.h = (int) ( drawing_area.h * EG_Draw_GetScale() );

  switch (type){

    case EG_Draw_Border_Normal:
      bright_col = SDL_MapRGB(surface->format
       , (Uint8) color->r
       , (Uint8) color->g
       , (Uint8) color->b );
      
      dull_col = bright_col;  
    break;

    case EG_Draw_Border_BoxHigh:
      bright_col = SDL_MapRGB(surface->format
       , (int) (1.3333*color->r >255.0 ? 255.0 : 1.3333*color->r)
       , (int) (1.3333*color->g >255.0 ? 255.0 : 1.3333*color->g)
       , (int) (1.3333*color->b >255.0 ? 255.0 : 1.3333*color->b) );
  
      dull_col = SDL_MapRGB(surface->format
       , (int) (color->r * 0.6666)
       , (int) (color->g * 0.6666)
       , (int) (color->b * 0.6666) );
    break;

    case EG_Draw_Border_BoxLow:
      dull_col = SDL_MapRGB(surface->format
       , (int) (1.3333*color->r >255.0 ? 255.0 : 1.3333*color->r)
       , (int) (1.3333*color->g >255.0 ? 255.0 : 1.3333*color->g)
       , (int) (1.3333*color->b >255.0 ? 255.0 : 1.3333*color->b) );
  
      bright_col = SDL_MapRGB(surface->format
       , (int) (color->r * 0.6666)
       , (int) (color->g * 0.6666)
       , (int) (color->b * 0.6666) );
    break;

    case EG_Draw_Border_Focused:
                        dull_col = SDL_MapRGB(surface->format
                         , (int) (color->r * 0.7)
                         , (int) (color->g * 0.7)
                         , (int) (color->b * 0.7) );

                        bright_col = dull_col;
    break;

  }

  // Top line:
  line.x = drawing_area.x  +1  ; line.y = drawing_area.y; 
  line.w = drawing_area.w  -2  ; line.h = 1;
  SDL_FillRect(surface, &line, bright_col);

  // Bottom line:
  line.x = drawing_area.x  +1  ; line.y = drawing_area.y + drawing_area.h-1;
  line.w = drawing_area.w  -2  ; 
  line.h = 1;
  SDL_FillRect(surface, &line, dull_col);

  // Left line:
  line.x=drawing_area.x; line.y=drawing_area.y  +1  ; line.h=drawing_area.h  -2  ;
  line.w = 1;
  SDL_FillRect(surface, &line, bright_col);

        // Right line:
        line.x=drawing_area.x+drawing_area.w-1; line.y=drawing_area.y  +1  ;
  line.w=1; line.h=drawing_area.h  -2  ;
  SDL_FillRect(surface, &line, dull_col);

  // Update: (if I was smart, I'd only draw the actual lines..)
  EG_Draw_UpdateSurface(surface, drawing_area.x, drawing_area.y
         , drawing_area.w, drawing_area.h);
}

Uint32 
EG_Draw_CalcTimePassed(Uint32 starttime, Uint32 endtime)
{
        if (starttime > endtime)
                return(0xffffffff - starttime + endtime);
        else
                return(endtime - starttime);
}

