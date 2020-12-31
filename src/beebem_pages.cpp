#include "include/main.h"
#include "include/gui.h"
#include "include/beebem_pages.h"
#include "include/beebwin.h"

#include "include/beebemrc.h"
#include "include/beebwin.h"

# if 0 //LUDO:
#include <gtk/gtk.h>
# endif

// [TODO] This shouldn't really be here.
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 512

/* -------------------
 */


static EG_BOOL  Make_Main(SDL_Surface *dst_ptr);
static void     Destroy_Main(void);

static EG_BOOL  Make_System(SDL_Surface *dst_ptr);
static void     Destroy_System(void);

static EG_BOOL  Make_Screen(SDL_Surface *dst_ptr);
static void     Destroy_Screen(void);

static EG_BOOL  Make_Sound(SDL_Surface *dst_ptr);
static void     Destroy_Sound(void);

static EG_BOOL  Make_ROMs(SDL_Surface *dst_ptr);
static void     Destroy_ROMs(void);

static EG_BOOL  Make_Speed(SDL_Surface *dst_ptr);
static void     Destroy_Speed(void);

static EG_BOOL  Make_About(SDL_Surface *dst_ptr);
static void     Destroy_About(void);

static EG_BOOL  Make_Devices(SDL_Surface *dst_ptr);
static void     Destroy_Devices(void);

static EG_BOOL  Make_Disks(SDL_Surface *dst_ptr);
static void     Destroy_Disks(void);

static EG_BOOL  Make_Keyboard(SDL_Surface *dst_ptr);
static void     Destroy_Keyboard(void);

static EG_BOOL  Make_AMX(SDL_Surface *dst_ptr);
static void     Destroy_AMX(void);


typedef struct{
        EG_Window *win_menu_ptr;
        EG_Window *win_system_ptr;
        EG_Window *win_screen_ptr;
        EG_Window *win_sound_ptr;
        EG_Window *win_roms_ptr;
        EG_Window *win_speed_ptr;
        EG_Window *win_about_ptr;
        EG_Window *win_devices_ptr;
        EG_Window *win_disks_ptr;
        EG_Window *win_keyboard_ptr;
        EG_Window *win_amx_ptr;

        EG_Widget *widget_okay_ptr;
        EG_Widget *widget_reset_ptr;
        EG_Widget *widget_no_reset_ptr;
        EG_Widget *fullscreen_widget_ptr;

        EG_Widget *widget_system_button;

        EG_Widget *widget_machine_bbc_b;
        EG_Widget *widget_machine_integra_b;
        EG_Widget *widget_machine_bbc_b_plus;
        EG_Widget *widget_machine_bbc_master_128;

        EG_Widget *widget_system_back;
}BeebEm_GUI;

static BeebEm_GUI gui={
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

        NULL, NULL, NULL, 
        NULL, NULL, NULL, NULL,

        NULL
};



//==================================================================

#define MAX_WINDOWS_MENU_ITEMS        512
typedef struct{
        int windows_menu_id;
        EG_Widget *widget_ptr;
}WindowsMenuItemBridge;

static WindowsMenuItemBridge win_menu[MAX_WINDOWS_MENU_ITEMS];

static int win_menu_count = 0;


static WindowsMenuItemBridge* CreateWindowsMenuItem(int win_id, EG_Widget *widget_ptr)
{
        if (win_menu_count>= MAX_WINDOWS_MENU_ITEMS){
                printf("*** SDL GUI to Windows Menu bridge: exceeded max menu"
                 "items. (inc MAX_WINDOWS_MENU_ITEMS)\n");
                exit(2);
        }

        win_menu[win_menu_count].windows_menu_id = win_id;
        win_menu[win_menu_count].widget_ptr = widget_ptr;

        return(&win_menu[win_menu_count++]);
}

// [TODO] MenuItemCheck function etc (based on ID only, searches for widget in above array and then changes it's state based on the widget type).

int UpdateGUIOption(int windows_menu_id, int is_selected)
{
        EG_Widget *ptr;
        int i;

        for(i=0; i<win_menu_count; i++){
                if (win_menu[i].windows_menu_id == windows_menu_id){
                        ptr = win_menu[i].widget_ptr;

                        //printf("Found: %s is of type %X %X %X\n", EG_Widget_GetName(ptr), (EG_StringHash) EG_Widget_GetType(ptr) , (EG_StringHash) EG_Widget_Type_TickBox, EG_Widget_Type_RadioButton);

                        if (EG_Widget_GetType(ptr) == ((EG_StringHash) EG_Widget_Type_TickBox) ){
                                if (EG_TickBox_GetGroupParent(ptr) == NULL ){
                                        if (is_selected == 1){
                                                EG_TickBox_Tick(ptr);
                                                EG_TickBox_RepaintLot(ptr);
                                        }else{
                                                EG_TickBox_Untick(ptr);
                                                EG_TickBox_RepaintLot(ptr);
                                        }
                                }else{
                                        if (is_selected == 1)
                                                EG_RadioGroup_Select(ptr);
                                }
                        }
                        return(1);
                }
        }

        return(0);
}


static void ProcessGUIOption(EG_Widget *widget_ptr, void *user_ptr)
{
        WindowsMenuItemBridge *ptr;

        ptr = (WindowsMenuItemBridge*) user_ptr;
        mainWin->HandleCommand(ptr->windows_menu_id);
}


// TEMP FILE SELECTOR ======================================================

# if 0 //LUDO:
static char *gtk_file_selector_filename_ptr;
static GtkWidget *filew;
# endif
int got_file;
bool was_full_screen = false;

# if 0 //LUDO:
/* Get the selected filename and print it to the console */
static void 
file_ok_sel( GtkWidget        *w,
             GtkFileSelection *fs )
{
        strcpy( gtk_file_selector_filename_ptr, gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs)) );

        got_file = true;

        gtk_widget_destroy(filew);
        if (was_full_screen == true){
                ToggleFullscreen();
                EG_TickBox_Tick(gui.fullscreen_widget_ptr);
        }
}
# endif

int 
Save_GTK_File_Selector(char *filename_ptr)
{
        if (fullscreen == 1){
                ToggleFullscreen();
                EG_TickBox_Untick(gui.fullscreen_widget_ptr);
                was_full_screen = true;
        }

        got_file = false;
# if 0 //LUDO:
        gtk_init (&__argc, &__argv);

        gtk_file_selector_filename_ptr = filename_ptr;

        filew = gtk_file_selection_new ("File selection");

        g_signal_connect (G_OBJECT (filew), "destroy", G_CALLBACK (gtk_main_quit), NULL);
        g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (filew)->ok_button), "clicked", G_CALLBACK (file_ok_sel), (gpointer) filew);
        g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION (filew)->cancel_button),"clicked"
         , G_CALLBACK (gtk_widget_destroy), G_OBJECT (filew));
  
        gtk_file_selection_set_filename (GTK_FILE_SELECTION(filew), DATA_DIR"/media/discs/");
    
        gtk_widget_show (filew);
        gtk_main ();
# endif
        was_full_screen = false;
        return(got_file);
}

int 
Open_GTK_File_Selector(char *filename_ptr)
{

        if (fullscreen == 1){
                ToggleFullscreen();
                EG_TickBox_Untick(gui.fullscreen_widget_ptr);
                was_full_screen = true;
        }

        got_file = false;

# if 0 //LUDO:
        gtk_init (&__argc, &__argv);

        gtk_file_selector_filename_ptr = filename_ptr;

        filew = gtk_file_selection_new ("File selection");

        g_signal_connect (G_OBJECT (filew), "destroy", G_CALLBACK (gtk_main_quit), NULL);
        g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (filew)->ok_button), "clicked", G_CALLBACK (file_ok_sel), (gpointer) filew);
        g_signal_connect_swapped (G_OBJECT (GTK_FILE_SELECTION (filew)->cancel_button),"clicked"
         , G_CALLBACK (gtk_widget_destroy), G_OBJECT (filew));
  
        gtk_file_selection_set_filename (GTK_FILE_SELECTION(filew), DATA_DIR"/media/discs/");
    
        gtk_widget_show (filew);
        gtk_main ();
# endif
        was_full_screen = false;
        return(got_file);
}

static void RunDisc(EG_Widget *widget_ptr, void *user_ptr)
{
# if 0 //LUDO:
        EG_Window *window_ptr = (EG_Window*) user_ptr;
        EG_Widget *tmp;
        tmp = widget_ptr;

        EG_Window_Hide(window_ptr);
        NoMenuShown();

        /* Setup page for next visit.
         */
        EG_RadioButton_Enable(gui.widget_no_reset_ptr);
        EG_RadioButton_Enable(gui.widget_reset_ptr);
        EG_RadioGroup_Select(gui.widget_no_reset_ptr);
        EG_Button_GetFocus(gui.widget_okay_ptr);

        mainWin->HandleCommand(IDM_RUNDISC);
# endif
}

static void 
SaveState(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Window *window_ptr = (EG_Window*) user_ptr;
        EG_Widget *tmp;
        tmp = widget_ptr;

        EG_Window_Hide(window_ptr);
        NoMenuShown();

        /* Setup page for next visit.
         */
        EG_RadioButton_Enable(gui.widget_no_reset_ptr);
        EG_RadioButton_Enable(gui.widget_reset_ptr);
        EG_RadioGroup_Select(gui.widget_no_reset_ptr);
        EG_Button_GetFocus(gui.widget_okay_ptr);

        printf("***************************************************************************************************\n");
        mainWin->HandleCommand(IDM_SAVESTATE);
        printf("***************************************************************************************************\n");
//        SDL_Delay(3000);
}

