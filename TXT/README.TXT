
                          Turaco - Arcade Games Editor  
                          ----------------------------

                                 Version 1.1.3
                                  04 Oct 1999


Freeware by:
    Scott "Jerry" Lawrence     jerry@absynth.com
    Ivan Mackintosh            ivan@absynth.com

With contributions by:
    Chris "Zwaxy" Moore        chris.moore@writeme.com

World Wide Web homepage:
    http://www.csh.rit.edu/~jerry/turaco/

Related Pages:
    The Turaco Message Board   http://www.csh.rit.edu/~jerry/turaco/wwwboard/
    The AGE homepage           http://dales.rmplc.co.uk/ivan/age/

Copyright Message:
    This program and source code may be freely copied, and used in other 
    programs as long as proper credit is given for the writers (us) for the
    modules or code blocks that you use.  You may distribute TURACO only
    in either the source or executable distribution packages as received by 
    you (you) from the web address above (above).  If you want to include 
    this program on a cd-rom or other media to be sold, or given away for 
    free, you must contact us... We want copies of it to show our families
    and friends to brag about being published!  ;)

Legal Mumbo-Jumbo:
    No warranty is neither expressed nor implied for any damages which 
    may occur during the proper or improper use of this program and its
    associated files.  Use it at your own risk!

    We make no moral or legal statement about the use of un-licensed rom
    images which are not in your ownrship.  In most cases, testing was 
    performed with rom-images for games that we own.  Many of the "supported" 
    games have not been tested. 

    Product and game names are trademarks of their respective companies.


0.0 Table of Contents
---------------------

 0.0 Table of Contents
 1.0 Introduction
     1.1 History
     1.2 What's in a name?
 2.0 System Requirements
 3.0 Currently Supported Games
 4.0 How to Use 
     4.1 Screen Layout
 5.0 Command Line Options
 6.0 Menu Options
 7.0 Keyboard Summary
     7.1 General 
     7.2 Sprite Editor
 8.0 User Preferences
     8.1 Use driver subdirectories
     8.2 Use new font
     8.3 Troll Magic
     8.4 Rom Paths
     8.5 Screen Resolution
     8.A The TURACO.INI file
 9.0 Error Messages
10.0 Creating a New Game Driver
11.0 How to Contribute Updates
 A.0 Known Issues



1.0 Introduction
----------------
    TURACO allows the graphics stored in the ROMs for certain arcade games 
    to be edited allowing you to modify or add completely new graphics to 
    the game.  There will be support for editing text strings, maps, and 
    attributes of some games in future versions.

1.1 History

    TURACO is the next generation of "AGE", the Arcade Games Editor.  AGE 
    has been under development since November of 1998.  It started out its
    life as a font editor that Ivan was working on. Then it was added to 
    by Jerry, and Chris.  Ivan created the original structure, many drivers, 
    and most of the origial functionality.  Jerry, who had started to write
    a pacman editor, "Stylist", earlier that spring then came by and added 
    bitmap manipulations, oodles more drivers, map editing, text editing, 
    and miscellaneuous functionalty.  Chris added the MAME loader, and 
    eventually, the initial version of the INI loaders, and hacked MAME 
    to create the INI driver files.

    But there was a problem.  Being that it was being added to, and added 
    to, and, well, added to, some problems were creeping up.  We were getting 
    random, but reproducable crashes, the user interface was never great, 
    and the user could not modify drivers themselves without recompiling the 
    program.  We (Ivan and Jerry) decided a re-write was in order.  The 
    re-write started about a month prior to the release of the final 
    version (v0.7) of AGE.  Jerry decided to use ALLEGRO's GUI functions, 
    and created the necessary GUI objects (palette display, palette chooser, 
    bitmap editor, sprite palette.)

    Jerry did about half of TURACO, with Ivan adding in pretty much all 
    of the internal driver handling, palette internals, and patch internals, 
    borrowing code from AGE where applicable.  Chris has only had a ghostly 
    influence, in that it was his effort in the INI drivers for AGE that was 
    being re-used, along with the drivers created from his MAME hack, since
    upgraded by Ivan.

