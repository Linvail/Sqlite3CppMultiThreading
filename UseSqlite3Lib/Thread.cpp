#include "Thread.h"


thread::thread(const shared_ptr<MyTable>& source, const string& dest)
{
	m_SourceDb = source;
	m_TempFileName = dest;
	init();

	m_hThread = reinterpret_cast<HANDLE>(
		::_beginthreadex(
			0,	// security
			0,	// stack size
			threadProc,	// thread routine
			static_cast<void*>(this),	// thread arg
			CREATE_SUSPENDED,	// initial state flag
			&m_tid	//	thread ID
		)
		);
	if (m_hThread == 0)
	{
		throw std::exception("failed to create thread");
	}
}

thread::~thread()
{
	try
	{
		::CloseHandle(GetHandle());
	}
	catch (...)
	{
		// suppress any exception; dtors should never throw
	}
}

const bool thread::wait() const
{
	bool bWaitSuccess = false;
	// a thread waiting on itself will cause a deadlock
	if (::GetCurrentThreadId() != m_tid)
	{
		DWORD nResult = ::WaitForSingleObject(GetHandle(), INFINITE);
		// nResult will be WAIT_OBJECT_0 if the thread has terminated;
		// other possible results: WAIT_FAILED, WAIT_TIMEOUT,
		// or WAIT_ABANDONED
		bWaitSuccess = (nResult == WAIT_OBJECT_0);
	}
	return bWaitSuccess;
}

DWORD thread::start() const
{
	return ::ResumeThread(m_hThread);
}

unsigned __stdcall thread::threadProc(void* a_param)
{
	thread* pthread = static_cast<thread*>(a_param);
	return pthread->run();
}

unsigned thread::run()
{
	m_Worker->BeginWork();
	return 0;
}

bool thread::init()
{
	shared_ptr<SQLite::Database> writeDb(new SQLite::Database(m_TempFileName, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE));
	m_Worker.reset(new MyTableWorker(m_SourceDb, writeDb));

	return true;
}