static void LoadState(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Window *window_ptr = (EG_Window*) user_ptr;
        EG_Widget *tmp;
        tmp = widget_ptr;

        EG_Window_Hide(window_ptr);
        NoMenuShown();

        /* Setup page for next visit.
         */
        EG_RadioButton_Enable(gui.widget_no_reset_ptr);
        EG_RadioButton_Enable(gui.widget_reset_ptr);
        EG_RadioGroup_Select(gui.widget_no_reset_ptr);
        EG_Button_GetFocus(gui.widget_okay_ptr);

        printf("***************************************************************************************************\n");
        mainWin->HandleCommand(IDM_LOADSTATE);
        printf("***************************************************************************************************\n");
//        SDL_Delay(3000);
}




// =========================================================================
//==================================================================

/* This will probably change.
 */
static SDL_Rect CalcRectCentered(int width, int height)
{
        SDL_Rect tmp;

        tmp.x = ( (SCREEN_WIDTH /* *(1/EG_Draw_GetScale()) */ ) - width ) /2;
        tmp.y = ( (SCREEN_HEIGHT /* *(1/EG_Draw_GetScale()) */ ) - height ) /2;
        tmp.w = width;
        tmp.h = height;

        //printf("%d %d   %d %d %d %d\n", SCREEN_WIDTH, SCREEN_HEIGHT, tmp.x, tmp.y, tmp.w, tmp.h);
        //SDL_Delay(4000);

        return(tmp);
}

static SDL_Rect CalcRect(int x, int y, int w, int h)
{
        SDL_Rect tmp;
        tmp.x = x; tmp.y = y; tmp.w = w; tmp.h = h;
        return(tmp);
}

static SDL_Color CalcColor(int r, int g, int b)
{
        SDL_Color tmp;
        tmp.r = r; tmp.g = g; tmp.b = b;
        tmp.unused = 0;
        return(tmp);
}

/*        =        =        =        =        =        =        =        =        =
 *        Initialize:
 *        =        =        =        =        =        =        =        =        =
 */



//static void SetState()
//{
//
//        /* Set machine type:
//         */
//        switch(MachineType)
//        {
//                case 0:
//                EG_RadioGroup_Select(gui.widget_machine_bbc_b);
//                break;
//
//                case 1:
//                EG_RadioGroup_Select(gui.widget_machine_integra_b);
//                break;
//
//                case 2:
//                EG_RadioGroup_Select(gui.widget_machine_bbc_b_plus);
//                break;
//
//                case 3:
//                EG_RadioGroup_Select(gui.widget_machine_bbc_master_128);
//                break;
//        }
//        (void) EG_Button_GetFocus(gui.widget_system_back);
//
//
//        /*
//         */
//}



void 
Show_Main(void)
{
        if (gui.win_menu_ptr == NULL){
                EG_Log(EG_LOG_ERROR, dL"Main window not made (bailing)!", dR);
        }else{
                EG_Window_Show(gui.win_menu_ptr);
                ShowingMenu();
        }

        SetActiveWindow(gui.win_menu_ptr);
}


EG_BOOL 
InitializeBeebEmGUI(SDL_Surface *screen_ptr)
{
# if 0 //LUDO:
        //printf("CALLED InitializeBeebEmGUI\n");
        //SDL_Delay(1000);

        /* Initialize gui structure:
         */

        // [TODO] Calc. machine type.

        /* Create configuration pages (the windows):
         */

        /* Build menus:
         */
        if (Make_Main(screen_ptr) != EG_TRUE){
                EG_Log(EG_LOG_FATAL, dL"Unable to create Main Menu - exiting!", dR);
                return(EG_FALSE);
        }       
        if (Make_System(screen_ptr) != EG_TRUE){
                EG_Log(EG_LOG_FATAL, dL"Unable to create System Menu - exiting!", dR);
                return(EG_FALSE);
        }
        if (Make_Screen(screen_ptr) != EG_TRUE){
                EG_Log(EG_LOG_FATAL, dL"Unable to create Screen Menu - exiting!", dR);
                return(EG_FALSE);
        }
        if (Make_Sound(screen_ptr) != EG_TRUE){
                EG_Log(EG_LOG_FATAL, dL"Unable to create Sound Menu - exiting!", dR);
                return(EG_FALSE);
        }
        if (Make_ROMs(screen_ptr) != EG_TRUE){
                EG_Log(EG_LOG_FATAL, dL"Unable to create ROMs Menu - exiting!", dR);
                return(EG_FALSE);
        }
        if (Make_Speed(screen_ptr) != EG_TRUE){
                EG_Log(EG_LOG_FATAL, dL"Unable to create Speed Menu - exiting!", dR);
                return(EG_FALSE); 
        }
        if (Make_About(screen_ptr) != EG_TRUE){
                EG_Log(EG_LOG_FATAL, dL"Unable to create About Menu - exiting!", dR);
                return(EG_FALSE);
        }
        if (Make_Devices(screen_ptr) != EG_TRUE){
                EG_Log(EG_LOG_FATAL, dL"Unable to create Devices Menu - exiting!", dR);
                return(EG_FALSE);
        }
        if (Make_Disks(screen_ptr) != EG_TRUE){
                EG_Log(EG_LOG_FATAL, dL"Unable to create Disks Menu - exiting!", dR);
                return(EG_FALSE);
        }
        if (Make_Keyboard(screen_ptr) != EG_TRUE){
                EG_Log(EG_LOG_FATAL, dL"Unable to create Keyboard Menu - exiting!", dR);
                return(EG_FALSE);
        }
        if (Make_AMX(screen_ptr) != EG_TRUE){
                EG_Log(EG_LOG_FATAL, dL"Unable to create AMX Menu - exiting!", dR);
                return(EG_FALSE);
        }

//        /* Set state:
//         */
//        //SetState();

# endif
        return(EG_TRUE);
}

void 
DestroyBeebEmGUI()
{
# if 0
        Destroy_Main();
        Destroy_System();
        Destroy_Screen();
        Destroy_Sound();
        Destroy_ROMs();
        Destroy_Speed();
        Destroy_About();
        Destroy_Devices();
        Destroy_Disks();
        Destroy_Keyboard();
        Destroy_AMX();
# endif
}

/*        =        =        =        =        =        =        =        =        =
 *        Shared events:
 *        =        =        =        =        =        =        =        =        =
 */

//static void Machine_Changed(EG_Widget *widget_ptr, void *user_ptr)
//{
//        static EG_BOOL i_disabled_it = EG_FALSE;
//
//        EG_BOOL force_reboot = EG_FALSE;
//        BeebEm_GUI *gui = (BeebEm_GUI*) user_ptr;
//
//        // Has the machine type changed?
//        if (strcmp(EG_Widget_GetName(widget_ptr), "machine_bbc_b") ==0 && MachineType != 0){
//                force_reboot = EG_TRUE;
//                printf("bbc_b\n");
//        }
//        if (strcmp(EG_Widget_GetName(widget_ptr), "machine_integra_b") ==0 && MachineType != 1){
//                force_reboot = EG_TRUE;
//                printf("integra_b\n");
//        }
//        if (strcmp(EG_Widget_GetName(widget_ptr), "machine_bbc_b_plus") == 0 && MachineType != 2){
//                force_reboot = EG_TRUE;
//                printf("b_plus\n");
//        }
//        if (strcmp(EG_Widget_GetName(widget_ptr), "machine_bbc_master_128") == 0 && MachineType != 3){
//                force_reboot = EG_TRUE;
//                printf("master_128\n");
//        }
//
//        // Yes, force reboot.
//        if (force_reboot == EG_TRUE){
//                if (EG_RadioButton_IsSelected(gui->widget_reset_ptr) != EG_TRUE){
//                        EG_RadioGroup_Select(gui->widget_reset_ptr);
//                        i_disabled_it = EG_TRUE;
//                }
//
//                EG_RadioButton_Disable(gui->widget_reset_ptr);
//                EG_RadioButton_Disable(gui->widget_no_reset_ptr);
//        }else{
//        // No, enable changes.
//                EG_RadioButton_Enable(gui->widget_reset_ptr);
//                EG_RadioButton_Enable(gui->widget_no_reset_ptr);
//
//                if ( i_disabled_it == EG_TRUE){
//                        EG_RadioGroup_Select(gui->widget_no_reset_ptr);
//                        i_disabled_it = EG_FALSE;
//                }
//        }
//
//        EG_Button_GetFocus(gui->widget_system_button);
//}

/*        =        =        =        =        =        =        =        =        =
 *        Main Menu:
 *        =        =        =        =        =        =        =        =        =
 */

static void Main_Button_Reset(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;

        tmp = widget_ptr;
        tmp2 = user_ptr;

        EG_Window_Hide(gui.win_menu_ptr);

        // [TODO] If 'reset BBC' ticked do it!

        NoMenuShown();

        /* Setup page for next visit.
         */
        EG_RadioButton_Enable(gui.widget_no_reset_ptr);
        EG_RadioButton_Enable(gui.widget_reset_ptr);
        EG_RadioGroup_Select(gui.widget_no_reset_ptr);
        EG_Button_GetFocus(gui.widget_okay_ptr);

        mainWin->HandleCommand(ID_FILE_RESET);
}



static void Main_Button_Okay(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;

        tmp = widget_ptr;
        tmp2 = user_ptr;

        EG_Window_Hide(gui.win_menu_ptr);

        // [TODO] If 'reset BBC' ticked do it!

        NoMenuShown();

        /* Setup page for next visit.
         */
        EG_RadioButton_Enable(gui.widget_no_reset_ptr);
        EG_RadioButton_Enable(gui.widget_reset_ptr);
        EG_RadioGroup_Select(gui.widget_no_reset_ptr);
        EG_Button_GetFocus(gui.widget_okay_ptr);
}

