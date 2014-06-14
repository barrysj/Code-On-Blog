
// ProcessViewDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"
#define MAX_NUM 400
#define MAX_PATH 260
typedef struct ProcessInfomainton//������Ϣ�ṹ�壬����������Ϣ�ṹ����һ����Ž���·�����ַ�������
{
	PROCESSENTRY32 Name_Pid;
	TCHAR szPath[MAX_PATH];
}ProcessInfomainton,*PProcessInfomainton;


// CProcessViewDlg �Ի���
class CProcessViewDlg : public CDialog
{
// ����
public:
	CProcessViewDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_PROCESSVIEW_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	//�б�ؼ�
	CListCtrl m_lProcess;
	CListCtrl m_lModule;
	CListCtrl m_lThread;

	//ȫ�ֱ���
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