1.2 What's in a name?

    This new version has also had a slight identity crisis in that we 
    weren't sure what to call it.  Jerry decided (as he was the one naming
    files, and writing the makefile) to call it "AGE 2", then "New Age" 
    and then briefly changed to "Helen Shapiro".  Her lawyers contacted 
    us, and made us change it.  Jerry then finally settled upon "TURACO", 
    based on a very pretty bird in the Rochester NY Zoo. 


2.0 System Requirements 
-----------------------
    486sx or better running MSDOS or Windows 95/98/NT
    A properly configured mouse 
    ROMs for the game that you want to edit

    I've tried it on at least 3 systems with no problems:
	Gateway 2000 Handbook 486sx-33, 4mb and 20mb ram, MS-DOS 6.22
	Homebuilt 486dx4-100, 16mb ram, Windows 95
	Homebuilt P166, 64mb ram, Windows 95

    Some of the larger games, like Bubble Bobble, may require a few more
    megabytes of free memory over smaller games, like Pac-Man.


3.0 Currently supported games
-----------------------------
    TURACO currently supports many different games and their variants.
    Be sure to get the latest driver .zip file from the above distribution
    address.  For a full list of games supported including the subdirectory
    names for the ROMs and the ROM names themselves type:

	turaco -listroms

    from the command line. Note that this displays many screens of 
    information typing:

	turaco -listroms | more

    That allows you to view a single screen of information at a time.

    If you wish to just see which romsets are supported, without listing
    each individual rom file required, try:

	turaco -list

    If you want to see a list of all supported romsets, the name of the 
    set and the directory that the driver is in, try typing:

	turaco -listfull
     or 
	turaco -listfull | more

    Keep in mind that it is doing a fair share of work to display this 
    information; loading in each driver file (more than 800 of them),
    then searching in them for the information, sorting it, then
    displaying it.  This is why the file selector inside the program 
    needs a few seconds to display the list of supported games.


4.0 How to Use
--------------
    The easiest way to use TURACO is to just run TURACO from the directory
    in which you uncompressed it into.  Make sure that the path to your
    uncompressed roms is included in the turaco.ini.  Some example paths 
    are included in that file.  Generally, you can point it to the path
    of MAME's rom directory, and you should be fine, as long as your 
    roms are not compressed.  You can change the path from the 
    EDIT->PREFERENCES menu option.

    If you don't have MAME then you can use the -listroms command line to 
    view what subdirectories you need to create and what files to copy there.

    If you have the correct game, but have merged roms, renamed roms, or 
    some other such nonsense, then if you are crafty enough, you can 
    change the driver files you need to reflect this.  Otherwise you will
    have to un-merge the roms, or rename them to the proper names.

    TURACO DOES NOT SUPPORT ROMS THAT ARE COMPRESSED INTO .ZIP FILES! 
    DE-COMPRESS THEM INTO THEIR APPROPRIATE DIRECTORY!  ie: mspacman.zip
    should decompres "5e.bin" and "5f.bin" (or whatever the files are 
    called) into the directory called "mspacman".

    Run TURACO and you will be presented with the graphics editor screen.
    To load a game, Select "Change Game" from the file menu.  From here 
    you should see it scanning the drivers to get filenames and other 
    information.  It should take no longer than a few seconds.  You will 
    then be presented with a file selector from which you can choose the 
    game to load.  In the upper right, you will see the driver's 
    category/directory name, filename, and other miscellaneous information
    about the driver.  To select a game, scroll through the (huge) list of 
    games, and select the game you'd like to load.  You can also press the 
    first letter of the game to skip to that group.  For example, if you 
    press "p", it will scroll to "Pac & Pal".  If you press it again, it 
    will go to "Pac Man".  Try it out... you'll get the idea.

    From there, select OK to load the game.

    If all was successful, you should now be presented with the first 
    graphics bank of that game.  Generally, the first bank includes the 
    alphabet, and numbers.  Pac-Man, for example, also includes the 
    graphics for the mazes.