static void Main_Tick_FullScreen(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;

        tmp = widget_ptr;
        tmp2 = user_ptr;

        // [TODO] Flush sound.
        (void) ToggleFullscreen();
        
        /* Slight delay to allow hardware to catchup.
         */
        //SDL_Delay(1000);
}

static void Main_Button_Quit(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;

        tmp = widget_ptr;
        tmp2 = user_ptr;

        Quit();
        Main_Button_Okay(gui.widget_okay_ptr, &gui);
}

static void Main_Button_System(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;

        tmp = widget_ptr;
        tmp2 = user_ptr;

        EG_Window_Hide(gui.win_menu_ptr);
        SDL_Delay(100);
        EG_Window_Show(gui.win_system_ptr);
        SetActiveWindow(gui.win_system_ptr);
}

static void Main_Button_Screen(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;

        tmp = widget_ptr;
        tmp2 = user_ptr;

        EG_Window_Hide(gui.win_menu_ptr);
        SDL_Delay(100);
        EG_Window_Show(gui.win_screen_ptr);
        SetActiveWindow(gui.win_screen_ptr);
}

static void Main_Button_Sound(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;

        tmp = widget_ptr;
        tmp2 = user_ptr;

        EG_Window_Hide(gui.win_menu_ptr);
        SDL_Delay(100);
        EG_Window_Show(gui.win_sound_ptr);
        SetActiveWindow(gui.win_sound_ptr);
}

static void Main_Button_ROMs(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;

        tmp = widget_ptr;
        tmp2 = user_ptr;

        EG_Window_Hide(gui.win_menu_ptr);
        SDL_Delay(100);
        EG_Window_Show(gui.win_roms_ptr);
        SetActiveWindow(gui.win_roms_ptr);
}

static void Main_Button_Speed(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;

        tmp = widget_ptr;
        tmp2 = user_ptr;

        EG_Window_Hide(gui.win_menu_ptr);
        SDL_Delay(100);
        EG_Window_Show(gui.win_speed_ptr);
        SetActiveWindow(gui.win_speed_ptr);
}

static void Main_Button_About(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;

        tmp = widget_ptr;
        tmp2 = user_ptr;

        EG_Window_Hide(gui.win_menu_ptr);
        SDL_Delay(100);
        EG_Window_Show(gui.win_about_ptr);
        SetActiveWindow(gui.win_about_ptr);
}

static void Main_Button_Devices(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;

        tmp = widget_ptr;
        tmp2 = user_ptr;

        EG_Window_Hide(gui.win_menu_ptr);
        SDL_Delay(100);
        EG_Window_Show(gui.win_devices_ptr);
        SetActiveWindow(gui.win_devices_ptr);
}

static void Main_Button_Disks(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;

        tmp = widget_ptr;
        tmp2 = user_ptr;

        EG_Window_Hide(gui.win_menu_ptr);
        SDL_Delay(100);
        EG_Window_Show(gui.win_disks_ptr);
        SetActiveWindow(gui.win_disks_ptr);
}

static void Main_Button_Keyboard(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;

        tmp = widget_ptr;
        tmp2 = user_ptr;

        EG_Window_Hide(gui.win_menu_ptr);
        SDL_Delay(100);
        EG_Window_Show(gui.win_keyboard_ptr);
        SetActiveWindow(gui.win_keyboard_ptr);
}

static void Main_Button_AMX(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;

        tmp = widget_ptr;
        tmp2 = user_ptr;

        EG_Window_Hide(gui.win_menu_ptr);
        SDL_Delay(100);
        EG_Window_Show(gui.win_amx_ptr);
        SetActiveWindow(gui.win_amx_ptr);
}




static EG_BOOL Make_Main(SDL_Surface *dst_ptr)
{
        EG_Window *window_ptr = NULL;
        EG_Widget *widget_ptr;

        SDL_Rect win, loc;
        SDL_Color col;

        /* Window
         */
        col = CalcColor(191+32, 191+32, 0);
        win = CalcRectCentered(320, 262+16);
        window_ptr = EG_Window_Create("win_menu", dst_ptr, col, win);
        win.x=0; win.y=0; win.w-=4; win.h-=4;

        /* Okay button
         */
        loc = CalcRect(win.w-110, win.h-10-17-17-17, 100, 14*3);
        gui.widget_okay_ptr = EG_Button_Create("main_okay", col, EG_BUTTON_ALIGN_CENTER, "Okay", loc);
        (void) EG_Button_SetMyCallback_OnClick(gui.widget_okay_ptr, Main_Button_Okay, (void*) window_ptr);
        (void) EG_Window_AddWidget(window_ptr, gui.widget_okay_ptr);

//        /* Reset BBC
//         */
//        col = CalcColor(191+32, 191+32, 0);
//        widget_ptr = EG_RadioGroup_Create("main_group1");
//
//        loc = CalcRect(10, win.h-10-17-17-17, 150, 16);        
//        gui.widget_reset_ptr = EG_RadioButton_Create("main_reset", col, "Reset BBC", loc);
//
//        loc = CalcRect(10, win.h-10-17-17, 150, 16);
//        gui.widget_no_reset_ptr = EG_RadioButton_Create("main_noreset", col, "No Reset", loc);
//        
//        EG_RadioGroup_AddButton(widget_ptr, gui.widget_reset_ptr);
//        EG_RadioGroup_AddButton(widget_ptr, gui.widget_no_reset_ptr);
//        EG_RadioGroup_Select(gui.widget_no_reset_ptr);
//        EG_Window_AddWidget(window_ptr, widget_ptr);

        /* Full screen toggle tickbox
         */
        loc = CalcRect(10, win.h-10-17, 150, 16);
        widget_ptr = EG_TickBox_Create("main_fullscreen", col, "Fullscreen", loc);
        gui.fullscreen_widget_ptr = widget_ptr;
        if (GetFullscreenState() != 0) EG_TickBox_Tick(widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, Main_Tick_FullScreen, &gui);
        //(void) EG_TickBox_Disable(widget_ptr);
        EG_Window_AddWidget(window_ptr, widget_ptr);






#       define MENU_CALC_LOC(c, r, b) loc = CalcRect(10+c*(win.w/2-30)+10+(14*c), 30 + (r+1)*20 + b*10, win.w/2-20-5, 19)

        /* Top title label
         */
        loc = CalcRect(0,10, win.w, 20);
        col = CalcColor(191+32, 64, 32);
        widget_ptr = EG_Label_Create("lab_title", col, EG_LABEL_ALIGN_CENTER, "BeebEm PSP (1.0.0)", loc);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);



        col = CalcColor(191+32, 191+32, 0);
        loc = CalcRect(10, 40, win.w-20, 25*6);
        widget_ptr = EG_Box_Create("menu_box", EG_BOX_BORDER_SUNK, col, loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);

        /* System menu
         */
        MENU_CALC_LOC(0,0,0); 


//        loc = CalcRect(10, 40, (win.w-20)/2-5, 20);
        widget_ptr = EG_Button_Create("but_system", col, EG_BUTTON_ALIGN_CENTER, "System", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, Main_Button_System, &gui);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);
        gui.widget_system_button = widget_ptr;

        /* Screen menu
         */
        MENU_CALC_LOC(1,0,0); 
//        loc = CalcRect(win.w-((win.w-20)/2)-10+5, 40, (win.w-20)/2-5, 20);
//        col = CalcColor(191, 191, 191);

        widget_ptr = EG_Button_Create("but_screen", col, EG_BUTTON_ALIGN_CENTER, "Screen", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, Main_Button_Screen, &gui);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);

        /* Sound menu
         */
        MENU_CALC_LOC(0,1,0);
//        loc = CalcRect(10, 40+10+20*1, (win.w-20)/2-5, 20);
//        col = CalcColor(191, 191, 191);
        widget_ptr = EG_Button_Create("but_sound", col, EG_BUTTON_ALIGN_CENTER, "Sound", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, Main_Button_Sound, &gui);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);

        /* ROMs menu
         */
        MENU_CALC_LOC(1,1,0);
        widget_ptr = EG_Button_Create("but_roms", col, EG_BUTTON_ALIGN_CENTER, "ROMs", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, Main_Button_ROMs, &gui);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);
        EG_Button_Disable(widget_ptr);        

        /* Speed menu
         */
        MENU_CALC_LOC(0,2,0);
        widget_ptr = EG_Button_Create("but_speed", col, EG_BUTTON_ALIGN_CENTER, "Speed", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, Main_Button_Speed, &gui);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);

        /* About menu
         */
        MENU_CALC_LOC(1,2,0);
        widget_ptr = EG_Button_Create("but_about", col, EG_BUTTON_ALIGN_CENTER, "About", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, Main_Button_About, &gui);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);
        EG_Button_Disable(widget_ptr);

        /* Devices menu
         */
        MENU_CALC_LOC(0,3,0);
        widget_ptr = EG_Button_Create("but_devices", col, EG_BUTTON_ALIGN_CENTER, "Devices", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, Main_Button_Devices, &gui);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);
        EG_Button_Disable(widget_ptr);

        /* Disks menu
         */
        MENU_CALC_LOC(1,3,0);
        widget_ptr = EG_Button_Create("but_disks", col, EG_BUTTON_ALIGN_CENTER, "Disks", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, Main_Button_Disks, &gui);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);
        //EG_Button_Disable(widget_ptr);

        /* Keyboard menu
         */
        MENU_CALC_LOC(0,4,0);
        widget_ptr = EG_Button_Create("but_keyboard", col, EG_BUTTON_ALIGN_CENTER, "Keyboard", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, Main_Button_Keyboard, &gui);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);
        EG_Button_Disable(widget_ptr);

        /* AMX menu
         */
        MENU_CALC_LOC(1,4,0);
        widget_ptr = EG_Button_Create("but_amx", col, EG_BUTTON_ALIGN_CENTER, "AMX", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, Main_Button_AMX, &gui);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);
        EG_Button_Disable(widget_ptr);

        /* Quit button
         */
        MENU_CALC_LOC(1,5,1);
        widget_ptr = EG_Button_Create("but_quit", col, EG_BUTTON_ALIGN_CENTER, "Quit", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, Main_Button_Quit, &gui);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);

        /* Reset
         */
        MENU_CALC_LOC(0,5,1);
        widget_ptr = EG_Button_Create("but_reset", col, EG_BUTTON_ALIGN_CENTER, "Reset", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, Main_Button_Reset, NULL);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);

        gui.win_menu_ptr = window_ptr;
        return(EG_TRUE);
}

