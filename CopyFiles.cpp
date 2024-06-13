#include <afxext.h>
#include <windows.h>
#include <vector>
#include <iostream>
#include "pch.h"
#include "CopyFiles.h"
#include <string>
#include <memory>

using namespace std;

extern HANDLE pauseEvent;

typedef struct file_op {
	CHAR buffer0[1024 * 10];
	CHAR buffer1[1024 * 10];
	HANDLE empty0;
	HANDLE empty1;
	HANDLE full0;
	HANDLE full1;
	CFile from;
	CFile to;
	UINT read_size0 = 1;
	UINT read_size1 = 1;
	bool eof = false;
	ULONGLONG offset = 0;
} file_op;

file_op* fop;
extern int canOpenFileNum;
CFile flog;

struct fileloadinfo {
	CString from;
	CString to;
	ULONGLONG offset;
};

vector<fileloadinfo> fileloadinfos;

void load_progress() {
	fileloadinfos.clear();
	CStdioFile file;
	char ch;
	if (!file.Open("progress.txt", CFile::modeRead)) {
		return;
	}
	int n = 0;
	file.Read(&n, sizeof(int));
	file.Read(&ch, sizeof(char));
	fileloadinfos.resize(n);
	for (int i = 0; i < n; i++) {
		CString from;
		CString to;
		ULONGLONG offset;
		file.ReadString(from);
		file.ReadString(to);
		file.Read(&offset, sizeof(ULONGLONG));
		file.Read(&ch, sizeof(char));
		fileloadinfos[i].from = from;
		fileloadinfos[i].to = to;
		fileloadinfos[i].offset = offset;
	}
	file.Close();
	//删除文件
	CFile::Remove("progress.txt");
}

// 读线程
DWORD WINAPI threadr(LPVOID pParam) {
	//shared_ptr<int> a = *reinterpret_cast<shared_ptr<int>*>(pParam);
	int* a = (int*)pParam;
	int i = *a;
	delete a;
	a = NULL;
	while (!fop[i].eof) {
		WaitForSingleObject(pauseEvent, INFINITE);

		// Use buffer0
		WaitForSingleObject(fop[i].empty0, 2000);
		ResetEvent(fop[i].empty0);

		fop[i].from.Seek(fop[i].offset, CFile::begin);
		fop[i].read_size0 = fop[i].from.Read(fop[i].buffer0, sizeof(fop[i].buffer0));
		if (fop[i].read_size0 <= 0) {
			fop[i].eof = true;
		}
		else {
			fop[i].offset += fop[i].read_size0;
		}
		SetEvent(fop[i].full0);

		if (fop[i].eof) break;

		// Use buffer1
		WaitForSingleObject(fop[i].empty1, 2000);
		ResetEvent(fop[i].empty1);

		fop[i].from.Seek(fop[i].offset, CFile::begin);
		fop[i].read_size1 = fop[i].from.Read(fop[i].buffer1, sizeof(fop[i].buffer1));
		if (fop[i].read_size1 <= 0) {
			fop[i].eof = true;
		}
		else {
			fop[i].offset += fop[i].read_size1;
		}
		SetEvent(fop[i].full1);
	}
	return 0;
}

// 写线程
DWORD WINAPI threadw(LPVOID pParam) {
	int* a = (int*)pParam;
	int i = *a;
	delete a;
	a = NULL;
	while (true) {
		WaitForSingleObject(pauseEvent, INFINITE);

		// Write buffer0
		WaitForSingleObject(fop[i].full0, 2000);
		ResetEvent(fop[i].full0);

		if (fop[i].read_size0 > 0) {
			fop[i].to.Write(fop[i].buffer0, fop[i].read_size0);
		}
		SetEvent(fop[i].empty0);

		if (fop[i].eof) break;

		// Write buffer1
		WaitForSingleObject(fop[i].full1, 2000);
		ResetEvent(fop[i].full1);

		if (fop[i].read_size1 > 0) {
			fop[i].to.Write(fop[i].buffer1, fop[i].read_size1);
		}
		SetEvent(fop[i].empty1);

		if (fop[i].eof) break;
	}
	return 0;
}
CopyFiles::CopyFiles() {
	load_progress();
	CFile f;
	for (fileloadinfo item : fileloadinfos) {
		if (!f.Open(item.from, CFile::modeRead)) {
			loadFilesIsGood = false;
			return;
		}
		f.Close();
		if (!f.Open(item.to, CFile::modeRead)) {
			loadFilesIsGood = false;
			return;
		}
		f.Close();
	}

	pauseEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	flog.Open("log.txt", CFile::modeCreate | CFile::modeWrite);
	canOpenFileNum = fileloadinfos.size();
	hThread = new HANDLE[2 * canOpenFileNum];
	fop = new file_op[canOpenFileNum];
}

