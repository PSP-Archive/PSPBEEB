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
/* Conveted to use DirectX - Mike Wyatt 11/1/98 */
// Econet added Rob O'Donnell. robert@irrelevant.com. 28/12/2004.

#if HAVE_CONFIG_H
#	include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/stat.h>
#include <psppower.h>
#include <pspdisplay.h>
#include "global.h"
#include "include/windows.h"
//-- #include <initguid.h>
#include "include/main.h"
#include "include/beebwin.h"
#include "include/port.h"
#include "include/6502core.h"
#include "include/disc8271.h"
#include "include/disc1770.h"
#include "include/sysvia.h"
#include "include/uservia.h"
#include "include/video.h"
#include "include/beebsound.h"
#include "include/beebmem.h"
#include "include/beebemrc.h"
#include "include/atodconv.h"
#include "include/userkybd.h"
#include "include/cregistry.h" // David Overton's registry access code.
#include "include/serial.h"
#include "include/econet.h"	 // Rob O'Donnell Christmas 2004.
#include "include/tube.h"
#include "include/ext1770.h"
#include "include/uefstate.h"
#include "include/debug.h"
#include "include/scsi.h"
#include "include/sasi.h"
#include "include/z80mem.h"
#include "include/z80.h"
#include "include/userkybd.h"
#include "include/speech.h"
#include "include/avi.h"

#include "include/beebem_pages.h"

#include "psp_danzeff.h"
#include "psp_sdl.h"
#include "psp_kbd.h"
#include "psp_fmgr.h"
#include "beebwinc.h"
#include <zlib.h>

// some LED based macros

#define LED_COLOUR_TYPE (LEDByte&4)>>2
#define LED_SHOW_KB (LEDByte&1)
#define LED_SHOW_DISC (LEDByte&2)>>1
#define LED_COL_BASE 64

//LUDO:
int psp_screenshot_mode = 0;

// Registry access stuff
cRegistry SysReg;
bool RegRes;
int rbs=1;
int *binsize=&rbs;
// End of registry access stuff

FILE *CMDF2;
unsigned char CMA2;

unsigned char HideMenuEnabled;
bool MenuOn;

struct LEDType LEDs;
char DiscLedColour=0; // 0 for red, 1 for green.

//-- AVIWriter *aviWriter = NULL;

// FDC Board extension DLL variables
//-- HMODULE hFDCBoard;

typedef void (*lGetBoardProperties)(struct DriveControlBlock *);
typedef unsigned char (*lSetDriveControl)(unsigned char);
typedef unsigned char (*lGetDriveControl)(unsigned char);


lGetBoardProperties PGetBoardProperties;
lSetDriveControl PSetDriveControl;
lGetDriveControl PGetDriveControl;

bool m_PageFlipping=0;

//-- EDCB ExtBoard={0,0,NULL};
bool DiscLoaded[2]={FALSE,FALSE}; // Set to TRUE when a disc image has been loaded.
char CDiscName[2][256]; // Filename of disc current in drive 0 and 1;
char CDiscType[2]; // Current disc types
bool MustEnableSound=FALSE; // Set to true if the directsound must be unmuted after menu loop exit.
char FDCDLL[256];

static const char *WindowTitle = "BeebEm - BBC Model B / Master 128 Emulator";
# if 0
static const char *AboutText = "BeebEm - Emulating:\n\nBBC Micro Model B\nBBC Micro Model B + IntegraB\n"
								"BBC Micro Model B Plus (128)\nAcorn Master 128\nAcorn 65C02 Second Processor\nTorch Z80 Second Processor\n\n"
								"Version 3.11, February 2006";

/* Configuration file strings */
static const char *CFG_FILE_NAME = "BeebEm.ini";
static char *CFG_REG_KEY = "Software\\BeebEm";

static char *CFG_VIEW_SECTION = "View";
static char *CFG_VIEW_WIN_SIZE = "WinSize";
static char *CFG_VIEW_SHOW_FPS = "ShowFSP";
static char *CFG_VIEW_DIRECT_ENABLED = "DirectDrawEnabled";
static char *CFG_VIEW_BUFFER_IN_VIDEO = "BufferInVideoRAM";
static char *CFG_VIEW_MONITOR = "Monitor";

static char *CFG_SOUND_SECTION = "Sound";
static char *CFG_SOUND_SAMPLE_RATE = "SampleRate";
static char *CFG_SOUND_VOLUME = "Volume";
static char *CFG_SOUND_ENABLED = "SoundEnabled";
static char *CFG_SOUND_DIRECT_ENABLED = "DirectSoundEnabled";

static char *CFG_OPTIONS_SECTION = "Options";
static char *CFG_OPTIONS_STICKS = "Sticks";
static char *CFG_OPTIONS_KEY_MAPPING = "KeyMapping";
static char *CFG_OPTIONS_USER_KEY_MAP = "UserKeyMap";
static char *CFG_OPTIONS_FREEZEINACTIVE = "FreezeWhenInactive";
static char *CFG_OPTIONS_HIDE_CURSOR = "HideCursor";

static char *CFG_SPEED_SECTION = "Speed";
static char *CFG_SPEED_TIMING = "Timing";

static char *CFG_AMX_SECTION = "AMX";
static char *CFG_AMX_ENABLED = "AMXMouseEnabled";
static char *CFG_AMX_LRFORMIDDLE = "AMXMouseLRForMiddle";
static char *CFG_AMX_SIZE = "AMXMouseSize";
static char *CFG_AMX_ADJUST = "AMXMouseAdjust";

static char *CFG_PRINTER_SECTION = "Printer";
static char *CFG_PRINTER_ENABLED = "PrinterEnabled";
static char *CFG_PRINTER_PORT = "PrinterPort";
static char *CFG_PRINTER_FILE = "PrinterFile";

static char *CFG_MODEL_SECTION = "Model";
static char *CFG_MACHINE_TYPE = "MachineType";
# endif

/* Prototypes */
//-- LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Row,Col - Physical mapping
static int transTable1[256][2]={
 { 0,0 }, { 0,0  }, { 0,0 }, { 0,0 },   // 0
 { 0,0 }, { 0,0  }, { 0,0 }, { 0,0 },   // 4
 { 5,9 }, { 6,0  }, { 0,0 }, { 0,0 },   // 8 [BS][TAB]..
 { 0,0 }, { 4,9  }, { 0,0 }, { 0,0 },   // 12 .RET..
 { 0,0 }, { 0,1  }, { 0,0 }, { -2,-2 }, // 16 .CTRL.BREAK
 { 4,0 }, { 0,0  }, { 0,0 }, { 0,0 },   // 20 CAPS...
 { 0,0 }, { 0,0  }, { 0,0 }, { 7,0 },   // 24 ...ESC
 { 0,0 }, { 0,0  }, { 0,0 }, { 0,0 },   // 28
 { 6,2 }, {-3,-3 }, { -3,-4}, {	6,9 },   // 32 SPACE..[End]      2nd is Page Up 3rd is Page Down
 { 0,0 }, { 1,9  }, { 3,9 }, {7,9},   // 36 .[Left][Up][Right]
 { 2,9 }, { 0,0  }, { 0,0 }, {0,0},   // 40 [Down]...
 { 0,0 }, { 0,0  }, { 5,9 }, {0,0},   // 44 ..[DEL].
 { 2,7 }, { 3,0  }, { 3,1 }, {1,1},   // 48 0123   
 { 1,2 }, { 1,3  }, { 3,4 }, {2,4},   // 52 4567
 { 1,5 }, { 2,6  }, { 0,0 }, {0,0},   // 56 89
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 60
 { 0,0 }, { 4,1  }, { 6,4 }, {5,2},   // 64.. ABC
 { 3,2 }, { 2,2  }, { 4,3 }, {5,3},   // 68  DEFG
 { 5,4 }, { 2,5  }, { 4,5 }, {4,6},   // 72  HIJK
 { 5,6 }, { 6,5  }, { 5,5 }, {3,6},   // 76  LMNO
 { 3,7 }, { 1,0  }, { 3,3 }, {5,1},   // 80  PQRS
 { 2,3 }, { 3,5  }, { 6,3 }, {2,1},   // 84  TUVW
 { 4,2 }, { 4,4  }, { 6,1 }, {0,0},   // 88  XYZ
 { 0,0 }, { 6,2  }, { 0,0 }, {0,0},   // 92  . SPACE ..
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 96
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 100
 { 0,0 }, { 0,0  }, { 0,0 }, {-4,0},  // 104 Keypad+
 { 0,0 }, { -4,1 }, { 0,0 }, {0,0},   // 108 Keypad-
 { 7,1 }, { 7,2  }, { 7,3 }, {1,4},   // 112 F1 F2 F3 F4
 { 7,4 }, { 7,5  }, { 1,6 }, {7,6},   // 116 F5 F6 F7 F8
 { 7,7 }, { 2,0  }, { 2,0 }, {-2,-2}, // 120 F9 F10 F11 F12
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 124 
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 128
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 132
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 136
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 140
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 144
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 148
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 152
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 156
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 160
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 164
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 168
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 172
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 176
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 180
 { 0,0 }, { 0,0  }, { 5,7 }, {1,8},   // 184  ..;=
 { 6,6 }, { 1,7  }, { 6,7 }, {6,8},   // 188  ,-./
 { 4,8 }, { 0,0  }, { 0,0 }, {0,0},   // 192  @ ...
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 196
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 200
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 204
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 208
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 212
 { 0,0 }, { 0,0  }, { 0,0 }, {3,8},   // 216 ...[
 { 7,8 }, { 5,8  }, { 2,8 }, {4,7},   // 220 \]#`
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 224
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 228
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0},   // 232
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0}, 
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0}, 
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0}, 
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0}, 
 { 0,0 }, { 0,0  }, { 0,0 }, {0,0}
};

/* Currently selected translation table */
static int (*transTable)[2] = transTable1;
long WindowPos[4];

/****************************************************************************/
BeebWin::BeebWin()
{   
}

/****************************************************************************/
void BeebWin::Initialise()
{
	m_LastStartY = 0;
	m_LastNLines = 256;
	m_LastTickCount = 0;
	m_KeyMapAS = 0;
	m_KeyMapFunc = 0;
	m_ShiftPressed = 0;
	m_ShiftBooted = false;
	for (int k = 0; k < 256; ++k)
	{
		m_vkeyPressed[k][0][0] = -1;
		m_vkeyPressed[k][1][0] = -1;
		m_vkeyPressed[k][0][1] = -1;
		m_vkeyPressed[k][1][1] = -1;
	}

	LoadPreferences();

	IgnoreIllegalInstructions = 1;

//--	aviWriter = NULL;

	m_WriteProtectDisc[0] = !IsDiscWritable(0);
	m_WriteProtectDisc[1] = !IsDiscWritable(1);
	UEFTapeName[0]=0;

	m_hBitmap = m_hOldObj = m_hDCBitmap = 0;
	m_screen = m_screen_blur = NULL;
	m_ScreenRefreshCount = 0;
	m_RelativeSpeed = 1;
	m_FramesPerSecond = 50;
	strcpy(m_szTitle, WindowTitle);

	m_AviDC = 0;
	m_AviDIB = 0;

//->
	for(int i=0;i<256;i++)
		cols[i] = i;
//++
	/* Setup SDL color palette
	 */
	// [TODO] Change this to SDL_SetupEmulatorPalette
	SetBeebEmEmulatorCoresPalette(cols);
//<-


	InitClass();
	CreateBeebWindow(); 
	CreateBitmap();

//--	m_hMenu = GetMenu(m_hWnd);

	InitMenu();

//--	m_hDC = GetDC(m_hWnd);

//--	if (m_DirectDrawEnabled)
//--		InitDirectX();

	/* Initialise printer */
	if (PrinterEnabled)
		PrinterEnable(m_PrinterDevice);
	else
		PrinterDisable();

	/* Joysticks can only be initialised after the window is created (needs hwnd) */
	if (m_MenuIdSticks == IDM_JOYSTICK)
		InitJoystick();

	/* Get the applications path - used for disc and state operations */
	// ... and ROMS! - Richard Gellman
//->	char app_path[_MAX_PATH];
//--	char app_drive[_MAX_DRIVE];
//--	char app_dir[_MAX_DIR];
//--	GetModuleFileName(NULL, app_path, _MAX_PATH);
//--	_splitpath(app_path, app_drive, app_dir, NULL, NULL);
//--	_makepath(m_AppPath, app_drive, app_dir, NULL, NULL);
//++
  getcwd(m_AppPath, sizeof(m_AppPath));
//-<

	m_frozen = FALSE;
//->	strcpy(RomPath, m_AppPath);
//--	strcpy(EconetCfgPath, m_AppPath);
//++
	strcpy(RomPath, m_AppPath);
	strcat(RomPath, "/roms");
  strcpy(EconetCfgPath, m_AppPath);
	strcat(EconetCfgPath, "/config");
//-<

	UpdateModelType();
	UpdateSFXMenu();
	UpdateLEDMenu(m_hMenu);
	MenuOn=TRUE;

//-> [TODO] [ISSUE] If not 1770 assumes DLL..  Fudged for now..
//--	LoadFDC(NULL, true);
//++
	NativeFDC = TRUE;
//<-

	SetTapeSpeedMenu();
	UpdateOptiMenu();

	SaveWindowPos();

	SoundReset();
	if (SoundDefault) SoundInit();

	if (SpeechDefault)
		tms5220_start();

	ResetBeebSystem(MachineType,TubeEnabled,1); 
}

/****************************************************************************/
BeebWin::~BeebWin()
{   
//--	if (aviWriter)
//--		delete aviWriter;

//--	if (m_DirectDrawEnabled)
//--	{
//--		ResetSurfaces();
//--		m_DD2->Release();
//--		m_DD->Release();
//--	}

	if (SoundEnabled)
		SoundReset();

//--	ReleaseDC(m_hWnd, m_hDC);

//--	if (m_hOldObj != NULL)
//--		SelectObject(m_hDCBitmap, m_hOldObj);
//--	if (m_hBitmap != NULL)
//--		DeleteObject(m_hBitmap);
//--	if (m_hDCBitmap != NULL)
//--		DeleteDC(m_hDCBitmap);
}

/****************************************************************************/
void BeebWin::SaveWindowPos(void)
{
//--	RECT r;
//--	GetWindowRect(GETHWND,&r);
//--	*binsize=sizeof(r);
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"WindowPos2",&r,binsize);
}
/****************************************************************************/
extern void InitializeSoundBuffer(void);

void BeebWin::ResetBeebSystem(unsigned char NewModelType,unsigned char TubeStatus,unsigned char LoadRoms) 
{
	SwitchOnCycles=0; // Reset delay
	SoundChipReset();
	SwitchOnSound();
	EnableTube=TubeStatus;
	MachineType=NewModelType;
	BeebMemInit(LoadRoms,m_ShiftBooted);
	Init6502core();
	if (EnableTube) Init65C02core();
    Enable_Z80 = 0;
    if (TorchTube)
    {
        R1Status = 0;
        ResetTube();
        init_z80();
        Enable_Z80 = 1;
    }
    SysVIAReset();
	UserVIAReset();
	VideoInit();
	Disc8271_reset();
	if (EconetEnabled) EconetReset();	//Rob:
	Reset1770();
	AtoDReset();
	SetRomMenu();
	FreeDiscImage(0);
	// Keep the disc images loaded
	FreeDiscImage(1);
	Close1770Disc(0);
	Close1770Disc(1);
	SCSIReset();
	SASIReset();
	if (MachineType==3) InvertTR00=FALSE;
	if (MachineType!=3) {
		LoadFDC(NULL, false);
	}
	if ((MachineType!=3) && (NativeFDC)) {
		// 8271 disc
		if ((DiscLoaded[0]) && (CDiscType[0]==0)) LoadSimpleDiscImage(CDiscName[0],0,0,80);
		if ((DiscLoaded[0]) && (CDiscType[0]==1)) LoadSimpleDSDiscImage(CDiscName[0],0,80);
		if ((DiscLoaded[1]) && (CDiscType[1]==0)) LoadSimpleDiscImage(CDiscName[1],1,0,80);
		if ((DiscLoaded[1]) && (CDiscType[1]==1)) LoadSimpleDSDiscImage(CDiscName[1],1,80);
	}
	if (((MachineType!=3) && (!NativeFDC)) || (MachineType==3)) {
		// 1770 Disc
		if (DiscLoaded[0]) Load1770DiscImage(CDiscName[0],0,CDiscType[0],m_hMenu);
		if (DiscLoaded[1]) Load1770DiscImage(CDiscName[1],1,CDiscType[1],m_hMenu);
	}
  InitializeSoundBuffer();
  BeebReleaseAllKeys();
}
/****************************************************************************/
void BeebWin::CreateBitmap()
{
//--	if (m_hBitmap != NULL)
//--		DeleteObject(m_hBitmap);
//--	if (m_hDCBitmap != NULL)
//--		DeleteDC(m_hDCBitmap);
//--	if (m_screen_blur != NULL)
//--		free(m_screen_blur);
//--
//--	m_hDCBitmap = CreateCompatibleDC(NULL);
//--
//--	m_bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
//--	m_bmi.bmiHeader.biWidth = 800;
//--	m_bmi.bmiHeader.biHeight = -512;
//--	m_bmi.bmiHeader.biPlanes = 1;
//--	m_bmi.bmiHeader.biBitCount = 8;
//--	m_bmi.bmiHeader.biXPelsPerMeter = 0;
//--	m_bmi.bmiHeader.biYPelsPerMeter = 0;
//--	m_bmi.bmiHeader.biCompression = BI_RGB;
//--	m_bmi.bmiHeader.biSizeImage = 800*512;
//--	m_bmi.bmiHeader.biClrUsed = 68;
//--	m_bmi.bmiHeader.biClrImportant = 68;
//--
//-- #ifdef USE_PALETTE
//--	__int16 *pInts = (__int16 *)&m_bmi.bmiColors[0];
//--    
//--	for(int i=0; i<12; i++)
//--		pInts[i] = i;
//--    
//--	m_hBitmap = CreateDIBSection(m_hDCBitmap, (BITMAPINFO *)&m_bmi, DIB_PAL_COLORS,
//--							(void**)&m_screen, NULL,0);
//-- #else
//--	for (int i = 0; i < 64; ++i)
//--	{
//--		float r,g,b;
//--		r = (float) (i & 1);
//--		g = (float) ((i & 2) >> 1);
//--		b = (float) ((i & 4) >> 2);
//--
//--		if (palette_type != RGB)
//--		{
//--			r = g = b = (float) (0.299 * r + 0.587 * g + 0.114 * b);
//--
//--			switch (palette_type)
//--			{
//--			case AMBER:
//--				r *= (float) 1.0;
//--				g *= (float) 0.8;
//--				b *= (float) 0.1;
//--				break;
//--			case GREEN:
//--				r *= (float) 0.2;
//--				g *= (float) 0.9;
//--				b *= (float) 0.1;
//--				break;
//--			}
//--		}
//--
//--		m_bmi.bmiColors[i].rgbRed   = (BYTE) (r * m_BlurIntensities[i >> 3] / 100.0 * 255);
//--		m_bmi.bmiColors[i].rgbGreen = (BYTE) (g * m_BlurIntensities[i >> 3] / 100.0 * 255);
//--		m_bmi.bmiColors[i].rgbBlue  = (BYTE) (b * m_BlurIntensities[i >> 3] / 100.0 * 255);
//--		m_bmi.bmiColors[i].rgbReserved = 0;
//--	}
//--
//--	// Red Leds - left is dark, right is lit.
//--	m_bmi.bmiColors[LED_COL_BASE].rgbRed=80;		m_bmi.bmiColors[LED_COL_BASE+1].rgbRed=255;
//--	m_bmi.bmiColors[LED_COL_BASE].rgbGreen=0;		m_bmi.bmiColors[LED_COL_BASE+1].rgbGreen=0;
//--	m_bmi.bmiColors[LED_COL_BASE].rgbBlue=0;		m_bmi.bmiColors[LED_COL_BASE+1].rgbBlue=0;
//--	m_bmi.bmiColors[LED_COL_BASE].rgbReserved=0;	m_bmi.bmiColors[LED_COL_BASE+1].rgbReserved=0;
//--	// Green Leds - left is dark, right is lit.
//--	m_bmi.bmiColors[LED_COL_BASE+2].rgbRed=0;		m_bmi.bmiColors[LED_COL_BASE+3].rgbRed=0;
//--	m_bmi.bmiColors[LED_COL_BASE+2].rgbGreen=80;	m_bmi.bmiColors[LED_COL_BASE+3].rgbGreen=255;
//--	m_bmi.bmiColors[LED_COL_BASE+2].rgbBlue=0;		m_bmi.bmiColors[LED_COL_BASE+3].rgbBlue=0;
//--	m_bmi.bmiColors[LED_COL_BASE+2].rgbReserved=0;	m_bmi.bmiColors[LED_COL_BASE+3].rgbReserved=0;
//--
//--	m_hBitmap = CreateDIBSection(m_hDCBitmap, (BITMAPINFO *)&m_bmi, DIB_RGB_COLORS,
//--							(void**)&m_screen, NULL,0);
//-- #endif
//--
//--	m_screen_blur = (char *)calloc(m_bmi.bmiHeader.biSizeImage,1);
//--
//--	m_hOldObj = SelectObject(m_hDCBitmap, m_hBitmap);
//--	if(m_hOldObj == NULL)
//--		MessageBox(m_hWnd,"Cannot select the screen bitmap\n"
//--					"Try running in a 256 colour mode",WindowTitle,MB_OK|MB_ICONERROR);
}

/****************************************************************************/
BOOL BeebWin::InitClass(void)
{
//--	WNDCLASS  wc;
//--
//--	// Fill in window class structure with parameters that describe the
//--	// main window.
//--
//--	wc.style		 = CS_HREDRAW | CS_VREDRAW;// Class style(s).
//--	wc.lpfnWndProc	 = (WNDPROC)WndProc;	   // Window Procedure
//--	wc.cbClsExtra	 = 0;					   // No per-class extra data.
//--	wc.cbWndExtra	 = 0;					   // No per-window extra data.
//--	wc.hInstance	 = hInst;				   // Owner of this class
//--	wc.hIcon		 = LoadIcon(hInst, MAKEINTRESOURCE(IDI_BEEBEM));
//--	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
//--	wc.hbrBackground = NULL; //(HBRUSH)(COLOR_WINDOW+1);// Default color
//--	wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU); // Menu from .RC
//--	wc.lpszClassName = "BEEBWIN"; //szAppName;				// Name to register as
//--
//--	// Register the window class and return success/failure code.
//--	return (RegisterClass(&wc));
//+>
	return(0);
//<-
}

/****************************************************************************/
void BeebWin::CreateBeebWindow(void)
{
//--	DWORD style;
//--	int x,y;
//--
//--	x = m_XWinPos;
//--	y = m_YWinPos;
//--	if (x == -1 || y == -1)
//--	{
//--		x = CW_USEDEFAULT;
//--		y = 0;
//--		m_XWinPos = 0;
//--		m_YWinPos = 0;
//--	}
//--
//--	if (!m_DirectDrawEnabled && m_isFullScreen)
//--	{
//--		RECT scrrect;
//--		SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&scrrect, 0);
//--		x = scrrect.left;
//--		y = scrrect.top;
//--	}
//--
//--	if (m_DirectDrawEnabled && m_isFullScreen)
//--	{
//--		style = WS_POPUP;
//--	}
//--	else
//--	{
//--		style = WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;
//--	}
//--
//--	m_hWnd = CreateWindow(
//--				"BEEBWIN",				// See RegisterClass() call.
//--				m_szTitle, 		// Text for window title bar.
//--				style,
//--				x, y,
//--				m_XWinSize + GetSystemMetrics(SM_CXFIXEDFRAME) * 2,
//--				m_YWinSize + GetSystemMetrics(SM_CYFIXEDFRAME) * 2
//--					+ GetSystemMetrics(SM_CYMENUSIZE)
//--					+ GetSystemMetrics(SM_CYCAPTION)
//--					+ 1,
//--				NULL,					// Overlapped windows have no parent.
//--				NULL,				 // Use the window class menu.
//--				hInst,			 // This instance owns this window.
//--				NULL				 // We don't use any data in our WM_CREATE
//--		); 
//--
//--	ShowWindow(m_hWnd, SW_SHOW); // Show the window
//--	UpdateWindow(m_hWnd);		  // Sends WM_PAINT message
}

