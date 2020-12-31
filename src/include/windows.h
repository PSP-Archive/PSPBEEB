/* Fake windows stuff in here.
 */

#ifndef _FAKE_MS_WINDOWS_H_
#define _FAKE_MS_WINDOWS_H_

#if HAVE_CONFIG_H
#	include <config.h>
#endif

#include <SDL.h>
#include "include/log.h"
#include "include/sdl.h"

#include "include/beebem_pages.h"

/* NOTE: We cannot define WIN32 and fake our way through the BeebEm code..
 *       This would break the SDL library - as if it's header was ever included
 *       after WIN32 being defined, then we get the Windows header instead!
 *
 *       So we assume this version is NOT the windows version for now
 *       (no #ifdef WIN32 defined).  This assumption may course problemse
 */


#define _MAX_PATH 1024


/* Command line args (variables in main.cpp)
 */
extern int __argc;
extern char **__argv;

#define stricmp strcasecmp

typedef Uint64 __int64;

typedef struct{
	Uint16 lowpart;
	Uint16 highpart;
}LARGE_INTEGER;

typedef Uint8* PBYTE;


// this is its correct value.
#define MF_BYCOMMAND 0x00



#define MOVEFILE_REPLACE_EXISTING 	0x00000001
#define MOVEFILE_COPY_ALLOWED       	0x00000002


// cregistry.h
typedef int HKEY;
typedef int LPSTR;

// Not sure what this is, but its not a pointer.
typedef int PTR;

typedef void* PVOID;
typedef char* LPCTSTR;

typedef Uint32 DWORD;
typedef unsigned int UINT;

typedef Uint32 COLORREF;

// Menus
#define MF_CHECKED 		1
#define MF_UNCHECKED 		0
#define	MF_ENABLED		0x0000	
#define MF_GRAYED		1

#ifndef BOOL
#	define BOOL Uint8
#	define FALSE false
#	define TRUE true
#endif


// beebwin
typedef int HMENU;
typedef int HDC;
typedef int HWND;
typedef int JOYCAPS;
typedef int HGDIOBJ;
typedef int bmiData;
typedef int HBITMAP;
typedef int LPDIRECTDRAW;
typedef int LPDIRECTDRAW2;
typedef int LPDIRECTDRAWSURFACE;
typedef int LPDIRECTDRAWSURFACE2;
typedef int HRESULT;
typedef int LPDIRECTDRAWCLIPPER;



// beebsound
typedef int LPDIRECTSOUND;
typedef int LPDIRECTSOUNDBUFFER;

// serial
typedef int HINSTANCE;

// --- Windows message box:

// buttons
#define MB_OKCANCEL 0
#define MB_OK 0
#define MB_YESNO 4

#define IDOK 1
#define IDCANCEL 2
#define IDYES 6


// icons
#define MB_ICONERROR 0
#define MB_ICONWARNING 0x30
#define MB_ICONQUESTION 32

// ----------


int 	MessageBox(HWND hwnd, const char *message_p, const char *title_p, int type);
void 	SetWindowText(HWND hwnd, const char *title_p);
void 	Sleep(Uint32 ticks);
DWORD 	GetTickCount(void);
DWORD	CheckMenuItem(HMENU hmenu, UINT uIDCheckItem, UINT uCheck);
BOOL	ModifyMenu(HMENU hMnu, UINT uPosition, UINT uFlags, PTR uIDNewItem, LPCTSTR lpNewItem);
BOOL	MoveFileEx(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, DWORD dwFlags);
BOOL	EnableMenuItem(HMENU hMenu,UINT uIDEnableItem,UINT uEnable);




#endif
