
#include "pch.h"
#include "framework.h"
#include "CopyFileWithMFC.h"
#include "FileCopyDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(CCopyFileWithMFCApp, CWinApp)

END_MESSAGE_MAP()




CCopyFileWithMFCApp::CCopyFileWithMFCApp() {
	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的 CCopyFileWithMFCApp 对象

CCopyFileWithMFCApp theApp;


// CCopyFileWithMFCApp 初始化

BOOL CCopyFileWithMFCApp::InitInstance() {
	CWinApp::InitInstance();
	FileCopyDialog fcd;
	m_pMainWnd = &fcd;
	fcd.DoModal();
	return TRUE;
}