4.1 Screen Layout

    The layout of this screen is as follows:

    At the top of the screen is the menu bar.

    Below that to the left is the sprite editor, and immediately to 
    it's upper right is the 1x zoom of the sprite being edited.

    To the right of the sprite editor is the color displayer, with the 
    current foreground color (left mouse button) in the center, and the 
    background color (right mouse button) surrrounding it.

    To the right of that is the color palette.  Click on this with the left
    button to select the new foreground color, and with the right button to 
    select the new background color.  

    To the right of that, you should see a listing of the current palette 
    number and the total palettes available.  as well as the buttons to 
    change the current palette.  ie 1/15 means that palette 2 is being 
    displayed, out of 15.  

    Just below that are the "EDIT" and "BACK" buttons.  We'll get to those 
    below...

    Below all of this is the name of the game loaded in, ie: "Mega Bunny 
    Blaster".

    To the right of that is the bank number display and selector.  If a 
    game has multiple graphics banks, you can use these gadgets to change 
    which bank you are looking at as with the palette above.

    Below that is the sprite palette.  By clicking on a sprite in this gadget,
    the sprite that the mouse cursor is over will be selected.  Double-clicking
    with the left mouse button on a sprite will bring it up to the bitmap 
    editor window.  Clicking on the sprite, then the "EDIT" button does the 
    same thing.  Double-clicking with the right mouse button will return 
    the sprite in the bitmap edit window back to the location in the sprite 
    palette. Clicking on the sprite, then on the "BACK" button does the same 
    thing.

    To the right of the sprite palette is a display showing the sprite number
    selected, over the total number of sprites in the current bank.  If the
    mouse is over the sprite palette, then the upper number will be in 
    brackets [] showing which sprite the cursor is currently over.

    When in the sprite editor, you can do some simple manipulations of the
    image using the arrow keys to move it around, and a few alt-keys to rotate
    and flip the image.  These are also available from the "Sprite" menu.

    Reference section 7.0 below to find out what keyboard shortcuts exist.



5.0 Command Line Options
------------------------

    Syntax:

    TURACO.EXE [short game name] [options ...]

    [short game name] 

	This option parameter allows you to specify the game you wish to 
	edit bypassing the need to use the game load selector. The short 
	game name is the up to 8 character directory where the roms for 
	the game are stored.  ie:  turaco pacman

    [options ... ]

    -help
	Display all supported command line options

    -list / -listroms / -listfull / -listhtml

	These options allow you to list the roms currently supported by 
	TURACO.  They are described in more detail in section 3.0 above.

    -v
	Display version and build information.  Useful when reporting a bug.



6.0 Menu Options
----------------

The menus are arranged by functional group.  We tried arranging them 
alphabetically, but it did not work out too well.  ;)

FILE               ALT + F     File Operations
----
  Change Game      C    Load in a new game.  This will destroy any
                        changes you have not saved.

  -----------

  Save Graphics    S    Save out any changes you have made.

  Revert           R    Forget about any changes you made in this session,
                        and re-load the graphics.  Modified palettes remain.

  -----------

  Generate Patch   G    Generates a copyright-free patch to be applied to the
                        romset.

  Apply Patch      A    Apply a previously-defined patch to the romset.


  -----------

  Save C Source    E    Saves out the current graphics banks as a c structure.
                        (This feature is experimental.)

  -----------

  Exit             X    Exit from TURACO.


EDIT               ALT + E     Edit Operations
----
  Copy Sprite      C    Copy the current sprite into the clipboard

  Paste Sprite     P    Copy the clipboard into the current sprite
  
  -----------

  Save Bank PCX    S    Save out the current bank to a PCX file.  

  Load From PCX    L    Load in a sprite from a PCX file into the edit window.

  -----------

  Map Editor       M    Invoke the map editor (if supported by the game)

  Text Editor      T    Invoke the text editor (if supported by the game)

  -----------

  Add Palette      A    Create a new palette, after the previously last one,
                        with the same colors as the current palette.

  Edit Palette     E    Edit the current palette

  -----------

  Preferences      F    Modify Turaco to meet your system configuration 
                        and personal tastes.  See section 8.0 below for 
                        more information.


