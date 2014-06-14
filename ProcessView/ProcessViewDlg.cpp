
// ProcessViewDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ProcessView.h"
#include "ProcessViewDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAX_NUM 400
#define MAX_PATH 260

// CProcessViewDlg 对话框



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


// CProcessViewDlg 消息处理程序

BOOL CProcessViewDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	//初始化全局变量
	pProcessInfo = ProcessInfo;
	pProcessInfo->Name_Pid.dwSize=sizeof(PROCESSENTRY32);
	pModuleInfo = ModuleInfo;
	pModuleInfo->dwSize = sizeof(MODULEENTRY32);
	pThreadInfo = ThreadInfo;
	pThreadInfo->dwSize = sizeof(THREADENTRY32);
	lastPid = -1;
	procNum = -1;
	m_ImageList.Create(16,16,ILC_COLOR32,100,400);

	//初始化控件格式
	DWORD dwStyle = m_lProcess.GetExtendedStyle(); 
	dwStyle |= LVS_EX_FULLROWSELECT;// 选中某行使整行高亮（只适用与report 风格的listctrl ） 
	dwStyle |= LVS_EX_SUBITEMIMAGES;//使可以设置图标
	m_lProcess.SetExtendedStyle(dwStyle);
	m_lThread.SetExtendedStyle(dwStyle);
	m_lModule.SetExtendedStyle(dwStyle);

	m_lProcess.SetImageList(&m_ImageList,LVSIL_SMALL);

	m_lProcess.InsertColumn(0,_T("进程名"));
	m_lProcess.InsertColumn(1,_T("PID"));
	m_lProcess.InsertColumn(2,_T("用户名"));
	m_lProcess.InsertColumn(3,_T("优先级"));
	m_lProcess.InsertColumn(4,_T("进程路径"));
	m_lProcess.SetColumnWidth(0,80);
	m_lProcess.SetColumnWidth(1,40);
	m_lProcess.SetColumnWidth(2,60);
	m_lProcess.SetColumnWidth(3,50);
	m_lProcess.SetColumnWidth(4,500);

	m_lModule.InsertColumn(0,_T("模块基址"));
	m_lModule.InsertColumn(1,_T("模块大小"));
	m_lModule.InsertColumn(2,_T("模块映像路径"));
	m_lModule.SetColumnWidth(0,80);
	m_lModule.SetColumnWidth(1,80);
	m_lModule.SetColumnWidth(2,480);

	m_lThread.InsertColumn(0,_T("进程Cid"));
	m_lThread.SetColumnWidth(0,500);


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CProcessViewDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CProcessViewDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
//获取可以OpenProcess()的权限
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
//获取指定进程的优先级
CString CProcessViewDlg::GetPriority(HANDLE hProcess)
{
	DWORD dwPriClass = 0;//优先级
	dwPriClass = GetPriorityClass(hProcess);
	switch(dwPriClass)
	{
	case HIGH_PRIORITY_CLASS:
		return _T("高");
	case NORMAL_PRIORITY_CLASS:
		return _T("正常");
	case IDLE_PRIORITY_CLASS:
		return _T("空闲");
	case REALTIME_PRIORITY_CLASS:
		return _T("实时");
	case ABOVE_NORMAL_PRIORITY_CLASS:
		return _T("较高");
	case BELOW_NORMAL_PRIORITY_CLASS:
		return _T("较低");
	default:
		return _T("其他");
	}
}
//获取进程用户名
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

		if( bFuncReturn == 0) // 失败
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
//显示进程的函数
void  CProcessViewDlg::ShowProcess(PProcessInfomainton ProcessInfo)
{
	HANDLE snap;//快照句柄
	int i=0;//进程计数
	DWORD BufferSize = MAX_PATH;
	HANDLE hProcess = NULL;
	HICON hIcon = NULL;
	TCHAR userName[100] = _T("");
	CString priClass = _T("");


	snap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);//获取快照
	if (snap==INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("进程快照失败"),_T("error"),MB_OK);
	}

	Process32First(snap,&ProcessInfo->Name_Pid);//查找第一个进程，将信息存入结构体
	PProcessInfomainton pPI  = ProcessInfo;

	EnableDebugPrivilege();


	do //循环遍历进程
	{  
		if (pPI->Name_Pid.th32ProcessID==0|pPI->Name_Pid.th32ProcessID==4)//无法显示路径的两个进程（权限不够）
		{
			continue;
		}
		hProcess=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,
			FALSE,pPI->Name_Pid.th32ProcessID);//打开进程

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

		CloseHandle(hProcess);//关闭句柄

		i++;
		pPI++;
		pPI->Name_Pid.dwSize=sizeof(PROCESSENTRY32);//将第I个结构体进行初始化
		wsprintf(pPI->szPath,_T(""));
	} while (Process32Next(snap,&(pPI->Name_Pid)) );//得到下一个进程信息，存入结构
	CloseHandle(snap);//关闭句柄
	procNum = i;
}

//显示指定pid的模块
void CProcessViewDlg::ListModule(int Pid,MODULEENTRY32* pME)
{
	   HANDLE snap;
	   int i=0;
	   snap=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,Pid);
	   if(!Module32First(snap,pME))
	   {
	   printf("模块获取失败\n");
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

//显示线程的函数
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
	// TODO: 在此添加控件通知处理程序代码
	ShowProcess(pProcessInfo);
}

void CProcessViewDlg::OnNMClickProcess(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
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