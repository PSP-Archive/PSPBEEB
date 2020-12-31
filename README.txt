
    Welcome to PSPBEEB

Original Authors of BeebEm

  David Alan Gilbert first Unix version
  Richard Gellman and Mike Wyatt first windows version
  David Eggleston new version for FreeBSD based on GTK/SDL.

Author of the PSP port version 

  Ludovic.Jacomme also known as Zx-81 (zx81.zx81@gmail.com)


1. INTRODUCTION
   ------------

  BeebEm is a famous emulator of the BBC Micro computer series running on both
  windows and unix. (see http://www.mikebuk.dsl.pipex.com/beebem/)

  Originally developed for UNIX systems in 1994 by David Alan Gilbert, it was
  then ported to windows by Richard Gellman and Mike Wyatt in 1997.  At the
  start of October 2005, David Eggleston started porting the current Windows
  version of BeebEm (version 2.3) to FreeBSD. 

  PSPBEEB is a port of the version 0.0.4 from Dave Eggleston, using GTK 
  and SDL environement.
  
  Thanks to Danzel and Jeff Chen for their virtual keyboard,
  and to all PSPSDK developpers.

  Big big thanks goes to Mr Nick666 for the graphics icons and
  background images !

  Thanks should goes also to Pou-chan who developped the multiple 
  keyboard mapping feature.

  This package is under "Beebem (c) Copyright", read COPYING.txt file for
  more information about it.


2. INSTALLATION
   ------------

  Unzip the zip file, and copy the content of the directory fw3.x or fw1.5
  (depending of the version of your firmware) on the psp/game, psp/game150,
  or psp/game3xx if you use custom firmware 3.xx-OE.

  Put your disk image files on "discs" sub-directory, and save states 
  in "state" sub-directory.

  It has been developped on linux for Firmware 3.71-m33 and i hope it works
  also for other firmwares.

  For any comments or questions on this version, please visit 
  http://zx81.zx81.free.fr, http://www.dcemu.co.uk


3. CONTROL
   ------------

3.1 - Virtual keyboard

  In the BBC emulator window, there are three different mapping (standard,
  left trigger, and right Trigger mappings).  
  You can toggle between while playing inside the emulator using the two PSP
  trigger keys.

  -------------------------------------
  PSP        BBC            (standard)

  Square     Delete
  Triangle   Enter
  Circle     Break
  Cross      Space        
  Up         Up
  Down       Down
  Left       Left 
  Right      Right

  Analog     C,D,E,F

  -------------------------------------
  PSP        BBC   (left trigger)

  Square     Delete
  Triangle   Return
  Circle     Break
  Cross      Space        
  Up         Up
  Down       Down
  Left       Left 
  Right      Right

  Analog     G,H,I,J

  -------------------------------------
  PSP        BBC   (right trigger)

  Square     Delete
  Triangle   Return
  Circle     Break
  Cross      Space        
  Up         Up
  Down       Down
  Left       Left 
  Right      Right

  Analog     K,L,M,N
  
  Press Start+L+R   to exit and return to eloader.
  Press Select      to enter in emulator main menu.
  Press Start       open/close the On-Screen keyboard

  In the main menu

  RTrigger   Reset the emulator

  Triangle   Go Up directory
  Cross      Valid
  Circle     Valid
  Square     Go Back to the emulator window

  The On-Screen Keyboard of "Danzel" and "Jeff Chen"

  Use Analog stick to choose one of the 9 squares, and
  use Triangle, Square, Cross and Circle to choose one
  of the 4 letters of the highlighted square.

  Use LTrigger and RTrigger to see other 9 squares
  figures.

3.2 - IR keyboard

  You can also use IR keyboard. Edit the pspirkeyb.ini
  file to specify your IR keyboard model, and modify
  eventually layout keyboard files in the keymap
  directory.

  The following mapping is done :

  IR-keyboard   PSP

  Cursor        Digital Pad
  
  Tab           Start
  Ctrl-W        Start
  
  Escape        Select
  Ctrl-Q        Select
  
  Ctrl-E        Triangle
  Ctrl-X        Cross
  Ctrl-S        Square
  Ctrl-F        Circle
  Ctrl-Z        L-trigger
  Ctrl-C        R-trigger
  
  In the emulator window you can use the IR keyboard to
  enter letters, special characters and digits.
  

4. LOADING BBC BOOTABLE DISK FILES
   ------------

  If you want to load disk image in the virtual drive of your emulator,
  you have to put your disk file (with .ssd, .dsd, .adl, .adf, .uef  
  file extension) on your PSP memory stick in the 'discs' directory.

  Then, while inside PSPBEEB emulator, just press SELECT to enter in 
  the emulator main menu, and then using the file selector choose one 
  disk file to load in the virtual drive of your emulator.

  Back to the emulator window, the disk should stard automatically.

5. LOADING BBC DISK IMAGE FILES
   ------------

  If you want to load disk image in the virtual drive of your emulator,
  you have to put your disk file (with .img file extension) on your PSP 
  memory stick in the 'discs' directory.

  Then, while inside PSPBEEB emulator, just press SELECT to enter in 
  the emulator main menu, and then using the file selector choose one 
  disk image file to load in the virtual drive of your emulator, and 
  your disk is then inserted in the drive '0' or drive '1' (depending
  of the menu you have choosen).

  (NOTE): You may find on the web .inf and data files for BBC. You will
  need to use a convertor tool (such as bbcim) to convert those file in
  a disk image usable with PSPBEEB. For example if you want to build an
  image disc with the two files FOO and FOO.INF, you can use the following
  command to build the disk image :

     bbcim -a my-foo-image.img FOO.INF FOO
  
  To display the content of your drive, you have to use the virtual keyboard
  (press START key) and type the CPC command *CAT followed by ENTER (Triangle).
  For example if you have loaded the disk file of the game "Knight Lore" :

       *CAT

  You should see something like this :

       (00)
     Drive  0                   Option 3 (EXEC)
     Dir. : 0.$                 Lib. :0.$
       
       KNIGHT                       KNIGHT1
       KNIGHT2

  (NOTE-1) You can also use directly the shortcut in the emulator menu (Command CAT option)
  (NOTE-2) To display the content of the second drive, use *CAT1 command instead of *CAT.

  Then if you want to run a basic program KNIGHT that is on your drive '0', you have to
  use the BBC command 'CHAIN' as follow :

       CHAIN "KNIGHT"

  (NOTE-1) If the filename of the .img (here knightlore.img) is also the name of the 
  program you want to run (here knight) then you can use directly the Command CHAIN" 
  in the emulator menu. 

  If you want to run a machine code program then you have to the *RUN command
  instead of CHAIN command :

     *RUN KNIGHT (without doublequote)

  If you want to run a script program then you have to the *EXEC command instead 
  of *RUN command :

     *EXEC KNIGHT (without doublequote)


  (NOTE-2) If the command to run is different from the filename then you can
  specify the proper command to run in the file run.txt, for example :

       mygame=CHAIN "DISC"
   or  mygame=*RUN DISC
   or  mygame=*EXEC DISC

  Then, while using the Command menu, the emulator will type CHAIN "DISC"
  (*RUN, or *EXEC) instead of CHAIN "mygame" (i.e *RUN mygame or *EXEC mygame). 

  Have a look the file run.txt for details about the syntax.


6. LOADING KEY MAPPING FILES
   ------------

  For given games, the default keyboard mapping between PSP Keys and BBC keys,
  is not suitable, and the game can't be played on PSPBEEB.

  To overcome the issue, you can write your own mapping file. Using notepad for
  example you can edit a file with the .kbd extension and put it in the kbd 
  directory.

  For the exact syntax of those mapping files, have a look on sample files already
  presents in the kbd directory (default.kbd etc ...).

  After writting such keyboard mapping file, you can load them using the main menu
  inside the emulator.

  If the keyboard filename is the same as the disk filename (.ssd etc ...)
  then when you load this snapshot file or this disk, the corresponding keyboard 
  file is automatically loaded !

  You can now use the Keyboard menu and edit, load and save your
  keyboard mapping files inside the emulator. The Save option save the .kbd
  file in the kbd directory using the "Game Name" as filename. The game name
  is displayed on the right corner in the emulator menu.
  
7. COMPILATION
   ------------

  It has been developped under Linux using gcc with PSPSDK. 
  To rebuild the homebrew run the Makefile in the src archive.


8. KEYBOARD
   ------------

  You will find keyboard files from Bah and Pou-chan, in the keyboard
  directory of this archive.
