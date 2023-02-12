; === This inno_setup script 'compiles' TICKR win32 installer ===

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
; previous app id (for News): AppId={{4C30F291-1B7B-4E53-AB2E-8F515B8F99E5}
AppId={{A3E7BF61-5796-451F-8DC4-753E4BA6B048}
AppName=Tickr
AppVersion=0.7.1
;AppVerName=Tickr 0.7.1
AppPublisher=ETMSoftware
AppPublisherURL=http://www.open-tickr.net/
AppSupportURL=http://www.open-tickr.net/help.php
AppUpdatesURL=http://www.open-tickr.net/download.php
DefaultDirName={pf}\Tickr
DisableDirPage=no
DefaultGroupName=Tickr
DisableProgramGroupPage=no
LicenseFile=C:\MinGW\msys\1.0\home\manutm\src\tickr-0.7.1\win32_install_stuff\LICENSE
OutputDir=C:\Users\manutm\Desktop\binaries
OutputBaseFilename=Tickr-0.7.1-Setup
SetupIconFile=C:\MinGW\msys\1.0\home\manutm\src\tickr-0.7.1\images\tickr-icon.ico
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
; (not available in new version) Name: "basque"; MessagesFile: "compiler:Languages\Basque.isl"
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: "catalan"; MessagesFile: "compiler:Languages\Catalan.isl"
Name: "czech"; MessagesFile: "compiler:Languages\Czech.isl"
Name: "danish"; MessagesFile: "compiler:Languages\Danish.isl"
Name: "dutch"; MessagesFile: "compiler:Languages\Dutch.isl"
Name: "finnish"; MessagesFile: "compiler:Languages\Finnish.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "hebrew"; MessagesFile: "compiler:Languages\Hebrew.isl"
Name: "hungarian"; MessagesFile: "compiler:Languages\Hungarian.isl"
Name: "italian"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"
Name: "norwegian"; MessagesFile: "compiler:Languages\Norwegian.isl"
Name: "polish"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "portuguese"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
; (not available in new version) Name: "slovak"; MessagesFile: "compiler:Languages\Slovak.isl"
Name: "slovenian"; MessagesFile: "compiler:Languages\Slovenian.isl"
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}";
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]
Source: "C:\MinGW\msys\1.0\home\manutm\src\tickr-0.7.1\src\tickr\tickr-win32-bin\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\Tickr"; Filename: "{app}\tickr.exe"
Name: "{group}\{cm:ProgramOnTheWeb,Tickr}"; Filename: "http://www.open-tickr.net"
Name: "{group}\{cm:UninstallProgram,Tickr}"; Filename: "{uninstallexe}"
Name: "{commondesktop}\Tickr"; Filename: "{app}\tickr.exe"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\Tickr"; Filename: "{app}\tickr.exe"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\tickr.exe"; Description: "{cm:LaunchProgram,Tickr}"; Flags: nowait postinstall skipifsilent