void BeebWin::ShowMenu(bool on) {
//--	if (on!=MenuOn) {
//--		if (on)
//--    SetMenu(m_hWnd, m_hMenu);
//--	else
//--    SetMenu(m_hWnd, NULL);
//--	}
	MenuOn=on;
}

void BeebWin::TrackPopupMenu(int x, int y) {
//--  ::TrackPopupMenu(m_hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON,
//--                   x, y,
//--                   0,
//--                   m_hWnd,
//--                   NULL);
}

/****************************************************************************/
void BeebWin::InitMenu(void)
{
	char menu_string[256];
	HMENU hMenu = m_hMenu;

	CheckMenuItem(hMenu, m_MenuIdAviResolution, MF_CHECKED);
	CheckMenuItem(hMenu, m_MenuIdAviSkip, MF_CHECKED);
	CheckMenuItem(hMenu, IDM_SPEEDANDFPS, m_ShowSpeedAndFPS ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, m_MenuIdWinSize, MF_CHECKED);
	CheckMenuItem(hMenu, IDM_FULLSCREEN, m_isFullScreen ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_DD32ONOFF, m_isDD32 ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_SOUNDONOFF, SoundEnabled ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, m_MenuIdSampleRate, MF_CHECKED);
	CheckMenuItem(hMenu, m_MenuIdVolume, MF_CHECKED);
	CheckMenuItem(hMenu, m_MenuIdTiming, MF_CHECKED);
	if (m_MenuIdSticks != 0)
		CheckMenuItem(hMenu, m_MenuIdSticks, MF_CHECKED);
	CheckMenuItem(hMenu, IDM_FREEZEINACTIVE, m_FreezeWhenInactive ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_HIDECURSOR, m_HideCursor ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_IGNOREILLEGALOPS,
					IgnoreIllegalInstructions ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, m_MenuIdKeyMapping, MF_CHECKED);
	CheckMenuItem(hMenu, IDM_MAPAS, m_KeyMapAS ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_MAPFUNCS, m_KeyMapFunc ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_WPDISC0, m_WriteProtectDisc[0] ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_WPDISC1, m_WriteProtectDisc[1] ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_AMXONOFF, AMXMouseEnabled ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_AMX_LRFORMIDDLE, AMXLRForMiddle ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, m_MenuIdAMXSize, MF_CHECKED);
	if (m_MenuIdAMXAdjust != 0)
		CheckMenuItem(hMenu, m_MenuIdAMXAdjust, MF_CHECKED);
	CheckMenuItem(hMenu, IDM_PRINTERONOFF, PrinterEnabled ? MF_CHECKED : MF_UNCHECKED);
	strcpy(menu_string, "File: ");
	strcat(menu_string, m_PrinterFileName);
	ModifyMenu(hMenu, IDM_PRINTER_FILE, MF_BYCOMMAND, IDM_PRINTER_FILE, menu_string);
	CheckMenuItem(hMenu, m_MenuIdPrinterPort, MF_CHECKED);
	CheckMenuItem(hMenu, IDM_DDRAWONOFF, m_DirectDrawEnabled ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, IDM_DDINVIDEORAM, m_DDS2InVideoRAM ? MF_CHECKED : MF_UNCHECKED);

	CheckMenuItem(hMenu, m_DDFullScreenMode, MF_CHECKED);

	CheckMenuItem(hMenu, ID_SERIAL, SerialPortEnabled ? MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_COM1, (SerialPort==1)? MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_COM2, (SerialPort==2)? MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_COM3, (SerialPort==3)? MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_COM4, (SerialPort==4)? MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(hMenu, ID_HIDEMENU, HideMenuEnabled ? MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(m_hMenu,ID_TAPESOUND,(TapeSoundEnabled)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(m_hMenu,IDM_SOUNDCHIP,(SoundChipEnabled)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(m_hMenu,ID_PSAMPLES,(PartSamples)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(m_hMenu, IDM_TUBE, (TubeEnabled)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(m_hMenu, IDM_TORCH, (TorchTube)?MF_CHECKED:MF_UNCHECKED);

	CheckMenuItem(m_hMenu,ID_UNLOCKTAPE,(UnlockTape)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(m_hMenu,m_MotionBlur,MF_CHECKED);
	CheckMenuItem(hMenu, ID_ECONET, EconetEnabled ? MF_CHECKED:MF_UNCHECKED);	//Rob
	CheckMenuItem(hMenu, IDM_SPEECH, SpeechDefault ? MF_CHECKED:MF_UNCHECKED);

	UpdateMonitorMenu();

	/* Initialise the ROM Menu. */
	SetRomMenu();

	SetSoundMenu();
}

void BeebWin::UpdateMonitorMenu() {
	HMENU hMenu = m_hMenu;
	CheckMenuItem(hMenu, ID_MONITOR_RGB, (palette_type == RGB) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_MONITOR_BW , (palette_type == BW) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_MONITOR_GREEN , (palette_type == GREEN) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_MONITOR_AMBER , (palette_type == AMBER) ? MF_CHECKED : MF_UNCHECKED);
}

void BeebWin::UpdateModelType() {
	HMENU hMenu= m_hMenu;
	CheckMenuItem(hMenu, ID_MODELB, (MachineType == 0) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_MODELBINT, (MachineType == 1) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_MODELBP, (MachineType == 2) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_MASTER128, (MachineType == 3) ? MF_CHECKED : MF_UNCHECKED);
}

void BeebWin::UpdateSFXMenu() {
	HMENU hMenu = m_hMenu;
	CheckMenuItem(hMenu,ID_SFX_RELAY,RelaySoundEnabled?MF_CHECKED:MF_UNCHECKED);
}

/****************************************************************************/
void BeebWin::InitDirectX(void)
{
//--	HRESULT ddrval;
//--
//--	ddrval = DirectDrawCreate( NULL, &m_DD, NULL );
//--	if( ddrval == DD_OK )
//--	{
//--		ddrval = m_DD->QueryInterface(IID_IDirectDraw2, (LPVOID *)&m_DD2);
//--	}
//--	if( ddrval == DD_OK )
//--	{
//--		ddrval = InitSurfaces();
//--	}
//--	if( ddrval != DD_OK )
//--	{
//--		char  errstr[200];
//--		sprintf(errstr,"DirectX initialisation failed\nFailure code %X",ddrval);
//--		MessageBox(m_hWnd,errstr,WindowTitle,MB_OK|MB_ICONERROR);
//--	}
}

/****************************************************************************/
HRESULT BeebWin::InitSurfaces(void)
{
//->	DDSURFACEDESC ddsd;
//--	DDSCAPS ddscaps2;
//--	HRESULT ddrval;
//--
//--	if (m_isFullScreen)
//--		ddrval = m_DD2->SetCooperativeLevel( m_hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN);
//--	else
//--		ddrval = m_DD2->SetCooperativeLevel( m_hWnd, DDSCL_NORMAL );
//--	if( ddrval == DD_OK )
//--	{
//--		if (m_isFullScreen)
//--		{
//--			if (m_isDD32)
//--				ddrval = m_DD2->SetDisplayMode(m_XWinSize, m_YWinSize, 32, 0, 0);
//--			else
//--				ddrval = m_DD2->SetDisplayMode(m_XWinSize, m_YWinSize, 8, 0, 0);
//--		}
//--	}
//--	if( ddrval == DD_OK )
//--	{
//--		ddsd.dwSize = sizeof( ddsd );
//--		ddsd.dwFlags = DDSD_CAPS;
//--		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
//--		if (m_PageFlipping) {
//--			ddsd.dwFlags|=DDSD_BACKBUFFERCOUNT;
//--			ddsd.ddsCaps.dwCaps|=DDSCAPS_FLIP|DDSCAPS_COMPLEX|DDSCAPS_VIDEOMEMORY;
//--			ddsd.dwBackBufferCount=1;
//--		}
//--
//--		ddrval = m_DD2->CreateSurface( &ddsd, &m_DDSPrimary, NULL );
//--	}
//--
//--	if (m_PageFlipping) {
//--		ddscaps2.dwCaps=DDSCAPS_BACKBUFFER;
//--		if (ddrval==DD_OK) 
//--			ddrval=m_DDSPrimary->GetAttachedSurface(&ddscaps2,&m_BackBuffer);
//--		if (ddrval==DD_OK)
//--			ddrval=m_BackBuffer->QueryInterface(IID_IDirectDrawSurface2, (LPVOID *)&m_BackBuffer2);
//--	}
//--
//--	if( ddrval == DD_OK )
//--		ddrval = m_DDSPrimary->QueryInterface(IID_IDirectDrawSurface2, (LPVOID *)&m_DDS2Primary);
//--	if( ddrval == DD_OK )
//--		ddrval = m_DD2->CreateClipper( 0, &m_Clipper, NULL );
//--	if( ddrval == DD_OK )
//--		ddrval = m_Clipper->SetHWnd( 0, m_hWnd );
//--	if( ddrval == DD_OK )
//--		ddrval = m_DDS2Primary->SetClipper( m_Clipper );
//--	if( ddrval == DD_OK )
//--	{
//--		ZeroMemory(&ddsd, sizeof(ddsd));
//--		ddsd.dwSize = sizeof(ddsd);
//--		ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
//--		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
//--		if (m_DDS2InVideoRAM)
//--			ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
//--		else
//--			ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
//--		ddsd.dwWidth = 800;
//--		ddsd.dwHeight = 512;
//--		ddrval = m_DD2->CreateSurface(&ddsd, &m_DDSOne, NULL);
//--	}
//--	if( ddrval == DD_OK )
//--	{
//--		ddrval = m_DDSOne->QueryInterface(IID_IDirectDrawSurface2, (LPVOID *)&m_DDS2One);
//--	}
//--
//--	m_DXInit = TRUE;
//--
//--	return ddrval;
//++
	return(0);
//<-
}

/****************************************************************************/
void BeebWin::ResetSurfaces(void)
{
//--	m_Clipper->Release();
//--	m_DDS2One->Release();
//--	m_DDSOne->Release();
//--	m_DDS2Primary->Release();
//--	m_DDSPrimary->Release();
//--
//--	m_DXInit = FALSE;
}

/****************************************************************************/
void BeebWin::SetRomMenu(void)
{
//--	HMENU hMenu = m_hMenu;
//--
//--	// Set the ROM Titles in the ROM/RAM menu.
//--	CHAR Title[19];
//--	
//--	int	 i;
//--
//--	for( i=0; i<16; i++ )
//--	{
//--		Title[0] = '&';
//--		_itoa( i, &Title[1], 16 );
//--		Title[2] = ' ';
//--		
//--		// Get the Rom Title.
//--		ReadRomTitle( i, &Title[3], sizeof( Title )-4);
//--	
//--		if ( Title[3]== '\0' )
//--			strcpy( &Title[3], "Empty" );
//--
//--		ModifyMenu( hMenu,	// handle of menu 
//--					IDM_ALLOWWRITES_ROM0 + i,
//--					MF_BYCOMMAND,	// menu item to modify
//--				//	MF_STRING,	// menu item flags 
//--					IDM_ALLOWWRITES_ROM0 + i,	// menu item identifier or pop-up menu handle
//--					Title		// menu item content 
//--					);
//--
//--		/* LRW Now uncheck the Roms which are NOT writable, that have already been loaded. */
//--		CheckMenuItem(hMenu, IDM_ALLOWWRITES_ROM0 + i, RomWritable[i] ? MF_CHECKED : MF_UNCHECKED );
//--	}
}

/****************************************************************************/
void BeebWin::GetRomMenu(void)
{
//--	HMENU hMenu = m_hMenu;
//--
//--	for (int i=0; i<16; ++i)
//--	  /* LRW Now uncheck the Roms as NOT writable, that have already been loaded. */
//--	  RomWritable[i] = ( GetMenuState(hMenu, IDM_ALLOWWRITES_ROM0 + i, MF_BYCOMMAND ) & MF_CHECKED );
}

/****************************************************************************/
void BeebWin::InitJoystick(void)
{
//--	MMRESULT mmresult;
//--
//--	/* Get joystick updates 10 times a second */
//--	mmresult = joySetCapture(m_hWnd, JOYSTICKID1, 100, FALSE);
//--	if (mmresult == JOYERR_NOERROR)
//--		mmresult = joyGetDevCaps(JOYSTICKID1, &m_JoystickCaps, sizeof(JOYCAPS));
//--
//--	if (mmresult == JOYERR_NOERROR)
//--	{
		AtoDInit();
//--	}
//--	else if (mmresult == JOYERR_UNPLUGGED)
//--	{
//--		MessageBox(m_hWnd, "Joystick is not plugged in",
//--					WindowTitle, MB_OK|MB_ICONERROR);
//--	}
//--	else
//--	{
//--		MessageBox(m_hWnd, "Failed to initialise the joystick",
//--					WindowTitle, MB_OK|MB_ICONERROR);
//--	}
}

/****************************************************************************/
void BeebWin::ScaleJoystick(unsigned int x, unsigned int y)
{
//--	/* Scale and reverse the readings */
//--	JoystickX = (int)((double)(m_JoystickCaps.wXmax - x) * 65535.0 /
//--						(double)(m_JoystickCaps.wXmax - m_JoystickCaps.wXmin));
//--	JoystickY = (int)((double)(m_JoystickCaps.wYmax - y) * 65535.0 /
//--						(double)(m_JoystickCaps.wYmax - m_JoystickCaps.wYmin));
}

/****************************************************************************/
void BeebWin::ResetJoystick(void)
{
//--	joyReleaseCapture(JOYSTICKID1);
	AtoDReset();
}

/****************************************************************************/
void BeebWin::SetMousestickButton(int button)
{
	if (m_MenuIdSticks == IDM_MOUSESTICK)
		JoystickButton = button;
}

/****************************************************************************/
void BeebWin::ScaleMousestick(unsigned int x, unsigned int y)
{
//--	if (m_MenuIdSticks == IDM_MOUSESTICK)
//--	{
//--		JoystickX = (m_XWinSize - x) * 65535 / m_XWinSize;
//--		JoystickY = (m_YWinSize - y) * 65535 / m_YWinSize;
//--	}
//--
//--	if (m_HideCursor)
//--		SetCursor(NULL);
}

/****************************************************************************/
void BeebWin::SetAMXPosition(unsigned int x, unsigned int y)
{
	if (AMXMouseEnabled)
	{
		// Scale the window coords to the beeb screen coords
		AMXTargetX = x * m_AMXXSize * (100 + m_AMXAdjust) / 100 / m_XWinSize;
		AMXTargetY = y * m_AMXYSize * (100 + m_AMXAdjust) / 100 / m_YWinSize;

		AMXMouseMovement();
	}
}

/****************************************************************************/
//--LRESULT CALLBACK WndProc(
//--				HWND hWnd,		   // window handle
//--				UINT message,	   // type of message
//--				WPARAM uParam,	   // additional information
//--				LPARAM lParam)	   // additional information
//--{
//--	int wmId, wmEvent;
//--	HDC hdc;
//--	int row, col;
//--
//--	switch (message)
//--	{
//--		case WM_COMMAND:  // message: command from application menu
//--			wmId	= LOWORD(uParam);
//--			wmEvent = HIWORD(uParam);
//--			if (mainWin)
//--				mainWin->HandleCommand(wmId);
//--			break;						  
//--
//--		case WM_PALETTECHANGED:
//--			if(!mainWin)
//--				break;
//--			if ((HWND)uParam == hWnd)
//--				break;
//--
//--			// fall through to WM_QUERYNEWPALETTE
//--		case WM_QUERYNEWPALETTE:
//--			if(!mainWin)
//--				break;
//--
//--			hdc = GetDC(hWnd);
//--			mainWin->RealizePalette(hdc);
//--			ReleaseDC(hWnd,hdc);    
//--			return TRUE;							    
//--			break;
//--
//--		case WM_PAINT:
//--			if(mainWin != NULL)
//--			{
//--				PAINTSTRUCT ps;
//--				HDC 		hDC;
//--
//--				hDC = BeginPaint(hWnd, &ps);
//--				mainWin->RealizePalette(hDC);
//--				mainWin->updateLines(hDC, 0, 0);
//--				EndPaint(hWnd, &ps);
//--			}
//--			break;
//--
//--		case WM_SYSKEYDOWN:
//--			if (uParam != VK_F10)
//--				break;
//--		case WM_KEYDOWN:
//--			// Reset shift state if it was set by Run Disc
//--			if (mainWin->m_ShiftBooted)
//--			{
//--				mainWin->m_ShiftBooted = false;
//--				BeebKeyUp(0, 0);
//--			}
//--
//--			mainWin->TranslateKey(uParam, false, row, col);
//--			break;
//--
//--		case WM_SYSKEYUP:
//--			if (uParam != VK_F10)
//--				break;
//--		case WM_KEYUP:
//--			if (uParam == VK_DIVIDE)
//--			{
//--				mainWin->QuickSave();
//--				// Let user know state has been saved
//--				FlashWindow(GETHWND, TRUE);
//--				MessageBeep(MB_ICONEXCLAMATION);
//--			}
//--			else if (uParam == VK_MULTIPLY)
//--			{
//--				mainWin->QuickLoad();
//--				// Let user know state has been loaded
//--				FlashWindow(GETHWND, TRUE);
//--				MessageBeep(MB_ICONEXCLAMATION);
//--			}
//--			else if(mainWin->TranslateKey(uParam, true, row, col) < 0)
//--			{
//--				if(row==-2)
//--				{ // Must do a reset!
//--					Init6502core();
//--					if (EnableTube) Init65C02core();
//--					Disc8271_reset();
//--					Reset1770();
//--					if (EconetEnabled) EconetReset();//Rob
//--					SCSIReset();
//--					SASIReset();
//--					//SoundChipReset();
//--				}
//--				else if(row==-3)
//--				{
//--					if (col==-3) SoundTuning+=0.1; // Page Up
//--					if (col==-4) SoundTuning-=0.1; // Page Down
//--				}
//--				else if(row==-4)
//--				{
//--					mainWin->AdjustSpeed(col == 0);
//--				}
//--			}
//--			break;					  
//--
//--		case WM_SETFOCUS:
//--			if (mainWin)
//--				mainWin->Focus(TRUE);
//--			break;
//--
//--		case WM_KILLFOCUS:
//--			BeebReleaseAllKeys();
//--			if (mainWin)
//--				mainWin->Focus(FALSE);
//--			break;					  
//--
//--		case MM_JOY1MOVE:
//--			if (mainWin)
//--				mainWin->ScaleJoystick(LOWORD(lParam), HIWORD(lParam));
//--			break;
//--
//--		case MM_JOY1BUTTONDOWN:
//--		case MM_JOY1BUTTONUP:
//--			JoystickButton = ((UINT)uParam & (JOY_BUTTON1 | JOY_BUTTON2)) ? 1 : 0;
//--			break; 
//--
//--		case WM_MOUSEMOVE:
//--			if (mainWin)
//--			{
//--				mainWin->ScaleMousestick(LOWORD(lParam), HIWORD(lParam));
//--				mainWin->SetAMXPosition(LOWORD(lParam), HIWORD(lParam));
//--				// Experiment: show menu in full screen when cursor moved to top of window
//--				if (HideMenuEnabled)
//--				{
//--					if (HIWORD(lParam) <= 2)
//--						mainWin->ShowMenu(true);
//--					else
//--						mainWin->ShowMenu(false);
//--				}
//--			}
//--			break;
//--
//--		case WM_LBUTTONDOWN:
//--			if (mainWin)
//--				mainWin->SetMousestickButton(((UINT)uParam & MK_LBUTTON) ? TRUE : FALSE);
//--			AMXButtons |= AMX_LEFT_BUTTON;
//--			break;
//--		case WM_LBUTTONUP:
//--			if (mainWin)
//--				mainWin->SetMousestickButton(((UINT)uParam & MK_LBUTTON) ? TRUE : FALSE);
//--			AMXButtons &= ~AMX_LEFT_BUTTON;
//--			break;
//--
//--		case WM_MBUTTONDOWN:
//--			AMXButtons |= AMX_MIDDLE_BUTTON;
//--			break;
//--		case WM_MBUTTONUP:
//--			AMXButtons &= ~AMX_MIDDLE_BUTTON;
//--			break;
//--
//--		case WM_RBUTTONDOWN:
//--			AMXButtons |= AMX_RIGHT_BUTTON;
//--			break;
//--		case WM_RBUTTONUP:
//--			AMXButtons &= ~AMX_RIGHT_BUTTON;
//--			break;
//--
//--		case WM_DESTROY:  // message: window being destroyed
//--			mainWin->SaveOnExit();
//--			PostQuitMessage(0);
//--			break;
//--
//--		case WM_ENTERMENULOOP: // entering menu, must mute directsound
//--			SetSound(MUTED);
//--			break;
//--
//--		case WM_EXITMENULOOP:
//--			SetSound(UNMUTED);
//--			if (mainWin)
//--				mainWin->ResetTiming();
//--			break;
//--
//--		default:		  // Passes it on if unproccessed
//--			return (DefWindowProc(hWnd, message, uParam, lParam));
//--		}
//--	return (0);
//--}

/****************************************************************************/
int BeebWin::TranslateKey(int vkey, int keyUp, int &row, int &col)
{
//--	// Key track of shift state
//--	if (vkey == 16)
//--	{
//--		if (keyUp)
//--			m_ShiftPressed = false;
//--		else
//--			m_ShiftPressed = true;
//--	}
//--
//--	if (keyUp)
//--	{
//--		// Key released, lookup beeb row + col that this vkey 
//--		// mapped to when it was pressed.  Need to release
//--		// both shifted and non-shifted presses.
//--		row = m_vkeyPressed[vkey][0][0];
//--		col = m_vkeyPressed[vkey][1][0];
//--		m_vkeyPressed[vkey][0][0] = -1;
//--		m_vkeyPressed[vkey][1][0] = -1;
//--		if (row >= 0)
//--			BeebKeyUp(row, col);
//--
//--		row = m_vkeyPressed[vkey][0][1];
//--		col = m_vkeyPressed[vkey][1][1];
//--		m_vkeyPressed[vkey][0][1] = -1;
//--		m_vkeyPressed[vkey][1][1] = -1;
//--		if (row >= 0)
//--			BeebKeyUp(row, col);
//--	}
//--	else // New key press - convert to beeb row + col
//--	{
//--		int needShift = m_ShiftPressed;
//--
//--		row = transTable[vkey][0];
//--		col = transTable[vkey][1];
//--
//--		if (m_KeyMapAS)
//--		{
//--			// Map A & S to CAPS & CTRL - good for some games
//--			if (vkey == 65)
//--			{
//--				row = 4;
//--				col = 0;
//--			}
//--			else if (vkey == 83)
//--			{
//--				row = 0;
//--				col = 1;
//--			}
//--		}
//--
//--		if (m_KeyMapFunc)
//--		{
//--			// Map F1-F10 to f0-f9
//--			if (vkey >= 113 && vkey <= 121)
//--			{
//--				row = transTable[vkey - 1][0];
//--				col = transTable[vkey - 1][1];
//--			}
//--			else if (vkey == 112)
//--			{
//--				row = 2;
//--				col = 0;
//--			}
//--		}
//--
//--		if (m_MenuIdKeyMapping == IDM_LOGICALKYBDMAPPING)
//--		{
//--			switch (vkey)
//--			{
//--			case 187: // =
//--				if (m_ShiftPressed)
//--				{
//--					row = 5;
//--					col = 7;
//--					needShift = true;
//--				}
//--				else
//--				{
//--					row = 1;
//--					col = 7;
//--					needShift = true;
//--				}
//--				break;
//--			case 192: // '
//--				if (m_ShiftPressed)
//--				{
//--					row = 4;
//--					col = 7;
//--					needShift = true;
//--				}
//--				else
//--				{
//--					row = 2;
//--					col = 4;
//--					needShift = true;
//--				}
//--				break;
//--			case 222: // #
//--				if (m_ShiftPressed)
//--				{
//--					row = 1;
//--					col = 8;
//--					needShift = true;
//--				}
//--				else
//--				{
//--					row = 1;
//--					col = 1;
//--					needShift = true;
//--				}
//--				break;
//--			case 51: // 3
//--				if (m_ShiftPressed)
//--				{
//--					row = 2;
//--					col = 8;
//--					needShift = true;
//--				}
//--				break;
//--			case 54: // 6
//--				if (m_ShiftPressed)
//--				{
//--					row = 1;
//--					col = 8;
//--					needShift = false;
//--				}
//--				break;
//--			case 55: // 7
//--				if (m_ShiftPressed)
//--				{
//--					row = 3;
//--					col = 4;
//--					needShift = true;
//--				}
//--				break;
//--			case 56: // 8
//--				if (m_ShiftPressed)
//--				{
//--					row = 4;
//--					col = 8;
//--					needShift = true;
//--				}
//--				break;
//--			case 57: // 9
//--				if (m_ShiftPressed)
//--				{
//--					row = 1;
//--					col = 5;
//--					needShift = true;
//--				}
//--				break;
//--			case 48: // 0
//--				if (m_ShiftPressed)
//--				{
//--					row = 2;
//--					col = 6;
//--					needShift = true;
//--				}
//--				break;
//--			case 189: // -
//--				if (m_ShiftPressed)
//--				{
//--					row = 2;
//--					col = 8;
//--					needShift = false;
//--				}
//--				break;
//--			case 186: // ;
//--				if (m_ShiftPressed)
//--				{
//--					row = 4;
//--					col = 8;
//--					needShift = false;
//--				}
//--				break;
//--			}
//--		}
//--
//--		if (row >= 0)
//--		{
//--			// Make sure shift state is correct
//--			if (needShift)
//--				BeebKeyDown(0, 0);
//--			else
//--				BeebKeyUp(0, 0);
//--
//--			BeebKeyDown(row, col);
//--
//--			// Record beeb row + col for key release
//--			m_vkeyPressed[vkey][0][m_ShiftPressed ? 1 : 0] = row;
//--			m_vkeyPressed[vkey][1][m_ShiftPressed ? 1 : 0] = col;
//--		}
//--		else
//--		{
//--			// Special key!  Record so key up returns correct codes
//--			m_vkeyPressed[vkey][0][1] = row;
//--			m_vkeyPressed[vkey][1][1] = col;
//--		}
//--	}
//--
	return(row);
}

/****************************************************************************/
int BeebWin::StartOfFrame(void)
{
	int FrameNum = 1;

	if (UpdateTiming())
		FrameNum = 0;

	return FrameNum;
}

void BeebWin::doLED(int sx,bool on) {
	int tsy; char colbase;
	colbase=(DiscLedColour*2)+LED_COL_BASE; // colour will be 0 for red, 1 for green.
	if (sx<100) colbase=LED_COL_BASE; // Red leds for keyboard always
	if (TeletextEnabled)
		tsy=496;
	else
		tsy=m_LastStartY+m_LastNLines-2;
	doUHorizLine(mainWin->cols[((on)?1:0)+colbase],tsy,sx,8);
	doUHorizLine(mainWin->cols[((on)?1:0)+colbase],tsy,sx,8);
};

/****************************************************************************/
void BeebWin::updateLines(HDC hDC, int starty, int nlines)
{
//--	WINDOWPLACEMENT wndpl;
//--	HRESULT ddrval;
//--	HDC hdc;
//--	int TTLines=0;
//--	int TextStart=240;
//--	int i,j;
//--
//--	// Not initialised yet?
//--	if (m_screen == NULL)
//--		return;
//--	
//--	// Use last stored params?
//--	if (starty == 0 && nlines == 0)
//--	{
//--		starty = m_LastStartY;
//--		nlines = m_LastNLines;
//--	}
//--	else
//--	{
//--		m_LastStartY = starty;
//--		m_LastNLines = nlines;
//--	}
//--
//--	++m_ScreenRefreshCount;
//--	TTLines=500/TeletextStyle;
//--
//--	// Do motion blur
//--	if (m_MotionBlur != IDM_BLUR_OFF)
//--	{
//--		if (m_MotionBlur == IDM_BLUR_2)
//--			j = 32;
//--		else if (m_MotionBlur == IDM_BLUR_4)
//--			j = 16;
//--		else // blur 8 frames
//--			j = 8;
//--
//--		for (i = 0; i < 800*512; ++i)
//--		{
//--			if (m_screen[i] != 0)
//--			{
//--				m_screen_blur[i] = m_screen[i];
//--			}
//--			else if (m_screen_blur[i] != 0)
//--			{
//--				m_screen_blur[i] += j;
//--				if (m_screen_blur[i] > 63)
//--					m_screen_blur[i] = 0;
//--			}
//--		}
//--		memcpy(m_screen, m_screen_blur, 800*512);
//--	}
//--
//--	if (!m_DirectDrawEnabled)
//--	{
//--		int win_nlines = 256 * m_YWinSize / 256;
//--
//--		TextStart = m_YWinSize - 20;
//--
//--		StretchBlt(hDC, 0, 0, m_XWinSize, win_nlines,
//--			m_hDCBitmap, 0, starty, (TeletextEnabled)?552:ActualScreenWidth, (TeletextEnabled==1)?TTLines:nlines, SRCCOPY);
//--
//--		if ((DisplayCycles>0) && (hFDCBoard!=NULL))
//--		{
//--			SetBkMode(hDC,TRANSPARENT);
//--			SetTextColor(hDC,0x808080);
//--			TextOut(hDC,0,TextStart,ExtBoard.BoardName,strlen(ExtBoard.BoardName));
//--		}
//--	}
//--	else
//--	{
//--		if (m_DXInit == FALSE)
//--			return;
//--
//--		if (TeletextEnabled)
//--			TextStart=(480/TeletextStyle)-((TeletextStyle==2)?12:0);
//--
//--		wndpl.length = sizeof(WINDOWPLACEMENT);
//--		if (GetWindowPlacement(m_hWnd, &wndpl))
//--		{
//--			if (wndpl.showCmd == SW_SHOWMINIMIZED)
//--				return;
//--		}
//--
//--		// Blit the beeb bitmap onto the secondary buffer
//--		ddrval = m_DDS2One->GetDC(&hdc);
//--		if (ddrval == DDERR_SURFACELOST)
//--		{
//--			ddrval = m_DDS2One->Restore();
//--			if (ddrval == DD_OK)
//--				ddrval = m_DDS2One->GetDC(&hdc);
//--		}
//--		if (ddrval == DD_OK)
//--		{
//--			BitBlt(hdc, 0, 0, 800, nlines, m_hDCBitmap, 0, starty, SRCCOPY);
//--
//--			if ((DisplayCycles>0) && (hFDCBoard!=NULL))
//--			{
//--				SetBkMode(hdc,TRANSPARENT);
//--				SetTextColor(hdc,0x808080);
//--				TextOut(hdc,0,TextStart,ExtBoard.BoardName,strlen(ExtBoard.BoardName));
//--			}
//--
//--			if (m_ShowSpeedAndFPS && m_isFullScreen)
//--			{
//--				char fps[50];
//--				sprintf(fps, "%2.2f %2d", m_RelativeSpeed, (int)m_FramesPerSecond);
//--				SetBkMode(hdc,TRANSPARENT);
//--				SetTextColor(hdc,0x808080);
//--				TextOut(hdc,(TeletextEnabled)?490:580,TextStart,fps,strlen(fps));
//--			}
//--
//--			m_DDS2One->ReleaseDC(hdc);
//--
//--			// Work out where on screen to blit image
//--			RECT destRect;
//--			RECT srcRect;
//--			POINT pt;
//--			GetClientRect( m_hWnd, &destRect );
//--			pt.x = pt.y = 0;
//--			ClientToScreen( m_hWnd, &pt );
//--			OffsetRect(&destRect, pt.x, pt.y);
//--
//--			// Blit the whole of the secondary buffer onto the screen
//--			srcRect.left = 0;
//--			srcRect.top = 0;
//--			srcRect.right = (TeletextEnabled)?552:ActualScreenWidth;
//--			srcRect.bottom = (TeletextEnabled)?TTLines:nlines;
//--			
//--			if (!m_PageFlipping)
//--			{
//--				ddrval = m_DDS2Primary->Blt( &destRect, m_DDS2One, &srcRect, DDBLT_ASYNC, NULL);
//--				if (ddrval == DDERR_SURFACELOST)
//--				{
//--					ddrval = m_DDS2Primary->Restore();
//--					if (ddrval == DD_OK)
//--						ddrval = m_DDS2Primary->Blt( &destRect, m_DDS2One, &srcRect, DDBLT_ASYNC, NULL );
//--				}
//--			}
//--			else
//--			{
//--				ddrval = m_BackBuffer2->Blt( &destRect, m_DDS2One, &srcRect, DDBLT_ASYNC, NULL);
//--				if (ddrval == DDERR_SURFACELOST)
//--				{
//--					ddrval = m_BackBuffer2->Restore();
//--					if (ddrval == DD_OK)
//--						ddrval = m_BackBuffer2->Blt( &destRect, m_DDS2One, &srcRect, DDBLT_ASYNC, NULL );
//--				}
//--				if (ddrval == DD_OK)
//--					m_DDSPrimary->Flip(NULL,NULL);
//--			}
//--		}
//--		
//--		if (ddrval != DD_OK && ddrval != DDERR_WASSTILLDRAWING)
//--		{
//--			char  errstr[200];
//--			sprintf(errstr,"DirectX failure while updating screen\nFailure code %X",ddrval);
//--			// Ignore DX errors for now - swapping between full screen and windowed DX 
//--			// apps causes an error while transitioning between display modes.  It
//--			// appears to correct itself after a second or two though.
//--			// MessageBox(m_hWnd,errstr,WindowTitle,MB_OK|MB_ICONERROR);
//--		}
//--	}
//--
//--	if (aviWriter)
//--	{
//--		m_AviFrameSkipCount++;
//--		if (m_AviFrameSkipCount > m_AviFrameSkip)
//--		{
//--			m_AviFrameSkipCount = 0;
//--
//--			StretchBlt(m_AviDC, 0, 0, m_Avibmi.bmiHeader.biWidth, m_Avibmi.bmiHeader.biHeight,
//--				m_hDCBitmap, 0, starty, (TeletextEnabled)?552:ActualScreenWidth, (TeletextEnabled==1)?TTLines:nlines, SRCCOPY);
//--
//--			HRESULT hr = aviWriter->WriteVideo((BYTE*)m_AviScreen);
//--			if (hr != E_UNEXPECTED && FAILED(hr))
//--			{
//--				MessageBox(m_hWnd, "Failed to write video to AVI file",
//--					WindowTitle, MB_OK|MB_ICONERROR);
//--				delete aviWriter;
//--				aviWriter = NULL;
//--			}
//--		}
//--	}

//+>
	int i, TTLines=0;


// [TODO] Might want to make m_screen point to the SDL_Surface.
//	if (m_screen == NULL)
//		return;

	if (starty == 0 && nlines == 0){
		starty = m_LastStartY;
		nlines = m_LastNLines;
	}else{
		m_LastStartY = starty;
		m_LastNLines = nlines;
	}

	++m_ScreenRefreshCount;
	TTLines=500/TeletextStyle;

  bool do_flip = false;

  if (mainWin->m_skip_cur_frame <= 0) { 
    mainWin->m_skip_cur_frame = mainWin->m_skip_max_frame;

	  for(i = 0; i < nlines; i++){
# if 0 //LUDO: FOR_TEST
		  if (i+starty < 600) 
# else
		  if (i+starty < 512) 
# endif
      {
			  RenderLine(i+starty, (int) TeletextEnabled, ScreenAdjust);
      }
	  }
    do_flip = true;
    RenderScreen(TeletextEnabled, ScreenAdjust);

    mainWin->m_render_blank_line--;
    if (mainWin->m_render_blank_line < 0) {
      mainWin->m_render_blank_line = mainWin->m_render_max_blank_line;
    }

  } else if (mainWin->m_skip_max_frame) {
    mainWin->m_skip_cur_frame--;
  }
	
  if (psp_kbd_is_danzeff_mode()) {
    sceDisplayWaitVblankStart();
    danzeff_moveTo(-130, -22);
    danzeff_render();
  }

  if (psp_screenshot_mode) {
    psp_screenshot_mode--;
    if (psp_screenshot_mode <= 0) {
      psp_sdl_save_screenshot();
      psp_screenshot_mode = 0;
    }
    do_flip = true;
  }

  if (do_flip) {
    if (mainWin->m_view_fps) {
      char buffer[32];
      sprintf(buffer, "%3d", (int)mainWin->m_FramesPerSecond);
      psp_sdl_fill_print(0, 0, buffer, 0xffffff, 0 );
    }

    if (mainWin->m_display_lr) {
      psp_kbd_display_active_mapping();
    }
    psp_sdl_flip();
  }
//<+

}

/****************************************************************************/
void BeebWin::ResetTiming(void)
{

	m_LastTickCount = GetTickCount();
	m_LastStatsTickCount = m_LastTickCount;
	m_LastTotalCycles = TotalCycles;
	m_LastStatsTotalCycles = TotalCycles;
	m_TickBase = m_LastTickCount;
	m_CycleBase = TotalCycles;
	m_MinFrameCount = 0;
	m_LastFPSCount = m_LastTickCount;
	m_ScreenRefreshCount = 0;
}

/****************************************************************************/
BOOL BeebWin::UpdateTiming(void)
{
	DWORD TickCount;
	DWORD Ticks;
	DWORD SpareTicks;
	int Cycles;
	int CyclesPerSec;
	bool UpdateScreen = FALSE;

	TickCount = GetTickCount();

	/* Don't do anything if this is the first call or there has
	   been a long pause due to menu commands, or when something
	   wraps. */
	if (m_LastTickCount == 0 ||
		TickCount < m_LastTickCount ||
		(TickCount - m_LastTickCount) > 1000 ||
		TotalCycles < m_LastTotalCycles)
	{
		ResetTiming();
		return TRUE;
	}

	/* Update stats every second */
	if (TickCount >= m_LastStatsTickCount + 1000)
	{
		m_FramesPerSecond = m_ScreenRefreshCount;
		m_ScreenRefreshCount = 0;
		m_RelativeSpeed = ((TotalCycles - m_LastStatsTotalCycles) / 2000.0) /
								(TickCount - m_LastStatsTickCount);
		m_LastStatsTotalCycles = TotalCycles;
		m_LastStatsTickCount += 1000;
		DisplayTiming();
	}

	// Now we work out if BeebEm is running too fast or not
# if 0
	if (m_RealTimeTarget > 0.0)
	{
		Ticks = TickCount - m_TickBase;
		Cycles = (int)((double)(TotalCycles - m_CycleBase) / m_RealTimeTarget);

		if (Ticks <= (DWORD)(Cycles / 2000))
		{
			// Need to slow down, show frame (max 50fps though) 
			// and sleep a bit
			if (TickCount >= m_LastFPSCount + 20)
			{
				UpdateScreen = TRUE;
				m_LastFPSCount += 20;
			}
			else
			{
				UpdateScreen = FALSE;
			}

			SpareTicks = (DWORD)(Cycles / 2000) - Ticks;

			if (SpareTicks>0) 
				Sleep(SpareTicks);

			m_MinFrameCount = 0;
		}
		else
		{
			// Need to speed up, skip a frame
			UpdateScreen = FALSE;

			// Make sure we show at least one in 100 frames
			++m_MinFrameCount;
			if (m_MinFrameCount >= 100)
			{
				UpdateScreen = TRUE;
				m_MinFrameCount = 0;
			}
		}

		/* Move counter bases forward */
//--		CyclesPerSec = 2000000.0 * m_RealTimeTarget;
		CyclesPerSec = (int)(2000000.0 * m_RealTimeTarget);

		while ((TickCount - m_TickBase) > 1000 && (TotalCycles - m_CycleBase) > CyclesPerSec)
		{
			m_TickBase += 1000;
			m_CycleBase += CyclesPerSec;
		}
	}
	else
# endif
	{
		/* Fast as possible with a certain frame rate */
		if (TickCount >= m_LastFPSCount + (1000 / m_FPSTarget))
		{
			UpdateScreen = TRUE;
			m_LastFPSCount += 1000 / m_FPSTarget;
		}
		else
		{
			UpdateScreen = FALSE;
		}
	}

	m_LastTickCount = TickCount;
	m_LastTotalCycles = TotalCycles;

	return UpdateScreen;
}

/****************************************************************************/
void BeebWin::DisplayTiming(void)
{
# if 0
	if (m_ShowSpeedAndFPS && (!m_DirectDrawEnabled || !m_isFullScreen))
	{
		sprintf(m_szTitle, "%s  Speed: %2.2f  fps: %2d",
				WindowTitle, m_RelativeSpeed, (int)m_FramesPerSecond);
	}
# endif
}

/****************************************************************************/
void BeebWin::TranslateWindowSize(void)
{
//--	if (m_isFullScreen)
//--	{
//--		if (m_DirectDrawEnabled)
//--		{
//--			switch (m_DDFullScreenMode)
//--			{
//--			case ID_VIEW_DD_640X480:
//--			  m_XWinSize = 640;
//--			  m_YWinSize = 480;
//--			  break;
//--			case ID_VIEW_DD_1024X768:
//--			  m_XWinSize = 1024;
//--			  m_YWinSize = 768;
//--			  break;
//--			case ID_VIEW_DD_1280X1024:
//--			  m_XWinSize = 1280;
//--			  m_YWinSize = 1024;
//--			  break;
//--			}
//--		}
//--		else
//--		{
//--			RECT scrrect;
//--			SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&scrrect, 0);
//--			m_XWinSize = scrrect.right - scrrect.left - GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
//--			m_YWinSize = scrrect.bottom - scrrect.top - GetSystemMetrics(SM_CYFIXEDFRAME) * 2
//--					- GetSystemMetrics(SM_CYMENUSIZE) - GetSystemMetrics(SM_CYCAPTION);
//--		}
//--	}
//--	else
//--	{
//--	  switch (m_MenuIdWinSize)
//--	  {
//--	  case IDM_160X128:
//--		  m_XWinSize = 160;
//--		  m_YWinSize = 128;
//--		  break;
//--
//--	  case IDM_240X192:
//--		  m_XWinSize = 240;
//--		  m_YWinSize = 192;
//--		  break;
//--
//--	  case IDM_640X256:
//--		  m_XWinSize = 640;
//--		  m_YWinSize = 256;
//--		  break;
//--
//--	  case IDM_320X256:
//--		  m_XWinSize = 320;
//--		  m_YWinSize = 256;
//--		  break;
//--
//--	  default:
//--	  case IDM_640X512:
//--		  m_XWinSize = 640;
//--		  m_YWinSize = 512;
//--		  break;
//--
//--	  case IDM_800X600:
//--		  m_XWinSize = 800;
//--		  m_YWinSize = 600;
//--		  break;
//--
//--	  case IDM_1024X768:
//--		  m_XWinSize = 1024;
//--		  m_YWinSize = 768;
//--		  break;
//--
//--	  case IDM_1024X512:
//--		  m_XWinSize = 1024;
//--		  m_YWinSize = 512;
//--		  break;
//--	  }
//--	}
}

/****************************************************************************/
void BeebWin::TranslateSampleRate(void)
{
	switch (m_MenuIdSampleRate)
	{
	case IDM_44100KHZ:
		SoundSampleRate = 44100;
		break;

	default:
	case IDM_22050KHZ:
		SoundSampleRate = 22050;
		break;

	case IDM_11025KHZ:
		SoundSampleRate = 11025;
		break;
	}
}

/****************************************************************************/
void BeebWin::TranslateVolume(void)
{
	switch (m_MenuIdVolume)
	{
	case IDM_FULLVOLUME:
		SoundVolume = 1;
		break;

	case IDM_HIGHVOLUME:
		SoundVolume = 2;
		break;

	default:
	case IDM_MEDIUMVOLUME:
		SoundVolume = 3;
		break;

	case IDM_LOWVOLUME:
		SoundVolume = 4;
		break;
	}
}

/****************************************************************************/
void BeebWin::TranslateTiming(void)
{
	m_FPSTarget = 0;
	m_RealTimeTarget = 1.0;

	if (m_MenuIdTiming == IDM_3QSPEED)
		m_MenuIdTiming = IDM_FIXEDSPEED0_75;
	if (m_MenuIdTiming == IDM_HALFSPEED)
		m_MenuIdTiming = IDM_FIXEDSPEED0_5;

	switch (m_MenuIdTiming)
	{
	default:
	case IDM_REALTIME:
		m_RealTimeTarget = 1.0;
		m_FPSTarget = 0;
		break;

	case IDM_FIXEDSPEED100:
		m_RealTimeTarget = 100.0;
		m_FPSTarget = 0;
		break;

	case IDM_FIXEDSPEED50:
		m_RealTimeTarget = 50.0;
		m_FPSTarget = 0;
		break;

	case IDM_FIXEDSPEED10:
		m_RealTimeTarget = 10.0;
		m_FPSTarget = 0;
		break;

	case IDM_FIXEDSPEED5:
		m_RealTimeTarget = 5.0;
		m_FPSTarget = 0;
		break;

	case IDM_FIXEDSPEED2:
		m_RealTimeTarget = 2.0;
		m_FPSTarget = 0;
		break;

	case IDM_FIXEDSPEED1_5:
		m_RealTimeTarget = 1.5;
		m_FPSTarget = 0;
		break;

	case IDM_FIXEDSPEED1_25:
		m_RealTimeTarget = 1.25;
		m_FPSTarget = 0;
		break;

	case IDM_FIXEDSPEED1_1:
		m_RealTimeTarget = 1.1;
		m_FPSTarget = 0;
		break;

	case IDM_FIXEDSPEED0_9:
		m_RealTimeTarget = 0.9;
		m_FPSTarget = 0;
		break;

	case IDM_FIXEDSPEED0_5:
		m_RealTimeTarget = 0.5;
		m_FPSTarget = 0;
		break;

	case IDM_FIXEDSPEED0_75:
		m_RealTimeTarget = 0.75;
		m_FPSTarget = 0;
		break;

	case IDM_FIXEDSPEED0_25:
		m_RealTimeTarget = 0.25;
		m_FPSTarget = 0;
		break;

	case IDM_FIXEDSPEED0_1:
		m_RealTimeTarget = 0.1;
		m_FPSTarget = 0;
		break;

	case IDM_50FPS:
		m_FPSTarget = 50;
		m_RealTimeTarget = 0;
		break;

	case IDM_25FPS:
		m_FPSTarget = 25;
		m_RealTimeTarget = 0;
		break;

	case IDM_10FPS:
		m_FPSTarget = 10;
		m_RealTimeTarget = 0;
		break;

	case IDM_5FPS:
		m_FPSTarget = 5;
		m_RealTimeTarget = 0;
		break;

	case IDM_1FPS:
		m_FPSTarget = 1;
		m_RealTimeTarget = 0;
		break;
	}
	ResetTiming();
}

# if 0 //LUDO:
void BeebWin::AdjustSpeed(bool up)
{
	static int speeds[] = {
				IDM_FIXEDSPEED100,
				IDM_FIXEDSPEED50,
				IDM_FIXEDSPEED10,
				IDM_FIXEDSPEED5,
				IDM_FIXEDSPEED2,
				IDM_FIXEDSPEED1_5,
				IDM_FIXEDSPEED1_25,
				IDM_FIXEDSPEED1_1,
				IDM_REALTIME,
				IDM_FIXEDSPEED0_9,
				IDM_FIXEDSPEED0_75,
				IDM_FIXEDSPEED0_5,
				IDM_FIXEDSPEED0_25,
				IDM_FIXEDSPEED0_1,
				0};
	int s = 0;
	int t = m_MenuIdTiming;

	while (speeds[s] != 0 && speeds[s] != m_MenuIdTiming)
		s++;

	if (speeds[s] == 0)
	{
		t = IDM_REALTIME;
	}
	else if (up)
	{
		if (s > 0)
			t = speeds[s-1];
	}
	else
	{
		if (speeds[s+1] != 0)
			t = speeds[s+1];
	}

	if (t != m_MenuIdTiming)
	{
		CheckMenuItem(m_hMenu, m_MenuIdTiming, MF_UNCHECKED);
		m_MenuIdTiming = t;
		CheckMenuItem(m_hMenu, m_MenuIdTiming, MF_CHECKED);
		TranslateTiming();
	}
}
# endif

/****************************************************************************/
void BeebWin::TranslateKeyMapping(void)
{
	switch (m_MenuIdKeyMapping)
	{
	default:
	case IDM_DEFAULTKYBDMAPPING:
		transTable = transTable1;
		break;

	case IDM_LOGICALKYBDMAPPING:
		transTable = transTable1;
		break;

	case IDM_USERKYBDMAPPING:
		transTable = UserKeymap;
		break;
	}
}

void BeebWin::SetImageName(char *DiscName,char Drive,char DType) {
  if (DiscName != CDiscName[(int)Drive]) {
	  strcpy(CDiscName[(int)Drive],DiscName);
  }
	CDiscType[(int)Drive]=DType;
	DiscLoaded[(int)Drive]=TRUE;
}

/****************************************************************************/
# if 0 //LUDO:
int BeebWin::ReadDisc(int Drive,HMENU dmenu)
# else
int BeebWin::ReadDisc(int Drive,char *FileName)
# endif
{
//--	char DefaultPath[_MAX_PATH];
# if 0 //LUDO:
	char FileName[256];
# endif
//--	OPENFILENAME ofn;
  HMENU dmenu = 0; 
	int gotName = false;
//--
//--	strcpy(DefaultPath, m_AppPath);
//--	strcat(DefaultPath, "discims");
//--	SysReg.GetStringValue(HKEY_CURRENT_USER,CFG_REG_KEY,"DiscsPath",DefaultPath);
//--
//--	ofn.nFilterIndex = 1;
# if 0 //LUDO:
	FileName[0] = '\0';
# endif
//--
//--	/* Hmm, what do I put in all these fields! */
//--	ofn.lStructSize = sizeof(OPENFILENAME);
//--	ofn.hwndOwner = m_hWnd;
//--	ofn.hInstance = NULL;
//--	ofn.lpstrFilter = "Auto (*.ssd;*.dsd;*.ad*;*.img)\0*.ssd;*.dsd;*.adl;*.adf;*.img\0"
//--                    "ADFS Disc (*.adl *.adf)\0*.adl;*.adf\0"
//--                    "Single Sided Disc (*.ssd)\0*.ssd\0"
//--                    "Double Sided Disc (*.dsd)\0*.dsd\0"
//--                    "Single Sided Disc (*.*)\0*.*\0"
//--                    "Double Sided Disc (*.*)\0*.*\0";
//--	ofn.lpstrCustomFilter = NULL;
//--	ofn.nMaxCustFilter = 0;
//--	ofn.lpstrFile = FileName;
//--	ofn.nMaxFile = sizeof(FileName);
//--	ofn.lpstrFileTitle = NULL;
//--	ofn.nMaxFileTitle = 0;
//--	ofn.lpstrInitialDir = DefaultPath;
//--	ofn.lpstrTitle = NULL;
//--	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
//--	ofn.nFileOffset = 0;
//--	ofn.nFileExtension = 0;
//--	ofn.lpstrDefExt = NULL;
//--	ofn.lCustData = 0;
//--	ofn.lpfnHook = NULL;
//--	ofn.lpTemplateName = NULL;
//--
//--	gotName = GetOpenFileName(&ofn);

  gotName = 1;

	if (gotName)
	{
//--		unsigned PathLength = strrchr(FileName, '\\') - FileName;
//--		strncpy(DefaultPath, FileName, PathLength);
//--		DefaultPath[PathLength] = 0;
//--		SysReg.SetStringValue(HKEY_CURRENT_USER,CFG_REG_KEY,"DiscsPath",DefaultPath);
//--
		bool dsd = false;
		bool adfs = false;
//--		switch (ofn.nFilterIndex)
//--		{
//--		case 1:
//--			{
			char *ext = strrchr(FileName, '.');
			if (ext != NULL)
			  if (stricmp(ext+1, "dsd") == 0)
				dsd = true;
			  if (stricmp(ext+1, "adl") == 0)
				adfs = true;
			  if (stricmp(ext+1, "adf") == 0)
				adfs = true;
//--			break;
//--			}
//--		case 2:
//--			adfs=true;
//--			break;
//--		case 4:
//--		case 6:
//--			dsd = true;
//--		}
//--
		// Another Master 128 Update, brought to you by Richard Gellman
		if (MachineType!=3)
		{
			if (dsd)
			{
				if (NativeFDC)
					LoadSimpleDSDiscImage(FileName, Drive, 80);
				else
					Load1770DiscImage(FileName,Drive,1,dmenu); // 1 = dsd
			}
			if ((!dsd) && (!adfs))
			{
				if (NativeFDC)
					LoadSimpleDiscImage(FileName, Drive, 0, 80);
				else
					Load1770DiscImage(FileName,Drive,0,dmenu); // 0 = ssd
			}
			if (adfs)
			{
				if (NativeFDC)
					MessageBox(GETHWND,"The native 8271 FDC cannot read ADFS discs\n","BeebEm",MB_OK|MB_ICONERROR);
				else
					Load1770DiscImage(FileName,Drive,2,dmenu); // 2 = adfs
			}
		}

		if (MachineType==3)
		{
			if (dsd)
				Load1770DiscImage(FileName,Drive,1,dmenu); // 0 = ssd
			if (!dsd && !adfs)						 // Here we go a transposing...
				Load1770DiscImage(FileName,Drive,0,dmenu); // 1 = dsd
			if (adfs)
				Load1770DiscImage(FileName,Drive,2,dmenu); // ADFS OO La La!
		}

		/* Write protect the disc */
		if (!m_WriteProtectDisc[Drive])
			ToggleWriteProtect(Drive);
	}

	return(gotName);
}

/****************************************************************************/
void BeebWin::LoadTape(void)
{
//--	char DefaultPath[_MAX_PATH];
//--	char FileName[256];
//--	OPENFILENAME ofn;
//--
//--	strcpy(DefaultPath, m_AppPath);
//--	strcat(DefaultPath, "tapes");
//--	SysReg.GetStringValue(HKEY_CURRENT_USER,CFG_REG_KEY,"TapesPath",DefaultPath);
//--
//--	ofn.nFilterIndex = 1;
//--	FileName[0] = '\0';
//--
//--	/* Hmm, what do I put in all these fields! */
//--	ofn.lStructSize = sizeof(OPENFILENAME);
//--	ofn.hwndOwner = m_hWnd;
//--	ofn.hInstance = NULL;
//--	ofn.lpstrFilter = "UEF Tape File (*.uef)\0*.uef\0";
//--	ofn.lpstrCustomFilter = NULL;
//--	ofn.nMaxCustFilter = 0;
//--	ofn.lpstrFile = FileName;
//--	ofn.nMaxFile = sizeof(FileName);
//--	ofn.lpstrFileTitle = NULL;
//--	ofn.nMaxFileTitle = 0;
//--	ofn.lpstrInitialDir = DefaultPath;
//--	ofn.lpstrTitle = NULL;
//--	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
//--	ofn.nFileOffset = 0;
//--	ofn.nFileExtension = 0;
//--	ofn.lpstrDefExt = NULL;
//--	ofn.lCustData = 0;
//--	ofn.lpfnHook = NULL;
//--	ofn.lpTemplateName = NULL;
//--
//--	if (GetOpenFileName(&ofn))
//--	{
//--		unsigned PathLength = strrchr(FileName, '\\') - FileName;
//--		strncpy(DefaultPath, FileName, PathLength);
//--		DefaultPath[PathLength] = 0;
//--		SysReg.SetStringValue(HKEY_CURRENT_USER,CFG_REG_KEY,"TapesPath",DefaultPath);
//--
//--		LoadUEF(FileName);
//--	}
}

void BeebWin::NewTapeImage(char *FileName)
{
//--	char DefaultPath[_MAX_PATH];
//--	OPENFILENAME ofn;
//--
//--	strcpy(DefaultPath, m_AppPath);
//--	strcat(DefaultPath, "tapes");
//--	SysReg.GetStringValue(HKEY_CURRENT_USER,CFG_REG_KEY,"TapesPath",DefaultPath);
//--
//--	ofn.nFilterIndex = 1;
//--	FileName[0] = '\0';
//--
//--	ofn.lStructSize = sizeof(OPENFILENAME);
//--	ofn.hwndOwner = m_hWnd;
//--	ofn.hInstance = NULL;
//--	ofn.lpstrFilter = "UEF Tape File (*.uef)\0*.uef\0";
//--	ofn.lpstrCustomFilter = NULL;
//--	ofn.nMaxCustFilter = 0;
//--	ofn.lpstrFile = FileName;
//--	ofn.nMaxFile = 256;
//--	ofn.lpstrFileTitle = NULL;
//--	ofn.nMaxFileTitle = 0;
//--	ofn.lpstrInitialDir = DefaultPath;
//--	ofn.lpstrTitle = NULL;
//--	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
//--	ofn.nFileOffset = 0;
//--	ofn.nFileExtension = 0;
//--	ofn.lpstrDefExt = NULL;
//--	ofn.lCustData = 0;
//--	ofn.lpfnHook = NULL;
//--	ofn.lpTemplateName = NULL;
//--
//--	if (GetSaveFileName(&ofn))
//--	{
//--		/* Add a file extension if the user did not specify one */
//--		if (strchr(FileName, '.') == NULL)
//--		{
//--			strcat(FileName, ".uef");
//--		}
//--	}
//--	else
//--	{
//--		FileName[0] = '\0';
//--	}
}

/*******************************************************************/

void BeebWin::SelectFDC(void)
{
//--	char DefaultPath[_MAX_PATH];
//--	char FileName[256];
//--	OPENFILENAME ofn;
//--
//--	strcpy(DefaultPath, m_AppPath);
//--	strcat(DefaultPath, "Hardware");
//--
//--	ofn.nFilterIndex = 1;
//--	FileName[0] = '\0';
//--
//--	/* Hmm, what do I put in all these fields! */
//--	ofn.lStructSize = sizeof(OPENFILENAME);
//--	ofn.hwndOwner = m_hWnd;
//--	ofn.hInstance = NULL;
//--	ofn.lpstrFilter = "FDC Extension Board Plugin DLL (*.dll)\0*.dll\0";
//--	ofn.lpstrCustomFilter = NULL;
//--	ofn.nMaxCustFilter = 0;
//--	ofn.lpstrFile = FileName;
//--	ofn.nMaxFile = sizeof(FileName);
//--	ofn.lpstrFileTitle = NULL;
//--	ofn.nMaxFileTitle = 0;
//--	ofn.lpstrInitialDir = DefaultPath;
//--	ofn.lpstrTitle = NULL;
//--	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
//--	ofn.nFileOffset = 0;
//--	ofn.nFileExtension = 0;
//--	ofn.lpstrDefExt = NULL;
//--	ofn.lCustData = 0;
//--	ofn.lpfnHook = NULL;
//--	ofn.lpTemplateName = NULL;
//--
//--	if (GetOpenFileName(&ofn))
//--	{
//--		strcpy(FDCDLL,FileName);
//--		LoadFDC(FDCDLL, true);
//--	}
}

/****************************************************************************/
void BeebWin::NewDiscImage(int Drive)
{
//--	char DefaultPath[_MAX_PATH];
//--	char FileName[256];
//--	OPENFILENAME ofn;
//--
//--	strcpy(DefaultPath, m_AppPath);
//--	strcat(DefaultPath, "discims");
//--	SysReg.GetStringValue(HKEY_CURRENT_USER,CFG_REG_KEY,"DiscsPath",DefaultPath);
//--
//--	ofn.nFilterIndex = 1;
//--	SysReg.GetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY,"NewDiscFilter",ofn.nFilterIndex);
//--
//--	if (MachineType!=3 && NativeFDC && ofn.nFilterIndex >= 5)
//--		ofn.nFilterIndex = 1;
//--
//--	FileName[0] = '\0';
//--
//--	ofn.lStructSize = sizeof(OPENFILENAME);
//--	ofn.hwndOwner = m_hWnd;
//--	ofn.hInstance = NULL;
//--	ofn.lpstrFilter = "Single Sided Disc (*.ssd)\0*.ssd\0"
//--					"Double Sided Disc (*.dsd)\0*.dsd\0"
//--					"Single Sided Disc\0*.*\0"
//--					"Double Sided Disc\0*.*\0"
//--					"ADFS M ( 80 Track) Disc\0*.adf\0"
//--					"ADFS L (160 Track) Disc\0*.adl\0";
//--	ofn.lpstrCustomFilter = NULL;
//--	ofn.nMaxCustFilter = 0;
//--	ofn.lpstrFile = FileName;
//--	ofn.nMaxFile = sizeof(FileName);
//--	ofn.lpstrFileTitle = NULL;
//--	ofn.nMaxFileTitle = 0;
//--	ofn.lpstrInitialDir = DefaultPath;
//--	ofn.lpstrTitle = NULL;
//--	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
//--	ofn.nFileOffset = 0;
//--	ofn.nFileExtension = 0;
//--	ofn.lpstrDefExt = NULL;
//--	ofn.lCustData = 0;
//--	ofn.lpfnHook = NULL;
//--	ofn.lpTemplateName = NULL;
//--
//--	if (GetSaveFileName(&ofn))
//--	{
//--		unsigned PathLength = strrchr(FileName, '\\') - FileName;
//--		strncpy(DefaultPath, FileName, PathLength);
//--		DefaultPath[PathLength] = 0;
//--		SysReg.SetStringValue(HKEY_CURRENT_USER,CFG_REG_KEY,"DiscsPath",DefaultPath);
//--		SysReg.SetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY,"NewDiscFilter",ofn.nFilterIndex);
//--
//--		/* Add a file extension if the user did not specify one */
//--		if (strchr(FileName, '.') == NULL)
//--		{
//--			if (ofn.nFilterIndex == 1 || ofn.nFilterIndex == 3)
//--				strcat(FileName, ".ssd");
//--			if (ofn.nFilterIndex == 2 || ofn.nFilterIndex == 4)
//--				strcat(FileName, ".dsd");
//--			if (ofn.nFilterIndex==5)
//--				strcat(FileName, ".adf");
//--			if (ofn.nFilterIndex==6)
//--				strcat(FileName, ".adl");
//--		}
//--
//--		if (ofn.nFilterIndex == 1 || ofn.nFilterIndex == 3)
//--		{
//--			CreateDiscImage(FileName, Drive, 1, 80);
//--		}
//--		if (ofn.nFilterIndex == 2 || ofn.nFilterIndex == 4)
//--		{
//--			CreateDiscImage(FileName, Drive, 2, 80);
//--		}
//--		if (ofn.nFilterIndex == 5) CreateADFSImage(FileName,Drive,80,m_hMenu);
//--		if (ofn.nFilterIndex == 6) CreateADFSImage(FileName,Drive,160,m_hMenu);
//--
//--		/* Allow disc writes */
//--		if (m_WriteProtectDisc[Drive])
//--			ToggleWriteProtect(Drive);
//--		DWriteable[Drive]=1;
//--		DiscLoaded[Drive]=TRUE;
//--		strcpy(CDiscName[1],FileName);
//--	}
}

/****************************************************************************/
void BeebWin::SaveState()
{
//--	char DefaultPath[_MAX_PATH];
	char FileName[260];
//--	OPENFILENAME ofn;
//--
//--	strcpy(DefaultPath, m_AppPath);
//--	strcat(DefaultPath, "beebstate");
//--	SysReg.GetStringValue(HKEY_CURRENT_USER,CFG_REG_KEY,"StatesPath",DefaultPath);
//--
	FileName[0] = '\0';
//--
//--	ofn.lStructSize = sizeof(OPENFILENAME);
//--	ofn.hwndOwner = m_hWnd;
//--	ofn.hInstance = NULL;
//--	ofn.lpstrFilter = "UEF State File\0*.uef\0";
//--	ofn.lpstrCustomFilter = NULL;
//--	ofn.nMaxCustFilter = 0;
//--	ofn.nFilterIndex = 1;
//--	ofn.lpstrFile = FileName;
//--	ofn.nMaxFile = sizeof(FileName);
//--	ofn.lpstrFileTitle = NULL;
//--	ofn.nMaxFileTitle = 0;
//--	ofn.lpstrInitialDir = DefaultPath;
//--	ofn.lpstrTitle = NULL;
//--	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
//--	ofn.nFileOffset = 0;
//--	ofn.nFileExtension = 0;
//--	ofn.lpstrDefExt = NULL;
//--	ofn.lCustData = 0;
//--	ofn.lpfnHook = NULL;
//--	ofn.lpTemplateName = NULL;
//--
//--	if (GetSaveFileName(&ofn))

# if 0 //LUDO: TO_BE_DONE !
	if (Save_GTK_File_Selector(FileName) == true)
	{
//--		unsigned PathLength = strrchr(FileName, '\\') - FileName;
//--		strncpy(DefaultPath, FileName, PathLength);
//--		DefaultPath[PathLength] = 0;
//--		SysReg.SetStringValue(HKEY_CURRENT_USER,CFG_REG_KEY,"StatesPath",DefaultPath);

		printf("%s\n", FileName);

		// Add UEF extension if not already set and is UEF
		if ((strcmp(FileName+(strlen(FileName)-4),".UEF")!=0) &&
			(strcmp(FileName+(strlen(FileName)-4),".uef")!=0))
		{
			strcat(FileName,".uef");
		}
		SaveUEFState(FileName);
	}
# endif

}

/****************************************************************************/
void BeebWin::RestoreState()
{
//--	char DefaultPath[_MAX_PATH];
	char FileName[256];
//--	OPENFILENAME ofn;
//--
//--	strcpy(DefaultPath, m_AppPath);
//--	strcat(DefaultPath, "beebstate");
//--	SysReg.GetStringValue(HKEY_CURRENT_USER,CFG_REG_KEY,"StatesPath",DefaultPath);
//--  
	FileName[0] = '\0';
//--
//--	ofn.lStructSize = sizeof(OPENFILENAME);
//--	ofn.hwndOwner = m_hWnd;
//--	ofn.hInstance = NULL;
//--	ofn.lpstrFilter = "UEF State File\0*.uef\0";
//--	ofn.lpstrCustomFilter = NULL;
//--	ofn.nMaxCustFilter = 0;
//--	ofn.nFilterIndex = 1;
//--	ofn.lpstrFile = FileName;
//--	ofn.nMaxFile = sizeof(FileName);
//--	ofn.lpstrFileTitle = NULL;
//--	ofn.nMaxFileTitle = 0;
//--	ofn.lpstrInitialDir = DefaultPath;
//--	ofn.lpstrTitle = NULL;
//--	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
//--	ofn.nFileOffset = 0;
//--	ofn.nFileExtension = 0;
//--	ofn.lpstrDefExt = NULL;
//--	ofn.lCustData = 0;
//--	ofn.lpfnHook = NULL;
//--	ofn.lpTemplateName = NULL;
//--
//--	if (GetOpenFileName(&ofn))

# if 0 //LUDO: TO_BE_DONE
	if (Open_GTK_File_Selector(FileName))	
# else
  if (0)
# endif
	{
//--		unsigned PathLength = strrchr(FileName, '\\') - FileName;
//--		strncpy(DefaultPath, FileName, PathLength);
//--		DefaultPath[PathLength] = 0;
//--		SysReg.SetStringValue(HKEY_CURRENT_USER,CFG_REG_KEY,"StatesPath",DefaultPath);
//--
		LoadUEFState(FileName);
	}
}

/****************************************************************************/
void BeebWin::ToggleWriteProtect(int Drive)
{
	HMENU hMenu = m_hMenu;
	if (MachineType!=3)
	{
		if (m_WriteProtectDisc[Drive])
		{
			m_WriteProtectDisc[Drive] = 0;
			DiscWriteEnable(Drive, 1);
		}
		else
		{
			m_WriteProtectDisc[Drive] = 1;
			DiscWriteEnable(Drive, 0);
		}

		if (Drive == 0)
			CheckMenuItem(hMenu, IDM_WPDISC0, m_WriteProtectDisc[0] ? MF_CHECKED : MF_UNCHECKED);
		else
			CheckMenuItem(hMenu, IDM_WPDISC1, m_WriteProtectDisc[1] ? MF_CHECKED : MF_UNCHECKED);
	}
	if (MachineType==3)
	{
		DWriteable[Drive]=1-DWriteable[Drive];
		if (Drive == 0)
			CheckMenuItem(hMenu, IDM_WPDISC0, DWriteable[0] ? MF_UNCHECKED : MF_CHECKED);
		else
			CheckMenuItem(hMenu, IDM_WPDISC1, DWriteable[1] ? MF_UNCHECKED : MF_CHECKED);
	}
}

void BeebWin::SetDiscWriteProtects(void)
{
	if (MachineType!=3)
	{
		m_WriteProtectDisc[0] = !IsDiscWritable(0);
		m_WriteProtectDisc[1] = !IsDiscWritable(1);
		CheckMenuItem(m_hMenu, IDM_WPDISC0, m_WriteProtectDisc[0] ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(m_hMenu, IDM_WPDISC1, m_WriteProtectDisc[1] ? MF_CHECKED : MF_UNCHECKED);
	}
	else
	{
		CheckMenuItem(m_hMenu, IDM_WPDISC0, DWriteable[0] ? MF_UNCHECKED : MF_CHECKED);
		CheckMenuItem(m_hMenu, IDM_WPDISC1, DWriteable[1] ? MF_UNCHECKED : MF_CHECKED);
	}
}

/****************************************************************************/
BOOL BeebWin::PrinterFile()
{
//--	char StartPath[_MAX_PATH];
//--	char FileName[256];
//--	OPENFILENAME ofn;
//--	BOOL changed;
//--
//--	if (strlen(m_PrinterFileName) == 0)
//--	{
//--		strcpy(StartPath, m_AppPath);
//--		FileName[0] = '\0';
//--	}
//--	else
//--	{
//--		char drive[_MAX_DRIVE];
//--		char dir[_MAX_DIR];
//--		char fname[_MAX_FNAME];
//--		char ext[_MAX_EXT];
//--		_splitpath(m_PrinterFileName, drive, dir, fname, ext);
//--		_makepath(StartPath, drive, dir, NULL, NULL);
//--		_makepath(FileName, NULL, NULL, fname, ext);
//--	}
//--
//--	ofn.lStructSize = sizeof(OPENFILENAME);
//--	ofn.hwndOwner = m_hWnd;
//--	ofn.hInstance = NULL;
//--	ofn.lpstrFilter = "Printer Output\0*\0";
//--	ofn.lpstrCustomFilter = NULL;
//--	ofn.nMaxCustFilter = 0;
//--	ofn.nFilterIndex = 0;
//--	ofn.lpstrFile = FileName;
//--	ofn.nMaxFile = sizeof(FileName);
//--	ofn.lpstrFileTitle = NULL;
//--	ofn.nMaxFileTitle = 0;
//--	ofn.lpstrInitialDir = StartPath;
//--	ofn.lpstrTitle = NULL;
//--	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
//--	ofn.nFileOffset = 0;
//--	ofn.nFileExtension = 0;
//--	ofn.lpstrDefExt = NULL;
//--	ofn.lCustData = 0;
//--	ofn.lpfnHook = NULL;
//--	ofn.lpTemplateName = NULL;
//--
//--	changed = GetSaveFileName(&ofn);
//--	if (changed)
//--	{
//--		strcpy(m_PrinterFileName, FileName);
//--	}
//--
//--	return(changed);
  return false;
}

/****************************************************************************/
void BeebWin::TogglePrinter()
{
//--	BOOL FileOK = TRUE;
//--	HMENU hMenu = m_hMenu;
//--
//--	if (PrinterEnabled)
//--	{
//--		PrinterDisable();
//--	}
//--	else
//--	{
//--		if (m_MenuIdPrinterPort == IDM_PRINTER_FILE)
//--		{
//--			if (strlen(m_PrinterFileName) == 0)
//--				PrinterFile();
//--			if (strlen(m_PrinterFileName) != 0)
//--			{
//--				/* First check if file already exists */
//--				FILE *outfile;
//--				outfile=fopen(m_PrinterFileName,"rb");
//--				if (outfile != NULL)
//--				{
//--					fclose(outfile);
//--					char errstr[200];
//--					sprintf(errstr, "File already exists:\n  %s\n\nOverwrite file?", m_PrinterFileName);
//--					if (MessageBox(m_hWnd,errstr,WindowTitle,MB_YESNO|MB_ICONQUESTION) != IDYES)
//--						FileOK = FALSE;
//--				}
//--				if (FileOK == TRUE)
//--					PrinterEnable(m_PrinterFileName);
//--			}
//--		}
//--		else
//--		{
//--			PrinterEnable(m_PrinterDevice);
//--		}
//--	}
//--
//--	CheckMenuItem(hMenu, IDM_PRINTERONOFF, PrinterEnabled ? MF_CHECKED : MF_UNCHECKED);
}

/****************************************************************************/
void BeebWin::TranslatePrinterPort()
{
	switch (m_MenuIdPrinterPort)
	{
	case IDM_PRINTER_FILE:
		strcpy(m_PrinterDevice, m_PrinterFileName);
		break;
	default:
	case IDM_PRINTER_LPT1:
		strcpy(m_PrinterDevice, "LPT1");
		break;
	case IDM_PRINTER_LPT2:
		strcpy(m_PrinterDevice, "LPT2");
		break;
	case IDM_PRINTER_LPT3:
		strcpy(m_PrinterDevice, "LPT3");
		break;
	case IDM_PRINTER_LPT4:
		strcpy(m_PrinterDevice, "LPT4");
		break;
	}
}

/****************************************************************************/
void BeebWin::LoadPreferences()
{
	int LEDByte=0;
	char CfgValue[256];
# if 0
	char DefValue[256];
	char CfgName[256];
	unsigned char flag;
	DWORD dword;
	char keyData[256];
	int key;
	int row, col;
# endif
//--	RECT rect;

	*binsize=1;


// [TODO] Make a fake registry.


	MachineType=0;
	m_DirectDrawEnabled=0;
	m_DDFullScreenMode=ID_VIEW_DD_640X480;
	m_isDD32=0;
	m_MenuIdWinSize=IDM_640X512;
	m_isFullScreen=0;

	TranslateWindowSize();

	m_ShowSpeedAndFPS=1;
	palette_type = RGB;
  HideMenuEnabled=0;
	LEDByte=0;
	DiscLedColour=LED_COLOUR_TYPE;
	LEDs.ShowDisc=LED_SHOW_DISC;
	LEDs.ShowKB=LED_SHOW_KB;
	m_MotionBlur = IDM_BLUR_OFF;
		m_BlurIntensities[0]=100;
		m_BlurIntensities[1]=88;
		m_BlurIntensities[2]=75;
		m_BlurIntensities[3]=62;
		m_BlurIntensities[4]=50;
		m_BlurIntensities[5]=38;
		m_BlurIntensities[6]=25;
		m_BlurIntensities[7]=12;
# if 0 //LUDO: FOR_TEST
	m_MenuIdTiming = IDM_REALTIME;
# else
	m_MenuIdTiming = IDM_50FPS;
# endif
//<-
	TranslateTiming();



//->	RegRes=SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_SOUND_ENABLED,&SoundDefault,binsize);
//--	if (!RegRes) {
		SoundDefault=1;
//--	}
//<-	RegRes=SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"SoundChipEnabled",&SoundChipEnabled,binsize);



//->	if (!RegRes) {
//--		SoundChipEnabled=1;
//<-	}



//->	if (SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_SOUND_DIRECT_ENABLED,&flag,binsize)) {
//--		DirectSoundEnabled = flag;
//--	}
//--	else {
//--		GetPrivateProfileString(CFG_SOUND_SECTION, CFG_SOUND_DIRECT_ENABLED, "0",
//--			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
//--		DirectSoundEnabled = atoi(CfgValue);
//--	}
//++
	DirectSoundEnabled=0;
//<-



//->	if (SysReg.GetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_SOUND_SAMPLE_RATE,dword)) {
//--		m_MenuIdSampleRate = dword;
//--	}
//--	else {
//--		sprintf(DefValue, "%d", IDM_22050KHZ);
//--		GetPrivateProfileString(CFG_SOUND_SECTION, CFG_SOUND_SAMPLE_RATE, DefValue,
//--			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
//--		m_MenuIdSampleRate = atoi(CfgValue);
//--	}
//++
	m_MenuIdSampleRate=IDM_22050KHZ;
//<-
	TranslateSampleRate();



//->	if (SysReg.GetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_SOUND_VOLUME,dword)) {
//--		m_MenuIdVolume = dword;
//--	}
//--	else {
//--		sprintf(DefValue, "%d", IDM_MEDIUMVOLUME);
//--		GetPrivateProfileString(CFG_SOUND_SECTION, CFG_SOUND_VOLUME, DefValue,
//--			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
//--		m_MenuIdVolume = atoi(CfgValue);
//--	}
//++
	m_MenuIdVolume=IDM_MEDIUMVOLUME;
//<-
	TranslateVolume();



//->	RegRes=SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"RelaySoundEnabled",&RelaySoundEnabled,binsize);
//--	if (!RegRes) {
//--		RelaySoundEnabled=0;
//<-	}



//->	RegRes=SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"TapeSoundEnabled",&TapeSoundEnabled,binsize);
//--	if (!RegRes) {
//--		TapeSoundEnabled=0; // BBC Tapes always were a bit harsh on the ears....
//<-	}



//->	RegRes=SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"UsePrimaryBuffer",&UsePrimaryBuffer,binsize);
//--	if (!RegRes) {
//--		UsePrimaryBuffer=0;
//<-	}



//->	RegRes=SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"Part Samples",&PartSamples,binsize);
//--	if (!RegRes) {
//--		PartSamples=1;
//--	}
//--//	RegRes=SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"UseHostClock",&UseHostClock,binsize);
//--//	if (!RegRes) {
//--//		UseHostClock=1;
//--//	}



//->	if (SysReg.GetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_OPTIONS_STICKS,dword)) {
//--		m_MenuIdSticks = dword;
//--	}
//--	else {
//--		GetPrivateProfileString(CFG_OPTIONS_SECTION, CFG_OPTIONS_STICKS, "0",
//--			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
//--		m_MenuIdSticks = atoi(CfgValue);
//--	}
//++
	m_MenuIdSticks=0;
//<-



//->	if (SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_OPTIONS_FREEZEINACTIVE,&flag,binsize)) {
//--		m_FreezeWhenInactive = flag;
//--	}
//--	else {
//--		GetPrivateProfileString(CFG_OPTIONS_SECTION, CFG_OPTIONS_FREEZEINACTIVE, "1",
//--			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
//--		m_FreezeWhenInactive = atoi(CfgValue);
//--	}
//++
	m_FreezeWhenInactive=1;
//<-



//->	if (SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_OPTIONS_HIDE_CURSOR,&flag,binsize)) {
//--		m_HideCursor = flag;
//--	}
//--	else {
//--		GetPrivateProfileString(CFG_OPTIONS_SECTION, CFG_OPTIONS_HIDE_CURSOR, "0",
//--			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
//--		m_HideCursor = atoi(CfgValue);
//--	}
//++
	m_HideCursor=0;
//<-



//->	if (SysReg.GetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_OPTIONS_KEY_MAPPING,dword)) {
//--		m_MenuIdKeyMapping = dword;
//--	}
//--	else {
//--		sprintf(DefValue, "%d", IDM_DEFAULTKYBDMAPPING);
//--		GetPrivateProfileString(CFG_OPTIONS_SECTION, CFG_OPTIONS_KEY_MAPPING, DefValue,
//--			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
//--		m_MenuIdKeyMapping = atoi(CfgValue);
//--	}
//++
	m_MenuIdKeyMapping=IDM_DEFAULTKYBDMAPPING;
//<-



//->	RegRes=SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"KeyMapAS",&flag,binsize);
//--	if (!RegRes) {
		m_KeyMapAS=0;
//--	}
//--	else {
//--		m_KeyMapAS = flag;
//<-	}



//->	RegRes=SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"KeyMapFunc",&flag,binsize);
//--	if (!RegRes) {
		m_KeyMapFunc=0;
//--	}
//--	else {
//--		m_KeyMapFunc = flag;
//--	}
	TranslateKeyMapping();
//--	*binsize=256;
//--	sprintf(CfgName, "%sRow", CFG_OPTIONS_USER_KEY_MAP);
//--	if (SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,CfgName,keyData,binsize)) {
//--		for (key=0; key<256; ++key)
//--			UserKeymap[key][0] = keyData[key];
//--
//--		sprintf(CfgName, "%sCol", CFG_OPTIONS_USER_KEY_MAP);
//--		SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,CfgName,keyData,binsize);
//--		for (key=0; key<256; ++key)
//--			UserKeymap[key][1] = keyData[key];
//--	}
//--	else
//--	{
//--		for (key=0; key<256; ++key)
//--		{
//--			sprintf(CfgName, "%s%d", CFG_OPTIONS_USER_KEY_MAP, key);
//--			GetPrivateProfileString(CFG_OPTIONS_SECTION, CfgName, "-1 -1",
//--					CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
//--			sscanf(CfgValue, "%d %d", &row, &col);
//--			if (row != -1 && col != -1)
//--			{
//--				UserKeymap[key][0] = row;
//--				UserKeymap[key][1] = col;
//--			}
//--		}
//--	}
//--	if (UserKeymap[107][0]==0 && UserKeymap[107][1]==0) {
//--		UserKeymap[107][0]=-4;
//--	}
//--	if (UserKeymap[109][0]==0 && UserKeymap[109][1]==0) {
//--		UserKeymap[109][0]=-4;
//--		UserKeymap[109][1]=1;
//--	}
//<-	*binsize=1;



//->	if (SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_AMX_ENABLED,&flag,binsize)) {
//--		AMXMouseEnabled = flag;
//--	}
//--	else {
//--		GetPrivateProfileString(CFG_AMX_SECTION, CFG_AMX_ENABLED, "0",
//--			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
//--		AMXMouseEnabled = atoi(CfgValue);
//--	}
//++
	AMXMouseEnabled=0;
//<-



//->	if (SysReg.GetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_AMX_LRFORMIDDLE,dword)) {
//--		AMXLRForMiddle = dword;
//--	}
//--	else {
//--		GetPrivateProfileString(CFG_AMX_SECTION, CFG_AMX_LRFORMIDDLE, "1",
//--			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
//--		AMXLRForMiddle = atoi(CfgValue);
//--	}
//++
	AMXLRForMiddle=1;
//<-



//->	if (SysReg.GetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_AMX_SIZE,dword)) {
//--		m_MenuIdAMXSize = dword;
//--	}
//--	else {
//--		sprintf(DefValue, "%d", IDM_AMX_320X256);
//--		GetPrivateProfileString(CFG_AMX_SECTION, CFG_AMX_SIZE, DefValue,
//--			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
//--		m_MenuIdAMXSize = atoi(CfgValue);
//--	}
//++
	m_MenuIdAMXSize=IDM_AMX_320X256;
//<-



//->	if (SysReg.GetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_AMX_ADJUST,dword)) {
//--		m_MenuIdAMXAdjust = dword;
//--	}
//--	else {
//--		sprintf(DefValue, "%d", IDM_AMX_ADJUSTP30);
//--		GetPrivateProfileString(CFG_AMX_SECTION, CFG_AMX_ADJUST, DefValue,
//--			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
//--		m_MenuIdAMXAdjust = atoi(CfgValue);
//--	}
//++
	m_MenuIdAMXAdjust=IDM_AMX_ADJUSTP30;
//<-
	TranslateAMX();



//->	if (SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_PRINTER_ENABLED,&flag,binsize)) {
//--		PrinterEnabled = flag;
//--	}
//--	else {
//--		GetPrivateProfileString(CFG_PRINTER_SECTION, CFG_PRINTER_ENABLED, "0",
//--			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
//--		PrinterEnabled = atoi(CfgValue);
//--	}
//++
	PrinterEnabled=0;
//<-



//->	if (SysReg.GetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_PRINTER_PORT,dword)) {
//--		m_MenuIdPrinterPort = dword;
//--	}
//--	else {
//--		sprintf(DefValue, "%d", IDM_PRINTER_LPT1);
//--		GetPrivateProfileString(CFG_PRINTER_SECTION, CFG_PRINTER_PORT, DefValue,
//--			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
//--		m_MenuIdPrinterPort = atoi(CfgValue);
//--	}
//++
	m_MenuIdPrinterPort=IDM_PRINTER_LPT1;
//<-



//->	if (!SysReg.GetStringValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_PRINTER_FILE,m_PrinterFileName)) {
//--		GetPrivateProfileString(CFG_PRINTER_SECTION, CFG_PRINTER_FILE, "",
//--			CfgValue, sizeof(CfgValue), CFG_FILE_NAME);
//--		strcpy(m_PrinterFileName, CfgValue);
//--	}
//++
	CfgValue[0]='\0'; strcpy(m_PrinterFileName, CfgValue);
//<-
	TranslatePrinterPort();



//->	*binsize=2;
//--	RegRes=SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"Tape Clock Speed",&TapeClockSpeed,binsize);
//--	if (!RegRes) {
		TapeClockSpeed=5600;
//<-	}



//->	*binsize=1;
//--	RegRes=SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"UnlockTape",&flag,binsize);
//--	if (!RegRes) {
//--		UnlockTape=0;
//--	}
//--	else {
//--		UnlockTape=flag;
//--	}



//->	RegRes=SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"SerialPortEnabled",&SerialPortEnabled,binsize);
//--	if (!RegRes) {
//--		SerialPortEnabled=0;
//<-	}



//->	RegRes=SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"SerialPort",&SerialPort,binsize);
//--	if (!RegRes) {
//--		SerialPort=2;
//<-	}



//->	//Rob
//--	RegRes=SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"EconetEnabled",&EconetEnabled,binsize);
//--	if (!RegRes) {
		EconetEnabled=0;
//<-	}



//->	RegRes=SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"SpeechEnabled",&flag,binsize);
//--	if (!RegRes) {
		SpeechDefault=0;
//--	}
//--	else {
//--		SpeechDefault=flag;
//<-	}



//->	RegRes=SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"TorchTube",&TorchTube,binsize);
//--	if (!RegRes) {
		TorchTube=0;
//<-	}



//->	RegRes=SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"TubeEnabled",&TubeEnabled,binsize);
//--	if (!RegRes) {
		TubeEnabled=0;
//<-	}



//->	RegRes=SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"OpCodes",&OpCodes,binsize);
//--	if (!RegRes) {
		OpCodes=2;
//<-	}



//->	RegRes=SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"Basic Hardware",&BHardware,binsize);
//--	if (!RegRes) {
		BHardware=0;
//<-	}



//->	RegRes=SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"Teletext Half Mode",&THalfMode,binsize);
//--	if (!RegRes) {
		THalfMode=0;
//<-	}



//->	RegRes=SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"SoundBlockSize",&SBSize,binsize);
//--	if (!RegRes) {
		SBSize=0;
//<-	}



//<-	*binsize=sizeof(rect);
//--	RegRes=SysReg.GetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"WindowPos2",&rect,binsize);
//--	if (RegRes)
//--	{
//--		m_XWinPos = rect.left;
//--		m_YWinPos = rect.top;
//--
//--		// Pos can get corrupted if two BeebEm's exited as same time
//--		RECT scrrect;
//--		SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&scrrect, 0);
//--		if (m_XWinPos < 0)
//--			m_XWinPos = 0;
//--		if (m_XWinPos > scrrect.right - 80)
//--			m_XWinPos = 0;
//--		if (m_YWinPos < 0)
//--			m_YWinPos = 0;
//--		if (m_YWinPos > scrrect.bottom - 80)
//--			m_YWinPos = 0;
//--	}
//--	else
//--	{
		m_XWinPos = -1;
		m_YWinPos = -1;
//->	}



//->	if (SysReg.GetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY,"CaptureResolution",dword)) {
//--		m_MenuIdAviResolution = dword;
//--	}
//--	else {
		m_MenuIdAviResolution = IDM_VIDEORES2;
//--	}
//<-



//->	if (SysReg.GetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY,"FrameSkip",dword)) {
//--		m_MenuIdAviSkip = dword;
//--	}
//--	else {
		m_MenuIdAviSkip = IDM_VIDEOSKIP1;
//--	}
//<-



	// Create all the reg keys
	SavePreferences();
}

/****************************************************************************/
void BeebWin::SavePreferences()
{
//--	int LEDByte=0;
//--	char CfgName[256];
//--	unsigned char flag;
//--	char keyData[256];
//--	int key;
//--
//--	*binsize=1;
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"MachineType",&MachineType,binsize);
//--
//--	flag = m_DirectDrawEnabled;
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_VIEW_DIRECT_ENABLED,&flag,binsize);
//--	flag = m_DDS2InVideoRAM;
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_VIEW_BUFFER_IN_VIDEO,&flag,binsize);
//--	SysReg.SetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY,"DDFullScreenMode",m_DDFullScreenMode);
//--	flag = m_isDD32;
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"isDD32",&flag,binsize);
//--	SysReg.SetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_VIEW_WIN_SIZE,m_MenuIdWinSize);
//--	flag = m_isFullScreen;
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"isFullScreen",&flag,binsize);
//--	flag = m_ShowSpeedAndFPS;
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_VIEW_SHOW_FPS,&flag,binsize);
//--	flag = palette_type;
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_VIEW_MONITOR,&flag,binsize);
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"HideMenuEnabled",&HideMenuEnabled,binsize);
//--	LEDByte=(DiscLedColour<<2)|((LEDs.ShowDisc?1:0)<<1)|(LEDs.ShowKB?1:0);
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"LED Information",&LEDByte,binsize);
//--	flag = m_MotionBlur;
//--	SysReg.SetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY, "MotionBlur", m_MotionBlur);
//--	*binsize=8;
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"MotionBlurIntensities",m_BlurIntensities,binsize);
//--	*binsize=1;
//--
//--	SysReg.SetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY, CFG_SPEED_TIMING, m_MenuIdTiming);
//--
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_SOUND_ENABLED,&SoundDefault,binsize);
//--	flag = SoundChipEnabled;
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"SoundChipEnabled",&flag,binsize);
//--	flag = DirectSoundEnabled;
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_SOUND_DIRECT_ENABLED,&flag,binsize);
//--	SysReg.SetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY, CFG_SOUND_SAMPLE_RATE, m_MenuIdSampleRate);
//--	SysReg.SetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY, CFG_SOUND_VOLUME, m_MenuIdVolume);
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"RelaySoundEnabled",&RelaySoundEnabled,binsize);
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"TapeSoundEnabled",&TapeSoundEnabled,binsize);
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"UsePrimaryBuffer",&UsePrimaryBuffer,binsize);
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"Part Samples",&PartSamples,binsize);
//-- //	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"UseHostClock",&UseHostClock,binsize);
//--
//--	SysReg.SetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY, CFG_OPTIONS_STICKS, m_MenuIdSticks);
//--	flag = m_FreezeWhenInactive;
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_OPTIONS_FREEZEINACTIVE,&flag,binsize);
//--	flag = m_HideCursor;
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_OPTIONS_HIDE_CURSOR,&flag,binsize);
//--	SysReg.SetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY, CFG_OPTIONS_KEY_MAPPING, m_MenuIdKeyMapping);
//--	flag = m_KeyMapAS;
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"KeyMapAS",&flag,binsize);
//--	flag = m_KeyMapFunc;
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"KeyMapFunc",&flag,binsize);
//--	*binsize=256;
//--	for (key=0; key<256; ++key)
//--		keyData[key] = UserKeymap[key][0];
//--	sprintf(CfgName, "%sRow", CFG_OPTIONS_USER_KEY_MAP);
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,CfgName,keyData,binsize);
//--	for (key=0; key<256; ++key)
//--		keyData[key] = UserKeymap[key][1];
//--	sprintf(CfgName, "%sCol", CFG_OPTIONS_USER_KEY_MAP);
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,CfgName,keyData,binsize);
//--	*binsize=1;
//--
//--	flag = AMXMouseEnabled;
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_AMX_ENABLED,&flag,binsize);
//--	SysReg.SetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY, CFG_AMX_LRFORMIDDLE, AMXLRForMiddle);
//--	SysReg.SetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY, CFG_AMX_SIZE, m_MenuIdAMXSize);
//--	SysReg.SetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY, CFG_AMX_ADJUST, m_MenuIdAMXAdjust);
//--
//--	flag = PrinterEnabled;
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,CFG_PRINTER_ENABLED,&flag,binsize);
//--	SysReg.SetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY, CFG_PRINTER_PORT, m_MenuIdPrinterPort);
//--	SysReg.SetStringValue(HKEY_CURRENT_USER,CFG_REG_KEY, CFG_PRINTER_FILE, m_PrinterFileName);
//--
//--	*binsize=2;
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"Tape Clock Speed",&TapeClockSpeed,binsize);
//--	*binsize=1;
//--	flag=UnlockTape;
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"UnlockTape",&flag,binsize);
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"SerialPortEnabled",&SerialPortEnabled,binsize);
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"SerialPort",&SerialPort,binsize);
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"EconetEnabled",&EconetEnabled,binsize); //Rob
//--	flag=SpeechDefault;
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"SpeechEnabled",&flag,binsize);
//--
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"TorchTube",&TorchTube,binsize);
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"TubeEnabled",&TubeEnabled,binsize);
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"OpCodes",&OpCodes,binsize);
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"Basic Hardware",&BHardware,binsize);
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"Teletext Half Mode",&THalfMode,binsize);
//--	SysReg.SetBinaryValue(HKEY_CURRENT_USER,CFG_REG_KEY,"SoundBlockSize",&SBSize,binsize);
//--
//--	SysReg.SetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY,"CaptureResolution",m_MenuIdAviResolution);
//--	SysReg.SetDWORDValue(HKEY_CURRENT_USER,CFG_REG_KEY,"FrameSkip",m_MenuIdAviSkip);
}