CopyFiles::CopyFiles(vector<CString> from_filepaths, vector<CString> to_filepaths) {
	m_from_filepaths = from_filepaths;
	m_to_filepaths = to_filepaths;
	flog.Open("log.txt", CFile::modeCreate | CFile::modeWrite);
	filenum = m_from_filepaths.size();
	fop = new file_op[filenum];
	pauseEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	canOpenFileNum = 0;
	for (int i = 0; i < filenum; i++) {
		if (!fop[i].from.Open(m_from_filepaths[i], CFile::modeRead)) {
			CString errorMessage = "!Can't open file: " + m_from_filepaths[i] + "\n";
			flog.Write(errorMessage, errorMessage.GetLength());
			m_from_filepaths.erase(m_from_filepaths.begin() + i);
			m_to_filepaths.erase(m_to_filepaths.begin() + i);
			filenum--;
			i--;
		}
		else {
			//如果目标文件存在，删除目标文件
			CFile f;
			if (f.Open(m_to_filepaths[i], CFile::modeRead)) {
				f.Close();
				CFile::Remove(m_to_filepaths[i]);
			}

			canOpenFileNum++;
		}
	}
	delete[] fop;
	fop = new file_op[canOpenFileNum];
	for (int i = 0; i < canOpenFileNum; i++) {
		if (!fop[i].from.Open(m_from_filepaths[i], CFile::modeRead)) {
			CString errorMessage = "!!!Can't open file: " + m_from_filepaths[i] + "\n";
			flog.Write(errorMessage, errorMessage.GetLength());
		}
	}
	hThread = new HANDLE[2 * canOpenFileNum];

	for (int i = 0; i < canOpenFileNum; i++) {
		fop[i].empty0 = CreateEvent(NULL, TRUE, FALSE, NULL);
		fop[i].empty1 = CreateEvent(NULL, TRUE, FALSE, NULL);
		fop[i].full0 = CreateEvent(NULL, TRUE, FALSE, NULL);
		fop[i].full1 = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!fop[i].to.Open(m_to_filepaths[i], CFile::modeCreate | CFile::modeWrite)) {
			CString errorMessage = "!!Can't create file: " + m_to_filepaths[i] + "\n";
			flog.Write(errorMessage, errorMessage.GetLength());
			continue;
		}
		//auto pIndexW = make_shared<int>(i);
		//auto pIndexR = make_shared<int>(i);
		int* pIndexW = new int(i);
		int* pIndexR = new int(i);
		hThread[2 * i] = CreateThread(NULL, 0, threadr, pIndexR, 0, &nID);
		hThread[2 * i + 1] = CreateThread(NULL, 0, threadw, pIndexW, 0, &nID);

	}
}

CopyFiles::~CopyFiles() {
	if (!loadFilesIsGood) {
		return;
	}
	for (int i = 0; i < canOpenFileNum; i++) {
		fop[i].from.Close();
		fop[i].to.Close();
		CloseHandle(fop[i].empty0);
		CloseHandle(fop[i].empty1);
		CloseHandle(fop[i].full0);
		CloseHandle(fop[i].full1);
	}
	delete[] fop;
	delete[] hThread;
	CloseHandle(pauseEvent);
	flog.Close();
}

