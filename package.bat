DEL /Q application
MKDIR application
MKDIR application\pictures
MKDIR application\Engine\dll(x86)
XCOPY Release\joseki_viewer.exe                  application
XCOPY /S /Y Siv3D(August2016v2)1\pictures        application\pictures
XCOPY /S /Y Siv3D(August2016v2)1\Engine\dll(x86) application\Engine\dll(x86)
"C:\Program Files\7-Zip\7z.exe" a joseki_viewer.zip  application

DEL /Q application_x64
MKDIR application_x64
MKDIR application_x64\pictures
MKDIR application_x64\Engine\dll(x64)
XCOPY x64\Release\joseki_viewer.exe              application_x64
XCOPY /S /Y Siv3D(August2016v2)1\pictures        application_x64\pictures
XCOPY /S /Y Siv3D(August2016v2)1\Engine\dll(x64) application_x64\Engine\dll(x64)
"C:\Program Files\7-Zip\7z.exe" a joseki_viewer_x64.zip  application_x64
