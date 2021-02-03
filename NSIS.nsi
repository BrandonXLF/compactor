!define APP_NAME "Compactor"
!define APP_ICON "icon.ico"
!define LICENSE_FILE "LICENSE"
!define REG_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"

Name "${APP_NAME}"
BrandingText " "
OutFile "x64\${APP_NAME} Installer.exe"
Unicode True

!define PRODUCT "${APP_NAME}"
!define SHORT_PRODUCT_NAME "${APP_NAME}"
!define MULTIUSER_EXECUTIONLEVEL "Highest"
!define MULTIUSER_INSTALLMODE_COMMANDLINE
!define MULTIUSER_MUI
!define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_KEY "${REG_KEY}"
!define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_VALUENAME "UninstallString"
!define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_KEY "${REG_KEY}"
!define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_VALUENAME "InstallLocation"
!define MULTIUSER_INSTALLMODE_INSTALL_REGISTRY_KEY "${APP_NAME}"
!define MULTIUSER_INSTALLMODE_UNINSTALL_REGISTRY_KEY "${APP_NAME}" 
!define MULTIUSER_INSTALLMODE_ALLOW_ELEVATION
!define MULTIUSER_INSTALLMODE_DEFAULT_ALLUSERS
!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_ICON "${APP_ICON}"
!define MUI_UNICON "${APP_ICON}"
!define MULTIUSER_INSTALLMODE_INSTDIR "${APP_NAME}"
!define MUI_FINISHPAGE_RUN "$INSTDIR\${APP_NAME}.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Launch ${APP_NAME}"

!include MUI2.nsh
!include MultiUser.nsh

!insertmacro MUI_PAGE_LICENSE "${LICENSE_FILE}"
!insertmacro MULTIUSER_PAGE_INSTALLMODE
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH
!insertmacro MUI_LANGUAGE "English"

InstallDir "$PROGRAMFILES\${APP_NAME}"
  
Function .onInit
	!insertmacro MULTIUSER_INIT
FunctionEnd

Function un.onInit
	!insertmacro MULTIUSER_UNINIT
FunctionEnd

Section "${APP_NAME}" S1
	SectionIn RO
	SetOutPath $INSTDIR
	File "x64\Release\${APP_NAME}.exe"
	WriteUninstaller "$INSTDIR\${APP_NAME} Uninstaller.exe"
	WriteRegStr SHCTX "${REG_KEY}" "DisplayName" "${APP_NAME}"
	WriteRegStr SHCTX "${REG_KEY}" "InstallLocation" "$INSTDIR"
	WriteRegDWORD SHCTX "${REG_KEY}" "NoModify" "1"
	WriteRegDWORD SHCTX "${REG_KEY}" "NoRepair" "1"
	WriteRegStr SHCTX "${REG_KEY}" "UninstallString" "$INSTDIR\uninstall.exe"
	WriteRegStr SHCTX "${REG_KEY}" "DisplayIcon" "$INSTDIR\${APP_NAME}.exe"
SectionEnd

Section "Add To Start Menu" S2
	CreateDirectory "$SMPROGRAMS\${APP_NAME}"
	CreateShortcut "$SMPROGRAMS\${APP_NAME}.lnk" "$INSTDIR\${APP_NAME}.exe"
SectionEnd

Section "Create Desktop Shortcut" S3
	CreateShortcut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${APP_NAME}.exe"
SectionEnd

Section "Run On Startup" S4
	CreateShortcut "$SMSTARTUP\${APP_NAME}.lnk" "$INSTDIR\${APP_NAME}.exe"
SectionEnd

Section "Uninstall"
	Delete "$INSTDIR\uninstall.exe"
	Delete "$INSTDIR\${APP_NAME}.exe"
	Delete "$SMPROGRAMS\${APP_NAME}.lnk"
	Delete "$DESKTOP\${APP_NAME}.lnk"
	RMDir $INSTDIR
	DeleteRegKey SHCTX "${REG_KEY}"
SectionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
!insertmacro MUI_DESCRIPTION_TEXT ${S1} "Install ${APP_NAME} and its core file(s)"
!insertmacro MUI_DESCRIPTION_TEXT ${S4} "Run ${APP_NAME} on startup"
!insertmacro MUI_DESCRIPTION_TEXT ${S2} "Add a shortcut to ${APP_NAME} to the Start Menu"
!insertmacro MUI_DESCRIPTION_TEXT ${S3} "Add a shortcut to ${APP_NAME} to the Desktop folder"
!insertmacro MUI_FUNCTION_DESCRIPTION_END