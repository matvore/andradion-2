[Setup]
CompressLevel=7
AppName=Andradion 2
AppVerName=Andradion 2 (10-Level Preview)
AppCopyright=by Doug Templeton and Matt DeVore
DefaultDirName={pf}\Andradion 2
DisableProgramGroupPage=no
Uninstallable=yes
MinVersion=4,4
UninstallDisplayIcon={app}\Andradion 2.exe
BackColor=$888800
BackColor2=$101000
DefaultGroupName=Andradion 2
WizardImageFile=setup.bmp

[Files]
Source: "Andradion 2.exe"; DestDir: "{app}"
Source: "Technical Details.htm"; DestDir: "{app}"
Source: "story.htm"; DestDir: "{app}"
Source: "LevelsLib.dat"; DestDir: "{app}"
Source: "LevelsLib2.dat"; DestDir: "{app}"

[Icons]
Name: "{group}\Andradion 2"; Filename: "{app}\Andradion 2.exe"; WorkingDir: "{app}"
Name: "{userdesktop}\Andradion 2"; Filename: "{app}\Andradion 2.exe"; WorkingDir: "{app}"
Name: "{group}\Website"; Filename: "http://home.inreach.com/clkawa/"
Name: "{group}\Technical Details"; Filename: "{app}\Technical Details.htm"
Name: "{group}\Story"; Filename: "{app}\story.htm"
Name: "{group}\Uninstall"; Filename: "{app}\unins000.exe"; WorkingDir: "{app}"
