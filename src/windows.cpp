#if HAVE_CONFIG_H
#	include <config.h>
#endif

#include "include/log.h"
#include "include/windows.h"
#include "include/beebem_pages.h"

// Fake windows stuff:

/* Fake windows MessageBox
 */
int MessageBox(HWND hwnd, const char *message_p, const char *title_p, int type)
{
	const char *tmp_title_p;
	int tmp_type;
	HWND tmp_hwnd;

	tmp_title_p = title_p;
	tmp_type = type;
	tmp_hwnd = hwnd;

# if 0 //LUDO:
	printf("*** MESSAGEBOX *** \"%s\"\n", message_p);
# endif
	return(0);
}

void Sleep(DWORD ticks)
{
	SaferSleep( (unsigned int) ticks);
}

DWORD GetTickCount(void)
{
	return(SDL_GetTicks());
}

BOOL ModifyMenu(HMENU hMnu, UINT uPosition, UINT uFlags, PTR uIDNewItem, LPCTSTR lpNewItem)
{
	return(TRUE);
}

DWORD CheckMenuItem(HMENU hmenu, UINT uIDCheckItem, UINT uCheck)
{
	//printf("Asked to set %d to %d\n", uIDCheckItem, uCheck);

# if 0 //LUDO: NOT_NEEDED !
	if (uCheck == MF_CHECKED)
		return( UpdateGUIOption(uIDCheckItem, 1) );
	else
		return( UpdateGUIOption(uIDCheckItem, 0) );
# else
  return 0;
# endif
}

BOOL MoveFileEx(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, DWORD dwFlags)
{
	return(FALSE);
}

BOOL EnableMenuItem(HMENU hMenu,UINT uIDEnableItem,UINT uEnable)
{

	return(TRUE);
}