MODE               ALT + M     Editing Mode
----
  Paint            p    Mouse will draw in the sprite editor when clicked.

  Flood Fill       f    Mouse will flood-fill the sprite when clicked.
  
  Eyedropper       e    Mouse will get the Foreground (left) or Background
                        (right) color when clicked.

SPRITE             ALT + S     Sprite Manipulations Tools
------
  Clear            k    Clears the sprite with the background color.

  -----------
  
  Flip Horiz       x    Flips the sprite on the horizontal axis.
  
  Flip Vert        y    Flips the sprite on the vertical axis.
  
  Rotate 90        z    Rotates the sprite 90 degrees clockwise.
  
  Rotate 45        w    Rotates the sprite 45 degrees clockwise.
  
  -----------
  
  Roll Left        u    Roll-slides the sprite to the left by one pixel.
  
  Roll Down        i    Roll-slides the sprite down by one pixel.
  
  Roll Up          o    Roll-slides the sprite up by one pixel.
  
  Roll Right       p    Roll-slides the sprite to the right by one pixel.

  -----------

  Conway's Life    c    Apply the rules of Conway's Game of Life to the sprite.
  
  -----------

  Animate          a    Animate the selected range


HELP               ALT + H     Miscellaneous Help Option(s)
----  
  About...         a    Displays version information.



7.0 Key Summary
---------------

7.1 General 

    ALT + f      - File menu (see section 6.0 above)
    ALT + e      - Edit menu (see section 6.0 above)
    ALT + m      - Mode menu (see section 6.0 above)
    ALT + s      - Sprite menu (see section 6.0 above)
    ALT + h      - Help menu (see section 6.0 above)
    
    ALT + p      - Save a snapshot of the screen 
    
    ALT + A	 - Select an endpoint of the animation to be the current
    ALT + Z	 - Change the playback mode (forward, ping-pong, etc)
    F7           - Step through the defined animation steps
    F8           - Auto Animate (10 fps)

    F9           - Save out the current bank to a PCX Image file
    F10          - Load in a sprite from a PCX Image file to the edit window

    F12          - Apply "Conway's Game of Life" to the current sprite
    F11          - Reset the counter for the above

    PAGE-UP      - Slide the sprites in the palette upwards
    PAGE-DOWN    - Slide the sprites in the palette downwards
    HOME         - Reset the sprites in the palette to the first row

    [            - Previous graphics bank
    ]            - Next graphics bank
    
    <            - Previous palette
    >            - Next palette

    ,            - Eyedropper tool mode (Get fg/bg color)
    .            - Paint tool mode
    f            - Flood-fill tool mode

7.2 Sprite Editor

    Cursor Keys  - Slide the image around
    ALT + k      - Clear the image (with the background color)
    ALT + x      - Flip horizontal
    ALT + y      - Flip vertical
    ALT + z      - Rotate clockwise, 90 degrees
    ALT + w      - Rotate clockwise, 45 degrees
    ALT + g      - Toggle the gridlines



8.0 User Preferences
--------------------
    The user preferences screen will enable you to change some user options
    from within TURACO.  They are generally things that are stored in the 
    TURACO.INI file as explained below in section 8.A.


8.1 Use driver subdirectories

    The drivers are generally stored in subdirectories in the DRIVERS\
    directory.  For example the drivers for Neo-Geo based games are 
    stored in DRIVERS\NEOGEO\ and so on.  As you may know, TURACO has over
    800 drivers in over 60 directories.  That takes a long time to 
    scan through.  It is recommended that you move whatever drivers you 
    want to use into DRIVERS\ and disable this option, to prevent TURACO
    from scanning all these drivers each time you want to switch drivers.
    NOTE: This option DOES NOT disable scanning of subdirectories for 
    the -list command line options.  

8.2 Use new font

    If this is set, then my new sans-serif font will be used, otherwise 
    the original Allegro serif font will be used.

8.3 Troll Magic

    Enable the trolls to do their thing on the data.  Theirs is a dark
    and mystical power, unknown by most, feared by all.  Don't ask too 
    many questions...

