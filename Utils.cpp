#include "Utils.h"
#include "pch.h"
#include <iostream>
#include <afxext.h>
#include <vector>
using namespace std;

// 递归查找文件
void FindFilesRecursively(CString directory, vector<CString>& filePaths, CString childDir) {
	CFileFind finder;
	CString searchPath = directory + _T("\\*.*");
	BOOL bWorking = finder.FindFile(searchPath, 0);
	while (bWorking) {
		bWorking = finder.FindNextFile();
		if (finder.IsDots()) {
			continue;
		}

		if (finder.IsDirectory()) {
			CString cDir = finder.GetFilePath();
			//比较cDir和childDir是否一样，如果一样则不再递归
			if (cDir.CompareNoCase(childDir) == 0) {
				continue;
			}
			FindFilesRecursively(cDir, filePaths, childDir);
		}
		else {
			filePaths.push_back(finder.GetFilePath());
		}
	}
}

void ExtractRelativePath(CString& baseDirectory, const CString& directory, CString& relativePath) {
	if (baseDirectory[baseDirectory.GetLength() - 1] != '\\') {
		baseDirectory += '\\';
	}
	int pos = directory.Find(baseDirectory);
	if (pos != -1 && pos == 0) {
		relativePath = directory.Mid(baseDirectory.GetLength());
	}
	else {
		relativePath = _T("");
	}
}

//创建多级文件夹
void CreateMultiLevelDirectory(LPCTSTR path) {
	CString strPath = path;
	CFileFind finder;
	BOOL bFound = finder.FindFile(strPath);
	if (!bFound) {
		// 创建当前文件夹
		if (!CreateDirectory(strPath, NULL)) {
			// 创建当前文件夹失败，则递归创建上级文件夹
			if (strPath.ReverseFind('\\') > 0) {
				strPath = strPath.Left(strPath.ReverseFind('\\'));
				CreateMultiLevelDirectory(strPath);
				CreateDirectory(strPath, NULL);
			}
		}
	}
	CreateDirectory(path, NULL);
}

CString RemoveFileFromPath(LPCTSTR path) {
	CString strPath = path;
	int nPos = strPath.ReverseFind('\\');
	if (nPos >= 0) {
		strPath = strPath.Left(nPos);
	}
	return strPath;
}