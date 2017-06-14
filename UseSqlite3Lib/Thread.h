#pragma once

#include <memory>
#include <process.h>
#include <windows.h>
#include <string>
#include "MyTable.h"
//#include <iostream>

using namespace std;

class thread
{
public:
	thread( const shared_ptr<MyTable>& source, const string& dest );
	virtual ~thread();

	const HANDLE& GetHandle() const { return m_hThread; }
	const bool wait() const;

	DWORD start() const;
	
	shared_ptr<MyTableWorker> m_Worker;

private:
	// copy operations are private to prevent copying
	thread(const thread&);
	thread& operator=(const thread&);

	static unsigned __stdcall threadProc(void*);
	unsigned run();

	bool init();

	HANDLE m_hThread;
	unsigned m_tid;

	shared_ptr<MyTable> m_SourceDb;
	std::string m_TempFileName;
};
