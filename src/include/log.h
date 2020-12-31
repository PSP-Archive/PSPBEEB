#ifndef BEEBEM_LOG_H
#define BEEBEM_LOG_H

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include "include/line.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <pspdebug.h>


extern void Log_Init(void);
extern void Log_UnInit(void);

#define DEBUG_INIT() Log_Init();
#define DEBUG_FREE() Log_UnInit();

extern void pDIALOG(const char *psFormat, ...);

extern void pFATAL(const char *psFormat, ...);
extern void pERROR(const char *psFormat, ...);
extern void  pWARN(const char *psFormat, ...);
extern void  pINFO(const char *psFormat, ...);
extern void pDEBUG(const char *psFormat, ...);
extern void pDISABLED(const char *psFormat, ...);
extern void pTRACKER_E(const char *psFormat, ...);
extern void pTRACKER_L(const char *psFormat, ...);

// Tracking (It nasty but I want it for some stuff in beebwin.cc):

#define __tE_ pTRACKER_E("%s---> %s", __L__, __F__);
#define __tL_ pTRACKER_L("%s<--- %s", __L__, __F__);

// Blocks:
#define __dS_ pDEBUG("%s%s", __L__, "-=)");
#define __dE_ pDEBUG("%s%s", __L__, "(=-");

// Quickie wrappers for the functions above:
#define qFATAL(s) pFATAL("%s%s%s%s", __L__, __F__, __S__, s);
#define qERROR(s) pERROR("%s%s%s%s", __L__, __F__, __S__, s);
# if 0 //LUDO:
#define qDEBUG(s) pDEBUG("%s%s", __L__, s);
#define qDISABLED(s) pDISABLED("%s%s", __L__, s);
#define qWARN(s)  pWARN("%s%s", __L__, s);
#define qINFO(s)  pINFO("%s%s", __L__, s);
# else
#define qDEBUG(s)     do {} while (0)
#define qDISABLED(s)  do {} while (0)
#define qWARN(s)      do {} while (0)
#define qINFO(s)      do {} while (0)
# endif

// Shorter position indicators for printf etc:

#define dL "%s"
#define dR __L__

#define	dLEFT  "%s%s%s"
#define dRIGHT __L__, __F__, __S__


/* Error types, currently just a place holder, everything goes to stdout
 * (depending on SDL) at the moment and all log types are printed.
 */
typedef enum
{	EG_LOG_FATAL,	// Will terminate execution (doesn't yet)..
	EG_LOG_INFO,
	EG_LOG_ERROR,
	EG_LOG_WARNING,
	EG_LOG_DEBUG,	// Five levels of debugging
	EG_LOG_DEBUG2,
	EG_LOG_DEBUG3,
	EG_LOG_DEBUG4,
	EG_LOG_DEBUG5
} EG_LOGTYPE;

void 			EG_vprintf(const char*, va_list);
void 			EG_Log(EG_LOGTYPE, const char*, ...);



#endif  /* BEEBEM_LOG_H */
