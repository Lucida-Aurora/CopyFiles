#pragma once



void FindFilesRecursively(CString directory, std::vector<CString>& filePaths, CString childDir);

void ExtractRelativePath(CString& baseDirectory, const CString& directory, CString& relativePath);

void CreateMultiLevelDirectory(LPCTSTR path);

CString RemoveFileFromPath(LPCTSTR path);