
// ProcessViewDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ProcessView.h"
#include "ProcessViewDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAX_NUM 400
#define MAX_PATH 260

// CProcessViewDlg �Ի���



CProcessViewDlg::CProcessViewDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProcessViewDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CProcessViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROCESS, m_lProcess);
	DDX_Control(pDX, IDC_MODULE, m_lModule);
	DDX_Control(pDX, IDC_THREAD, m_lThread);
}

BEGIN_MESSAGE_MAP(CProcessViewDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(ID_SHOW, &CProcessViewDlg::OnBnClickedShow)
	ON_NOTIFY(NM_CLICK, IDC_PROCESS, &CProcessViewDlg::OnNMClickProcess)
END_MESSAGE_MAP()


// CProcessViewDlg ��Ϣ�������

BOOL CProcessViewDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	//��ʼ��ȫ�ֱ���
	pProcessInfo = ProcessInfo;
	pProcessInfo->Name_Pid.dwSize=sizeof(PROCESSENTRY32);
	pModuleInfo = ModuleInfo;
	pModuleInfo->dwSize = sizeof(MODULEENTRY32);
	pThreadInfo = ThreadInfo;
	pThreadInfo->dwSize = sizeof(THREADENTRY32);
	lastPid = -1;
	procNum = -1;
	m_ImageList.Create(16,16,ILC_COLOR32,100,400);

	//��ʼ���ؼ���ʽ
	DWORD dwStyle = m_lProcess.GetExtendedStyle(); 
	dwStyle |= LVS_EX_FULLROWSELECT;// ѡ��ĳ��ʹ���и�����ֻ������report ����listctrl �� 
	dwStyle |= LVS_EX_SUBITEMIMAGES;//ʹ��������ͼ��
	m_lProcess.SetExtendedStyle(dwStyle);
	m_lThread.SetExtendedStyle(dwStyle);
	m_lModule.SetExtendedStyle(dwStyle);

	m_lProcess.SetImageList(&m_ImageList,LVSIL_SMALL);

	m_lProcess.InsertColumn(0,_T("������"));
	m_lProcess.InsertColumn(1,_T("PID"));
	m_lProcess.InsertColumn(2,_T("�û���"));
	m_lProcess.InsertColumn(3,_T("���ȼ�"));
	m_lProcess.InsertColumn(4,_T("����·��"));
	m_lProcess.SetColumnWidth(0,80);
	m_lProcess.SetColumnWidth(1,40);
	m_lProcess.SetColumnWidth(2,60);
	m_lProcess.SetColumnWidth(3,50);
	m_lProcess.SetColumnWidth(4,500);

	m_lModule.InsertColumn(0,_T("ģ���ַ"));
	m_lModule.InsertColumn(1,_T("ģ���С"));
	m_lModule.InsertColumn(2,_T("ģ��ӳ��·��"));
	m_lModule.SetColumnWidth(0,80);
	m_lModule.SetColumnWidth(1,80);
	m_lModule.SetColumnWidth(2,480);

	m_lThread.InsertColumn(0,_T("����Cid"));
	m_lThread.SetColumnWidth(0,500);


	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CProcessViewDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CProcessViewDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
//��ȡ����OpenProcess()��Ȩ��
BOOL CProcessViewDlg::EnableDebugPrivilege()

{
	HANDLE hToken;
	BOOL fOk=FALSE;
	if(OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&hToken))
	{
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount=1;
		LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&tp.Privileges[0].Luid);
		tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;

		AdjustTokenPrivileges(hToken,FALSE,&tp,sizeof(tp),NULL,NULL);

		fOk=(GetLastError()==ERROR_SUCCESS);
		CloseHandle(hToken);
	}
	return fOk;
}
//��ȡָ�����̵����ȼ�
CString CProcessViewDlg::GetPriority(HANDLE hProcess)
{
	DWORD dwPriClass = 0;//���ȼ�
	dwPriClass = GetPriorityClass(hProcess);
	switch(dwPriClass)
	{
	case HIGH_PRIORITY_CLASS:
		return _T("��");
	case NORMAL_PRIORITY_CLASS:
		return _T("����");
	case IDLE_PRIORITY_CLASS:
		return _T("����");
	case REALTIME_PRIORITY_CLASS:
		return _T("ʵʱ");
	case ABOVE_NORMAL_PRIORITY_CLASS:
		return _T("�ϸ�");
	case BELOW_NORMAL_PRIORITY_CLASS:
		return _T("�ϵ�");
	default:
		return _T("����");
	}
}
//��ȡ�����û���
/*
int CProcessViewDlg::GetUserName(HANDLE hProcess,TCHAR* userName,int strLen)
{
	HANDLE hToken = NULL;
	PTOKEN_USER pTokenUser = NULL;
	DWORD cb = 0;
	SID_NAME_USE sid;
	TCHAR domin[1024];
	DWORD cbdomin = 1023;
	int ret = -1;

	memset(domin,0,sizeof(domin));

	ret = OpenProcessToken(hProcess,TOKEN_QUERY,&hToken);
	if (!ret)
		return -1;

	ret = GetTokenInformation(hToken,TokenUser,NULL,cb,&cb);
	if (!ret)
		return -1;

	pTokenUser = (PTOKEN_USER)GlobalAlloc(GPTR,cb);
	if(!pTokenUser)
		return -1;

	GetTokenInformation(hToken,TokenUser,pTokenUser,cb,&cb);

	ret = LookupAccountSid(NULL,pTokenUser->User.Sid,userName,(unsigned long*)&strLen,
						domin,&cbdomin,&sid);
	if(!ret)
		return -1;

	if (pTokenUser)
	{
		GlobalFree(pTokenUser);
		pTokenUser = NULL;
	}
	if (hToken)
	{
		CloseHandle(hToken);
		hToken = NULL;
	}
	return 0;
}*/
CString CProcessViewDlg::GetProcessUserName(HANDLE hProcess)
{
	HANDLE hToken = NULL;
	BOOL bFuncReturn = FALSE;
	CString strUserName = _T("");
	PTOKEN_USER pToken_User = NULL;
	DWORD dwTokenUser = 0;
	TCHAR szAccName[MAX_PATH] = {0};
	TCHAR szDomainName[MAX_PATH] = {0};
	HANDLE hProcessToken = NULL;

	if(hProcess != NULL)
	{
		bFuncReturn = ::OpenProcessToken(hProcess,TOKEN_QUERY,&hToken);

		if( bFuncReturn == 0) // ʧ��
		{
			return strUserName;
		}

		if(hToken != NULL)
		{
			::GetTokenInformation(hToken, TokenUser, NULL,0L, &dwTokenUser);

			if(dwTokenUser>0)
				pToken_User = (PTOKEN_USER)::GlobalAlloc( GPTR, dwTokenUser );

			if(pToken_User != NULL)
				bFuncReturn = ::GetTokenInformation(hToken, TokenUser, pToken_User, dwTokenUser, &dwTokenUser);

			if(bFuncReturn != FALSE && pToken_User != NULL)
			{
				SID_NAME_USE eUse = SidTypeUnknown;

				DWORD dwAccName = 0L;  
				DWORD dwDomainName = 0L;

				PSID pSid = pToken_User->User.Sid;

				bFuncReturn = ::LookupAccountSid(NULL, pSid, NULL, &dwAccName,
					NULL,&dwDomainName,&eUse );
				if(dwAccName>0 && dwAccName < MAX_PATH && dwDomainName>0 && dwDomainName <= MAX_PATH)
				{
					bFuncReturn = ::LookupAccountSid(NULL,pSid,szAccName,&dwAccName,
						szDomainName,&dwDomainName,&eUse );
				}

				if( bFuncReturn != 0)
					strUserName = szAccName;
			}
		}
	}
	if (pToken_User != NULL)
	{
		::GlobalFree( pToken_User );
	}

	if(hToken != NULL)
	{
		::CloseHandle(hToken);
	}

	return strUserName;
}  
//��ʾ���̵ĺ���
void  CProcessViewDlg::ShowProcess(PProcessInfomainton ProcessInfo)
{
	HANDLE snap;//���վ��
	int i=0;//���̼���
	DWORD BufferSize = MAX_PATH;
	HANDLE hProcess = NULL;
	HICON hIcon = NULL;
	TCHAR userName[100] = _T("");
	CString priClass = _T("");


	snap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);//��ȡ����
	if (snap==INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("���̿���ʧ��"),_T("error"),MB_OK);
	}

	Process32First(snap,&ProcessInfo->Name_Pid);//���ҵ�һ�����̣�����Ϣ����ṹ��
	PProcessInfomainton pPI  = ProcessInfo;

	EnableDebugPrivilege();


	do //ѭ����������
	{  
		if (pPI->Name_Pid.th32ProcessID==0|pPI->Name_Pid.th32ProcessID==4)//�޷���ʾ·�����������̣�Ȩ�޲�����
		{
			continue;
		}
		hProcess=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,
			FALSE,pPI->Name_Pid.th32ProcessID);//�򿪽���

		if (hProcess)
		{
			QueryFullProcessImageName(hProcess,0,pPI->szPath,&BufferSize);
			BufferSize = MAX_PATH;
		}
		priClass = GetPriority(hProcess);

		hIcon = ::ExtractIcon(NULL,pPI->szPath,0);
		m_ImageList.Add(hIcon);

		TCHAR str[20];
		wsprintf(str, _T("%u"), pPI->Name_Pid.th32ProcessID);		
		m_lProcess.InsertItem(i,_T(""),i);
		m_lProcess.SetItemText(i,0,pPI->Name_Pid.szExeFile);
		m_lProcess.SetItemText(i,1,str);
		m_lProcess.SetItemText(i,2,GetProcessUserName(hProcess));
		m_lProcess.SetItemText(i,3,priClass);
		m_lProcess.SetItemText(i,4,pPI->szPath);

		CloseHandle(hProcess);//�رվ��

		i++;
		pPI++;
		pPI->Name_Pid.dwSize=sizeof(PROCESSENTRY32);//����I���ṹ����г�ʼ��
		wsprintf(pPI->szPath,_T(""));
	} while (Process32Next(snap,&(pPI->Name_Pid)) );//�õ���һ��������Ϣ������ṹ
	CloseHandle(snap);//�رվ��
	procNum = i;
}

