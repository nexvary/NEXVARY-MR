#ifndef AppVersion
  #define AppVersion "0.4.4"
#endif
[Setup]
AppId={{47622673-61BA-45EA-92F5-F30F02FF2C07}
AppName=NEXVARY MR Government Edition
AppVersion={#AppVersion}
AppPublisher=NEXVARY
AppPublisherURL=https://github.com/nexvary/NEXVARY-MR
DefaultDirName={localappdata}\Programs\NEXVARY-MR
DefaultGroupName=NEXVARY MR
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=..\dist
OutputBaseFilename=NEXVARY-MR-Government-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
SetupIconFile=app.ico
UninstallDisplayIcon={app}\NEXVARY-MR.exe
[Files]
Source: "..\dist\NEXVARY-MR\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
[Icons]
Name: "{group}\NEXVARY MR"; Filename: "{app}\NEXVARY-MR.exe"; IconFilename: "{app}\NEXVARY-MR.exe"; IconIndex: 0
Name: "{autodesktop}\NEXVARY MR"; Filename: "{app}\NEXVARY-MR.exe"; IconFilename: "{app}\NEXVARY-MR.exe"; IconIndex: 0; Tasks: desktopicon
[Tasks]
Name: desktopicon; Description: "Create a desktop shortcut"; Flags: unchecked
[Run]
Filename: "{app}\NEXVARY-MR.exe"; Description: "Launch NEXVARY MR"; Flags: nowait postinstall skipifsilent