static void Destroy_Main(void)
{
        EG_Window_DestroyAllChildWidgets(gui.win_menu_ptr);
        EG_Window_Destroy(gui.win_menu_ptr);
}


/*      =       =       =       =       =       =       =       =       =
 *        System:
 *        =        =        =        =        =        =        =        =        =
 */

static void System_Button_Back(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;
        tmp = widget_ptr;
        tmp2 = user_ptr;

        EG_Window_Hide(gui.win_system_ptr);
        SDL_Delay(100);
        EG_Window_Show(gui.win_menu_ptr);
        SetActiveWindow(gui.win_menu_ptr);
}
static EG_BOOL Make_System(SDL_Surface *dst_ptr)
{
        EG_Window *window_ptr = NULL;
        EG_Widget *widget_ptr;
        EG_Widget *group1_ptr, *group2_ptr;

        SDL_Rect win, loc;
        SDL_Color col;

        /* Window
         */
        col = CalcColor(191+32, 191+32, 0);
        win = CalcRectCentered(256+48, 25*17);
        window_ptr = EG_Window_Create("system:window", dst_ptr, col, win);
        win.x=0; win.y=0; win.w-=4; win.h-=4;

        /* Title label
         */
#        define SYSTEM_CALC_LOC(r, b) loc = CalcRect(20, 10+r*17+b*10, win.w-40, 16)

        SYSTEM_CALC_LOC(0, 0);
        widget_ptr = EG_Label_Create("system:model:title", col, EG_LABEL_ALIGN_CENTER, "BBC model:", loc);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);

        group1_ptr = EG_RadioGroup_Create("system:model:group");


        SYSTEM_CALC_LOC(1,0); loc.x-=10; loc.y+=8; loc.h = 17*5; loc.w+=20;
        widget_ptr = EG_Box_Create("system:model:box", EG_BOX_BORDER_SUNK, col, loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);



        SYSTEM_CALC_LOC(2, 0);
        widget_ptr = EG_RadioButton_Create("system:model:bbc_b", col, "BBC Model B", loc);
        EG_RadioGroup_AddButton(group1_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(ID_MODELB, widget_ptr));
        gui.widget_machine_bbc_b = widget_ptr;

        SYSTEM_CALC_LOC(3, 0);
        widget_ptr = EG_RadioButton_Create("system:model:integra_b", col, "BBC Model B + Integra-B", loc);
        EG_RadioGroup_AddButton(group1_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(ID_MODELBINT, widget_ptr));
        gui.widget_machine_integra_b = widget_ptr;

        SYSTEM_CALC_LOC(4, 0);
        widget_ptr = EG_RadioButton_Create("system:model:bbc_b_plus", col, "BBC Model B Plus", loc);
        EG_RadioGroup_AddButton(group1_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(ID_MODELBP, widget_ptr));
        EG_TickBox_Disable(widget_ptr);
        gui.widget_machine_bbc_b_plus = widget_ptr;

        SYSTEM_CALC_LOC(5, 0);
        widget_ptr = EG_RadioButton_Create("system:model:bbc_master_128", col, "BBC Master 128", loc);
        EG_RadioGroup_AddButton(group1_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(ID_MASTER128, widget_ptr));
        gui.widget_machine_bbc_master_128 = widget_ptr;

        (void) EG_Window_AddWidget(window_ptr, group1_ptr);



        SYSTEM_CALC_LOC(7, 0);
        widget_ptr = EG_Label_Create("system:6502:title", col, EG_LABEL_ALIGN_CENTER, "Undocumented Instructions:", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);

        SYSTEM_CALC_LOC(8,0); loc.x-=10; loc.y+=8; loc.h = 17*4; loc.w+=20;
        widget_ptr = EG_Box_Create("system:6502:box", EG_BOX_BORDER_SUNK, col, loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);

        group2_ptr = EG_RadioGroup_Create("system:6502:instructions");

        SYSTEM_CALC_LOC(9, 0);
        widget_ptr = EG_RadioButton_Create("system:6502:documented", col, "Documented only", loc);
        EG_RadioGroup_AddButton(group2_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(ID_DOCONLY, widget_ptr));

        SYSTEM_CALC_LOC(10, 0);
        widget_ptr = EG_RadioButton_Create("system:6502:extras", col, "Common extras", loc);
        EG_RadioGroup_AddButton(group2_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(ID_EXTRAS, widget_ptr));

        SYSTEM_CALC_LOC(11, 0);
        widget_ptr = EG_RadioButton_Create("system:6502:fullset", col, "Full set", loc);
        EG_RadioGroup_AddButton(group2_ptr, widget_ptr);
        EG_RadioGroup_Select(widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(ID_FULLSET, widget_ptr));

        (void) EG_Window_AddWidget(window_ptr, group2_ptr);



        SYSTEM_CALC_LOC(13, 0);
        widget_ptr = EG_Label_Create("system:tube:title", col, EG_LABEL_ALIGN_CENTER, "Tube:", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);

        SYSTEM_CALC_LOC(14,0); loc.x-=10; loc.y+=8; loc.h = 17*3; loc.w+=20;
        widget_ptr = EG_Box_Create("system:tube:box", EG_BOX_BORDER_SUNK, col, loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);

        SYSTEM_CALC_LOC(15, 0);
        widget_ptr = EG_TickBox_Create("system:tube:65C02", col, "65C02", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);        
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(IDM_TUBE, widget_ptr));
        //EG_TickBox_Disable(widget_ptr);

        SYSTEM_CALC_LOC(16, 0);
        widget_ptr = EG_TickBox_Create("system:tube:Z80", col, "Torch Z80", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(IDM_TORCH, widget_ptr));
        



        SYSTEM_CALC_LOC(18, 0);
        widget_ptr = EG_Label_Create("system:network:title", col, EG_LABEL_ALIGN_CENTER, "Network:", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);

        SYSTEM_CALC_LOC(19,0); loc.x-=10; loc.y+=8; loc.h = 17*2; loc.w+=20;
        widget_ptr = EG_Box_Create("system:network:box", EG_BOX_BORDER_SUNK, col, loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);

        SYSTEM_CALC_LOC(20, 0);
        widget_ptr = EG_TickBox_Create("system:network:econet", col, "Econet", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(ID_ECONET, widget_ptr));
        EG_TickBox_Disable(widget_ptr);


        /* Back button
         */
        loc = CalcRect(win.w - 10 - 64, win.h-10-20, 64, 20);
        widget_ptr = EG_Button_Create("but_system_back", col
         , EG_BUTTON_ALIGN_CENTER, "Back", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, System_Button_Back
         , &gui);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);
        gui.widget_system_back = widget_ptr;

        gui.win_system_ptr = window_ptr;
        return(EG_TRUE);
}
static void Destroy_System(void)
{
        EG_Window_DestroyAllChildWidgets(gui.win_system_ptr);
        EG_Window_Destroy(gui.win_system_ptr);
}

/*        =        =        =        =        =        =        =        =        =
 *        Screen:
 *        =        =        =        =        =        =        =        =        =
 */


static void Screen_Button_Back(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;
        tmp = widget_ptr;
        tmp2 = user_ptr;

        EG_Window_Hide(gui.win_screen_ptr);
        SDL_Delay(100);
        EG_Window_Show(gui.win_menu_ptr);
        SetActiveWindow(gui.win_menu_ptr);
}
static EG_BOOL Make_Screen(SDL_Surface *dst_ptr)
{
        EG_Window *window_ptr = NULL;
        EG_Widget *widget_ptr;

        SDL_Rect win, loc;
        SDL_Color col;

        EG_Widget *group1_ptr, *group2_ptr;

        /* Window
         */
        col = CalcColor(191+32, 191+32, 0);
        win = CalcRectCentered(480, 512-256-32+14);
        window_ptr = EG_Window_Create("screen:window", dst_ptr, col, win);
        win.x=0; win.y=0; win.w-=4; win.h-=4;

        /* Title label
         */
        loc = CalcRect(0,10, win.w, 16);
        widget_ptr = EG_Label_Create("screen:title", col, EG_LABEL_ALIGN_CENTER, "Screen configuration:", loc);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);

        /* Options box
         */
        loc = CalcRect(10, 31, win.w - 20, win.h -31-40);
        widget_ptr = EG_Box_Create("screen:box", EG_BOX_BORDER_SUNK, col, loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);

