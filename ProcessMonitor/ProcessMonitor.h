
// ProcessMonitor.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CProcessMonitorApp: 
// �йش����ʵ�֣������ ProcessMonitor.cpp
//

class CProcessMonitorApp : public CWinApp
{
public:
	CProcessMonitorApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CProcessMonitorApp theApp;