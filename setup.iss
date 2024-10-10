[Setup]
AppName=CHIP-8 Emulator
AppVersion=1.1.0
WizardStyle=modern
DefaultDirName={autopf}\CHIP-8 Emulator
DefaultGroupName=CHIP-8 Emulator
UninstallDisplayIcon={app}\chip8.exe
Compression=lzma2
SolidCompression=yes
OutputDir=userdocs:Inno Setup Examples Output
ArchitecturesAllowed=x64compatible
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