#        define SCREEN_CALC_LOC(c, r, b) loc = CalcRect(20+c*(win.w/2-20), 26 + (r+1)*17 + b*10, win.w/2-20-5, 16)
        SCREEN_CALC_LOC(0, 0, 0);
        widget_ptr = EG_TickBox_Create("screen:low_grap", col, "Low-res graphics", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);
        EG_TickBox_Disable(widget_ptr);

        SCREEN_CALC_LOC(1, 0, 0);
        widget_ptr = EG_TickBox_Create("screen:low_text", col, "Low-res teletext", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);
        EG_TickBox_Disable(widget_ptr);

        SCREEN_CALC_LOC(0, 1, 0);
        widget_ptr = EG_TickBox_Create("screen:cursor", col, "Hide mouse pointer", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);
        EG_TickBox_Disable(widget_ptr);

        SCREEN_CALC_LOC(1, 1, 0);
        widget_ptr = EG_TickBox_Create("screen:display_fps", col, "Display FPS", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(IDM_SPEEDANDFPS, widget_ptr));

        SCREEN_CALC_LOC(0, 2, 0);
        widget_ptr = EG_TickBox_Create("screen:keyboard_leds",  col, "Show keyboard LEDs", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(ID_SHOW_KBLEDS, widget_ptr));
        EG_RadioButton_Disable(widget_ptr);

        SCREEN_CALC_LOC(1, 2, 0);
        widget_ptr = EG_TickBox_Create("screen:disc_leds", col, "Show disc LEDs", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(ID_SHOW_DISCLEDS, widget_ptr));
        EG_RadioButton_Disable(widget_ptr);

        SCREEN_CALC_LOC(0, 3, 1);
        widget_ptr = EG_TickBox_Create("screen:colormap", col, "Force colormap", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);
        EG_RadioButton_Disable(widget_ptr);

        /* LED colors
         */
        group1_ptr = EG_RadioGroup_Create("screen:leds");

        SCREEN_CALC_LOC(0, 4, 2); loc.w = 90;
        widget_ptr = EG_Label_Create("screen:leds:title", col, EG_LABEL_ALIGN_LEFT, "LEDs are:", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);

        loc.w = 55; loc.x = 115;
        widget_ptr = EG_RadioButton_Create("screen:red", col, "Red", loc);        
        EG_RadioGroup_AddButton(group1_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(ID_RED_LEDS, widget_ptr));
        EG_RadioButton_Disable(widget_ptr);

        loc.w = 75; loc.x = 175;
        widget_ptr = EG_RadioButton_Create("screen:green", col, "Green", loc);
        EG_RadioGroup_AddButton(group1_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(ID_GREEN_LEDS, widget_ptr));
        EG_RadioButton_Disable(widget_ptr);

        EG_Window_AddWidget(window_ptr, group1_ptr);

        /* Monitor:
         */
        group2_ptr = EG_RadioGroup_Create("screen:monitor");

        SCREEN_CALC_LOC(0, 5, 2); loc.w = 90;
        widget_ptr = EG_Label_Create("screen:monitor:title", col, EG_LABEL_ALIGN_LEFT, "Monitor:",loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);

        loc.w = 55; loc.x = 115;
        widget_ptr = EG_RadioButton_Create("screen:m_rgb", col, "RGB", loc);
        EG_RadioGroup_AddButton(group2_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(ID_MONITOR_RGB, widget_ptr));
        EG_RadioButton_Disable(widget_ptr);

        loc.w = 75; loc.x = 175;
        widget_ptr = EG_RadioButton_Create("screen:m_white", col, "B/W", loc);
        EG_RadioGroup_AddButton(group2_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(ID_MONITOR_BW, widget_ptr));
        EG_RadioButton_Disable(widget_ptr);

        loc.x = 255; loc.w = 75;
        widget_ptr = EG_RadioButton_Create("screen:m_amber", col, "Amber", loc);
        EG_RadioGroup_AddButton(group2_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(ID_MONITOR_AMBER, widget_ptr));
        EG_RadioButton_Disable(widget_ptr);

        loc.x = 335; 
        widget_ptr = EG_RadioButton_Create("screen:m_green", col, "Green", loc);
        EG_RadioGroup_AddButton(group2_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(ID_MONITOR_GREEN, widget_ptr));
        EG_RadioButton_Disable(widget_ptr);

        EG_Window_AddWidget(window_ptr, group2_ptr);

        /* FPS:
         * MOVE THIS TO SPEED
         */
        /*
        group3_ptr = EG_RadioGroup_Create("screen:fps");

        SCREEN_CALC_LOC(0, 6, 2); loc.w = 90;
        widget_ptr = EG_Label_Create("screen:fps:title", col, EG_LABEL_ALIGN_LEFT, "FPS:",loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);

        loc.w = 55; loc.x = 115;
        widget_ptr = EG_RadioButton_Create("screen:fps:50",  col, "50", loc);
        EG_RadioGroup_AddButton(group3_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(IDM_50FPS, widget_ptr));
        EG_RadioButton_Disable(widget_ptr);

        loc.w = 75; loc.x = 175;
        widget_ptr = EG_RadioButton_Create("screen:fps:25",  col, "25", loc);
        EG_RadioGroup_AddButton(group3_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(IDM_25FPS, widget_ptr));
        EG_RadioButton_Disable(widget_ptr);

        loc.x = 255; loc.w = 75;
        widget_ptr = EG_RadioButton_Create("screen:fps:10",  col, "10", loc);
        EG_RadioGroup_AddButton(group3_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(IDM_10FPS, widget_ptr));
        EG_RadioButton_Disable(widget_ptr);

        loc.x = 335;
        widget_ptr = EG_RadioButton_Create("screen:fps:5",  col, "5", loc);
        EG_RadioGroup_AddButton(group3_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(IDM_5FPS, widget_ptr));
        EG_RadioButton_Disable(widget_ptr);

        loc.x = 415; loc.w = 35;
        widget_ptr = EG_RadioButton_Create("screen:fps:1",  col, "1", loc);
        EG_RadioGroup_AddButton(group3_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(IDM_1FPS, widget_ptr));
        EG_RadioButton_Disable(widget_ptr);

        EG_Window_AddWidget(window_ptr, group3_ptr);
        */

        /* Back button
         */
        loc = CalcRect(win.w - 10 - 64, win.h-10-20, 64, 20);
        widget_ptr = EG_Button_Create("but_screen_back", col, EG_BUTTON_ALIGN_CENTER, "Back", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, Screen_Button_Back
         , &gui);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);
        (void) EG_Button_GetFocus(widget_ptr);

        gui.win_screen_ptr = window_ptr;
        return(EG_TRUE);
}
static void Destroy_Screen(void)
{
        EG_Window_DestroyAllChildWidgets(gui.win_screen_ptr);
        EG_Window_Destroy(gui.win_screen_ptr);
}


/*        =        =        =        =        =        =        =        =        =
 *        Sound:
 *        =        =        =        =        =        =        =        =        =
 */


static void Sound_Button_Back(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;
        tmp = widget_ptr;
        tmp2 = user_ptr;

        EG_Window_Hide(gui.win_sound_ptr);
        SDL_Delay(100);
        EG_Window_Show(gui.win_menu_ptr);
        SetActiveWindow(gui.win_menu_ptr);
}
static EG_BOOL Make_Sound(SDL_Surface *dst_ptr)
{
        EG_Window *window_ptr = NULL;
        EG_Widget *widget_ptr;

        EG_Widget *group1_ptr, *group2_ptr;

        SDL_Rect win, loc;
        SDL_Color col;

        /* Window
         */
        col = CalcColor(191+32, 191+32, 0);
        win = CalcRectCentered(256+64, 17*25);
        window_ptr = EG_Window_Create("sound:window", dst_ptr, col, win);
        win.x=0; win.y=0; win.w-=4; win.h-=4;

        /* Title label
         */
#        define SOUND_CALC_LOC(r, b) loc = CalcRect(20, 10+r*17+b*10, win.w-40, 16)

        SOUND_CALC_LOC(0, 0);
        widget_ptr = EG_Label_Create("sound:sound:title", col, EG_LABEL_ALIGN_CENTER, "Sound:", loc);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);

        SOUND_CALC_LOC(1,0); loc.x-=10; loc.y+=8; loc.h = 17*5; loc.w+=20;
        widget_ptr = EG_Box_Create("sound:sound:box", EG_BOX_BORDER_SUNK, col, loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);

        SOUND_CALC_LOC(2, 0);
        widget_ptr = EG_TickBox_Create("sound:sound:yes/no", col, "Sound support", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(IDM_SOUNDONOFF, widget_ptr));

        SOUND_CALC_LOC(3, 0);
        widget_ptr = EG_TickBox_Create("sound:speech:yes/no", col, "Speech support", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(IDM_SPEECH, widget_ptr));

        SOUND_CALC_LOC(4, 0);
        widget_ptr = EG_TickBox_Create("sound:Relay:yes/no", col, "Cassette relay effects", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(ID_SFX_RELAY, widget_ptr));

