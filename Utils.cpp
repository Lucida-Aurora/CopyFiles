#include "Utils.h"
#include "pch.h"
#include <iostream>
#include <afxext.h>
#include <vector>
using namespace std;

// �ݹ�����ļ�
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
			//�Ƚ�cDir��childDir�Ƿ�һ�������һ�����ٵݹ�
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

//�����༶�ļ���
void CreateMultiLevelDirectory(LPCTSTR path) {
	CString strPath = path;
	CFileFind finder;
	BOOL bFound = finder.FindFile(strPath);
	if (!bFound) {
		// ������ǰ�ļ���
		if (!CreateDirectory(strPath, NULL)) {
			// ������ǰ�ļ���ʧ�ܣ���ݹ鴴���ϼ��ļ���
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