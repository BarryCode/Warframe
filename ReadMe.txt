* Warframe D3D Hack Source V1.1 by Nseven

How to compile:
- download and install "Microsoft Visual Studio Express 2015 for Windows DESKTOP" https://www.visualstudio.com/en-us/products/visual-studio-express-vs.aspx

- open wfdxhook.vcxproj (not wfdxhook.vcxproj.filters) with Visual Studio 2015 (Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\WDExpress.exe)
- select x86(32bit) or x64(64bit)
- compile dll, press f7 or click the green triangle

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

Menu key:
- insert

Logging:
ALT + CTRL + L toggles logger
- press O to decrease values
- press P to increase, hold down P key until a texture changes
- press I to log values of changed textures
