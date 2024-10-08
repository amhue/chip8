; -- 64Bit.iss --
; Demonstrates installation of a program built for the x64 (a.k.a. AMD64)
; architecture.
; To successfully run this installation and the program it installs,
; you must have a "x64" edition of Windows or Windows 11 on Arm.

; SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!

[Setup]
AppName=CHIP-8 Emulator
AppVersion=1.0.3
WizardStyle=modern
DefaultDirName={autopf}\CHIP-8 Emulator
DefaultGroupName=CHIP-8 Emulator
UninstallDisplayIcon={app}\chip8.exe
Compression=lzma2
SolidCompression=yes
OutputDir=userdocs:Inno Setup Examples Output
; "ArchitecturesAllowed=x64compatible" specifies that Setup cannot run
; on anything but x64 and Windows 11 on Arm.
ArchitecturesAllowed=x64compatible
; "ArchitecturesInstallIn64BitMode=x64compatible" requests that the
; install be done in "64-bit mode" on x64 or Windows 11 on Arm,
; meaning it should use the native 64-bit Program Files directory and
; the 64-bit view of the registry.
ArchitecturesInstallIn64BitMode=x64compatible

[Files]
Source: "main.exe"; DestDir: "{app}"; DestName: "chip8.exe"
Source: "sdl2.dll"; DestDir: "{app}"
Source: "README.md"; DestDir: "{app}"; Flags: isreadme
Source: "LICENSE"; DestDir: "{app}"
Source: "assets/*"; DestDir: "{app}/assets"
Source: "roms/*"; DestDir: "{app}/roms"

[Icons]
Name: "{group}\CHIP-8 Emulator"; Filename: "{app}\chip8.exe"
