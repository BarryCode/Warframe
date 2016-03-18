* Warframe D3D Hack Source V1.0 by Nseven

How to compile:
- download and install "Microsoft Visual Studio Express 2015 for Windows DESKTOP" https://www.visualstudio.com/en-us/products/visual-studio-express-vs.aspx
- download and install "DirectX Software Development Kit (June 2010)" https://www.microsoft.com/en-us/download/details.aspx?id=6812

- open wfdxhook.vcxproj (not wfdxhook.vcxproj.filters) with Visual Studio 2015 (Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\WDExpress.exe)
- change Debug to Release
- select x86(32bit) or x64(64bit)
- click: project -> properties -> configuration properties -> C/C++ -> additional include directories and add this: $(DXSDK_DIR)include;$(IncludePath)
- click: project -> properties -> configuration properties -> linker -> additional library directories and add: $(DXSDK_DIR)lib\x86;$(LibraryPath) or $(DXSDK_DIR)lib\x64;$(LibraryPath)
- click: project -> properties -> configuration properties -> general -> character set -> change to "not set"
- to compile dll press f7 or click the green triangle

x86 compiled dll will be in WFDXHook\Release folder
x64 compiled dll will be in WFDXHook\x64\Release folder

If you share your dll with others, remove dependecy on vs runtime before compiling:
- click: project -> properties -> configuration properties -> C/C++ -> code generation -> runtime library: Multi-threaded (/MT)

How to use:
- start warframe launcher
- click options
- disable fullscreen, disable dx10, disable dx11, disable 64 bit if you compiled in x86, enable 64 bit if you compiled in x64
- click OK, press PLAY
- if x86 -> inject dll in main screen into Warframe.exe
- if x64 -> inject dll in main screen into Warframe.x64.exe
- inject in window mode or borderless fullscreen, real fullscreen can cause problems

Menu key:
- insert

Logging:
ALT + CTRL + L toggles logger
- press O to decrease values
- press P to increase, hold down P key until a texture changes
- press I to log values of changed textures