//        SOUND_CALC_LOC(5, 0);
//        // [TODO] Findout what this does.
//        widget_ptr = EG_TickBox_Create("sound:Input:yes/no", col, "Cassette input effects", loc);
//        EG_Window_AddWidget(window_ptr, widget_ptr);
//        EG_TickBox_Disable(widget_ptr);


        SOUND_CALC_LOC(7, 0);
        widget_ptr = EG_Label_Create("sound:quality:title", col, EG_LABEL_ALIGN_CENTER, "Quality:", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);

        group1_ptr = EG_RadioGroup_Create("quality");

        SOUND_CALC_LOC(8,0); loc.x-=10; loc.y+=8; loc.h = 17*6; loc.w+=20;
        widget_ptr = EG_Box_Create("sound:quality:box", EG_BOX_BORDER_SUNK, col, loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);

        SOUND_CALC_LOC(9, 0);
        widget_ptr = EG_RadioButton_Create("sound:quality:11_025", col, "11.025 kHz", loc);
        EG_RadioGroup_AddButton(group1_ptr, widget_ptr);
        EG_RadioButton_Disable(widget_ptr);

        SOUND_CALC_LOC(10, 0);
        widget_ptr = EG_RadioButton_Create("sound:quality:22_050", col, "22.050 kHz", loc);
        EG_RadioGroup_AddButton(group1_ptr, widget_ptr);
        EG_RadioGroup_Select(widget_ptr);
        EG_RadioButton_Disable(widget_ptr);

        SOUND_CALC_LOC(11, 0);
        widget_ptr = EG_RadioButton_Create("sound:quality:44_100", col, "44.100 kHz", loc);
        EG_RadioGroup_AddButton(group1_ptr, widget_ptr);
        EG_RadioButton_Disable(widget_ptr);

        EG_Window_AddWidget(window_ptr, group1_ptr);

        SOUND_CALC_LOC(13, 0);
        widget_ptr = EG_TickBox_Create("sound:latacy:yes/no", col, "Want low latency sound", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);
        EG_TickBox_Tick(widget_ptr);
        EG_TickBox_Disable(widget_ptr);




        SOUND_CALC_LOC(15, 0);
        widget_ptr = EG_Label_Create("sound:volume:label", col, EG_LABEL_ALIGN_CENTER, "Volume:", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);

        group2_ptr = EG_RadioGroup_Create("volume");

        SOUND_CALC_LOC(16,0); loc.x-=10; loc.y+=8; loc.h = 17*5; loc.w+=20;
        widget_ptr = EG_Box_Create("sound:volume:box", EG_BOX_BORDER_SUNK, col, loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);

        SOUND_CALC_LOC(17, 0);
        widget_ptr = EG_RadioButton_Create("sound:volume:full", col, "Full volume", loc);
        EG_RadioGroup_AddButton(group2_ptr, widget_ptr);
        (void) EG_RadioButton_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(IDM_FULLVOLUME, widget_ptr));
        
        SOUND_CALC_LOC(18, 0);
        widget_ptr = EG_RadioButton_Create("sound:volume:high", col, "High volume", loc);
        EG_RadioGroup_AddButton(group2_ptr, widget_ptr);
        (void) EG_RadioButton_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(IDM_HIGHVOLUME, widget_ptr));

        SOUND_CALC_LOC(19, 0);
        widget_ptr = EG_RadioButton_Create("sound:volume:medium", col, "Medium volume", loc);
        EG_RadioGroup_AddButton(group2_ptr, widget_ptr);
        (void) EG_RadioButton_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(IDM_MEDIUMVOLUME, widget_ptr));

        SOUND_CALC_LOC(20, 0);
        widget_ptr = EG_RadioButton_Create("sound:volume:low", col, "Low volume", loc);
        EG_RadioGroup_AddButton(group2_ptr, widget_ptr);
        (void) EG_RadioButton_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(IDM_LOWVOLUME, widget_ptr));
        
        EG_Window_AddWidget(window_ptr, group2_ptr);


        /* Back button
         */
        loc = CalcRect(win.w - 10 - 64, win.h-10-20, 64, 20);
        widget_ptr = EG_Button_Create("but_sound_back", col, EG_BUTTON_ALIGN_CENTER, "Back", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, Sound_Button_Back, &gui);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);
        (void) EG_Button_GetFocus(widget_ptr);

        gui.win_sound_ptr = window_ptr;
        return(EG_TRUE);
}
static void Destroy_Sound(void)
{
        EG_Window_DestroyAllChildWidgets(gui.win_sound_ptr);
        EG_Window_Destroy(gui.win_sound_ptr);
}




/*        =        =        =        =        =        =        =        =        =
 *        ROMs:
 *        =        =        =        =        =        =        =        =        =
 */


static void ROMs_Button_Back(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;
        tmp = widget_ptr;
        tmp2 = user_ptr;

        EG_Window_Hide(gui.win_roms_ptr);
        SDL_Delay(100);
        EG_Window_Show(gui.win_menu_ptr);
        SetActiveWindow(gui.win_menu_ptr);
}
static EG_BOOL Make_ROMs(SDL_Surface *dst_ptr)
{
        EG_Window *window_ptr = NULL;
        EG_Widget *widget_ptr;

        SDL_Rect win, loc;
        SDL_Color col;

        char rom_caption[64+1];
        int i;

        /* Window
         */
        col = CalcColor(191+32, 191+32, 0);
        win = CalcRectCentered(480, 23*17);
        window_ptr = EG_Window_Create("win_roms", dst_ptr, col, win);
        win.x=0; win.y=0; win.w-=4; win.h-=4;

        /* Title label
         */
#        define ROMS_CALC_LOC(r, b) loc = CalcRect(20, 10+r*17+b*10, win.w-40, 16)

        ROMS_CALC_LOC(0, 0);
        widget_ptr = EG_Label_Create("lab_sound", col, EG_LABEL_ALIGN_CENTER, "ROM configuration:", loc);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);

        ROMS_CALC_LOC(1,0); loc.x-=10; loc.y+=8; loc.h = 18*17; loc.w+=20;
        widget_ptr = EG_Box_Create("rom_box", EG_BOX_BORDER_SUNK, col, loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);

        ROMS_CALC_LOC(2, 0);
        widget_ptr = EG_Label_Create("lab_caption", col, EG_LABEL_ALIGN_LEFT, "R/W", loc);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);

        /* [TODO] These are dependent on the model.
         */
        for (i=0; i<16; i++){

                sprintf(rom_caption, "  %X %s", i, "[TODO]");

                ROMS_CALC_LOC( (i+3), 0);

                widget_ptr = EG_TickBox_Create(NULL, col, rom_caption, loc);
                EG_Window_AddWidget(window_ptr, widget_ptr);
//                EG_TickBox_Tick(widget_ptr);
                EG_TickBox_Disable(widget_ptr);
        }

        /* Back button
         */
        loc = CalcRect(win.w - 10 - 64, win.h-10-20, 64, 20);
        widget_ptr = EG_Button_Create("but_roms_back", col, EG_BUTTON_ALIGN_CENTER, "Back", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, ROMs_Button_Back, &gui);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);
        (void) EG_Button_GetFocus(widget_ptr);

        gui.win_roms_ptr = window_ptr;
        return(EG_TRUE);
}
static void Destroy_ROMs(void)
{
        EG_Window_DestroyAllChildWidgets(gui.win_roms_ptr);
        EG_Window_Destroy(gui.win_roms_ptr);
}


/*        =        =        =        =        =        =        =        =        =
 *        Speed:
 *        =        =        =        =        =        =        =        =        =
 */


static void Speed_Button_Back(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;
        tmp = widget_ptr;
        tmp2 = user_ptr;

        EG_Window_Hide(gui.win_speed_ptr);
        SDL_Delay(100);
        EG_Window_Show(gui.win_menu_ptr);
        SetActiveWindow(gui.win_menu_ptr);
}
static EG_BOOL Make_Speed(SDL_Surface *dst_ptr)
{
        EG_Window *window_ptr = NULL;
        EG_Widget *widget_ptr;

        EG_Widget *group_ptr;

        SDL_Rect win, loc;
        SDL_Color col;

#        define SPEED_COUNT 14
        float speed[]={100, 50, 10, 5, 2, 1.5, 1.25, 1.1, 1, 0.9, 0.75, 0.5, 0.25, 0.1};
        int speed_menu_id[]={IDM_FIXEDSPEED100, IDM_FIXEDSPEED50, IDM_FIXEDSPEED10
         , IDM_FIXEDSPEED5, IDM_FIXEDSPEED2, IDM_FIXEDSPEED1_5, IDM_FIXEDSPEED1_25
         , IDM_FIXEDSPEED1_1, IDM_REALTIME, IDM_FIXEDSPEED0_9, IDM_FIXEDSPEED0_75
         , IDM_FIXEDSPEED0_5, IDM_FIXEDSPEED0_25, IDM_FIXEDSPEED0_1};
        char speed_caption[64+1];
        int i;

        /* Window
         */
        col = CalcColor(191+32, 191+32, 0);
        win = CalcRectCentered(256-64, 17*20);
        window_ptr = EG_Window_Create("speed:window", dst_ptr, col, win);
        win.x=0; win.y=0; win.w-=4; win.h-=4;

        /* Title label
         */
#        define SPEED_CALC_LOC(r, b) loc = CalcRect(20, 10+r*17+b*10, win.w-40, 16)

        SPEED_CALC_LOC(0, 0); loc.x-=10; loc.w+=20;
        widget_ptr = EG_Label_Create("speed:title", col, EG_LABEL_ALIGN_CENTER, "Emulator speed:", loc);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);

        SPEED_CALC_LOC(1,0); loc.x-=10; loc.y+=8; loc.h = 15*17; loc.w+=20;
        widget_ptr = EG_Box_Create("speed:box", EG_BOX_BORDER_SUNK, col, loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);

        /* [TODO] These are dependent on the model.
         */
        group_ptr = EG_RadioGroup_Create("speed:group");

        for (i=0; i<SPEED_COUNT; i++){

                if (speed[i]==1.0)
                        strcpy(speed_caption, "Real time");
                else
                        sprintf(speed_caption, "%-1.1f", speed[i]);

                SPEED_CALC_LOC( (i+2), 0);

                widget_ptr = EG_RadioButton_Create(NULL, col, speed_caption, loc);
                EG_RadioGroup_AddButton(group_ptr, widget_ptr);

                (void) EG_RadioButton_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
                 , CreateWindowsMenuItem(speed_menu_id[i], widget_ptr));

                if (speed[i]==1.0)
                        EG_RadioGroup_Select(widget_ptr);
        }
        EG_Window_AddWidget(window_ptr, group_ptr);

        /* Back button
         */
        loc = CalcRect(win.w - 10 - 64, win.h-10-20, 64, 20);
        widget_ptr = EG_Button_Create("but_speed_back", col, EG_BUTTON_ALIGN_CENTER, "Back", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, Speed_Button_Back, &gui);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);
        (void) EG_Button_GetFocus(widget_ptr);

        gui.win_speed_ptr = window_ptr;
        return(EG_TRUE);
}
static void Destroy_Speed(void)
{
        EG_Window_DestroyAllChildWidgets(gui.win_speed_ptr);
        EG_Window_Destroy(gui.win_speed_ptr);
}






