Real-Time Audio Delay 2 for Windows
Version 1.4 (Interop C++ C# version 1.0)

This is a variation of the C/C++ Audio Delay 2 project for Windows version 1.4

In this version, I combined the existing C/C++ codebase with the dotnet C# and WPF resources.
The graphic user interface is built using C# and DotNet Core WPF resources.
Everything else (file management, signal processing, audio device control, etc) is still implemented in C/C++ using the Win32 APIs.

This application supports .wav files, 16bit and 24bit encoding.

This audio effect is the same as the GNU-Linux_AudioDelay2 project (https://github.com/RMSabe/GNU-Linux_AudioDelay2). Same logic, same controls, pretty much the same code, but for Windows.

Unfortunately, I still didn't figure out completely how Windows MMDeviceAPI works, so there are still a lot of limitations to this application:

1. It will only access the playback audio device in exclusive mode.
(I tried using shared mode, but that doesn't use either WAVE or IEEE FLOAT streaming format, instead it uses another format that I don't know how works).
This means that if any other application is using the intended audio device, this application will not work.
If this application is running, no other application will have access to the audio device.

2. Since the application accesses the audio device in exclusive mode, bit-depth, sample-rate and number of channels must be compatible with the audio device for it to work.

3. For this application, I'm focusing more on mono and stereo audio files. Files with more channels might work, but channels might be misplaced.
I do not recommend using this application for audio files with more than 2 channels.

Build Notes:

This code is meant to run on 32-bit and 64-bit x86 based CPUs. (This version does not yet fully supports ARM based CPUs).

The C/C++ codebase must be compiled to generate a DLL (Dynamic Link Library) binary file.
I normally use the MinGW64 compiler for C/C++ Windows projects, but for building Windows DLLs, I recommend using the MSVC (Microsoft C/C++ Compiler).
The C/C++ codebase uses resources from ole32.dll and ksuser.dll. These resources are part of the Win32 environment, however they must be specified to the linker.

The C# codebase uses a custom target CPU ("TARGETCPU") definition in the .csproj file.
This definition is required to compile the code, as it defines the target platform and some code macros.
See .csproj file for more details.

The C# application and the DLL must use the same target platform.
32-bit x86 C# application requires 32-bit x86 DLL
64-bit x86 C# application requires 64-bit x86 DLL

Once both the DLL and the C# codebase have been compiled, copy the DLL file to the same folder as the C# executable.

Rename the DLL file name to match the C# application:
For 32-bit applications, the DLL file should be named "core32.dll"
For 64-bit applications, the DLL file should be named "core64.dll"

Author: Rafael Sabe
Email: rafaelmsabe@gmail.com
