#pragma once
#include <afxext.h>
#include <windows.h>
#include <vector>
using namespace std;

extern int filenum;

class CopyFiles {
public:
	CopyFiles();
	CopyFiles(vector<CString> from_filepaths, vector<CString> to_filepaths);
	~CopyFiles();
	void StartCopy();
	void saveProgress();
	void loadProgress();
	HANDLE* hThread;
	bool loadFilesIsGood = true;
private:
	vector<CString> m_from_filepaths;
	vector<CString> m_to_filepaths;
	DWORD nID;
};