8.4 Rom Paths

    This is a list of directories where TURACO is to look for the rom 
    directory files.  For example, if it is looking for the pacman roms 
    (PACMAN.5F and PACMAN.5E or whatever) it will look first in 
    .\PACMAN\PACMAN.5F then ..\PACMAN\PACMAN.5F then
    C:\SPARCADE\PACMAN\PACMAN.5F and so on.  The direction of the slash 
    \ or / is not important.  Each directory is seperated by semicolons 
    (;), and there should be no spaces in the list.

    If the roms are compressed, TURACO WILL NOT FIND THEM!  TURACO does
    not have support for compressed roms yet.

    If the driver is called "Bunny.ini", and it requires a rom file 
    named "B001.BIN", then it will look for: .\BUNNY\B001.BIN then
    ..\BUNNY\B001.BIN then C:\SPARCADE\BUNNY\B001.BIN and so on.

    If you're reading this part, then you're in the minority... Many 
    people ask about exactly what was covered RIGHT HERE.  :{


8.5 Screen Resolution

    Changing this resolution here will change the resolution that TURACO
    runs in.  The new video mode will not be changed until the next time
    you run TURACO.  If your video card does not support the mode selected,
    then it will default back to 320x240, a known working resolution.
    In the future, the dialog boxes will be scaled according to the 
    selected resoution.  For now, the larger the resolution you choose,
    the smaller the window gadgets will be on your screen.


8.A The TURACO.INI file

    This file contains the application state, user settings and other
    keen information between executions of the application.  It is in
    the standard Micro$loth .ini format (as are the game drivers.)  The
    fields are as follows:

    Section "System":

      Item "Version_Major"
           Integer
           Default = 1
           The major version of the ini file.  Turaco 1.1 uses version 1.1 
           of the ini file, so this field should read "1"

      Item "Version_Minor"
           Integer
           Default = 1
           The minor version of the ini file.  Turaco 1.1 uses version 1.1
           of the ini file, so this field should read "1"

      Item "Info"
           String
           Default = ???
           This is just a descriptive string explaining something about 
           the file.  It is completely ignored by the application for now,
           but it may be displayed in the future.

      Item "Author"
           String
           Default = ???
           This is just a string storing the author of this file.  It is 
           completely ignored by the application for now,

      Item "ROMPath"
           String List
           Default = .;..;c:/sparcade;c:/games/mame/roms
           This is a list of directories where TURACO is to look for the 
           rom directory files.  For example, if it is looking for the
           pacman roms (PACMAN.5F and PACMAN.5E or whatever) it will look
           first in .\PACMAN\PACMAN.5F then ..\PACMAN\PACMAN.5F then
           C:\SPARCADE\PACMAN\PACMAN.5F and so on.  The direction of the
           slash \ or / is not important.  Each directory is seperated by
           semicolons (;), and there should be no spaces in the list.

      Item "H_Res"
           Integer
           Default = 320

      Item "V_Res"
           Integer
           Default = 240
           This is the screen resolution to run TURACO in.  The minimal
           size is 320x240.  If you try to change either to be below that
           size, or to an illegal size, or if your video card does not 
           support the resolution you selected, it will fall back to
           320x240.  Currently, there is no scaling support, so the screen
           will be opened, but the user interface will still only take up
           the upper left 320x240 of the screen.

      Item "Driver_Subdirs"
           String ("on" or "off")
           Default = on
           This is a toggle to change whether or not TURACO will look in
           subdirectories in DRIVERS\ for more .ini files.  The standard
           distribution package contains seperate directories for each
           style of game driver (atari, Sega 8-bit, etc).  If this is set
           to "on", then all 800+ drivers will be scanned.  If it is set
           to "off", then just the drivers in DRIVERS\ will be scanned.
           It is recommended that you copy any drivers you use often into 
           DRIVERS\ and then set this to "off"; making the driver selector
           run quicker.

      Item "Troll_Magic"
           String ("on" or "off")
           Default = on
           This toggles the troll magic as mentioned above in the 
           section 8.3 above.  This enables some code in TURACO that 
           works magic, and lets the trolls re-arrange bits for us.  
           Don't ask too many questions....

      Item "Pal_Grab_Path"
           String
           Default = .  (the current directory)
	   This is where the image grabber for the palette editor stores 
	   the path.  And that's all I'm gonna say.  That's all you get.
	   Really.  That's it.

      Item "C_Source"
           String
           Default = .  (the current directory)
	   This is where the path for the .C source file saving utility
	   will save its path.  

      Item "Font"
           Integer
           Default = 1
           If this is 1, then the new font will be used, otherwise the
           original font will be used.  In the future, this may change
           to a font name, or a value other than [1,0]. 


    Section "PCX_Rip":

      Item "Path"
           String
           Default = .  (the current directory)
	   This is where the path for the .PCX file saving/loading utility
	   will save its path.  

      Item "NumPerRow"
           Integer
           Default = 32
           This is the number of sprites which will be saved on each 
           horizontal row in the image.



9.0 Error Messages
------------------

    Can't load .ini driver!  
	There is no [General] Description field or it is "None"

    INI file Error 'GfxDecodes'
	There are no graphics decodes or it is 0

    GfxBanks malloc failed 
	Memory allocation error

    GfxBanksExtra malloc failed
	Memory allocation error

    INI file corrupt - graphics bank 
	One of the elements are -1 or an element is missing

    INI file corrupt - planeoffsets 
	Wrong number of elements - It should be the same as planes

    INI file corrupt - xoffsets 
	Wrong number of elements - It should be the same as width

    INI file corrupt - yoffsets
	Wrong number of elements - It should be the same as height

    INI file corrupt - graphicsroms 
	Number of graphics roms is 0

    GfxRoms malloc failed 
	Memory allocation error

    INI file corrupt - [romname] 
	The romname listed has the wrong number of elements in the list

    Malloc failed creating default palettes 
	Memory allocation error

    Palettes Corrupt, Cols = n, n
	Something wrong with the plane suggested

    Palettes Corrupt, argc = n
	The palette has the wrong number of elements in one of the lists

    Malloc failed creating palette
	Memory allocation error

    %s not found
	File was not found

    GfxRomData malloc failed
	Memory allocation error

    File not found
	The file could not be opened properly

    ROM Size load error 
	A rom was the wrong size

    CreateBitmap failed
	Memory allocation error



10.0 Creating a New Game Driver
------------------------------

    The easiest way to create a new driver for TURACO is to copy an 
    existing driver from the DRIVERS\ directory.

    The driver's name (ie: BUNNY.INI) will become the directory that 
    the roms are looked for in(*).  ie:  ROMS\BUNNY\

    The definitions of the structures as seen in the driver file can 
    be gleaned from the source to MAME.  

    If you change or create a driver, please send it to us!

    Read INIDRIV.TXT for an explanation of each field in a driver file.

         (*) A preposition is a bad word to end a sentence on(**).
        (**) This is also a sentence ending on a preposition(***).
       (***) Not this one(****).
      (****) Nor this one(*****).
     (*****) This is the last footnote.  I was thinking of adding as many
             footnotes as the cheat book for Infocom's "Hitchhiker's Guide
             to the Galaxy", but I decided against it(******).
    (******) Damn.  Another footnote.


11.0 Updates
------------

    Please send any modifications to functionality of this program or 
    addition graphics drivers to me so that they can be included in
    subsequent releases.

    All releases, along with experimental releases are available at:

	http://www.csh.rit.edu/~jerry/turaco/

    All comments and problems should be addressed at the message board 
    which is daily checked by Jerry & occasionally Ivan at:

	http://www.csh.rit.edu/~jerry/turaco/wwwboard/

    We can be contacted by email through the web addresses at the top 
    of this file.



A.0 Known Issues
----------------

    A.1 There is no map or text editing support yet.  
	These will be in a future version.  Use "AGE" version 0.6.3.

    A.2 Not all drivers have been checked to be sure that they all work.
        The list of unchecked drivers is in the drivers directory in
        the fine "readme.txt".  If you have any problems with any specific
        drivers, bugfixes, or cool modifications or palettes for any 
        drivers, please let us know!

    A.3 There is no issue number 3.

    A.4 If you try to sell this program without notifying us first, Vinny
        and Vito will be on your doorstep to crush your head.  There is no
        known fix for this.
