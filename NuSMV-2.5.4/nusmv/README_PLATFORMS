This file contains specific information for building NuSMV under the
most used platforms. NuSMV have been successfully built and tested
under these platforms/operating systems:

1) Pc/Linux 32 and 64 bit architectures
2) Ultrasparc 5/Solaris
3) Apple Mac/Mac OS X Panther
4) Pc/Microsoft Windows

If you experience any problem while trying to build NuSMV, please get
in touch with us using the e-mail address <nusmv-users@fbk.eu>.


======================================================================
1) Linux distributions
   -------------------
   Default tools chain "configure && make && make install" might build
   the system. File README contains more detailed information about 
   standard build steps. 


======================================================================
2) Sun Solaris
   -----------
   The compilation of NuSMV under Solaris should be as straightforward
   as under Linux. It might be the case that under certain system
   configurations the native Sun compiler would be required. To allow
   for the use of a different compiler in the building chain it is
   just sufficient to assign a value to the CC variable while invoking
   the "configure" program:

   $> ./configure CC=cc

   Following call to 'make' will then be using the specified compiler
   'cc' as default C compiler.


======================================================================
3) Mac OS X
   --------
   To run NuSMV, the X11 unix environment needs to be installed. This
   is available from Apple at:

      http://www.apple.com/macosx/features/x11/download/

   It is also on the 'Developer CD' that comes with some Macs.

   Please note that NuSMV has only been tested on Mac OS X 10.3
   (Panther) although the version of the operating system does not
   matter as long as X11 is installed.

   It has to be noticed that, the runtime statistics are not available
   under Mac OS X.

 3.1) Binary Distribution
      -------------------

   For the binary distribution it is also necessary to have the Expat
   XML parser library installed. This library can be downloaded from:

      http://sourceforge.net/projects/expat/

   We recommend you use the latest stable version (currently
   1.95.7). Unzip and untar the downloaded file if necessary (StuffIt
   Expander may do this automatically). Go into the directory and run
   the following three commands:

      $> ./configure
      $> make
      $> make install

   This will install libexpat into /usr/local/lib, expat.h into
   /usr/local/include and xmlwf into /usr/local/bin.

   If you encounter any problems see the README file or
   doc/reference.html (and click on Building and Installing).


 3.2) Source Distribution
      -------------------

   The following steps need to be taken in order for NuSMV to
   correctly compile under the X11 environment in Mac OS X (again,
   only Panther has been tested):

   1) Go to CUDD's direcory and build the package, by using 
      Mac OS X specific makefile:

      $> cd cudd-2.4.1.1
      $> make -f Makefile_os_x

   2) Go to the NuSMV directory and proceed as for Linux:

      $> cd ../nusmv
      $> ./configure [OPTIONS]
      $> make

   Note that to make use of the plugin which reads an XML format trace
   (previously output by the XML Format Printer plugin) the Expat XML
   Parser library is required. This DOES NOT come as standard with the
   X11 environment and has to be installed separately (see the "Binary
   Distribution" section for details of obtaining and installing this
   library).

   With the source distribution, the Expat library is NOT required -
   although without it, you lose the ability to read back saved
   traces.


 3.3) Generating Documentation
      ------------------------

   To generate part of the documentation (the user manual and the
   tutorial), LaTeX is needed. Before installing and configuring a
   version of it, you might consider that the NuSMV binary
   distribution already provides full documentation and help files.

   latex: There are several implementations for Mac OS X, one good
          page which provides links and installation information
          for various versions is at:
          http://www.rna.nl/tex.html
  
   To generate the help on-line available at the NuSMV shell, lynx
   (http://lynx.isc.org/) or links
   (http://atrey.karlin.mff.cuni.cz/~clock/twibright/links/) are
   required. Binary versions can be obtained by looking to Mac OS X
   repositories.


======================================================================
4) Microsoft Windows
   -----------------
   
 4.1) Binary distribution
      -------------------
      Binary distribution has been tested for MS Windows XP and 
      MS Windows 2000 Operating Systems. 
      Following steps must be followed to have NuSMV working under
      Windows:

   1. Download and install binary package expat_win32bin from: 
      http://sourceforge.net/projects/expat/

      After the installation, set the environment variable PATH to 
      the directory where DLL 'libexpat.dll' is. 

      Under Windows XP the env var PATH must be set at system level: 
      (Start/Control Panel/Performance and Maintenance/System/Advanced/
       Environment Variables).

      If it does not exist already, normal users can create and set a
      new PATH environment variable. Refer to the Windows official
      documentation for any further detail.

      It has been reported that installing libexpat into a path
      containing spaces might cause Windows to be unable to find the
      library. As a general rule, do not use spaces in paths. 
      
   2. Untar binary package of NuSMV into c:\nusmv, by using Winzip for
      example.

      After the unrolling, c:\nusmv must contain directories:
      c:\nusmv
          |- bin
          |- lib
          |- share
          |- include

      Append to the environment variable PATH the direcory "c:\nusmv\bin". 


 4.2) Source distribution
      -------------------
      NuSMV has been tested with two solutions on Microsoft Windows
      operating systems: MinGW (http://www.mingw.org/) and Cygwin
      (http://www.cygwin.com) environments. In order to build
      documentation and help files, latex, perl and lynx packages are
      required. Section 4.3 gives a few references about them.

 4.2.1) MinGW environment
        -----------------
        MinGW is a POSIX emulation environment, that provides tools
        and system libraries that allow NuSMV to be ported natively
        under Windows operating systems. 'Natively' means that once
        built, NuSMV will not require the MinGW environment to be ran.

        To install and configure the MinGW environment, the following
        steps must be carried out. Each of these steps can be
        performed as normal users, i.e. without any system
        administrator privileges.

     1. Download latest version of MSYS bin installer
        (e.g. MSYS-1.0.10.exe) from URL http://www.mingw.org . Run the
        installer and follow the steps of the wizard that will guide
        you toward a complete installation of the MSYS environment.

        We suggest to install the MSYS environment into folder
        'c:/msys/1.0' (from now on %MSYS_FOLDER%). In any case it is
        strongly recommended that you do not use an installation path
        containing 'blanks'. The build of NuSMV might not be
        successful in this case.

     2. Download latest version of MinGW bin installer
        (e.g. MinGW-3.1.0-1.exe) from URL http://www.mingw.org . Run
        the installer and follow the steps of the wizard that will
        guide you toward a complete installation of the MinGW
        environment.

        We suggest to choose the already existing folder
        '%MSYS_FOLDER%/MinGW' (from now on %MINGW_FOLDER%) as the
        folder where to install the MinGW environment.

     3. Download 'expat' package archive
        (e.g. expat-1.95.1-20010126.zip) from URL:
        http://sourceforge.net/projects/mingwrep/
        Unzip the package into folder: %MINGW_FOLDER%

     4. Download 'bison' and 'flex' package archives
        (e.g. bison-1.35-4-bin.zip and flex-2.5.4a-1-bin.zip) from URL
        http://sourceforge.net/projects/gnuwin32/ 

        Download the bin archives of the 'libintl' and 'libiconv'
        libraries (e.g. libintl-0.11.5-2-bin.zip and
        libiconv-1.8-1-bin.zip) from URL 
        http://sourceforge.net/projects/gnuwin32/ 

        Unzip all of those packages into folder: %MINGW_FOLDER%

        REMARK: do not download version 1.875* of the 'bison' tool
                since the build of the NuSMV tool might not be
                successful.

        REMARK: 'libintl' and 'libiconv' dll's are needed for this
                Windows version of the 'bison'. Their absence will
                produce run time error during the execution of the
                'bison' executable (yacc.exe).


 4.2.2) Cygwin environment
        ------------------
        The Cygwin library is necessary for compiling and using NuSMV
        on Windows. Cygwin is a UNIX emulation environment, that is, it
        makes the standard UNIX API available also on Windows.
        Further information and installation instructions for Cygwin
        can be found at

        http://www.cygwin.com/

        Once the Cygwin platform has been installed, NuSMV can be
        compiled following the instructions given in the "BUILDING
        NUSMV" section. Notice that resultant executables and
        libraries will require the Cygwin Runtime Environment
        installed to work properly.


 4.2.3) Other (optional) packages
        -------------------------
        To generate documentation and help files, a few additional
        packages and programs are needed. Before installing and
        configuring these additional programs, you might consider that
        binary distributions already provide full documentation and help
        files.

      latex: There are several implementations for Windows, one can be
             found at: 
             http://www.miktex.org/
       
      perl: One implementation can be found at:
            http://www.activestate.com/Products/ActivePerl/
            (registration is required)

      lynx: One implementation can be found at:
            http://csant.info/lynx.htm
            
            To have lynx properly working, you need to set the
            LYNX_CFG environment variable to the complete path 
	    


 4.2.4) Building of NuSMV   
        -----------------
        As for Linux, both MinGW and Cygwin environments allow to follow
        the standard build steps:

        1) Untar the NuSMV source distribution archive, for example by
           using Winzip. 

        2) Proceed like described in file README, from the 3rd step on. 
