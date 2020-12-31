//#include "include/gui.h"

#include <SDL.h>

void		Show_Main(void);
EG_BOOL 	InitializeBeebEmGUI(SDL_Surface *screen_ptr);
void	 	DestroyBeebEmGUI();


int UpdateGUIOption(int windows_menu_id, int is_selected);




int Open_GTK_File_Selector(char *filename_ptr);
int Save_GTK_File_Selector(char *filename_ptr);