//��ʾָ��pid��ģ��
void CProcessViewDlg::ListModule(int Pid,MODULEENTRY32* pME)
{
	   HANDLE snap;
	   int i=0;
	   snap=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,Pid);
	   if(!Module32First(snap,pME))
	   {
	   printf("ģ���ȡʧ��\n");
	   }
	   else{
	   do 
	   {   
		   TCHAR str[20] = _T("");
		   m_lModule.InsertItem(i,_T(""));
		   wsprintf(str,_T("0x%x"),pME->modBaseAddr);
		   m_lModule.SetItemText(i,0,str);
		   wsprintf(str, _T("0x%x"), pME->modBaseSize);		
		   m_lModule.SetItemText(i,1,str);
		   m_lModule.SetItemText(i,2,pME->szExePath);
		   i++;
		   pME++;
		   pME->dwSize=sizeof(MODULEENTRY32);
	   } while (Module32Next(snap,pME));
	   CloseHandle(snap);
	   }
    
}

//��ʾ�̵߳ĺ���
void CProcessViewDlg::ListThread(int Pid,THREADENTRY32* pTE)
{
	HANDLE snap;
	int i = 0;
	snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, Pid);

	if (!Thread32First(snap,pTE))
	{
		printf("fail to get the snap\n");
		return;
	}
	do 
	{
		if (pTE->th32OwnerProcessID == Pid)
		{
			TCHAR str[20];
			wsprintf(str,_T("%u"),pTE->th32ThreadID);
			//printf("%lu\n",pTE->th32ThreadID);
			m_lThread.InsertItem(i,_T(""));
			m_lThread.SetItemText(i,0,str);
			i++;
			pTE++;
			pTE->dwSize = sizeof(THREADENTRY32);
		}
	} while (Thread32Next(snap,pTE));
}
void CProcessViewDlg::OnBnClickedShow()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	ShowProcess(pProcessInfo);
}

