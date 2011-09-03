[Setup]
AppName=Andradion 2
AppVerName=Andradion 2 (11-Level Preview)
AppCopyright=by Doug Templeton and Matt DeVore
DefaultDirName={pf}\Andradion 2
DisableProgramGroupPage=no
Uninstallable=yes
MinVersion=4,4
UninstallDisplayIcon={app}\Andradion2.exe
BackColor=$888800
BackColor2=$101000
DefaultGroupName=Andradion 2
WizardImageFile=setup.bmp
WizardImageBackColor=clBlack

[Files]
Source: "Andradion2.exe"; DestDir: "{app}"

[Icons]
Name: "{group}\Andradion 2"; Filename: "{app}\Andradion2.exe"; WorkingDir: "{app}"
Name: "{userdesktop}\Andradion 2"; Filename: "{app}\Andradion2.exe"; WorkingDir: "{app}"
Name: "{group}\Website"; Filename: "http://www.calpoly.edu/~mdevore/"
Name: "{group}\Uninstall"; Filename: "{app}\unins000.exe"; WorkingDir: "{app}"