/*        =        =        =        =        =        =        =        =        =
 *        About:
 *        =        =        =        =        =        =        =        =        =
 */


static void About_Button_Back(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;
        tmp = widget_ptr;
        tmp2 = user_ptr;

        EG_Window_Hide(gui.win_about_ptr);
        SDL_Delay(100);
        EG_Window_Show(gui.win_menu_ptr);
        SetActiveWindow(gui.win_menu_ptr);
}
static EG_BOOL Make_About(SDL_Surface *dst_ptr)
{
        EG_Window *window_ptr = NULL;
        EG_Widget *widget_ptr;

        SDL_Rect win, loc;
        SDL_Color col;
        int i;

#define ABOUT_LINES 4
        char *caption_ptr[]={
"one", 
"two", 
"three",
"four"};


        /* Window
         */
        col = CalcColor(191+32, 191+32, 0);
        win = CalcRectCentered(480, 480);
        window_ptr = EG_Window_Create("win_about", dst_ptr, col, win);
        win.x=0; win.y=0; win.w-=4; win.h-=4;

        /* Title label
         */
#        define ABOUT_CALC_LOC(r, b) loc = CalcRect(20, 10+r*16+b*10, win.w-40, 16)

        ABOUT_CALC_LOC(0, 0);
        widget_ptr = EG_Label_Create("lab_about", col, EG_LABEL_ALIGN_CENTER, "BeebEm PSP (v1.0.0)", loc);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);

        ABOUT_CALC_LOC(1,0); loc.x-=10; loc.y+=8; loc.h = 25*16; loc.w+=20;
        widget_ptr = EG_Box_Create("about_box", EG_BOX_BORDER_SUNK, col, loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);

        for(i=0; i<ABOUT_LINES; i++){

                ABOUT_CALC_LOC( (i+2), 0);

                widget_ptr = EG_Label_Create(NULL, col, EG_LABEL_ALIGN_LEFT, caption_ptr[i], loc);
                (void) EG_Window_AddWidget(window_ptr, widget_ptr);
        }

        /* Back button
         */
        loc = CalcRect(win.w - 10 - 64, 480-32, 64, 20);
        widget_ptr = EG_Button_Create("but_about_back", col, EG_BUTTON_ALIGN_CENTER, "Back", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, About_Button_Back, &gui);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);
        (void) EG_Button_GetFocus(widget_ptr);

        gui.win_about_ptr = window_ptr;
        return(EG_TRUE);
}
static void Destroy_About(void)
{
        EG_Window_DestroyAllChildWidgets(gui.win_about_ptr);
        EG_Window_Destroy(gui.win_about_ptr);
}



/*      =       =       =       =       =       =       =       =       =
 *        Devices:
 *        =        =        =        =        =        =        =        =        =
 */

static void Devices_Button_Back(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;
        tmp = widget_ptr;
        tmp2 = user_ptr;

        EG_Window_Hide(gui.win_devices_ptr);
        SDL_Delay(100);
        EG_Window_Show(gui.win_menu_ptr);
        SetActiveWindow(gui.win_menu_ptr);
}
static EG_BOOL Make_Devices(SDL_Surface *dst_ptr)
{
        EG_Window *window_ptr = NULL;
        EG_Widget *widget_ptr;
//        EG_Widget *group1_ptr, *group2_ptr, *group3_ptr;

        SDL_Rect win, loc;
        SDL_Color col;

        /* Window
         */
        col = CalcColor(191+32, 191+32, 0);
        win = CalcRectCentered(480-64, 512-128);
        window_ptr = EG_Window_Create("win_devices", dst_ptr, col, win);
        win.x=0; win.y=0; win.w-=4; win.h-=4;


        /* Title label
         */
#        define DEVICES_CALC_LOC(r, b) loc = CalcRect(20, 10+r*20+b*10, win.w-40, 16)

        DEVICES_CALC_LOC(0, 0);
        widget_ptr = EG_Label_Create("lab_devices", col, EG_LABEL_ALIGN_CENTER, "Devices:", loc);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);

//        group1_ptr = EG_RadioGroup_Create("system_model");

//        SYSTEM_CALC_LOC(1,0); loc.x-=10; loc.y+=8; loc.h = 20*5; loc.w+=20;
  //      widget_ptr = EG_Box_Create("system_model_box", EG_BOX_BORDER_SUNK, col, loc);
    //    EG_Window_AddWidget(window_ptr, widget_ptr);

//        SYSTEM_CALC_LOC(2, 0);
//        widget_ptr = EG_RadioButton_Create("model_b", col, "BBC Model B", loc);
//        EG_RadioGroup_AddButton(group1_ptr, widget_ptr);

//        SYSTEM_CALC_LOC(3, 0);
//        widget_ptr = EG_RadioButton_Create("model_b_and_integra_b", col, "BBC Model B + Integra-B", loc);
//        EG_RadioButton_Disable(widget_ptr);
//        EG_RadioGroup_AddButton(group1_ptr, widget_ptr);

//        SYSTEM_CALC_LOC(4, 0);
//        widget_ptr = EG_RadioButton_Create("model_b_plus", col, "BBC Model B Plus", loc);
//        EG_RadioButton_Disable(widget_ptr);
//        EG_RadioGroup_AddButton(group1_ptr, widget_ptr);
 
//        SYSTEM_CALC_LOC(5, 0);
//        widget_ptr = EG_RadioButton_Create("model_master_128", col, "BBC Master 128", loc);
//        EG_RadioGroup_AddButton(group1_ptr, widget_ptr);

  //      (void) EG_Window_AddWidget(window_ptr, group1_ptr);



//        SYSTEM_CALC_LOC(19, 0);
//        widget_ptr = EG_Label_Create("lab_econet", col, EG_LABEL_ALIGN_CENTER, "Network:", loc);
//        EG_Window_AddWidget(window_ptr, widget_ptr);

//        SYSTEM_CALC_LOC(20,0); loc.x-=10; loc.y+=8; loc.h = 20*2; loc.w+=20;
//        widget_ptr = EG_Box_Create("eco_box", EG_BOX_BORDER_SUNK, col, loc);
//        EG_Window_AddWidget(window_ptr, widget_ptr);

//        SYSTEM_CALC_LOC(21, 0);
//        widget_ptr = EG_TickBox_Create("sys_eco", col, "Econet", loc);
//        EG_TickBox_Disable(widget_ptr);
//        EG_Window_AddWidget(window_ptr, widget_ptr);


        /* Back button
         */
        loc = CalcRect(win.w - 10 - 64, win.h-10-20, 64, 20);
        widget_ptr = EG_Button_Create("but_devices_back", col
         , EG_BUTTON_ALIGN_CENTER, "Back", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, Devices_Button_Back
         , &gui);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);

        gui.win_devices_ptr = window_ptr;
        return(EG_TRUE);
}
static void Destroy_Devices(void)
{
        EG_Window_DestroyAllChildWidgets(gui.win_devices_ptr);
        EG_Window_Destroy(gui.win_devices_ptr);
}


/*      =       =       =       =       =       =       =       =       =
 *        Disks:
 *        =        =        =        =        =        =        =        =        =
 */

