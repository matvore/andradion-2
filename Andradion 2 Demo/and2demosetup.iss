[Setup]
AppName=Andradion 2 Demo
AppVerName=Andradion 2 Demo
AppCopyright=by Matt DeVore
DefaultDirName={pf}\Andradion 2 Demo
DisableProgramGroupPage=no
Uninstallable=yes
MinVersion=4,5
UninstallDisplayIcon={app}\Andradion 2.exe
BackColor=$888800
BackColor2=$101000
DefaultGroupName=Andradion Series 

[Dirs] 
Name: "{app}\Images" 
Name: "{app}\Levels"
Name: "{app}\Sounds"
Name: "{app}\Music"

[Files]
Source: "Andradion 2.exe"; DestDir: "{app}"
Source: "Images\*.bmp"; DestDir: "{app}\Images"
Source: "Sounds\*.wav"; DestDir: "{app}\Sounds"
Source: "Music\*.mid"; DestDir: "{app}\Music"
Source: "Levels\*.dat"; DestDir: "{app}\Levels"

[Icons]
Name: "{group}\Andradion 2 Demo"; Filename: "{app}\Andradion 2.exe"; WorkingDir: "{app}"
Name: "{userdesktop}\Andradion 2 Demo"; Filename: "{app}\Andradion 2.exe"; WorkingDir: "{app}"
