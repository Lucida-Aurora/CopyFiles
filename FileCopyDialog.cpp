// FileCopyDialog.cpp: 实现文件
//

#include "pch.h"
#include "CopyFileWithMFC.h"
#include "afxdialogex.h"
#include "FileCopyDialog.h"
#include "Utils.h"
#include "CopyFiles.h"
#include <vector>

HANDLE pauseEvent;
bool isPause = false;
//是否加载进度
bool isLoad = false;

int canOpenFileNum = 0;

CopyFiles* globalCopyFiles; // 新增全局变量，用于访问复制对象
// FileCopyDialog 对话框

IMPLEMENT_DYNAMIC(FileCopyDialog, CDialogEx)
int filenum;
FileCopyDialog::FileCopyDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
	, m_fromFolder(_T("")), m_toFolder(_T("")) {

}

FileCopyDialog::~FileCopyDialog() {
}

void FileCopyDialog::DoDataExchange(CDataExchange* pDX) {
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_FROM_EDIT, m_fromFolder);
	DDX_Text(pDX, IDC_TO_EDIT, m_toFolder);
}


BEGIN_MESSAGE_MAP(FileCopyDialog, CDialogEx)
	ON_BN_CLICKED(IDSTARTCOPY, &FileCopyDialog::OnBnClickedStartcopy)
	ON_EN_SETFOCUS(IDC_FROM_EDIT, &FileCopyDialog::OnEnSetfocusFromEdit)
	ON_EN_SETFOCUS(IDC_TO_EDIT, &FileCopyDialog::OnEnSetfocusToEdit)
	ON_BN_CLICKED(IDSAVE, &FileCopyDialog::OnBnClickedSave)
	ON_BN_CLICKED(IDSTOPCOPY, &FileCopyDialog::OnBnClickedStopcopy)
	ON_BN_CLICKED(IDLOAD, &FileCopyDialog::OnBnClickedLoad)
END_MESSAGE_MAP()

DWORD WINAPI copyThread(LPVOID pParam) {
	globalCopyFiles->StartCopy();
	MessageBox(NULL, "复制完成", "提示", MB_OK);
	delete globalCopyFiles;
	globalCopyFiles = NULL;
	CFile file;
	if (file.Open("progress.txt", CFile::modeRead)) {
		file.Close();
		CFile::Remove("progress.txt");
	}
	return 0;
}


DWORD WINAPI continueCopyThreads(LPVOID pParam) {
	globalCopyFiles->loadProgress();
	globalCopyFiles->StartCopy();
	MessageBox(NULL, "复制完成", "提示", MB_OK);
	delete globalCopyFiles;
	globalCopyFiles = NULL;
	CFile file;
	if (file.Open("progress.txt", CFile::modeRead)) {
		file.Close();
		CFile::Remove("progress.txt");
	}
	return 0;
}

// FileCopyDialog 消息处理程序

void FileCopyDialog::OnBnClickedStartcopy() {
	// TODO: 在此添加控件通知处理程序代码
	if (isLoad) {
		CreateThread(NULL, 0, continueCopyThreads, NULL, 0, NULL);
		SetEvent(pauseEvent);
		isPause = false;
		isLoad = false;
		return;
	}
	if (isPause) {
		SetEvent(pauseEvent);
		isPause = false;
		return;
	}
	UpdateData(TRUE);
	if (m_fromFolder.IsEmpty() || m_toFolder.IsEmpty()) {
		MessageBox("请选择文件夹", "提示");
		return;
	}
	m_filePaths.clear();
	m_toFilePaths.clear();
	m_toFolderPaths.clear();

	FindFilesRecursively(m_fromFolder, m_filePaths, m_toFolder);
	CString relativePath;
	for (size_t i = 0; i < m_filePaths.size(); i++) {
		ExtractRelativePath(m_fromFolder, m_filePaths.at(i), relativePath);
		m_toFilePaths.push_back(m_toFolder + '\\' + relativePath);
		CString dir = RemoveFileFromPath(m_toFilePaths[i]);
		m_toFolderPaths.push_back(dir);
		CreateMultiLevelDirectory(m_toFolderPaths[i]);
	}
	filenum = m_filePaths.size();
	globalCopyFiles = new CopyFiles(m_filePaths, m_toFilePaths); // 赋值全局变量
	CreateThread(NULL, 0, copyThread, NULL, 0, NULL);
}

void FileCopyDialog::OnEnSetfocusFromEdit() {
	// TODO: 在此添加控件通知处理程序代码
	CEdit* edit = (CEdit*)this->GetDlgItem(IDC_FROM_EDIT);
	//使编辑框失去焦点
	if (edit != nullptr) {
		this->SetFocus();
	}
	// 设置过滤器   
	TCHAR szFilter[] = _T("文件夹(*.)|*\\|");

	// 构造打开文件夹对话框   
	CFolderPickerDialog folderDlg;
	CString strFolderPath;

	// 显示打开文件夹对话框   
	if (IDOK == folderDlg.DoModal()) {
		// 如果点击了文件夹对话框上的“确定”按钮，则将选择的文件夹路径显示到编辑框里   
		strFolderPath = folderDlg.GetPathName();
		SetDlgItemText(IDC_FROM_EDIT, strFolderPath);
	}

}

void FileCopyDialog::OnEnSetfocusToEdit() {
	// TODO: 在此添加控件通知处理程序代码
	CEdit* edit = (CEdit*)this->GetDlgItem(IDC_TO_EDIT);
	//使编辑框失去焦点
	if (edit != nullptr) {
		this->SetFocus();
	}
	// 设置过滤器   
	TCHAR szFilter[] = _T("文件夹(*.)|*\\|");

	// 构造打开文件夹对话框   
	CFolderPickerDialog folderDlg;
	CString strFolderPath;

	// 显示打开文件夹对话框   
	if (IDOK == folderDlg.DoModal()) {
		// 如果点击了文件夹对话框上的“确定”按钮，则将选择的文件夹路径显示到编辑框里   
		strFolderPath = folderDlg.GetPathName();
		SetDlgItemText(IDC_TO_EDIT, strFolderPath);
	}

}

void FileCopyDialog::OnBnClickedSave() {
	if (globalCopyFiles == NULL) {
		MessageBox("未开始复制", "提示");
		return;
	}
	// 暂停复制操作
	ResetEvent(pauseEvent);
	isPause = true;
	// 保存进度
	globalCopyFiles->saveProgress();
	MessageBox("复制进度已保存", "提示");
}

void FileCopyDialog::OnBnClickedStopcopy() {
	if (globalCopyFiles == NULL) {
		MessageBox("未开始复制", "提示");
		return;
	}
	//停止复制
	ResetEvent(pauseEvent);
	isPause = true;
	MessageBox("复制已停止", "提示");
}


void FileCopyDialog::OnBnClickedLoad() {
	if (isLoad) {
		MessageBox("已加载进度", "提示");
		return;

	}
	CFile file;
	if (!file.Open("progress.txt", CFile::modeRead)) {
		MessageBox("未保存进度", "提示");
		return;
	}
	file.Close();
	if (globalCopyFiles != NULL) {
		delete globalCopyFiles;
		globalCopyFiles = NULL;
	}

	globalCopyFiles = new CopyFiles();
	if (!globalCopyFiles->loadFilesIsGood) {
		MessageBox("源文件或目标文件不存在", "提示");
		delete globalCopyFiles;
		globalCopyFiles = NULL;
		return;
	}
	isLoad = true;
	MessageBox("进度已加载", "提示");

}
