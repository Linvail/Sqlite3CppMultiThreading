#include "MyTable.h"
#include <iostream>

using namespace std;

MyTable::MyTable(const shared_ptr<SQLite::Database>& db_connection)
	: m_Db(db_connection)
{
	InitializeCriticalSection(&CriticalSection);

	m_ReadAll.reset( new SQLite::Statement(*m_Db, "SELECT * FROM MyTable;") );
}

MyTable::~MyTable()
{
	DeleteCriticalSection(&CriticalSection);
}

bool MyTable::GetNextRecord(MyTable::MyTableRecord& record)
{
	EnterCriticalSection(&CriticalSection);

	bool ok = !m_ReadAll->isDone() && m_ReadAll->executeStep();

	if (ok)
	{
		record.m_Id = m_ReadAll->getColumn(0);
		record.m_Name = m_ReadAll->getColumn(1).getString();
	}

	LeaveCriticalSection(&CriticalSection);

	return ok;
}

bool MyTable::UpdateRow()
{
	// TODO
	return true;
}


MyTableWorker::MyTableWorker
	(
	const std::shared_ptr<MyTable>& sourceDb,
	const std::shared_ptr<SQLite::Database>& destinationDb
	)
	: m_SoureDb(sourceDb)
	, m_DestinationDb(destinationDb)
{

}


bool MyTableWorker::BeginWork()
{
	SQLite::Statement createtable(*m_DestinationDb, "CREATE TABLE IF NOT EXISTS ResultTable (id INTEGER PRIMARY KEY, value STRING);");
	createtable.exec();

	SQLite::Statement insert(*m_DestinationDb, "INSERT INTO ResultTable VALUES(?, ?);");

	shared_ptr<SQLite::Transaction> transaction;

	MyTable::MyTableRecord record;
	bool ok = true;
	int count = 0;
	//int row = 0;
	while (ok)
	{
		ok = m_SoureDb->GetNextRecord(record);

		if (ok && transaction.get() == NULL)
		{
			transaction.reset(new SQLite::Transaction(*m_DestinationDb));
		}

		if (!ok)
		{
			break;
		}

		//++row;

		// Spend some time here
		for (size_t i = 0; i < 100000; i++)
		{
			int j = 0;
			++j;
		}
		record.m_Name += "-processed";

		insert.bind(1, record.m_Id);
		insert.bind(2, record.m_Name);
		insert.exec();
		insert.reset();

		//cout << "[" << ::GetCurrentThreadId() << "] ";
		//cout << "[" << row << "]" << "id: " << record.m_Id << ", name: " << record.m_Name << endl;

		++count;
		if (count > 10000 && transaction.get() != NULL)
		{
			count = 0;
			transaction->commit();
			transaction.reset();
		}
	}

	if (transaction)
	{
		transaction->commit();
	}

	return ok;
}