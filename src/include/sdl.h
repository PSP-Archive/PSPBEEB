#ifndef SDL_HEADER
#define SDL_HEADER

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <SDL.h>

#include <stdlib.h>
#include "sdl.h"
#include "include/types.h"


#define BEEBEM_VIDEO_CORE_SCREEN_WIDTH		800
#define BEEBEM_VIDEO_CORE_SCREEN_HEIGHT		600
# if 0
#define SDL_WINDOW_WIDTH			640
#define SDL_WINDOW_HEIGHT			512
# else
#define SDL_WINDOW_WIDTH			480
#define SDL_WINDOW_HEIGHT			272
# endif

#define LOCK(s)   {if(SDL_MUSTLOCK(s))(void)SDL_LockSurface(s);}
#define UNLOCK(s) {if(SDL_MUSTLOCK(s))(void)SDL_UnlockSurface(s);}


extern "C" SDL_Surface *video_output;
extern SDL_Surface *screen_ptr;

extern "C" bool InitialiseSDL(int argc, char *argv[]);
extern "C" void UninitialiseSDL(void);
extern "C" void RenderLine(int, int, int);
extern "C" void RenderScreen(int, int);
extern "C" void SaferSleep(unsigned int);
extern "C" unsigned char* GetSDLScreenLinePtr(int);
extern "C" int ConvertSDLKeyToBBCKey(SDL_keysym, int*, int*, int*);
extern "C" void SetBeebEmEmulatorCoresPalette(unsigned int *);

extern "C" void AddBytesToSDLSoundBuffer(void*, int);




/* Delta time to realtime sync:
 */

#define MINIMUMDELAY            10      // Min. amount of time OS will can sleep
                                        // in milliseconds.

#define WAIT_IS_NICE		0	// Will free-up max amount of time to OS
#define WAIT_IS_OPTIMISED	1	// Will free-up min amount of time to OS
#define WAIT_IS_NASTY 		2	// Will free-up NO time to the OS..



/* If you mess with the above, you may
 * get better performance.  Also I'm worried that some PDA's and other systems
 * may have a problem with sleeping for n milliseconds etc, and of course the
 * first thing bad timing will effect is the sound..
 */

#define FRAMESPERSECOND 	50	// Number of times per second 'screen'
					// will update.



/* Prototypes
 */

#define LOCK(s)   {if(SDL_MUSTLOCK(s))(void)SDL_LockSurface(s);}
#define UNLOCK(s) {if(SDL_MUSTLOCK(s))(void)SDL_UnlockSurface(s);}


EG_BOOL EG_Initialize(void);
void EG_Quit(void);

Uint32 EG_Draw_GetCurrentTime(void);
Uint32 EG_Draw_GetTimePassed(Uint32 time_start);

void EG_Draw_FlushEventQueue(void);

float EG_Draw_GetScale(void);
void EG_Draw_SetToLowResolution(void);
void EG_Draw_SetToHighResolution(void);
void EG_Draw_UpdateSurface(SDL_Surface *surface, Sint32 x, Sint32 y, Sint32 w
 , Sint32 h);

void EG_Draw_Char(SDL_Surface *surface, SDL_Color *color, EG_BOOL bold, Uint16 x, Uint16 y
 , char c);

void EG_Draw_String(SDL_Surface *surface, SDL_Color *colour, EG_BOOL bold, SDL_Rect *area_ptr
 , int justify, char* string);

#define EG_Draw_Border_Normal	0
#define EG_Draw_Border_BoxHigh 	1
#define EG_Draw_Border_BoxLow 	2
#define EG_Draw_Border_Focused	3
void EG_Draw_Border(SDL_Surface *surface, SDL_Rect *area, SDL_Color *color
 , int type);

void EG_Draw_TabBorder(SDL_Surface *surface, SDL_Rect *area, SDL_Color *color
 , int type);

SDL_Rect EG_Draw_CalcDrawingArea(SDL_Surface *surface, SDL_Rect *update);
void EG_Draw_Box(SDL_Surface *surface, SDL_Rect *area, SDL_Color *color);
Uint32 EG_Draw_CalcTimePassed(Uint32 starttime, Uint32 endtime);



//static EG_BOOL EG_DrawString_Initialise(void);
//static void EG_DrawString_Free(void);


#endif
