#pragma once
#include "afxdialogex.h"
#include <vector>


// FileCopyDialog 对话框

class FileCopyDialog : public CDialogEx
{
	DECLARE_DYNAMIC(FileCopyDialog)

public:
	FileCopyDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~FileCopyDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedStartcopy();
	afx_msg void OnEnSetfocusFromEdit();
	afx_msg void OnEnSetfocusToEdit();

	CString m_fromFolder;
	CString m_toFolder;
	std::vector<CString> m_filePaths;
	std::vector<CString> m_toFilePaths;
	std::vector<CString> m_toFolderPaths;
	afx_msg void OnBnClickedSave();
	afx_msg void OnBnClickedStopcopy();
	afx_msg void OnBnClickedLoad();
};
