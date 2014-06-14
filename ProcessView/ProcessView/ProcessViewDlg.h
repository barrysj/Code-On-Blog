
// ProcessViewDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#define MAX_NUM 400
#define MAX_PATH 260
typedef struct ProcessInfomainton//进程信息结构体，包括进程信息结构，和一个存放进程路径的字符串数组
{
	PROCESSENTRY32 Name_Pid;
	TCHAR szPath[MAX_PATH];
}ProcessInfomainton,*PProcessInfomainton;


// CProcessViewDlg 对话框
class CProcessViewDlg : public CDialog
{
// 构造
public:
	CProcessViewDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_PROCESSVIEW_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	//列表控件
	CListCtrl m_lProcess;
	CListCtrl m_lModule;
	CListCtrl m_lThread;

	//全局变量
	ProcessInfomainton ProcessInfo[MAX_NUM];
	PProcessInfomainton pProcessInfo;
	MODULEENTRY32 ModuleInfo[MAX_NUM];
	MODULEENTRY32* pModuleInfo;
	THREADENTRY32 ThreadInfo[MAX_NUM];
	THREADENTRY32* pThreadInfo;
	int lastPid;
	int procNum;
	CImageList m_ImageList;

	afx_msg void OnBnClickedShow();
	void ShowProcess(PProcessInfomainton ProcessInfo);
	void ListModule(int Pid,MODULEENTRY32* pME);
	void ListThread(int Pid,THREADENTRY32* pTE);
	CString GetPriority(HANDLE hProcess);
	//int GetUserName(HANDLE hProcess,TCHAR* userName,int strLen);
	CString GetProcessUserName(HANDLE hProcess);

	BOOL EnableDebugPrivilege();

	afx_msg void OnNMClickProcess(NMHDR *pNMHDR, LRESULT *pResult);
};