void CopyFiles::saveProgress() {
	CStdioFile file;
	if (!file.Open("progress.txt", CFile::modeCreate | CFile::modeWrite)) {
		return;
	}
	int n = 0;
	for (int i = 0; i < canOpenFileNum; i++) {
		if (fop[i].eof) continue;
		n++;
	}
	file.Write(&n, sizeof(int));
	file.Write("\n", sizeof(char));
	bool flag = false;
	if (m_from_filepaths.size() == 0 && m_to_filepaths.size() == 0) {
		for (int i = 0; i < canOpenFileNum; i++) {
			m_from_filepaths.push_back(fileloadinfos[i].from);
			m_to_filepaths.push_back(fileloadinfos[i].to);
		}
		flag = true;
	}

	for (int i = 0; i < canOpenFileNum; i++) {
		if (fop[i].eof) continue;
		file.WriteString(m_from_filepaths[i] + "\n");
		file.WriteString(m_to_filepaths[i] + "\n");
		file.Write(&fop[i].offset, sizeof(ULONGLONG));
		file.Write("\n", sizeof(char));
	}
	file.Close();
}
void CopyFiles::loadProgress() {

	for (size_t i = 0; i < canOpenFileNum; i++) {
		fop[i].empty0 = CreateEvent(NULL, TRUE, TRUE, NULL);
		fop[i].empty1 = CreateEvent(NULL, TRUE, TRUE, NULL);
		fop[i].full0 = CreateEvent(NULL, TRUE, FALSE, NULL);
		fop[i].full1 = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!fop[i].from.Open(fileloadinfos[i].from, CFile::modeRead)) {
			int len = fileloadinfos[i].from.GetLength();
			flog.Write("@@Can't open file: " + fileloadinfos[i].from + "\n", len + 20);
			continue;
		}
		if (!fop[i].to.Open(fileloadinfos[i].to, CFile::modeWrite)) {
			int len = fileloadinfos[i].to.GetLength();
			flog.Write("@Can't open file: " + fileloadinfos[i].to + "\n", len + 21);
			continue;
		}
		int* pIndexW = new int(i);
		int* pIndexR = new int(i);
		hThread[2 * i] = CreateThread(NULL, 0, threadr, pIndexR, 0, &nID);
		hThread[2 * i + 1] = CreateThread(NULL, 0, threadw, pIndexW, 0, &nID);
	}
}

void CopyFiles::StartCopy() {
	//开始复制，最多有32个文件为一批开始复制，当其中一个文件复制完成后，不必等32个中其他文件全部复制完成后才开始下一批
	// 每批处理的最大文件数量
	const int maxConcurrentFiles = 32;
	int fileIndex = 0;

	// 启动最大并发文件复制
	while (fileIndex < canOpenFileNum && fileIndex < maxConcurrentFiles) {
		SetEvent(fop[fileIndex].empty0);
		SetEvent(fop[fileIndex].empty1);
		fileIndex++;
	}

	while (fileIndex < canOpenFileNum) {
		// 等待任一文件复制完成
		DWORD result = WaitForMultipleObjects(2 * maxConcurrentFiles, hThread, FALSE, INFINITE);

		// 查找完成的文件索引
		int completedIndex = (result >= WAIT_OBJECT_0 && result < WAIT_OBJECT_0 + 2 * maxConcurrentFiles) ? result - WAIT_OBJECT_0 : -1;

		if (completedIndex != -1) {
			// 找到完成的文件索引，计算实际文件索引
			int actualIndex = completedIndex / 2;
			// 启动下一个文件复制
			SetEvent(fop[fileIndex].empty0);
			SetEvent(fop[fileIndex].empty1);
			fileIndex++;
		}
	}

	// 由于WaitForMultipleObjects函数的限制，只能等待64个线程，所以这里需要分批次等待，每批次等待32个线程
	int num_batches = canOpenFileNum / 32 + 1;
	for (int batch = 0; batch < num_batches; batch++) {
		int start = batch * 32;
		int end = min((batch + 1) * 32, canOpenFileNum);
		WaitForMultipleObjects(end - start, &hThread[2 * start], TRUE, INFINITE);
	}

	for (int i = 0; i < canOpenFileNum; i++) {
		if (!hThread[2 * i]) {
			CloseHandle(hThread[2 * i]);
		}
		if (!hThread[2 * i + 1]) {
			CloseHandle(hThread[2 * i + 1]);
		}
	}
	fileloadinfos.clear();

}