
// ProcessMonitorDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include <vector>
using namespace std;


typedef struct _memory_node
{
	SYSTEMTIME st;
	SIZE_T stPrivateUsage;
}MEMORYNODE, *PMEMORYNODE;


// CProcessMonitorDlg �Ի���
class CProcessMonitorDlg : public CDialogEx
{
// ����
public:
	CProcessMonitorDlg(CWnd* pParent = NULL);	// ��׼���캯��
	virtual ~CProcessMonitorDlg();
// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROCESSMONITOR_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonStart();

	CEdit m_ctlProcessId;
	int m_nProcessId;
	CButton m_btnStart;

private:
	HANDLE m_hProcessMonitor;
	BOOL m_bExit;
	vector<MEMORYNODE>m_vMemoryNodes;

	SIZE_T m_stMinPrivateUsage;
	SIZE_T m_stMaxProvateUsage;

private:
	static void MonitorProc(void *pParam);
	void MoniteProcess();
	void PlaintBinary();
	void Summary();
	int GetY(SIZE_T stValue);
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedButtonPause();
	CButton m_btPause;
};
