//
// BeebEm debugger
//

#ifndef DEBUG_HEADER
#define DEBUG_HEADER

#if HAVE_CONFIG_H
#	include <config.h>
#endif

#ifdef __cplusplus
extern "C" {
# endif

#include "include/windows.h"
#include "include/viastate.h"

extern int DebugEnabled;

enum DebugType {
	DEBUG_VIDEO,
	DEBUG_USERVIA,
	DEBUG_SYSVIA,
	DEBUG_TUBE,
	DEBUG_SERIAL,
	DEBUG_ECONET
};

void DebugOpenDialog(HINSTANCE hinst, HWND hwndMain);
void DebugCloseDialog(void);
bool DebugDisassembler(int addr, int Accumulator, int XReg, int YReg, int PSR, bool host);
void DebugDisplayTrace(DebugType type, bool host, const char *info);
void DebugDisplayInfo(const char *info);
void DebugVideoState(void);
void DebugUserViaState(void);
void DebugSysViaState(void);
void DebugViaState(const char *s, VIAState *v);

#ifdef __cplusplus
}
# endif

#endif
