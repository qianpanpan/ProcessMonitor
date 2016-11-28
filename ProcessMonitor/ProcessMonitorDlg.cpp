
// ProcessMonitorDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ProcessMonitor.h"
#include "ProcessMonitorDlg.h"
#include "afxdialogex.h"
#include <windows.h>
#include <psapi.h>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

#pragma comment(lib,"psapi.lib")



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CProcessMonitorDlg 对话框


bool CompareMemoryNode(const MEMORYNODE & mn1, const MEMORYNODE & mn2)
{
	return mn1.stPrivateUsage < mn2.stPrivateUsage;
}


BOOL EnablePriv()
{
	// 取得当前进程的[Token](标识)句柄
	HANDLE handle;
	if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &handle))
		return FALSE;

	// 取得关闭系统的[LUID](本地唯一的标识符)值
	TOKEN_PRIVILEGES tokenPrivilege;
	if (!::LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tokenPrivilege.Privileges[0].Luid))
		return FALSE;

	// 设置特权数组的元素个数
	tokenPrivilege.PrivilegeCount = 1;

	// 设置[LUID]的属性值
	tokenPrivilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	// 为当前进程取得DEBUG权限
	if (!::AdjustTokenPrivileges(handle, FALSE, &tokenPrivilege, 0, NULL, NULL))
		return FALSE;

	return TRUE;
}

CProcessMonitorDlg::CProcessMonitorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_PROCESSMONITOR_DIALOG, pParent)
	, m_nProcessId(0)
	, m_hProcessMonitor(INVALID_HANDLE_VALUE)
	, m_bExit(FALSE)
	, m_stMinPrivateUsage(0)
	, m_stMaxProvateUsage(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	if (!EnablePriv())
	{
		MessageBox(L"提升权限失败");
	}
}

CProcessMonitorDlg::~CProcessMonitorDlg()
{
	m_bExit = TRUE;
	if (INVALID_HANDLE_VALUE != m_hProcessMonitor)
	{
		::WaitForSingleObject(m_hProcessMonitor, INFINITE);
	}
}

void CProcessMonitorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PROCESS_ID, m_ctlProcessId);
	DDX_Text(pDX, IDC_EDIT_PROCESS_ID, m_nProcessId);
	DDX_Control(pDX, IDC_BUTTON_START, m_btnStart);
	DDX_Control(pDX, IDC_BUTTON_PAUSE, m_btPause);
}

BEGIN_MESSAGE_MAP(CProcessMonitorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CProcessMonitorDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CProcessMonitorDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_START, &CProcessMonitorDlg::OnBnClickedButtonStart)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, &CProcessMonitorDlg::OnBnClickedButtonPause)
END_MESSAGE_MAP()


// CProcessMonitorDlg 消息处理程序

