#ifndef AppVersion
  #define AppVersion "0.4.7"
#endif
[Setup]
AppId={{47622673-61BA-45EA-92F5-F30F02FF2C07}
AppName=NEXVARY MR Government Edition
AppVersion={#AppVersion}
AppPublisher=Nexvary Inc.
AppPublisherURL=https://nexvary.com/
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
DisableWelcomePage=no
ShowLanguageDialog=yes
SetupIconFile=app.ico
UninstallDisplayIcon={app}\NEXVARY-MR.exe
[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"; LicenseFile: "institutional-license-en.txt"; InfoBeforeFile: "program-description-en.txt"
Name: "ar"; MessagesFile: "compiler:Languages\Arabic.isl"; LicenseFile: "institutional-license-ar.txt"; InfoBeforeFile: "program-description-ar.txt"
[CustomMessages]
en.Description=NEXVARY MR — Operations & Intelligence System
ar.Description=NEXVARY MR — منظومة العمليات والاستخبارات
en.CreateDesktop=Create a desktop shortcut
ar.CreateDesktop=إنشاء اختصار على سطح المكتب
en.Launch=Launch NEXVARY MR
ar.Launch=تشغيل NEXVARY MR
en.AuthorityTitle=Institutional eligibility and authority
ar.AuthorityTitle=الأهلية المؤسسية والتفويض
en.AuthoritySub=Confirm your authority before installing.
ar.AuthoritySub=أكد أهليتك وتفويضك قبل التثبيت.
en.AuthorityInfo=These declarations record your statement; they do not independently verify your institution.
ar.AuthorityInfo=توثق هذه الإقرارات تصريحك؛ ولا تُعد تحققًا مستقلًا من الجهة.
en.AuthorityOrg=I represent an eligible institution and am authorized to accept the license on its behalf.
ar.AuthorityOrg=أمثل جهة مؤهلة ومفوّض بقبول اتفاقية الترخيص نيابةً عنها.
en.AuthorityUse=I will use the system only within lawful duties and required permissions, respecting privacy and evidence integrity.
ar.AuthorityUse=ألتزم بالاستخدام المشروع ضمن التفويض والأذونات اللازمة واحترام الخصوصية وسلامة الأدلة.
en.AuthorityMissing=Accept both authority declarations to continue.
ar.AuthorityMissing=يلزم قبول إقراري التفويض للمتابعة.
en.SilentMissing=Managed installation requires explicit license and authority acceptance parameters from an authorized administrator.
ar.SilentMissing=يتطلب التثبيت المؤسسي الصامت معاملات قبول صريحة للاتفاقية والتفويض من مسؤول مخوّل.
[Messages]
en.WelcomeLabel2=Welcome to NEXVARY MR.%n%nAn institutional workstation for device diagnostics, cases, digital evidence and intelligence search. Read the description and license before installation.
ar.WelcomeLabel2=مرحبًا بك في NEXVARY MR.%n%nمحطة مؤسسية لتشخيص الأجهزة والقضايا والأدلة الرقمية والبحث الاستخباراتي. راجع الوصف والاتفاقية قبل التثبيت.
en.InfoBeforeLabel=About NEXVARY MR
ar.InfoBeforeLabel=عن منظومة NEXVARY MR
en.InfoBeforeClickLabel=Read the description, then click Next.
ar.InfoBeforeClickLabel=اقرأ وصف النظام، ثم اضغط التالي.

[Files]
Source: "..\dist\NEXVARY-MR\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
[Icons]
Name: "{group}\NEXVARY MR"; Filename: "{app}\NEXVARY-MR.exe"; IconFilename: "{app}\NEXVARY-MR.exe"; IconIndex: 0
Name: "{autodesktop}\NEXVARY MR"; Filename: "{app}\NEXVARY-MR.exe"; IconFilename: "{app}\NEXVARY-MR.exe"; IconIndex: 0; Tasks: desktopicon
[Tasks]
Name: desktopicon; Description: "{cm:CreateDesktop}"; Flags: unchecked
[Run]
Filename: "{app}\NEXVARY-MR.exe"; Description: "{cm:Launch}"; Flags: nowait postinstall skipifsilent

[Code]
var
  AuthorityPage: TInputOptionWizardPage;

procedure InitializeWizard;
begin
  AuthorityPage := CreateInputOptionPage(wpLicense,
    CustomMessage('AuthorityTitle'), CustomMessage('AuthoritySub'),
    CustomMessage('AuthorityInfo'), False, False);
  AuthorityPage.Add(CustomMessage('AuthorityOrg'));
  AuthorityPage.Add(CustomMessage('AuthorityUse'));
  AuthorityPage.Values[0] := False;
  AuthorityPage.Values[1] := False;
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
  Result := True;
  if (not WizardSilent) and (CurPageID = AuthorityPage.ID) then begin
    Result := AuthorityPage.Values[0] and AuthorityPage.Values[1];
    if not Result then
      SuppressibleMsgBox(CustomMessage('AuthorityMissing'), mbError, MB_OK, IDOK);
  end;
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
begin
  Result := '';
  if WizardSilent then begin
    if (ExpandConstant('{param:NEXVARY_ACCEPT_LICENSE|0}') <> '1') or
       (ExpandConstant('{param:NEXVARY_AUTHORIZED_ORG|0}') <> '1') or
       (ExpandConstant('{param:NEXVARY_LAWFUL_USE|0}') <> '1') then
      Result := CustomMessage('SilentMissing');
  end else if not (WizardForm.LicenseAcceptedRadio.Checked and
                   AuthorityPage.Values[0] and AuthorityPage.Values[1]) then
    Result := CustomMessage('AuthorityMissing');
end;
