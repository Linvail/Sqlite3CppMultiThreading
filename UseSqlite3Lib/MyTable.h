#pragma once


#include <string>
#include <memory>
#include "SQLiteCpp\SQLiteCpp.h"
#include <Windows.h>

class MyTable
{
public:

	struct MyTableRecord
	{
		int m_Id;
		std::string m_Name;
	};

	MyTable(const std::shared_ptr<SQLite::Database>& db_connection);

	virtual ~MyTable();

	bool GetNextRecord(MyTableRecord& record);

	bool UpdateRow();

	// Data
	std::shared_ptr<SQLite::Database> m_Db;
	std::shared_ptr<SQLite::Statement> m_ReadAll;
	CRITICAL_SECTION CriticalSection;
};


class MyTableWorker
{
public:
	MyTableWorker
	(
		const std::shared_ptr<MyTable>& sourceDb,
		const std::shared_ptr<SQLite::Database>& destinationDb
	);

	bool BeginWork();

	// Data
	std::shared_ptr<MyTable> m_SoureDb;
	std::shared_ptr<SQLite::Database> m_DestinationDb;
};