void BeebWin::SaveOnExit(void)
{
//--	char TmpPath[256];
//--
//--	// write out cmos ram first
//--	strcpy(TmpPath,RomPath);
//--	strcat(TmpPath,"/beebstate/cmos.ram");
//--	CMDF2=fopen(TmpPath,"wb");
//--	if (CMDF2 != NULL)
//--	{
//--		for(CMA2=0xe;CMA2<64;CMA2++)
//--			fputc(CMOSRAM[CMA2],CMDF2);
//--		fclose(CMDF2);
//--	}
//--
//--	if (!m_isFullScreen)
//--		SaveWindowPos(); // Save window co-ordinates
}
		
/****************************************************************************/
void BeebWin::SetTapeSpeedMenu(void) {
	CheckMenuItem(m_hMenu,ID_TAPE_FAST,(TapeClockSpeed==750)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(m_hMenu,ID_TAPE_MFAST,(TapeClockSpeed==1600)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(m_hMenu,ID_TAPE_MSLOW,(TapeClockSpeed==3200)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(m_hMenu,ID_TAPE_NORMAL,(TapeClockSpeed==5600)?MF_CHECKED:MF_UNCHECKED);
}

/****************************************************************************/
void BeebWin::SetWindowAttributes(bool wasFullScreen)
{
//--	HRESULT ddrval;
//--	RECT wndrect;
//--	RECT scrrect;
//--	long style;
//--
//--	if (m_isFullScreen)
//--	{
//--		if (!wasFullScreen)
//--		{
//--			GetWindowRect(m_hWnd, &wndrect);
//--			m_XWinPos = wndrect.left;
//--			m_YWinPos = wndrect.top;
//--		}
//--
//--		if (m_DirectDrawEnabled)
//--		{
//--			style = GetWindowLong(m_hWnd, GWL_STYLE);
//--			style &= ~(WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX);
//--			style |= WS_POPUP;
//--			SetWindowLong(m_hWnd, GWL_STYLE, style);
//--
//--			if (m_DXInit == TRUE)
//--			{
//--				ResetSurfaces();
//--				ddrval = InitSurfaces();
//--				if( ddrval != DD_OK )
//--				{
//--					char  errstr[200];
//--					sprintf(errstr,"DirectX failure changing screen size\nFailure code %X",ddrval);
//--					MessageBox(m_hWnd,errstr,WindowTitle,MB_OK|MB_ICONERROR);
//--				}
//--			}
//--		}
//--		else
//--		{
//--			style = GetWindowLong(m_hWnd, GWL_STYLE);
//--			style &= ~WS_POPUP;
//--			style |= WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;
//--			SetWindowLong(m_hWnd, GWL_STYLE, style);
//--
//--			SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&scrrect, 0);
//--			SetWindowPos(m_hWnd, HWND_TOP, scrrect.left, scrrect.top,
//--				m_XWinSize + GetSystemMetrics(SM_CXFIXEDFRAME) * 2,
//--				m_YWinSize + GetSystemMetrics(SM_CYFIXEDFRAME) * 2
//--					+ GetSystemMetrics(SM_CYMENUSIZE)
//--					+ GetSystemMetrics(SM_CYCAPTION)
//--					+ 1,
//--				0);
//--		}
//--
//--		// Experiment: hide menu in full screen
//--		if (HideMenuEnabled)
//--			ShowMenu(false);
//--	}
//--	else
//--	{
//--		if (m_DirectDrawEnabled && m_DXInit == TRUE)
//--		{
//--			ResetSurfaces();
//--			ddrval = InitSurfaces();
//--			if( ddrval != DD_OK )
//--			{
//--				char  errstr[200];
//--				sprintf(errstr,"DirectX failure changing screen size\nFailure code %X",ddrval);
//--				MessageBox(m_hWnd,errstr,WindowTitle,MB_OK|MB_ICONERROR);
//--			}
//--		}
//--
//--		style = GetWindowLong(m_hWnd, GWL_STYLE);
//--		style &= ~WS_POPUP;
//--		style |= WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;
//--		SetWindowLong(m_hWnd, GWL_STYLE, style);
//--
//--		SetWindowPos(m_hWnd, HWND_TOP, m_XWinPos, m_YWinPos,
//--			m_XWinSize + GetSystemMetrics(SM_CXFIXEDFRAME) * 2,
//--			m_YWinSize + GetSystemMetrics(SM_CYFIXEDFRAME) * 2
//--				+ GetSystemMetrics(SM_CYMENUSIZE) * (m_MenuIdWinSize == IDM_160X128 ? 2:1)
//--				+ GetSystemMetrics(SM_CYCAPTION)
//--				+ 1,
//--			!wasFullScreen ? SWP_NOMOVE : 0);
//--
//--		// Experiment: hide menu in full screen
//--		if (HideMenuEnabled)
//--			ShowMenu(true);
//--	}
}

/****************************************************************************/
void BeebWin::TranslateAMX(void)
{
	switch (m_MenuIdAMXSize)
	{
	case IDM_AMX_160X256:
		m_AMXXSize = 160;
		m_AMXYSize = 256;
		break;
	default:
	case IDM_AMX_320X256:
		m_AMXXSize = 320;
		m_AMXYSize = 256;
		break;
	case IDM_AMX_640X256:
		m_AMXXSize = 640;
		m_AMXYSize = 256;
		break;
	}

	switch (m_MenuIdAMXAdjust)
	{
	case 0:
		m_AMXAdjust = 0;
		break;
	case IDM_AMX_ADJUSTP50:
		m_AMXAdjust = 50;
		break;
	default:
	case IDM_AMX_ADJUSTP30:
		m_AMXAdjust = 30;
		break;
	case IDM_AMX_ADJUSTP10:
		m_AMXAdjust = 10;
		break;
	case IDM_AMX_ADJUSTM10:
		m_AMXAdjust = -10;
		break;
	case IDM_AMX_ADJUSTM30:
		m_AMXAdjust = -30;
		break;
	case IDM_AMX_ADJUSTM50:
		m_AMXAdjust = -50;
		break;
	}
}

void BeebWin::UpdateSerialMenu(HMENU hMenu) {
	CheckMenuItem(hMenu, ID_SERIAL, (SerialPortEnabled)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_COM1, (SerialPort==1)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_COM2, (SerialPort==2)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_COM3, (SerialPort==3)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_COM4, (SerialPort==4)?MF_CHECKED:MF_UNCHECKED);
}

//Rob
void BeebWin::UpdateEconetMenu(HMENU hMenu) {	
	CheckMenuItem(hMenu, ID_ECONET, (EconetEnabled)?MF_CHECKED:MF_UNCHECKED);
}

void BeebWin::UpdateLEDMenu(HMENU hMenu) {
	// Update the LED Menu
	CheckMenuItem(hMenu,ID_RED_LEDS,(DiscLedColour>0)?MF_UNCHECKED:MF_CHECKED);
	CheckMenuItem(hMenu,ID_GREEN_LEDS,(DiscLedColour>0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,ID_SHOW_KBLEDS,(LEDs.ShowKB)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hMenu,ID_SHOW_DISCLEDS,(LEDs.ShowDisc)?MF_CHECKED:MF_UNCHECKED);
}

void BeebWin::UpdateOptiMenu(void) {
	CheckMenuItem(m_hMenu,ID_DOCONLY,(OpCodes==1)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(m_hMenu,ID_EXTRAS ,(OpCodes==2)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(m_hMenu,ID_FULLSET,(OpCodes==3)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(m_hMenu,ID_BHARDWARE,(BHardware==1)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(m_hMenu,ID_TSTYLE,(THalfMode==1)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(m_hMenu,ID_1773,(SBSize==1)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(m_hMenu,ID_443,(SBSize==0)?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(m_hMenu,ID_PSAMPLES,(PartSamples)?MF_CHECKED:MF_UNCHECKED);
}
/***************************************************************************/
void BeebWin::HandleCommand(int MenuId)
{
# if 0
	char TmpPath[256];
	HRESULT ddrval;
# endif
	HMENU hMenu = m_hMenu;
	int prev_palette_type = palette_type;

	SetSound(MUTED);

	switch (MenuId)
	{
# if 0
	case IDM_RUNDISC:
		if (ReadDisc(0,hMenu))
		{
			m_ShiftBooted = true;
			ResetBeebSystem(MachineType,TubeEnabled,0);
			BeebKeyDown(0, 0); // Shift key
		}
		MustEnableSound=TRUE;
		break;
# endif

	case IDM_LOADSTATE:
		RestoreState();
		MustEnableSound=TRUE;
		break;
	case IDM_SAVESTATE:
		SaveState();
		MustEnableSound=TRUE;
		break;

	case IDM_QUICKLOAD:
		QuickLoad();
		break;
	case IDM_QUICKSAVE:
		QuickSave();
		break;

# if 0 //LUDO:
	case IDM_LOADDISC0:
		ReadDisc(0,hMenu);
		MustEnableSound=TRUE;
		break;
	case IDM_LOADDISC1:
		ReadDisc(1,hMenu);
		MustEnableSound=TRUE;
		break;
# endif
	
	case ID_LOADTAPE:
		LoadTape();
		MustEnableSound=TRUE;
		break;

	case IDM_NEWDISC0:
		NewDiscImage(0);
		MustEnableSound=TRUE;
		break;
	case IDM_NEWDISC1:
		NewDiscImage(1);
		MustEnableSound=TRUE;
		break;

	case IDM_WPDISC0:
		ToggleWriteProtect(0);
		break;
	case IDM_WPDISC1:
		ToggleWriteProtect(1);
		break;

	case IDM_PRINTER_FILE:
		if (PrinterFile())
		{
			/* If printer is enabled then need to
				disable it before changing file */
			if (PrinterEnabled)
				TogglePrinter();

			/* Add file name to menu */
			char menu_string[256];
			strcpy(menu_string, "File: ");
			strcat(menu_string, m_PrinterFileName);
			ModifyMenu(hMenu, IDM_PRINTER_FILE,
				MF_BYCOMMAND, IDM_PRINTER_FILE,
				menu_string);

			if (MenuId != m_MenuIdPrinterPort)
			{
				CheckMenuItem(hMenu, m_MenuIdPrinterPort, MF_UNCHECKED);
				m_MenuIdPrinterPort = MenuId;
				CheckMenuItem(hMenu, m_MenuIdPrinterPort, MF_CHECKED);
   			}
			TranslatePrinterPort();
	}
		MustEnableSound=TRUE;
		break;
	case IDM_PRINTER_LPT1:
	case IDM_PRINTER_LPT2:
	case IDM_PRINTER_LPT3:
	case IDM_PRINTER_LPT4:
		if (MenuId != m_MenuIdPrinterPort)
		{
			/* If printer is enabled then need to
				disable it before changing file */
			if (PrinterEnabled)
				TogglePrinter();

			CheckMenuItem(hMenu, m_MenuIdPrinterPort, MF_UNCHECKED);
			m_MenuIdPrinterPort = MenuId;
			CheckMenuItem(hMenu, m_MenuIdPrinterPort, MF_CHECKED);
			TranslatePrinterPort();
   		}
		break;
	case IDM_PRINTERONOFF:
		TogglePrinter();
		break;

	case ID_SERIAL:
		SerialPortEnabled=1-SerialPortEnabled;
		bSerialStateChanged=TRUE;
		UpdateSerialMenu(hMenu);
		break;
	case ID_COM1:
		SerialPort=1; UpdateSerialMenu(hMenu); bSerialStateChanged=TRUE; break;
	case ID_COM2:
		SerialPort=2; UpdateSerialMenu(hMenu); bSerialStateChanged=TRUE; break;
	case ID_COM3:
		SerialPort=3; UpdateSerialMenu(hMenu); bSerialStateChanged=TRUE; break;
	case ID_COM4:
		SerialPort=4; UpdateSerialMenu(hMenu); bSerialStateChanged=TRUE; break;

	//Rob
	case ID_ECONET:
		EconetEnabled= 1-EconetEnabled;
		if (EconetEnabled)
		{
			// Need hard reset for DNFS to detect econet HW
			ResetBeebSystem(MachineType,TubeEnabled,0);
			EconetStateChanged=TRUE;
		}
		else
		{
			EconetReset();
		}
		UpdateEconetMenu(hMenu);
		break;

//--	case IDM_DDRAWONOFF:
//--		{
//--		int enabled;
//--		if (m_DirectDrawEnabled == TRUE)
//--		{
//--			m_DirectDrawEnabled = FALSE;
//--			ResetSurfaces();
//--			m_DD2->Release();
//--			m_DD->Release();
//--			TranslateWindowSize();
//--			SetWindowAttributes(m_isFullScreen);
//--			enabled = MF_GRAYED;
//--		}
//--		else
//--		{
//--			m_DirectDrawEnabled = TRUE;
//--			TranslateWindowSize();
//--			SetWindowAttributes(m_isFullScreen);
//--			InitDirectX();
//--			enabled = MF_ENABLED;
//--		}
//--		CheckMenuItem(hMenu, IDM_DDRAWONOFF, m_DirectDrawEnabled ? MF_CHECKED : MF_UNCHECKED);
//--		EnableMenuItem(hMenu, ID_VIEW_DD_640X480, enabled);
//--		EnableMenuItem(hMenu, ID_VIEW_DD_1024X768, enabled);
//--		EnableMenuItem(hMenu, ID_VIEW_DD_1280X1024, enabled);
//--		EnableMenuItem(hMenu, IDM_DDINVIDEORAM, enabled);
//--		EnableMenuItem(hMenu, IDM_DD32ONOFF, enabled);
//--		}
//--		break;
//--
//--	case IDM_DD32ONOFF:
//--		m_isDD32 = !m_isDD32;
//--		if (m_DirectDrawEnabled == TRUE)
//--		{
//--			ResetSurfaces();
//--			ddrval = InitSurfaces();
//--			if( ddrval != DD_OK )
//--			{
//--				char  errstr[200];
//--				sprintf(errstr,"DirectX failure changing buffer RAM\nFailure code %X",ddrval);
//--				MessageBox(m_hWnd,errstr,WindowTitle,MB_OK|MB_ICONERROR);
//--				m_isDD32 = !m_isDD32;
//--			}
//--			else
//--				CheckMenuItem(hMenu, IDM_DD32ONOFF, m_isDD32 ? MF_CHECKED : MF_UNCHECKED);
//--		}
//--		break;

//--	case IDM_DDINVIDEORAM:
//--		m_DDS2InVideoRAM = !m_DDS2InVideoRAM;
//--		if (m_DirectDrawEnabled == TRUE)
//--		{
//--			ResetSurfaces();
//--			ddrval = InitSurfaces();
//--			if( ddrval != DD_OK )
//--			{
//--				char  errstr[200];
//--				sprintf(errstr,"DirectX failure changing buffer RAM\nFailure code %X",ddrval);
//--				MessageBox(m_hWnd,errstr,WindowTitle,MB_OK|MB_ICONERROR);
//--				m_DDS2InVideoRAM = !m_DDS2InVideoRAM;
//--			}
//--			else
//--				CheckMenuItem(hMenu, IDM_DDINVIDEORAM, m_DDS2InVideoRAM ? MF_CHECKED : MF_UNCHECKED);
//--		}
//--		break;

//--	case IDM_160X128:
//--	case IDM_240X192:
//--	case IDM_640X256:
//--	case IDM_320X256:
//--	case IDM_640X512:
//--	case IDM_800X600:
//--	case IDM_1024X768:
//--	case IDM_1024X512:
//--		{
//--			if (m_isFullScreen)
//--				HandleCommand(IDM_FULLSCREEN);
//--			CheckMenuItem(hMenu, m_MenuIdWinSize, MF_UNCHECKED);
//--			m_MenuIdWinSize = MenuId;
//--			CheckMenuItem(hMenu, m_MenuIdWinSize, MF_CHECKED);
//--			TranslateWindowSize();
//--			SetWindowAttributes(m_isFullScreen);
//--		}
//--		break;
//--
//--	case ID_VIEW_DD_640X480:
//--	case ID_VIEW_DD_1024X768:
//--	case ID_VIEW_DD_1280X1024:
//--		{
//--		if (!m_isFullScreen)
//--			HandleCommand(IDM_FULLSCREEN);
//--			if (!m_DirectDrawEnabled) 
//--				// Should not happen since the items are grayed out, but anyway...
//--				HandleCommand(IDM_DDRAWONOFF);
//--
//--			CheckMenuItem(hMenu, m_DDFullScreenMode, MF_UNCHECKED);
//--			m_DDFullScreenMode = MenuId;
//--			CheckMenuItem(hMenu, m_DDFullScreenMode, MF_CHECKED);
//--			TranslateWindowSize();
//--			SetWindowAttributes(m_isFullScreen);
//--		}
//--		break;
//--
	case IDM_FULLSCREEN:
		m_isFullScreen = !m_isFullScreen;
		CheckMenuItem(hMenu, IDM_FULLSCREEN, m_isFullScreen ? MF_CHECKED : MF_UNCHECKED);
		TranslateWindowSize();
		SetWindowAttributes(!m_isFullScreen);
		break;

	case IDM_SPEEDANDFPS:
		if (m_ShowSpeedAndFPS)
		{
			m_ShowSpeedAndFPS = FALSE;
			CheckMenuItem(hMenu, IDM_SPEEDANDFPS, MF_UNCHECKED);
		}
		else
		{
			m_ShowSpeedAndFPS = TRUE;
			CheckMenuItem(hMenu, IDM_SPEEDANDFPS, MF_CHECKED);
		}
		break;
	
	case IDM_SOUNDONOFF:
		if (SoundDefault)
		{
			CheckMenuItem(hMenu, IDM_SOUNDONOFF, MF_UNCHECKED);
			SoundReset();
			SoundDefault=0;
		}
		else
		{
			SoundInit();
			if (SoundEnabled) {
				CheckMenuItem(hMenu, IDM_SOUNDONOFF, MF_CHECKED);
				SoundDefault=1;
			}
		}
		break;
//--	case IDM_SOUNDCHIP:
//--		SoundChipEnabled=1-SoundChipEnabled;
//--		CheckMenuItem(hMenu, IDM_SOUNDCHIP, SoundChipEnabled?MF_CHECKED:MF_UNCHECKED);
//--		break;

	case ID_SFX_RELAY:
		RelaySoundEnabled=1-RelaySoundEnabled;
		CheckMenuItem(hMenu,ID_SFX_RELAY,RelaySoundEnabled?MF_CHECKED:MF_UNCHECKED);
		break;

	case IDM_44100KHZ:
	case IDM_22050KHZ:
	case IDM_11025KHZ:
		if (MenuId != m_MenuIdSampleRate)
		{
			CheckMenuItem(hMenu, m_MenuIdSampleRate, MF_UNCHECKED);
			m_MenuIdSampleRate = MenuId;
			CheckMenuItem(hMenu, m_MenuIdSampleRate, MF_CHECKED);
			TranslateSampleRate();

			if (SoundEnabled)
			{
				SoundReset();
				SoundInit();
			}

			if (SpeechDefault)
			{
				tms5220_stop();
				tms5220_start();
			}
		}
		break;
	
	case IDM_FULLVOLUME:
	case IDM_HIGHVOLUME:
	case IDM_MEDIUMVOLUME:
	case IDM_LOWVOLUME:
		if (MenuId != m_MenuIdVolume)
		{
			CheckMenuItem(hMenu, m_MenuIdVolume, MF_UNCHECKED);
			m_MenuIdVolume = MenuId;
			CheckMenuItem(hMenu, m_MenuIdVolume, MF_CHECKED);
			TranslateVolume();
		}
		break;
	
	/* LRW Added switch individual ROMS Writable ON/OFF */
	case IDM_ALLOWWRITES_ROM0:
	case IDM_ALLOWWRITES_ROM1:
	case IDM_ALLOWWRITES_ROM2:
	case IDM_ALLOWWRITES_ROM3:
	case IDM_ALLOWWRITES_ROM4:
	case IDM_ALLOWWRITES_ROM5:
	case IDM_ALLOWWRITES_ROM6:
	case IDM_ALLOWWRITES_ROM7:
	case IDM_ALLOWWRITES_ROM8:
	case IDM_ALLOWWRITES_ROM9:
	case IDM_ALLOWWRITES_ROMA:
	case IDM_ALLOWWRITES_ROMB:
	case IDM_ALLOWWRITES_ROMC:
	case IDM_ALLOWWRITES_ROMD:
	case IDM_ALLOWWRITES_ROME:
	case IDM_ALLOWWRITES_ROMF:

		CheckMenuItem(hMenu,  MenuId, RomWritable[( MenuId-IDM_ALLOWWRITES_ROM0)] ? MF_UNCHECKED : MF_CHECKED );
		GetRomMenu();	// Update the Rom/Ram state for all the roms.
		break;				

	case IDM_REALTIME:
	case IDM_FIXEDSPEED100:
	case IDM_FIXEDSPEED50:
	case IDM_FIXEDSPEED10:
	case IDM_FIXEDSPEED5:
	case IDM_FIXEDSPEED2:
	case IDM_FIXEDSPEED1_5:
	case IDM_FIXEDSPEED1_25:
	case IDM_FIXEDSPEED1_1:
	case IDM_FIXEDSPEED0_9:
	case IDM_FIXEDSPEED0_5:
	case IDM_FIXEDSPEED0_75:
	case IDM_FIXEDSPEED0_25:
	case IDM_FIXEDSPEED0_1:
	case IDM_50FPS:
	case IDM_25FPS:
	case IDM_10FPS:
	case IDM_5FPS:
	case IDM_1FPS:
		if (MenuId != m_MenuIdTiming)
		{
			CheckMenuItem(hMenu, m_MenuIdTiming, MF_UNCHECKED);
			m_MenuIdTiming = MenuId;
			CheckMenuItem(hMenu, m_MenuIdTiming, MF_CHECKED);
			TranslateTiming();
		}
		break;

	case IDM_JOYSTICK:
	case IDM_MOUSESTICK:
		/* Disable current selection */
		if (m_MenuIdSticks != 0)
		{
			CheckMenuItem(hMenu, m_MenuIdSticks, MF_UNCHECKED);
			if (m_MenuIdSticks == IDM_JOYSTICK)
			{
				ResetJoystick();
			}
			else /* mousestick */
			{
				AtoDReset();
			}
		}

		if (MenuId == m_MenuIdSticks)
		{
			/* Joysticks switched off completely */
			m_MenuIdSticks = 0;
		}
		else
		{
			/* Initialise new selection */
			m_MenuIdSticks = MenuId;
			if (m_MenuIdSticks == IDM_JOYSTICK)
			{
				InitJoystick();
			}
			else /* mousestick */
			{
				AtoDInit();
			}
			if (JoystickEnabled)
				CheckMenuItem(hMenu, m_MenuIdSticks, MF_CHECKED);
			else
				m_MenuIdSticks = 0;
		}
		break;

	case IDM_FREEZEINACTIVE:
		if (m_FreezeWhenInactive)
		{
			m_FreezeWhenInactive = FALSE;
			CheckMenuItem(hMenu, IDM_FREEZEINACTIVE, MF_UNCHECKED);
		}
		else
		{
			m_FreezeWhenInactive = TRUE;
			CheckMenuItem(hMenu, IDM_FREEZEINACTIVE, MF_CHECKED);
		}
		break;

	case IDM_HIDECURSOR:
		if (m_HideCursor)
		{
			m_HideCursor = FALSE;
			CheckMenuItem(hMenu, IDM_HIDECURSOR, MF_UNCHECKED);
		}
		else
		{
			m_HideCursor = TRUE;
			CheckMenuItem(hMenu, IDM_HIDECURSOR, MF_CHECKED);
		}
		break;

	case IDM_IGNOREILLEGALOPS:
		if (IgnoreIllegalInstructions)
		{
			IgnoreIllegalInstructions = FALSE;
			CheckMenuItem(hMenu, IDM_IGNOREILLEGALOPS, MF_UNCHECKED);
		}
		else
		{
			IgnoreIllegalInstructions = TRUE;
			CheckMenuItem(hMenu, IDM_IGNOREILLEGALOPS, MF_CHECKED);
		}
		break;

	case IDM_DEFINEKEYMAP:
		UserKeyboardDialog( m_hWnd );
		MustEnableSound=TRUE;
		break;

	case IDM_USERKYBDMAPPING:
	case IDM_DEFAULTKYBDMAPPING:
	case IDM_LOGICALKYBDMAPPING:
		if (MenuId != m_MenuIdKeyMapping)
		{
			CheckMenuItem(hMenu, m_MenuIdKeyMapping, MF_UNCHECKED);
			m_MenuIdKeyMapping = MenuId;
			CheckMenuItem(hMenu, m_MenuIdKeyMapping, MF_CHECKED);
			TranslateKeyMapping();
		}
		break;

	case IDM_MAPAS:
		if (m_KeyMapAS)
		{
			m_KeyMapAS = FALSE;
			CheckMenuItem(hMenu, IDM_MAPAS, MF_UNCHECKED);
		}
		else
		{
			m_KeyMapAS = TRUE;
			CheckMenuItem(hMenu, IDM_MAPAS, MF_CHECKED);
		}
		break;
	case IDM_MAPFUNCS:
		if (m_KeyMapFunc)
		{
			m_KeyMapFunc = FALSE;
			CheckMenuItem(hMenu, IDM_MAPFUNCS, MF_UNCHECKED);
		}
		else
		{
			m_KeyMapFunc = TRUE;
			CheckMenuItem(hMenu, IDM_MAPFUNCS, MF_CHECKED);
		}
		break;

//--	case IDM_ABOUT:
//--		MessageBox(m_hWnd, AboutText, WindowTitle, MB_OK);
//--		break;

//--	case IDM_VIEWREADME:
//--		strcpy(TmpPath, m_AppPath);
//--		strcat(TmpPath, "/README.txt");
//--		ShellExecute(m_hWnd, NULL, TmpPath, NULL, NULL, SW_SHOWNORMAL);;
//--		break;

//--	case IDM_EXIT:
//--		PostMessage(m_hWnd, WM_CLOSE, 0, 0L);
//--		break;

	case IDM_SAVE_PREFS:
		SavePreferences();
		break;

	case IDM_AMXONOFF:
		if (AMXMouseEnabled)
		{
			CheckMenuItem(hMenu, IDM_AMXONOFF, MF_UNCHECKED);
			AMXMouseEnabled = FALSE;
		}
		else
		{
			CheckMenuItem(hMenu, IDM_AMXONOFF, MF_CHECKED);
			AMXMouseEnabled = TRUE;
		}
		break;

	case IDM_AMX_LRFORMIDDLE:
		if (AMXLRForMiddle)
		{
			CheckMenuItem(hMenu, IDM_AMX_LRFORMIDDLE, MF_UNCHECKED);
			AMXLRForMiddle = FALSE;
		}
		else
		{
			CheckMenuItem(hMenu, IDM_AMX_LRFORMIDDLE, MF_CHECKED);
			AMXLRForMiddle = TRUE;
		}
		break;

	case IDM_AMX_160X256:
	case IDM_AMX_320X256:
	case IDM_AMX_640X256:
		if (MenuId != m_MenuIdAMXSize)
		{
			CheckMenuItem(hMenu, m_MenuIdAMXSize, MF_UNCHECKED);
			m_MenuIdAMXSize = MenuId;
			CheckMenuItem(hMenu, m_MenuIdAMXSize, MF_CHECKED);
   		}
		TranslateAMX();
		break;

	case IDM_AMX_ADJUSTP50:
	case IDM_AMX_ADJUSTP30:
	case IDM_AMX_ADJUSTP10:
	case IDM_AMX_ADJUSTM10:
	case IDM_AMX_ADJUSTM30:
	case IDM_AMX_ADJUSTM50:
		if (m_MenuIdAMXAdjust != 0)
		{
			CheckMenuItem(hMenu, m_MenuIdAMXAdjust, MF_UNCHECKED);
		}

		if (MenuId != m_MenuIdAMXAdjust)
		{
			m_MenuIdAMXAdjust = MenuId;
			CheckMenuItem(hMenu, m_MenuIdAMXAdjust, MF_CHECKED);
   		}
		else
		{
			m_MenuIdAMXAdjust = 0;
		}
		TranslateAMX();
		break;

	case ID_MONITOR_RGB:
		palette_type = RGB;
		CreateBitmap();
		break;
	case ID_MONITOR_BW:
		palette_type = BW;
		CreateBitmap();
		break;
	case ID_MONITOR_GREEN:
		palette_type = GREEN;
		CreateBitmap();
		break;
	case ID_MONITOR_AMBER:
		palette_type = AMBER;
		CreateBitmap();
		break;

	case IDM_TUBE:
		TubeEnabled=1-TubeEnabled;
        TorchTube = 0;
        CheckMenuItem(hMenu, IDM_TUBE, (TubeEnabled)?MF_CHECKED:MF_UNCHECKED);
        CheckMenuItem(hMenu, IDM_TORCH, (TorchTube)?MF_CHECKED:MF_UNCHECKED);
		ResetBeebSystem(MachineType,TubeEnabled,0);
		break;

	case IDM_TORCH:
		TorchTube=1-TorchTube;
		TubeEnabled=0;
		CheckMenuItem(hMenu, IDM_TUBE, (TubeEnabled)?MF_CHECKED:MF_UNCHECKED);
		CheckMenuItem(hMenu, IDM_TORCH, (TorchTube)?MF_CHECKED:MF_UNCHECKED);
		ResetBeebSystem(MachineType,TubeEnabled,0);
		break;

    
    case ID_FILE_RESET:
		ResetBeebSystem(MachineType,TubeEnabled,0);
		break;

	case ID_MODELB:
		if (MachineType!=0)
		{
			ResetBeebSystem(0,EnableTube,1);
			UpdateModelType();
		}
		break;
	case ID_MODELBINT:
		if (MachineType!=1)
		{
			ResetBeebSystem(1,EnableTube,1);
			UpdateModelType();
		}
		break;
	case ID_MODELBP:
		if (MachineType!=2)
		{
			ResetBeebSystem(2,EnableTube,1);
			UpdateModelType();
		}
		break;
	case ID_MASTER128:
		if (MachineType!=3)
		{
			ResetBeebSystem(3,EnableTube,1);
			UpdateModelType();
		}
		break;

	case ID_REWINDTAPE:
		RewindTape();
		break;
	case ID_UNLOCKTAPE:
		UnlockTape=1-UnlockTape;
		SetUnlockTape(UnlockTape);
		CheckMenuItem(hMenu, ID_UNLOCKTAPE, (UnlockTape)?MF_CHECKED:MF_UNCHECKED);
		break;

	case ID_HIDEMENU:
		HideMenuEnabled=1-HideMenuEnabled;
		CheckMenuItem(hMenu, ID_HIDEMENU, (HideMenuEnabled)?MF_CHECKED:MF_UNCHECKED);
		break;

	case ID_RED_LEDS:
		DiscLedColour=0;
		UpdateLEDMenu(hMenu);
		break;
	case ID_GREEN_LEDS:
		DiscLedColour=1;
		UpdateLEDMenu(hMenu);
		break;
	case ID_SHOW_KBLEDS:
		LEDs.ShowKB=!LEDs.ShowKB;
		UpdateLEDMenu(hMenu);
		break;
	case ID_SHOW_DISCLEDS:
		LEDs.ShowDisc=!LEDs.ShowDisc;
		UpdateLEDMenu(hMenu);
		break;

	case ID_FDC_DLL:
		if (MachineType != 3)
			SelectFDC();
		MustEnableSound=TRUE;
		break;
	case ID_8271:
		KillDLLs();
		NativeFDC=TRUE;
		CheckMenuItem(m_hMenu,ID_8271,MF_CHECKED);
		CheckMenuItem(m_hMenu,ID_FDC_DLL,MF_UNCHECKED);
//--		if (MachineType != 3)
//--		{
//--			char CfgName[20];
//--			if (MachineType == 0)
//--				strcpy(CfgName, "FDCDLL");
//--			else
//--				sprintf(CfgName, "FDCDLL%d", MachineType);
//--			SysReg.SetStringValue(HKEY_CURRENT_USER,CFG_REG_KEY,CfgName,"None");
//--		}
		break;

	case ID_TAPE_FAST:
		SetTapeSpeed(750);
		SetTapeSpeedMenu();
		break;
	case ID_TAPE_MFAST:
		SetTapeSpeed(1600);
		SetTapeSpeedMenu();
		break;
	case ID_TAPE_MSLOW:
		SetTapeSpeed(3200);
		SetTapeSpeedMenu();
		break;
	case ID_TAPE_NORMAL:
		SetTapeSpeed(5600);
		SetTapeSpeedMenu();
		break;
	case ID_TAPESOUND:
		TapeSoundEnabled=!TapeSoundEnabled;
		CheckMenuItem(m_hMenu,ID_TAPESOUND,(TapeSoundEnabled)?MF_CHECKED:MF_UNCHECKED);
		break;

//--	case ID_TAPECONTROL:
//--		if (TapeControlEnabled)
//--			TapeControlCloseDialog();
//--		else
//--			TapeControlOpenDialog(hInst, m_hWnd);
//--		break;

	case ID_HOSTCLOCK:
		UseHostClock=1-UseHostClock;
		CheckMenuItem(m_hMenu,ID_HOSTCLOCK,(UseHostClock)?MF_CHECKED:MF_UNCHECKED);
		SoundReset();
		if (SoundDefault) SoundInit();
		break;
	case ID_PBUFF:
		UsePrimaryBuffer=1-UsePrimaryBuffer;
		SetPBuff();
		SoundReset();
		if (SoundDefault) SoundInit();
		break;
	case ID_TSTYLE:
		THalfMode=1-THalfMode;
		UpdateOptiMenu();
		break;

	case ID_DOCONLY:
		OpCodes=1;
		UpdateOptiMenu();
		break;
	case ID_EXTRAS:
		OpCodes=2;
		UpdateOptiMenu();
		break;
	case ID_FULLSET:
		OpCodes=3;
		UpdateOptiMenu();
		break;
	case ID_BHARDWARE:
		BHardware=1-BHardware;
		UpdateOptiMenu();
		break;
	case ID_PSAMPLES:
		PartSamples=1-PartSamples;
		UpdateOptiMenu();
		break;
	case ID_1773:
		SBSize=1;
		UpdateOptiMenu();
		break;
	case ID_443:
		SBSize=0;
		UpdateOptiMenu();
		break;

//--	case IDM_SHOWDEBUGGER:
//--		break;

	case IDM_BLUR_OFF:
	case IDM_BLUR_2:
	case IDM_BLUR_4:
	case IDM_BLUR_8:
		if (MenuId != m_MotionBlur)
		{
			CheckMenuItem(hMenu, m_MotionBlur, MF_UNCHECKED);
			m_MotionBlur = MenuId;
			CheckMenuItem(hMenu, m_MotionBlur, MF_CHECKED);
		}
		break;

	case IDM_VIDEORES1:
	case IDM_VIDEORES2:
	case IDM_VIDEORES3:
		if (MenuId != m_MenuIdAviResolution)
		{
			CheckMenuItem(hMenu, m_MenuIdAviResolution, MF_UNCHECKED);
			m_MenuIdAviResolution = MenuId;
			CheckMenuItem(hMenu, m_MenuIdAviResolution, MF_CHECKED);
		}
		break;
	case IDM_VIDEOSKIP0:
	case IDM_VIDEOSKIP1:
	case IDM_VIDEOSKIP2:
	case IDM_VIDEOSKIP3:
	case IDM_VIDEOSKIP4:
	case IDM_VIDEOSKIP5:
		if (MenuId != m_MenuIdAviSkip)
		{
			CheckMenuItem(hMenu, m_MenuIdAviSkip, MF_UNCHECKED);
			m_MenuIdAviSkip = MenuId;
			CheckMenuItem(hMenu, m_MenuIdAviSkip, MF_CHECKED);
		}
		break;
//--	case IDM_CAPTUREVIDEO:
//--		CaptureVideo();
//--		break;
//--	case IDM_ENDVIDEO:
//--		if (aviWriter != NULL)
//--		{
//--			delete aviWriter;
//--			aviWriter = NULL;
//--		}
//--		break;

	case IDM_SPEECH:
		if (SpeechDefault)
		{
			CheckMenuItem(hMenu, IDM_SPEECH, MF_UNCHECKED);
			tms5220_stop();
			SpeechDefault = 0;
		}
		else
		{
			tms5220_start();
			if (SpeechEnabled)
			{
				CheckMenuItem(hMenu, IDM_SPEECH, MF_CHECKED);
				SpeechDefault = 1;
			}
		}
		break;
	}

	SetSound(UNMUTED);
	if (palette_type != prev_palette_type)
	{
		CreateBitmap();
		UpdateMonitorMenu();
	}
}

void BeebWin::SetPBuff(void) {
	CheckMenuItem(m_hMenu,ID_PBUFF,(UsePrimaryBuffer)?MF_CHECKED:MF_UNCHECKED);
}

void BeebWin::SetSoundMenu(void) {
	CheckMenuItem(m_hMenu,IDM_SOUNDONOFF,(SoundEnabled)?MF_CHECKED:MF_UNCHECKED);
	SetPBuff();
}

void BeebWin::Focus(BOOL gotit)
{
	if (gotit)
		m_frozen = FALSE;
	else
		if (m_FreezeWhenInactive)
			m_frozen = TRUE;
}

BOOL BeebWin::IsFrozen(void)
{
	return m_frozen;
}

void SaveEmuUEF(FILE *SUEF) {
	char EmuName[16];
	fput16(0x046C,SUEF);
	fput32(16,SUEF);
	// BeebEm Title Block
	strcpy(EmuName,"BeebEm");
	EmuName[14]=1;
	EmuName[15]=4; // Version, 1.4
	fwrite(EmuName,16,1,SUEF);
	//
	fput16(0x046a,SUEF);
	fput32(16,SUEF);
	// Emulator Specifics
	// Note about this block: It should only be handled by beebem from uefstate.cpp if
	// the UEF has been determined to be from BeebEm (Block 046C)
	fputc(MachineType,SUEF);
	fputc((NativeFDC)?0:1,SUEF);
	fputc(TubeEnabled,SUEF);
	fputc(0,SUEF); // Monitor type, reserved
	fputc(0,SUEF); // Speed Setting, reserved
	fput32(0,SUEF);
	fput32(0,SUEF);
	fput16(0,SUEF);
	fputc(0,SUEF);
}

void LoadEmuUEF(FILE *SUEF, int Version) {
	MachineType=fgetc(SUEF);
	if (Version <= 8 && MachineType == 1)
		MachineType = 3;
	NativeFDC=(fgetc(SUEF)==0)?TRUE:FALSE;
	TubeEnabled=fgetc(SUEF);
	mainWin->ResetBeebSystem(MachineType,TubeEnabled,1);
	mainWin->UpdateModelType();
}

/*****************************************************************************/
// Handle a file 
	
int 
BeebWin::HandleFile(char *FileName, int disk_id)
{
	bool ssd = false;
	bool dsd = false;
	bool adfs = false;
	bool cont = false;
	char TmpPath[256];
	bool uef = false;
  bool img  = false;
	
	// Work out which type of files it is
	char *ext = strrchr(FileName, '.');
	if (ext != NULL)
	{
		qDEBUG("Has file extension");

		cont = true;
		if (stricmp(ext+1, "ssd") == 0)
			ssd = true;
		else if (stricmp(ext+1, "dsd") == 0)
			dsd = true;
		else if (stricmp(ext+1, "adl") == 0)
			adfs = true;
		else if (stricmp(ext+1, "adf") == 0)
			adfs = true;
		else if (stricmp(ext+1, "uef") == 0)
			uef = true;
		else if (stricmp(ext+1, "img") == 0)
			img = true;
		else
			cont = false;
	}
	

	if (cont)
	{
		cont = false;

		pDEBUG(dL"Testing file as: '%s'", dR, FileName);

		FILE *fd = fopen(FileName, "rb");
		if (fd != NULL)
		{
			cont = true;
			fclose(fd);

			qDEBUG("Is OK");
		}
		else if (uef)
		{
			// Try getting it from BeebState directory
			strcpy(TmpPath, m_AppPath);
			strcat(TmpPath, "/state/");
			strcat(TmpPath, FileName);
			FILE *fd = fopen(FileName, "rb");
			if (fd != NULL)
			{
				cont = true;
				FileName = TmpPath;
				fclose(fd);
			}
		}
		else
		{
			// Try getting it from DiscIms directory
			strcpy(TmpPath, m_AppPath);
			strcat(TmpPath, "/discs/");
			strcat(TmpPath, FileName);

			pDEBUG(dL"Testing file as: '%s'", dR, TmpPath);

			FILE *fd = fopen(TmpPath, "rb");
			if (fd != NULL)
			{
				cont = true;
				FileName = TmpPath;
				fclose(fd);

				pDEBUG("Is OK");
			}
		}
	}

	if (cont)
	{
		if (uef)
		{
			qDEBUG("Load UEF State");
			LoadUEFState(FileName);
			cont = false;

      return 0;
		}
    else
    if (img)
    {
      ReadDisc(disk_id,FileName);
			cont = false;

      return 0;
    }
	}

	if (cont)
	{
		if (MachineType!=3)
		{
			if (dsd)
			{
				if (NativeFDC){
					qDEBUG("Load Simple DSD Disc");
					LoadSimpleDSDiscImage(FileName, disk_id, 80);
				}else{
					qDEBUG("Load 1770 DSD Disc");
					Load1770DiscImage(FileName,disk_id,1,m_hMenu); // 1 = dsd
				}
			}
			if (ssd)
			{
				if (NativeFDC){
					qDEBUG("Load Simple SSD Disc");
					LoadSimpleDiscImage(FileName, disk_id, 0, 80);
				}else{
					qDEBUG("Load 1770 SSD Disc");
					Load1770DiscImage(FileName, disk_id,0,m_hMenu); // 0 = ssd
				}
			}
			if (adfs)
			{
				if (!NativeFDC){
					qDEBUG("Load Simple ADFS Disc");
					Load1770DiscImage(FileName, disk_id,2,m_hMenu); // 2 = adfs
				}else{
					qDEBUG("Cannot load ADFS with native DFS");
					cont = false;  // cannot load adfs with native DFS
				}
			}
		}
		else if (MachineType==3)
		{
			if (dsd){
				qDEBUG("Load 1770 DSD Disc (for Master)");
				Load1770DiscImage(FileName, disk_id,1,m_hMenu); // 0 = ssd
			}
			if (ssd){
				qDEBUG("Load 1770 SSD Disc (for Master)");
				Load1770DiscImage(FileName, disk_id,0,m_hMenu); // 1 = dsd
			}
			if (adfs){
				qDEBUG("Load 1770 ADFS Disc (for Master)");
				Load1770DiscImage(FileName, disk_id,2,m_hMenu); // 2 = adfs
			}
		}
	}

	if (cont)
	{
		// Do a shift + break
		qDEBUG("Doing a shift + break reset");
		mainWin->ResetBeebSystem(MachineType,TubeEnabled,0);
		BeebKeyDown(0, 0); // Shift key
		m_ShiftBooted = true;

    return 0;
	}

  return 1;
}

/****************************************************************************/
// if DLLName is NULL then FDC setting is read from the registry
// else the named DLL is read in
// if save is true then DLL selection is saved in registry
void BeebWin::LoadFDC(char *DLLName, bool save) {

	qDEBUG("Tried to load DFC");

//--	char CfgName[20];
//--
//--	if (MachineType == 0)
//--		strcpy(CfgName, "FDCDLL");
//--	else
//--		sprintf(CfgName, "FDCDLL%d", MachineType);
//--
//--	if (hFDCBoard!=NULL) FreeLibrary(hFDCBoard); 
//--	hFDCBoard=NULL; NativeFDC=TRUE;
//--
//--	if (DLLName == NULL) {
//--		RegRes=SysReg.GetStringValue(HKEY_CURRENT_USER,CFG_REG_KEY,CfgName,FDCDLL);
//--		if (!RegRes) {
//--			strcpy(FDCDLL,"None");
//--			if (MachineType == 2) {
//--				// Default B+ to Acorn FDC
//--				sprintf(FDCDLL, "%sHardware\\Acorn1770.dll", m_AppPath);
//--			}
//--			SysReg.SetStringValue(HKEY_CURRENT_USER,CFG_REG_KEY,CfgName,FDCDLL);
//--		}
//--		DLLName = FDCDLL;
//--	}
//--
//--	if (strcmp(DLLName, "None")) {
//--		hFDCBoard=LoadLibrary(DLLName);
//--		if (hFDCBoard==NULL) {
//--			MessageBox(GETHWND,"Unable to load FDD Extension Board DLL\nReverting to native 8271\n",WindowTitle,MB_OK|MB_ICONERROR); 
//--			strcpy(DLLName, "None");
//--		}
//--		else {
//--			PGetBoardProperties=(lGetBoardProperties) GetProcAddress(hFDCBoard,"GetBoardProperties");
//--			PSetDriveControl=(lSetDriveControl) GetProcAddress(hFDCBoard,"SetDriveControl");
//--			PGetDriveControl=(lGetDriveControl) GetProcAddress(hFDCBoard,"GetDriveControl");
//--			if ((PGetBoardProperties==NULL) || (PSetDriveControl==NULL) || (PGetDriveControl==NULL)) {
//--				MessageBox(GETHWND,"Invalid FDD Extension Board DLL\nReverting to native 8271\n",WindowTitle,MB_OK|MB_ICONERROR);
//--				strcpy(DLLName, "None");
//--			}
//--			else {
//--				PGetBoardProperties(&ExtBoard);
//--				EFDCAddr=ExtBoard.FDCAddress;
//--				EDCAddr=ExtBoard.DCAddress;
//--				NativeFDC=FALSE; // at last, a working DLL!
//--				InvertTR00=ExtBoard.TR00_ActiveHigh;
//--			}
//--		} 
//--	}
//--
//--	if (save)
//--		SysReg.SetStringValue(HKEY_CURRENT_USER,CFG_REG_KEY,CfgName,DLLName);
//--
//--	// Set menu options
//--	if (NativeFDC) {
//--		CheckMenuItem(m_hMenu,ID_8271,MF_CHECKED);
//--		CheckMenuItem(m_hMenu,ID_FDC_DLL,MF_UNCHECKED);
//--	} else {
//--		CheckMenuItem(m_hMenu,ID_8271,MF_UNCHECKED);
//--		CheckMenuItem(m_hMenu,ID_FDC_DLL,MF_CHECKED);
//--	}
//--
//--	DisplayCycles=7000000;
//--	if ((NativeFDC) || (MachineType==3))
//--		DisplayCycles=0;
}

void BeebWin::KillDLLs(void) {

	qDEBUG("Tried to Kill FDC");

//--	if (hFDCBoard!=NULL)
//--		FreeLibrary(hFDCBoard);
//--	hFDCBoard=NULL;
}

void BeebWin::SetDriveControl(unsigned char value) {
	// This takes a value from the mem/io decoder, as written by the cpu, runs it through the 
	// DLL's translator, then sends it on to the 1770 FDC in master 128 form.
	WriteFDCControlReg(PSetDriveControl(value));
}

unsigned char BeebWin::GetDriveControl(void) {
	// Same as above, but in reverse, i.e. reading
	unsigned char temp,temp2;
	temp=ReadFDCControlReg();
	temp2=PGetDriveControl(temp);
	return(temp2);
}

/****************************************************************************/
void BeebWin::QuickLoad()
{
	char FileName[_MAX_PATH];
	strcpy(FileName, m_AppPath);
	strcat(FileName, "beebstate\\quicksave.uef");
	LoadUEFState(FileName);
}

void BeebWin::QuickSave()
{
	char FileName1[_MAX_PATH];
	char FileName2[_MAX_PATH];
	int i;

	// Bump old quicksave files down
	for (i = 1; i <= 9; ++i)
	{
		sprintf(FileName1, "%sbeebstate\\quicksave%d.uef", m_AppPath, i);

		if (i == 9)
			sprintf(FileName2, "%sbeebstate\\quicksave.uef", m_AppPath);
		else
			sprintf(FileName2, "%sbeebstate\\quicksave%d.uef", m_AppPath, i+1);

		MoveFileEx(FileName2, FileName1, MOVEFILE_REPLACE_EXISTING);
	}

	SaveUEFState(FileName2);
}

extern "C" {

void
beeb_emulator_reset()
{
  mainWin->HandleCommand(ID_FILE_RESET);
}

//Get functions

char *
beeb_get_save_name()
{
  return mainWin->m_save_name;
}

BBC_save_t*
beeb_get_save_state(int cur_id)
{
  return &mainWin->m_save_state[cur_id];
}

int   
beeb_get_snd_enabled()
{
  return mainWin->m_snd_enable;
}

int
beeb_get_scr_cpu()
{
  return mainWin->m_scr_cpu;
}

int
beeb_get_render_mode()
{
  return mainWin->m_render_mode;
}

int
beeb_get_view_fps()
{
  return mainWin->m_view_fps;
}

int
beeb_get_display_lr()
{
  return mainWin->m_display_lr;
}

int
beeb_get_skip_max_frame()
{
  return mainWin->m_skip_max_frame;
}

int
beeb_get_psp_cpu_clock()
{
  return mainWin->m_psp_cpu_clock;
}

int
beeb_get_psp_reverse_analog()
{
  return mainWin->m_psp_reverse_analog;
}

int
beeb_get_psp_screenshot_id()
{
  return mainWin->m_psp_screenshot_id;
}

//Set functions

void
beeb_set_snd_enabled(int value)
{
  mainWin->m_snd_enable = (value != 0);
}

void
beeb_set_scr_cpu(int value)
{
  mainWin->m_scr_cpu = (value != 0);
}

void
beeb_set_render_mode(int value)
{
  mainWin->m_render_mode = value;
}

void
beeb_set_view_fps(int value)
{
  mainWin->m_view_fps = value;
}

void
beeb_set_display_lr(int value)
{
  mainWin->m_display_lr = value;
}

void
beeb_set_skip_max_frame(int value)
{
  mainWin->m_skip_max_frame = value;
  mainWin->m_skip_cur_frame = 0;
}

void
beeb_set_psp_cpu_clock(int value) 
{
  mainWin->m_psp_cpu_clock = value;
}

void
beeb_set_psp_reverse_analog(int value)
{
  mainWin->m_psp_reverse_analog = (value != 0);
}

void
beeb_set_psp_screenshot_id(int value)
{
  mainWin->m_psp_screenshot_id = value;
}

//Load Functions

typedef struct {
   char *pchZipFile;
   char *pchExtension;
   char *pchFileNames;
   char *pchSelection;
   int iFiles;
   unsigned int dwOffset;
} t_zip_info;

t_zip_info zip_info;

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;

#define ERR_FILE_NOT_FOUND       13
#define ERR_FILE_BAD_ZIP         14
#define ERR_FILE_EMPTY_ZIP       15
#define ERR_FILE_UNZIP_FAILED    16

FILE *pfileObject;
char *pbGPBuffer = NULL;

static dword
loc_get_dword(byte *buff)
{
  return ( (((dword)buff[3]) << 24) |
           (((dword)buff[2]) << 16) |
           (((dword)buff[1]) <<  8) |
           (((dword)buff[0]) <<  0) );
}

static void
loc_set_dword(byte *buff, dword value)
{
  buff[3] = (value >> 24) & 0xff;
  buff[2] = (value >> 16) & 0xff;
  buff[1] = (value >>  8) & 0xff;
  buff[0] = (value >>  0) & 0xff;
}

static word
loc_get_word(byte *buff)
{
  return( (((word)buff[1]) <<  8) |
          (((word)buff[0]) <<  0) );
}


int 
zip_dir(t_zip_info *zi)
{
   int n, iFileCount;
   long lFilePosition;
   dword dwCentralDirPosition, dwNextEntry;
   word wCentralDirEntries, wCentralDirSize, wFilenameLength;
   byte *pbPtr;
   char *pchStrPtr;
   dword dwOffset;

   iFileCount = 0;
   if ((pfileObject = fopen(zi->pchZipFile, "rb")) == NULL) {
      return ERR_FILE_NOT_FOUND;
   }

   if (pbGPBuffer == (char *)0) {
     pbGPBuffer = (char *)malloc( sizeof(byte) * 128*1024); 
   }

   wCentralDirEntries = 0;
   wCentralDirSize = 0;
   dwCentralDirPosition = 0;
   lFilePosition = -256;
   do {
      fseek(pfileObject, lFilePosition, SEEK_END);
      if (fread(pbGPBuffer, 256, 1, pfileObject) == 0) {
         fclose(pfileObject);
         return ERR_FILE_BAD_ZIP; // exit if loading of data chunck failed
      }
      pbPtr = (byte*)(pbGPBuffer + (256 - 22)); // pointer to end of central directory (under ideal conditions)
      while (pbPtr != (byte *)pbGPBuffer) {
         if (loc_get_dword(pbPtr) == 0x06054b50) { // check for end of central directory signature
            wCentralDirEntries = loc_get_word(pbPtr + 10);
            wCentralDirSize = loc_get_word(pbPtr + 12);
            dwCentralDirPosition = loc_get_dword(pbPtr + 16);
            break;
         }
         pbPtr--; // move backwards through buffer
      }
      lFilePosition -= 256; // move backwards through ZIP file
   } while (wCentralDirEntries == 0);
   if (wCentralDirSize == 0) {
      fclose(pfileObject);
      return ERR_FILE_BAD_ZIP; // exit if no central directory was found
   }
   fseek(pfileObject, dwCentralDirPosition, SEEK_SET);
   if (fread(pbGPBuffer, wCentralDirSize, 1, pfileObject) == 0) {
      fclose(pfileObject);
      return ERR_FILE_BAD_ZIP; // exit if loading of data chunck failed
   }

   pbPtr = (byte *)pbGPBuffer;
   if (zi->pchFileNames) {
      free(zi->pchFileNames); // dealloc old string table
   }
   zi->pchFileNames = (char *)malloc(wCentralDirSize); // approximate space needed by using the central directory size
   pchStrPtr = zi->pchFileNames;

   for (n = wCentralDirEntries; n; n--) {
      wFilenameLength = loc_get_word(pbPtr + 28);
      dwOffset = loc_get_dword(pbPtr + 42);
      dwNextEntry = wFilenameLength + loc_get_word(pbPtr + 30) + loc_get_word(pbPtr + 32);
      pbPtr += 46;
      char *pchThisExtension = zi->pchExtension;
      while (*pchThisExtension != '\0') { // loop for all extensions to be checked
         if (strncasecmp((char *)pbPtr + (wFilenameLength - 4), pchThisExtension, 4) == 0) {
            strncpy(pchStrPtr, (char *)pbPtr, wFilenameLength); // copy filename from zip directory
            pchStrPtr[wFilenameLength] = 0; // zero terminate string
            pchStrPtr += wFilenameLength+1;
            loc_set_dword((byte*)pchStrPtr, dwOffset);
            pchStrPtr += 4;
            iFileCount++;
            break;
         }
         pchThisExtension += 4; // advance to next extension
      }
      pbPtr += dwNextEntry;
   }
   fclose(pfileObject);

   if (iFileCount == 0) { // no files found?
      return ERR_FILE_EMPTY_ZIP;
   }

   zi->iFiles = iFileCount;
   return 0; // operation completed successfully
}

int 
zip_extract(char *pchZipFile, char *pchFileName, dword dwOffset, char *ext)
{
   int iStatus, iCount;
   dword dwSize;
   byte *pbInputBuffer, *pbOutputBuffer;
   FILE *pfileOut, *pfileIn;
   z_stream z;

   strcpy(pchFileName, chAppPath);
   strcat(pchFileName, "/unzip.");
   strcat(pchFileName, ext);

   if (!(pfileOut = fopen(pchFileName, "wb"))) {
      return ERR_FILE_UNZIP_FAILED; // couldn't create output file
   }
   if (pbGPBuffer == (char *)0) {
     pbGPBuffer = (char *)malloc( sizeof(byte) * 128*1024); 
   }
   pfileIn = fopen(pchZipFile, "rb"); // open ZIP file for reading
   fseek(pfileIn, dwOffset, SEEK_SET); // move file pointer to beginning of data block
   fread(pbGPBuffer, 30, 1, pfileIn); // read local header
   dwSize = loc_get_dword((byte *)(pbGPBuffer + 18)); // length of compressed data
   dwOffset += 30 + loc_get_word((byte *)(pbGPBuffer + 26)) + loc_get_word((byte *)(pbGPBuffer + 28));
   fseek(pfileIn, dwOffset, SEEK_SET); // move file pointer to start of compressed data

   pbInputBuffer = (byte *)pbGPBuffer; // space for compressed data chunck
   pbOutputBuffer = pbInputBuffer + 16384; // space for uncompressed data chunck
   z.zalloc = (alloc_func)0;
   z.zfree = (free_func)0;
   z.opaque = (voidpf)0;
   iStatus = inflateInit2(&z, -MAX_WBITS); // init zlib stream (no header)
   do {
      z.next_in = pbInputBuffer;
      if (dwSize > 16384) { // limit input size to max 16K or remaining bytes
         z.avail_in = 16384;
      } else {
         z.avail_in = dwSize;
      }
      z.avail_in = fread(pbInputBuffer, 1, z.avail_in, pfileIn); // load compressed data chunck from ZIP file
      while ((z.avail_in) && (iStatus == Z_OK)) { // loop until all data has been processed
         z.next_out = pbOutputBuffer;
         z.avail_out = 16384;
         iStatus = inflate(&z, Z_NO_FLUSH); // decompress data
         iCount = 16384 - z.avail_out;
         if (iCount) { // save data to file if output buffer is full
            fwrite(pbOutputBuffer, 1, iCount, pfileOut);
         }
      }
      dwSize -= 16384; // advance to next chunck
   } while ((dwSize > 0) && (iStatus == Z_OK)) ; // loop until done
   if (iStatus != Z_STREAM_END) {
      return ERR_FILE_UNZIP_FAILED; // abort on error
   }
   iStatus = inflateEnd(&z); // clean up
   fclose(pfileIn);
   fclose(pfileOut);

   return 0; // data was successfully decompressed
}


extern "C" int psp_fmgr_getExtId(const char *szFilePath) ;
extern "C" int beeb_update_save_name(char *name);
extern "C" int psp_kbd_load_mapping(char *name);
extern "C" int psp_kbd_save_mapping(char *name);

void
beeb_kbd_load(void)
{
  char        TmpFileName[_MAX_PATH + 1];
  struct stat aStat;

  snprintf(TmpFileName, _MAX_PATH, "%s/kbd/%s.kbd", mainWin->GetAppPath(), mainWin->m_save_name );
  if (! stat(TmpFileName, &aStat)) {
    psp_kbd_load_mapping(TmpFileName);
  }
}

int
beeb_kbd_save(void)
{
  char TmpFileName[_MAX_PATH + 1];
  snprintf(TmpFileName, _MAX_PATH, "%s/kbd/%s.kbd", mainWin->GetAppPath(), mainWin->m_save_name );
  return( psp_kbd_save_mapping(TmpFileName) );
}

static int
loc_beeb_load_settings(char *chFileName)
{
  char  Buffer[512];
  char *Scan;
  unsigned int Value;
  FILE* FileDesc;

  FileDesc = fopen(chFileName, "r");
  if (FileDesc == (FILE *)0 ) return 0;

  while (fgets(Buffer,512, FileDesc) != (char *)0) {

    Scan = strchr(Buffer,'\n');
    if (Scan) *Scan = '\0';
    /* For this #@$% of windows ! */
    Scan = strchr(Buffer,'\r');
    if (Scan) *Scan = '\0';
    if (Buffer[0] == '#') continue;

    Scan = strchr(Buffer,'=');
    if (! Scan) continue;

    *Scan = '\0';
    Value = atoi(Scan+1);

    if (!strcasecmp(Buffer,"snd_enable")) mainWin->m_snd_enable = Value;
    else
    if (!strcasecmp(Buffer,"render_mode")) mainWin->m_render_mode = Value;
    else
    if (!strcasecmp(Buffer,"view_fps")) mainWin->m_view_fps = Value;
    else
    if (!strcasecmp(Buffer,"display_lr")) mainWin->m_display_lr = Value;
    else
    if (!strcasecmp(Buffer,"skip_max_frame")) mainWin->m_skip_max_frame = Value;
    else
    if (!strcasecmp(Buffer,"psp_reverse_analog")) mainWin->m_psp_reverse_analog = Value;
    else
    if (!strcasecmp(Buffer,"psp_cpu_clock")) mainWin->m_psp_cpu_clock = Value;

  }

  fclose(FileDesc);

  scePowerSetClockFrequency(mainWin->m_psp_cpu_clock, mainWin->m_psp_cpu_clock, mainWin->m_psp_cpu_clock/2);

  return 0;
}

int
beeb_load_settings()
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  snprintf(FileName, MAX_PATH, "%s/set/%s.set", mainWin->GetAppPath(), mainWin->m_save_name);
  error = loc_beeb_load_settings(FileName);

  return error;
}

int
beeb_load_file_settings(char *FileName)
{
  return loc_beeb_load_settings(FileName);
}

int
beeb_disk_load(char *FileName, int disk_id, int zip_format)
{
  char *pchPtr;
  char *scan;
  char  SaveName[_MAX_PATH+1];
  char  TmpFileName[_MAX_PATH + 1];
  dword n;
  int   format;
  int   error;

  error = 1;

  if (zip_format) {

    zip_info.pchZipFile   = FileName;
    zip_info.pchExtension = ".ssd.dsd.adl.adf.uef.img";

    if (!zip_dir(&zip_info)) 
    {
      pchPtr = zip_info.pchFileNames;
      for (n = zip_info.iFiles; n != 0; n--) 
      {
        format = psp_fmgr_getExtId(pchPtr);
        if ((format == FMGR_FORMAT_DISK ) || 
            (format == FMGR_FORMAT_STATE)) break;
        pchPtr += strlen(pchPtr) + 5; // skip offset
      }
      if (n) {
        strncpy(SaveName,pchPtr,_MAX_PATH);
        scan = strrchr(SaveName,'.');
        if (scan) *scan = '\0';
        beeb_update_save_name(SaveName);
        zip_info.dwOffset = loc_get_dword((byte *)(pchPtr + (strlen(pchPtr)+1)));
        if (!zip_extract(FileName, TmpFileName, zip_info.dwOffset, scan+1)) {
          error = mainWin->HandleFile(TmpFileName, disk_id);
          remove(TmpFileName);
        }
      }
    }

  } else {
    strncpy(SaveName,FileName,_MAX_PATH);
    scan = strrchr(SaveName,'.');
    if (scan) *scan = '\0';
    beeb_update_save_name(SaveName);
    error = mainWin->HandleFile(FileName, disk_id);
  }

  if (! error ) {
    beeb_kbd_load();
    beeb_load_settings();
  }

  return error;
}

int
beeb_state_load(char *filename, int zip_format) 
{
  return beeb_disk_load(filename, 0, zip_format);
}

static int
loc_beeb_save_settings(char *chFileName)
{
  FILE* FileDesc;
  int   error = 0;

  FileDesc = fopen(chFileName, "w");
  if (FileDesc != (FILE *)0 ) {

    fprintf(FileDesc, "snd_enable=%d\n"         , mainWin->m_snd_enable);
    fprintf(FileDesc, "render_mode=%d\n"        , mainWin->m_render_mode);
    fprintf(FileDesc, "view_fps=%d\n"           , mainWin->m_view_fps);
    fprintf(FileDesc, "display_lr=%d\n"         , mainWin->m_display_lr);
    fprintf(FileDesc, "skip_max_frame=%d\n"     , mainWin->m_skip_max_frame);
    fprintf(FileDesc, "psp_reverse_analog=%d\n" , mainWin->m_psp_reverse_analog);
    fprintf(FileDesc, "psp_cpu_clock=%d\n"      , mainWin->m_psp_cpu_clock);

    fclose(FileDesc);

  } else {
    error = 1;
  }

  return error;
}

int
beeb_save_settings()
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  snprintf(FileName, MAX_PATH, "%s/set/%s.set", mainWin->GetAppPath(), mainWin->m_save_name);
  error = loc_beeb_save_settings(FileName);

  return error;
}

void
beeb_default_settings()
{
  mainWin->m_snd_enable         = 1;
  mainWin->m_scr_cpu            = 0;
  mainWin->m_render_mode        = BEEB_RENDER_NORMAL;
  mainWin->m_display_lr         = 0;
  mainWin->m_view_fps           = 0;
  mainWin->m_skip_cur_frame     = 0;
  mainWin->m_skip_max_frame     = 0;
  mainWin->m_psp_reverse_analog = 0;
  mainWin->m_psp_cpu_clock      = 300;
  mainWin->m_psp_screenshot_id  = 0;

  mainWin->m_render_blank_line     =   0;
  mainWin->m_render_max_blank_line =  50;

  scePowerSetClockFrequency(mainWin->m_psp_cpu_clock, mainWin->m_psp_cpu_clock, mainWin->m_psp_cpu_clock/2);
}


int
beeb_update_save_name(char *Name)
{
  char        TmpFileName[_MAX_PATH];
# ifdef LINUX_MODE
  struct stat aStat;
# else
  SceIoStat   aStat;
# endif
  int         index;
  char       *SaveName;
  char       *Scan1;
  char       *Scan2;

  SaveName = strrchr(Name,'/');
  if (SaveName != (char *)0) SaveName++;
  else                       SaveName = Name;

  if (!strncasecmp(SaveName, "sav_", 4)) {
    Scan1 = SaveName + 4;
    Scan2 = strrchr(Scan1, '_');
    if (Scan2 && (Scan2[1] >= '0') && (Scan2[1] <= '5')) {
      strncpy(mainWin->m_save_name, Scan1, _MAX_PATH);
      mainWin->m_save_name[Scan2 - Scan1] = '\0';
    } else {
      strncpy(mainWin->m_save_name, SaveName, _MAX_PATH);
    }
  } else {
    strncpy(mainWin->m_save_name, SaveName, _MAX_PATH);
  }

  if (mainWin->m_save_name[0] == '\0') {
    strcpy(mainWin->m_save_name,"default");
  }

  for (index = 0; index < BEEB_MAX_SAVE_STATE; index++) {
    mainWin->m_save_state[index].used = 0;
    memset(&mainWin->m_save_state[index].date, 0, sizeof(ScePspDateTime));
    mainWin->m_save_state[index].thumb = 0;

    snprintf(TmpFileName, _MAX_PATH, "%s/save/sav_%s_%d.uef", mainWin->GetAppPath(), mainWin->m_save_name, index);
# ifdef LINUX_MODE
    if (! stat(TmpFileName, &aStat)) 
# else
    if (! sceIoGetstat(TmpFileName, &aStat))
# endif
    {
      mainWin->m_save_state[index].used = 1;
      mainWin->m_save_state[index].date = aStat.st_mtime;
      snprintf(TmpFileName, MAX_PATH, "%s/save/sav_%s_%d.png", mainWin->GetAppPath(), mainWin->m_save_name, index);
# ifdef LINUX_MODE
      if (! stat(TmpFileName, &aStat)) 
# else
      if (! sceIoGetstat(TmpFileName, &aStat))
# endif
      {
        if (psp_sdl_load_thumb_png(mainWin->m_save_state[index].surface, TmpFileName)) {
          mainWin->m_save_state[index].thumb = 1;
        }
      }
    }
  }
  return 0;
}

void
beeb_reset_save_name()
{
  DiscLoaded[0] = 0;
  DiscLoaded[1] = 0;
  beeb_update_save_name("");
}

void 
beeb_audio_pause(void)
{
  if (mainWin->m_snd_enable) 
  {
    SDL_PauseAudio(1);
  }
}

void 
beeb_audio_resume(void)
{
  if (mainWin->m_snd_enable) 
  {
    SDL_PauseAudio(0);
  }
}

int
beeb_is_save_used(int slot_id)
{
  return mainWin->m_save_state[slot_id].used;
}

ScePspDateTime*
beeb_get_save_date(int slot_id)
{
  return &mainWin->m_save_state[slot_id].date;
}

int
beeb_snapshot_load_slot(int slot_id)
{
  char  FileName[_MAX_PATH+1];
  int   error;

  error = 1;

  if (slot_id < BEEB_MAX_SAVE_STATE) {
    snprintf(FileName, _MAX_PATH, "%s/save/sav_%s_%d.uef", mainWin->GetAppPath(), mainWin->m_save_name, slot_id);
    error = beeb_disk_load(FileName, 0, 0);
  }

  return error;
}

int
beeb_snapshot_save_slot(int slot_id)
{
  char        FileName[_MAX_PATH+1];
#ifdef LINUX_MODE
  struct stat aStat;
#else
  SceIoStat   aStat;
# endif
  int         error;

  error = 1;

  if (slot_id < BEEB_MAX_SAVE_STATE) {
    snprintf(FileName, _MAX_PATH, "%s/save/sav_%s_%d.uef", mainWin->GetAppPath(), mainWin->m_save_name, slot_id);
    error = SaveUEFState(FileName);
    if (! error) {
# ifdef LINUX_MODE
      if (! stat(FileName, &aStat)) 
# else
      if (! sceIoGetstat(FileName, &aStat))
# endif
      {
        mainWin->m_save_state[slot_id].used = 1;
        mainWin->m_save_state[slot_id].thumb = 0;
        mainWin->m_save_state[slot_id].date  = aStat.st_mtime;
        snprintf(FileName, _MAX_PATH, "%s/save/sav_%s_%d.png", mainWin->GetAppPath(), mainWin->m_save_name, slot_id);
        if (psp_sdl_save_thumb_png(mainWin->m_save_state[slot_id].surface, FileName)) {
          mainWin->m_save_state[slot_id].thumb = 1;
        }
      }
    }
  }

  return error;
}

int
beeb_snapshot_del_slot(int slot_id)
{
  char  FileName[_MAX_PATH+1];
  struct stat aStat;
  int   error;

  error = 1;

  if (slot_id < BEEB_MAX_SAVE_STATE) {
    snprintf(FileName, _MAX_PATH, "%s/save/sav_%s_%d.uef", mainWin->GetAppPath(), mainWin->m_save_name, slot_id);
    error = remove(FileName);
    if (! error) {
      mainWin->m_save_state[slot_id].used = 0;
      mainWin->m_save_state[slot_id].thumb = 0;
      memset(&mainWin->m_save_state[slot_id].date, 0, sizeof(ScePspDateTime));

      snprintf(FileName, _MAX_PATH, "%s/save/sav_%s_%d.png", mainWin->GetAppPath(), mainWin->m_save_name, slot_id);
      if (! stat(FileName, &aStat)) {
        remove(FileName);
      }
    }
  }

  return error;
}

void
beeb_set_model(int model_id)
{
  switch (model_id) 
  {
    case BEEB_MODEL_B    : mainWin->HandleCommand(ID_MODELB);
    break;
    case BEEB_MODEL_BINT : mainWin->HandleCommand(ID_MODELBINT);
    break;
    case BEEB_MODEL_M128 : mainWin->HandleCommand(ID_MASTER128);
    break;
  }
}

int
beeb_get_model(void)
{
  int model_id = 0;

  if (MachineType == 0)  model_id = BEEB_MODEL_B;
  else 
  if (MachineType == 1)  model_id = BEEB_MODEL_BINT;
  else 
  if (MachineType == 3)  model_id = BEEB_MODEL_M128;

  return model_id;
}

char*
beeb_get_homedir(void)
{
  return chAppPath;
}
  
  
} /* extern "C" */