void CProcessViewDlg::OnNMClickProcess(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	*pResult = 0;

	POSITION ps;
	int index;
	int pID;
	PProcessInfomainton pProcInfo = NULL;

	ps = m_lProcess.GetFirstSelectedItemPosition();
	index = m_lProcess.GetNextSelectedItem(ps);

	pProcInfo = pProcessInfo+index;
	pID = (int)(pProcInfo->Name_Pid.th32ProcessID);

	//EnableDebugPrivilege();
	ListModule(pID,pModuleInfo);
	ListThread(pID,pThreadInfo);
	
}
/*
DWORD GetThreadStartAddr1(DWORD dwThreadId)  
{  
	HMODULE hNtdll = LoadLibrary(_T("ntdll.dll"));  
	if (!hNtdll)  
	{  
		return 0;  
	}  

	NTQUERYINFORMATIONTHREAD NtQueryInformationThread = NULL;  
	NtQueryInformationThread = (NTQUERYINFORMATIONTHREAD)  
		GetProcAddress(hNtdll, "NtQueryInformationThread");  
	if (!NtQueryInformationThread)  
	{  
		return 0;  
	}  

	HANDLE ThreadHandle = NULL;  
	ThreadHandle = OpenThread(THREAD_QUERY_INFORMATION, FALSE, dwThreadId);  
	if (!ThreadHandle)  
	{  
		return 0;  
	}  

	DWORD dwStaAddr = NULL;  
	DWORD dwReturnLength = 0;  
	if(NtQueryInformationThread(ThreadHandle, ThreadQuerySetWin32StartAddress,  
		&dwStaAddr, sizeof(dwStaAddr), &dwReturnLength))  
	{  
		return 0;  
	}  

	return dwStaAddr;  
}  */