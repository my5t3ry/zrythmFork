CODE STRUCTURE:
  - inc contains the header files
  - src contains the source files. It should mimic the structure inside inc
  - resources/ui contains GTK UI files
  - resources/icons contains icons
  - ext should be used in case a statically linked library is needed (preferably git submodule)

BUILDING:
  The project uses autoconf and a custom Makefile.in, so the steps are
  1. run `autoreconf -fi` (or just run the included autogen.sh file). This will generate the configure script
  2. then `./configure`. This will generate the Makefile
  3. then `make -jN` (where N is the number of cores you want to use. the higher the number the faster it will build). The program is now built. It will need to be installed the first time before it can run. After that it can just be built and run.
  4. `sudo make install` to install the program
  The built program will be in build/debug/zrythm by default.

DEBUGGING:
  Use `gdb build/debug/zrythm`
  Can also do `G_DEBUG=fatal_warnings,signals,actions G_ENABLE_DIAGNOSTIC=1 gdb build/debug/zrythm`. G_DEBUG will trigger break points at GTK warning messages and  G_ENABLE_DIAGNOSTIC is used to break at deprecation  warnings for GTK+3 (must all be fixed before porting to GTK+4).

COMMENTING:
  - Document how to use the function in the header file
  - Document how the function works (if it's not obvious from the code) in the source file
  - Document each logical block within a function (what it does so one can understand what the function does at each step by only reading the comments)

CODING STYLE:
  GNU style is preferable

INCLUDE ORDER:
standard C library headers go first in alphabetic order, then local headers in alphabetic order, then GTK or libdazzle headers, then all other headers in alphabetic order

OTHER:
  1. Backend files should have a _new() and a _init() function. The _new is used to create the object and the _init is used to initialize it. This is so that objects can be created somewhere and initialized somewhere else, for whatever reason.
  2. GUI widgets that exist no matter what (like the tracklist, mixer, browser, etc.) should be initialized by the UI files with no _new function. Widgets that are created dynamically like channels and tracks should have a _new function. In both cases they should have a _setup function to initialize them with data and a _refresh function to update the widget to reflect the backend data. _setup must only be called once and _refresh can be called any time, but excessive calling should be avoided on widgets with many children such as the mixer and tracklist to avoid performance issues.

  Anything else just ask in the chatrooms!
