/****************************************************************************/
/*                               Beebem                                     */
/*                               ------                                     */
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
/****************************************************************************/
/* Mike Wyatt and NRM's port to win32 - 7/6/97 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <SDL.h>

#include <pspctrl.h>
#include <pspdebug.h>

#include <iostream>
#include <sys/stat.h>
#include "include/windows.h"

#include "include/6502core.h"
#include "include/beebmem.h"
#include "include/beebsound.h"
#include "include/sysvia.h"
#include "include/uservia.h"
#include "include/beebwin.h"
#include "include/disc8271.h"
#include "include/video.h"
#include "include/via.h"
#include "include/atodconv.h"
#include "include/disc1770.h"
#include "include/serial.h"
#include "include/tube.h"
#include "include/econet.h"	//Rob

#include "include/line.h"	// SDL Stuff
#include "include/log.h"
#include "include/sdl.h"

#include "include/gui.h"

#include "include/beebem_pages.h"

#include "beebwinc.h"
#include "psp_sdl.h"
#include "psp_kbd.h"
#include "psp_run.h"
#include "global.h"

#ifdef MULTITHREAD
#undef MULTITHREAD
#endif

/* Make global reference to command line args
 */
int __argc = 0;
char **__argv = NULL;

char chAppPath[MAX_PATH];

extern VIAState SysVIAState;
int DumpAfterEach=0;

unsigned char MachineType;
BeebWin *mainWin = NULL;

FILE *tlog;

int done = 0;
int fullscreen = 0;
int showing_menu = 0;
EG_Window *displayed_window_ptr = NULL;


void SetActiveWindow(EG_Window *window_ptr)
{
	displayed_window_ptr = window_ptr;
}

int GetFullscreenState(void)
{
	return(fullscreen);
}

int 
ToggleFullscreen(void)
{
	if (fullscreen != 0)
		fullscreen = 0;
	else
		fullscreen = 1;

# if 0 //LUDO:
	if (SDL_WM_ToggleFullScreen(screen_ptr) != 1)
		EG_Log(EG_LOG_WARNING, dL"Could not toggle full-screen mode.", dR);
# endif

	return(fullscreen);
}

void ShowingMenu(void)
{
	showing_menu = 1;
}

void NoMenuShown(void)
{
	showing_menu = 0;
}

void Quit(void)
{
	done=1;
}

void
beeb_main_loop()
{
	/* Main loop converted to SDL: */
  while (1) {
	  Exec6502Instruction();
    psp_update_keys();
	}
}

int 
SDL_main(int argc, char *argv[])
{
	/* Create global reference to command line args (like windows does)
	 */
	__argc = argc;
	__argv = (char**) argv;

  getcwd(chAppPath, sizeof(chAppPath)-1); // get the location of the executable

	/* Initialise debugging subsystem.
	 */
	DEBUG_INIT();

	mainWin=new BeebWin();

	/* Initialize SDL resources.
	 */
	if (! InitialiseSDL(argc, argv)){
		pFATAL("Unable to initialise SDL library!");
		exit(1);
	}

 /* Initialize GUI API
  */
	if (EG_Initialize() == EG_TRUE){
		//LUDO: printf("EG initialized\n");
	}else{
		printf("EG FAILED TO INITIALIZE\n");
    SDL_Quit();
		exit(1);
	}

	mainWin->Initialise();

  psp_sdl_black_screen();

  beeb_default_settings();

  beeb_update_save_name("");

  beeb_load_settings();

  psp_run_load_file();

  beeb_main_loop();

	delete mainWin;

	UninitialiseSDL();

	DEBUG_FREE();

  SDL_Quit();
  exit(0);

	return(0);  
}