BOOL CProcessMonitorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	ShowWindow(SW_SHOWMAXIMIZED);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CProcessMonitorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CProcessMonitorDlg::OnPaint()
{
	CPaintDC dc(this); // 用于绘制的设备上下文

	CRect rect;
	GetClientRect(&rect);
	dc.FillSolidRect(rect, RGB(0, 0, 0));
		
	if (m_vMemoryNodes.empty())
	{
		CDialogEx::OnPaint();
		return;
	}

	rect.DeflateRect(40, 55);

	CPen penWriteDot;
	penWriteDot.CreatePen(PS_DOT, 1, RGB(255, 255, 255));

	CPen penRedSolid;
	penRedSolid.CreatePen(PS_SOLID,1, RGB(255, 0, 0 ));

	CGdiObject* pOldPen = dc.SelectObject(&penWriteDot);

	CPoint ptStart;
	CPoint ptEnd;

	int nRulerLength = 11;
	CString strText;
	SIZE_T stUnitValue = (m_stMaxProvateUsage - m_stMinPrivateUsage) / (nRulerLength - 1);

	dc.SetTextColor(RGB(255, 0 , 0));
	for (int index = 0; index < nRulerLength; ++index)
	{
		ptStart = CPoint(rect.left, rect.top + index * rect.Height() / (nRulerLength - 1));
		ptEnd = CPoint(rect.right, rect.top + index * rect.Height() / (nRulerLength - 1));
		dc.MoveTo(ptStart);
		dc.LineTo(ptEnd);

		strText.Format(L"%.2fM", (m_stMaxProvateUsage - index * stUnitValue)/ (1024 *1024*1.0));

		CRect rcText(0, ptStart.y - 8, 60, ptStart.y + 20);
		dc.DrawText(strText, rcText, DT_SINGLELINE|DT_LEFT);
	}
		
	dc.SelectObject(&penRedSolid);
	int nCounts = m_vMemoryNodes.size();
	float fLenUnit = rect.Width() * 1.0/ (nCounts - 1);
	for (int index = 0; index < nCounts; ++index)
	{
		ptStart = CPoint(rect.left + index * fLenUnit, GetY(m_vMemoryNodes[index].stPrivateUsage));
		if (0 == index)
		{
			dc.MoveTo(ptStart);
		}
		else
		{
			dc.LineTo(ptStart);
		}
	}

	int nIndexStep = 1;

	if (m_vMemoryNodes.size() < nRulerLength)
	{
		nRulerLength = m_vMemoryNodes.size();
	}
	else
	{
		nIndexStep = m_vMemoryNodes.size() / (nRulerLength - 1);
	}

	float fUnitLen = 1;
	fUnitLen = rect.Width() * 1.0/ (nRulerLength - 1);

	for (int index = 0; index < nRulerLength; index += 1)
	{
		ptStart = CPoint(rect.left, rect.top + index * rect.Height() / (nRulerLength - 1));
		ptEnd = CPoint(rect.right, rect.top + index * rect.Height() / (nRulerLength - 1));
	
		MEMORYNODE & mn = m_vMemoryNodes[index * nIndexStep];
		strText.Format(L"%02d.%02d %02d:%02d",mn.st.wMonth, mn.st.wDay,
			mn.st.wHour, mn.st.wMinute);

		CRect rcText(rect.left + index  * fUnitLen - 38,
			rect.bottom + 20,
			rect.left + index * fUnitLen + 38,
			rect.bottom + 45
			);

		dc.DrawText(strText, rcText, DT_SINGLELINE | DT_LEFT);
	}

	dc.SelectObject(pOldPen);

	CDialogEx::OnPaint();
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CProcessMonitorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CProcessMonitorDlg::OnBnClickedOk()
{
	UpdateData(TRUE);

	PlaintBinary();
	Summary();
}

void CProcessMonitorDlg::PlaintBinary()
{
	CString strFileSource;
	strFileSource.Format(L"%d.dat", m_nProcessId);
	CFile fileSource;

	if (fileSource.Open(strFileSource, CFile::modeRead))
	{
		CString strFileDestination;
		strFileDestination.Format(L"%d_plaint.dat", m_nProcessId);
		CFile fileDestination;
		CString strText;

		if (fileDestination.Open(strFileDestination, CFile::modeCreate | CFile::modeWrite))
		{
			MEMORYNODE mn;
			ZeroMemory(&mn, sizeof(MEMORYNODE));

			m_vMemoryNodes.clear();

			while (fileSource.Read(&mn, sizeof(MEMORYNODE)) > 0)
			{
				m_vMemoryNodes.push_back(mn);

				strText.Format(L"[%04d%02d%02d %02d:%02d:%02d]--%ld\n",
					mn.st.wYear, mn.st.wMonth, mn.st.wDay,
					mn.st.wHour, mn.st.wMinute, mn.st.wSecond,
					mn.stPrivateUsage/ 1024);

				fileDestination.Write(strText, strText.GetLength() * sizeof(wchar_t));
				fileDestination.SeekToEnd();
			}
			fileDestination.Close();
		}

		fileSource.Close();
	}
}

int CProcessMonitorDlg::GetY(SIZE_T stValue)
{
	CRect rect;
	GetClientRect(&rect);
	rect.DeflateRect(10, 55);

	return rect.bottom -
		(stValue - m_stMinPrivateUsage)*1.0 / (m_stMaxProvateUsage - m_stMinPrivateUsage) * rect.Height();
}

void CProcessMonitorDlg::Summary()
{
	vector<MEMORYNODE>::iterator finder =  min_element(m_vMemoryNodes.begin(), m_vMemoryNodes.end(), CompareMemoryNode);
	if (finder != m_vMemoryNodes.end())
	{
		m_stMinPrivateUsage = (*finder).stPrivateUsage;
	}

	finder = max_element(m_vMemoryNodes.begin(), m_vMemoryNodes.end(), CompareMemoryNode);
	if (finder != m_vMemoryNodes.end())
	{
		m_stMaxProvateUsage = (*finder).stPrivateUsage;
	}

	Invalidate();
}

void CProcessMonitorDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}


void CProcessMonitorDlg::MonitorProc(void *pParam)
{
	CProcessMonitorDlg *pProcessMoitor = (CProcessMonitorDlg*)pParam;
	pProcessMoitor->MoniteProcess();
}


void CProcessMonitorDlg::MoniteProcess()
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, m_nProcessId);
	if (NULL == hProcess)
	{
		CString strError;
		strError.Format(L"获取进程句柄失败，失败码:%d", GetLastError());
		MessageBox(strError);
		return;
	}

	PROCESS_MEMORY_COUNTERS_EX pmc;
	pmc.cb = sizeof(pmc);

	CString strFileName;
	strFileName.Format(L"%d.dat", m_nProcessId);

	CFile fileOutPut;

	SYSTEMTIME st;

	while (!m_bExit)
	{
		GetProcessMemoryInfo(hProcess, (PPROCESS_MEMORY_COUNTERS)&pmc, sizeof(pmc));
		GetLocalTime(&st);
		
		MEMORYNODE memoryNode;
		memoryNode.st = st;
		memoryNode.stPrivateUsage = pmc.PrivateUsage;

		fileOutPut.Open(strFileName, CFile::modeWrite| CFile::modeCreate | CFile::modeNoTruncate);
		fileOutPut.SeekToEnd();
		fileOutPut.Write(&memoryNode, sizeof(MEMORYNODE));

		fileOutPut.Close();
		
		Sleep(1000 * 5 );
	}
}

void CProcessMonitorDlg::OnBnClickedButtonStart()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);

	if (m_nProcessId <= 0)
	{
		MessageBox(L"进程Id非法，请填写正确进程Id", L"提示", MB_OKCANCEL| MB_ICONWARNING);
	}
	else
	{
		m_ctlProcessId.EnableWindow(FALSE);
		m_btnStart.EnableWindow(FALSE);
		m_btPause.EnableWindow(TRUE);

		m_hProcessMonitor = (HANDLE)_beginthread(MonitorProc, 0 , this);
	}
}


void CAboutDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	// TODO: 在此处添加消息处理程序代码
	Invalidate();

}


BOOL CProcessMonitorDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	return FALSE;
}


void CProcessMonitorDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	Invalidate();
}


void CProcessMonitorDlg::OnBnClickedButtonPause()
{
	// TODO: 在此添加控件通知处理程序代码
	if (INVALID_HANDLE_VALUE != m_hProcessMonitor)
	{
		SuspendThread(m_hProcessMonitor);
		m_ctlProcessId.EnableWindow(TRUE);
		m_btnStart.EnableWindow(TRUE);
		m_btPause.EnableWindow(FALSE);
	}

}
