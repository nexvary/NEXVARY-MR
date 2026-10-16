#ifndef AppVersion
  #define AppVersion "0.4.0"
#endif
[Setup]
AppId={{47622673-61BA-45EA-92F5-F30F02FF2C07}
AppName=NEXVARY MR
AppVersion={#AppVersion}
AppPublisher=NEXVARY
AppPublisherURL=https://github.com/nexvary/NEXVARY-MR
DefaultDirName={localappdata}\Programs\NEXVARY-MR
DefaultGroupName=NEXVARY MR
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=..\dist
OutputBaseFilename=NEXVARY-MR-Windows-x64-setup-unsigned
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayIcon={app}\NEXVARY-MR.exe
[Files]
Source: "..\dist\NEXVARY-MR\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
[Icons]
Name: "{group}\NEXVARY MR"; Filename: "{app}\NEXVARY-MR.exe"
Name: "{autodesktop}\NEXVARY MR"; Filename: "{app}\NEXVARY-MR.exe"; Tasks: desktopicon
[Tasks]
Name: desktopicon; Description: "Create a desktop shortcut"; Flags: unchecked
[Run]
Filename: "{app}\NEXVARY-MR.exe"; Description: "Launch NEXVARY MR"; Flags: nowait postinstall skipifsilent