static void Disks_Button_Back(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;
        tmp = widget_ptr;
        tmp2 = user_ptr;

        EG_Window_Hide(gui.win_disks_ptr);
        SDL_Delay(100);
        EG_Window_Show(gui.win_menu_ptr);
        SetActiveWindow(gui.win_menu_ptr);
}
static EG_BOOL Make_Disks(SDL_Surface *dst_ptr)
{
        EG_Window *window_ptr = NULL;
        EG_Widget *widget_ptr;
//        EG_Widget *group1_ptr, *group2_ptr, *group3_ptr;

        SDL_Rect win, loc;
        SDL_Color col;

        /* Window
         */
        col = CalcColor(191+32, 191+32, 0);
        win = CalcRectCentered(480, 480);
        window_ptr = EG_Window_Create("win_disks", dst_ptr, col, win);
        win.x=0; win.y=0; win.w-=4; win.h-=4;


        /* Title label
         */
#        define DISKS_CALC_LOC(r, b) loc = CalcRect(20, 10+r*20+b*10, win.w-40, 19)

        DISKS_CALC_LOC(0, 0);
        widget_ptr = EG_Label_Create("lab_disks", col, EG_LABEL_ALIGN_CENTER, "Disks:", loc);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);





        DISKS_CALC_LOC(3, 0);
        widget_ptr = EG_Button_Create("disks:Drive:run_disc", col, EG_BUTTON_ALIGN_CENTER, "Select, load and run a disc", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, RunDisc, window_ptr);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);

        DISKS_CALC_LOC(4, 0);
        widget_ptr = EG_Button_Create("disks:Drive0:load_state", col, EG_BUTTON_ALIGN_CENTER, "Load state", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, LoadState, window_ptr);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);

        DISKS_CALC_LOC(5, 0);
        widget_ptr = EG_Button_Create("disks:Drive0:load_disk", col, EG_BUTTON_ALIGN_CENTER, "Change disk in drive 0", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(IDM_LOADDISC0, widget_ptr));

        DISKS_CALC_LOC(6, 0);
        widget_ptr = EG_Button_Create("disks:Drive1:load_disk", col, EG_BUTTON_ALIGN_CENTER, "Change disk in drive 1", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(IDM_LOADDISC1, widget_ptr));


        DISKS_CALC_LOC(8, 0);
        widget_ptr = EG_Button_Create("disks:Drive0:save_state", col, EG_BUTTON_ALIGN_CENTER, "Save state", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, SaveState, window_ptr);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);


        DISKS_CALC_LOC(10, 0);
        widget_ptr = EG_TickBox_Create("disks:WriteProtect0:yes/no", col, "Disk 0 is write protected.", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(IDM_WPDISC0, widget_ptr));

        DISKS_CALC_LOC(11, 0);
        widget_ptr = EG_TickBox_Create("disks:WriteProtect1:yes/no", col, "Disk 1 is write protected.", loc);
        EG_Window_AddWidget(window_ptr, widget_ptr);
        (void) EG_TickBox_SetMyCallback_OnClick(widget_ptr, ProcessGUIOption
         , CreateWindowsMenuItem(IDM_WPDISC1, widget_ptr));


        col = CalcColor(191+32, 0, 0);
        DISKS_CALC_LOC(13, 0);
        widget_ptr = EG_Label_Create("disks:lab_1", col, EG_LABEL_ALIGN_LEFT, "Watch out for GTK opening the file selector", loc);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);

        DISKS_CALC_LOC(14, 0); loc.y--; 
        widget_ptr = EG_Label_Create("disks:lab_2", col, EG_LABEL_ALIGN_LEFT, "behind the emulator window!                ", loc);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);
        col = CalcColor(191+32, 191+32, 0);

        DISKS_CALC_LOC(18, 0); loc.y--;
        widget_ptr = EG_Label_Create("disks:lab_2", col, EG_LABEL_ALIGN_CENTER, "(this mess is only temporary!)", loc);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);



//      SYSTEM_CALC_LOC(21, 0);
//      widget_ptr = EG_TickBox_Create("sys_eco", col, "Econet", loc);
//      EG_TickBox_Disable(widget_ptr);
//      EG_Window_AddWidget(window_ptr, widget_ptr);





//        group1_ptr = EG_RadioGroup_Create("system_model");

//        SYSTEM_CALC_LOC(1,0); loc.x-=10; loc.y+=8; loc.h = 20*5; loc.w+=20;
  //      widget_ptr = EG_Box_Create("system_model_box", EG_BOX_BORDER_SUNK, col, loc);
    //    EG_Window_AddWidget(window_ptr, widget_ptr);

//        SYSTEM_CALC_LOC(2, 0);
//        widget_ptr = EG_RadioButton_Create("model_b", col, "BBC Model B", loc);
//        EG_RadioGroup_AddButton(group1_ptr, widget_ptr);

//        SYSTEM_CALC_LOC(3, 0);
//        widget_ptr = EG_RadioButton_Create("model_b_and_integra_b", col, "BBC Model B + Integra-B", loc);
//        EG_RadioButton_Disable(widget_ptr);
//        EG_RadioGroup_AddButton(group1_ptr, widget_ptr);

//        SYSTEM_CALC_LOC(4, 0);
//        widget_ptr = EG_RadioButton_Create("model_b_plus", col, "BBC Model B Plus", loc);
//        EG_RadioButton_Disable(widget_ptr);
//        EG_RadioGroup_AddButton(group1_ptr, widget_ptr);
 
//        SYSTEM_CALC_LOC(5, 0);
//        widget_ptr = EG_RadioButton_Create("model_master_128", col, "BBC Master 128", loc);
//        EG_RadioGroup_AddButton(group1_ptr, widget_ptr);

  //      (void) EG_Window_AddWidget(window_ptr, group1_ptr);



//        SYSTEM_CALC_LOC(19, 0);
//        widget_ptr = EG_Label_Create("lab_econet", col, EG_LABEL_ALIGN_CENTER, "Network:", loc);
//        EG_Window_AddWidget(window_ptr, widget_ptr);

//        SYSTEM_CALC_LOC(20,0); loc.x-=10; loc.y+=8; loc.h = 20*2; loc.w+=20;
//        widget_ptr = EG_Box_Create("eco_box", EG_BOX_BORDER_SUNK, col, loc);
//        EG_Window_AddWidget(window_ptr, widget_ptr);

//        SYSTEM_CALC_LOC(21, 0);
//        widget_ptr = EG_TickBox_Create("sys_eco", col, "Econet", loc);
//        EG_TickBox_Disable(widget_ptr);
//        EG_Window_AddWidget(window_ptr, widget_ptr);


        /* Back button
         */
        loc = CalcRect(win.w - 10 - 64, win.h-10-20, 64, 20);
        widget_ptr = EG_Button_Create("but_disks_back", col
         , EG_BUTTON_ALIGN_CENTER, "Back", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, Disks_Button_Back
         , &gui);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);

        gui.win_disks_ptr = window_ptr;
        return(EG_TRUE);
}
static void Destroy_Disks(void)
{
        EG_Window_DestroyAllChildWidgets(gui.win_disks_ptr);
        EG_Window_Destroy(gui.win_disks_ptr);
}


/*      =       =       =       =       =       =       =       =       =
 *        Keyboard:
 *        =        =        =        =        =        =        =        =        =
 */

static void Keyboard_Button_Back(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;
        tmp = widget_ptr;
        tmp2 = user_ptr;

        EG_Window_Hide(gui.win_keyboard_ptr);
        SDL_Delay(100);
        EG_Window_Show(gui.win_menu_ptr);
        SetActiveWindow(gui.win_menu_ptr);
}
static EG_BOOL Make_Keyboard(SDL_Surface *dst_ptr)
{
        EG_Window *window_ptr = NULL;
        EG_Widget *widget_ptr;
//        EG_Widget *group1_ptr, *group2_ptr, *group3_ptr;

        SDL_Rect win, loc;
        SDL_Color col;

        /* Window
         */
        col = CalcColor(191+32, 191+32, 0);
        win = CalcRectCentered(480-256, 512-256);
        window_ptr = EG_Window_Create("win_keyboard", dst_ptr, col, win);
        win.x=0; win.y=0; win.w-=4; win.h-=4;


        /* Title label
         */
#        define KEYBOARD_CALC_LOC(r, b) loc = CalcRect(20, 10+r*20+b*10, win.w-40, 16)

        KEYBOARD_CALC_LOC(0, 0);
        widget_ptr = EG_Label_Create("lab_keyboard", col, EG_LABEL_ALIGN_CENTER, "Keyboard:", loc);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);

        /* Back button
         */
        loc = CalcRect(win.w - 10 - 64, win.h-10-20, 64, 20);
        widget_ptr = EG_Button_Create("but_keyboard_back", col
         , EG_BUTTON_ALIGN_CENTER, "Back", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, Keyboard_Button_Back
         , &gui);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);

        gui.win_keyboard_ptr = window_ptr;
        return(EG_TRUE);
}
static void Destroy_Keyboard(void)
{
        EG_Window_DestroyAllChildWidgets(gui.win_keyboard_ptr);
        EG_Window_Destroy(gui.win_keyboard_ptr);
}


/*      =       =       =       =       =       =       =       =       =
 *        AMX:
 *        =        =        =        =        =        =        =        =        =
 */

static void AMX_Button_Back(EG_Widget *widget_ptr, void *user_ptr)
{
        EG_Widget *tmp;
        void *tmp2;
        tmp = widget_ptr;
        tmp2 = user_ptr;

        EG_Window_Hide(gui.win_amx_ptr);
        SDL_Delay(100);
        EG_Window_Show(gui.win_menu_ptr);
        SetActiveWindow(gui.win_menu_ptr);
}
static EG_BOOL Make_AMX(SDL_Surface *dst_ptr)
{
        EG_Window *window_ptr = NULL;
        EG_Widget *widget_ptr;
//        EG_Widget *group1_ptr, *group2_ptr, *group3_ptr;

        SDL_Rect win, loc;
        SDL_Color col;

        /* Window
         */
        col = CalcColor(191+32, 191+32, 0);
        win = CalcRectCentered(480-256, 512-256);
        window_ptr = EG_Window_Create("win_amx", dst_ptr, col, win);
        win.x=0; win.y=0; win.w-=4; win.h-=4;


        /* Title label
         */
#        define AMX_CALC_LOC(r, b) loc = CalcRect(20, 10+r*20+b*10, win.w-40, 16)

        AMX_CALC_LOC(0, 0);
        widget_ptr = EG_Label_Create("lab_amx", col, EG_LABEL_ALIGN_CENTER, "AMX:", loc);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);

        /* Back button
         */
        loc = CalcRect(win.w - 10 - 64, win.h-10-20, 64, 20);
        widget_ptr = EG_Button_Create("but_amx_back", col
         , EG_BUTTON_ALIGN_CENTER, "Back", loc);
        (void) EG_Button_SetMyCallback_OnClick(widget_ptr, AMX_Button_Back
         , &gui);
        (void) EG_Window_AddWidget(window_ptr, widget_ptr);

        gui.win_amx_ptr = window_ptr;
        return(EG_TRUE);
}
static void Destroy_AMX(void)
{
        EG_Window_DestroyAllChildWidgets(gui.win_amx_ptr);
        EG_Window_Destroy(gui.win_amx_ptr);
}


