// UseSqlite3Lib.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <ios>
#include <iostream>
#include <chrono>
#include <memory>
#include "sqlite3.h"
#include "SQLiteCpp\Database.h"
#include "SQLiteCpp\Transaction.h"
#include <Windows.h>

#include "MyTable.h"
#include "Thread.h"

using namespace std;

// Global variable
CRITICAL_SECTION CriticalSection;

bool AddDummyData(int count)
{
	DeleteFileA("MyDb.db");

	SQLite::Database myDb("MyDb.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
	SQLite::Statement createtable(myDb, "CREATE TABLE IF NOT EXISTS MyTable (id INTEGER PRIMARY KEY, value STRING);");
	createtable.exec();

	SQLite::Statement insertDummy(myDb, "INSERT INTO MyTable VALUES(?, 'Test string');");

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	SQLite::Transaction transaction(myDb);
	bool ok = true;
	for (size_t i = 0; ok && i < count; i++)
	{
		insertDummy.bind(1, static_cast<int>(i));
		int affectedRows = insertDummy.exec();
		ok = affectedRows > 0;
		insertDummy.reset();
	}

	transaction.commit();

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	int elapsedInMiliSecs = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
	std::cout << "Time difference = " << elapsedInMiliSecs << std::endl;

	return ok;
}

bool DeleteAll()
{
	SQLite::Database myDb("MyDb.db", SQLite::OPEN_READWRITE);
	SQLite::Statement deleteAll(myDb, "DELETE from MyTable");
	deleteAll.exec();
	return true;
}

bool DeleteTempAll()
{
	SQLite::Database myDb("MyDb_temp.db", SQLite::OPEN_READWRITE);
	SQLite::Statement deleteAll(myDb, "DELETE from ResultTable");
	deleteAll.exec();
	return true;
}

bool RealAll()
{
	// Open the db
	SQLite::Database myDb("MyDb.db", SQLite::OPEN_READONLY);

	SQLite::Statement readall(myDb, "SELECT * FROM MyTable;");

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	int row = 0;
	while (readall.executeStep())
	{
		++row;
		int id = readall.getColumn(0);
		string name = readall.getColumn(1);
		cout << "[" << row << "]" << "id: " << id << ", name: " << name << endl;
	}

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	int elapsedInMiliSecs = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

	std::cout << "Time difference = " << elapsedInMiliSecs << std::endl;

	return true;
}

void runSingleCore()
{
	shared_ptr<SQLite::Database> readonlyDb(new SQLite::Database("MyDb.db", SQLite::OPEN_READONLY));
	shared_ptr<MyTable> readonlyMyTable(new MyTable(readonlyDb));

	DeleteFileA("MyDb_temp.db");
	shared_ptr<SQLite::Database> writeDb(new SQLite::Database("MyDb_temp.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE));
	MyTableWorker worker(readonlyMyTable, writeDb);

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	worker.BeginWork();

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	int elapsedInMiliSecs = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
	std::cout << "[" << __FUNCTION__ << "]" << "Time difference = " << elapsedInMiliSecs << std::endl;
}

void runMultiThreads()
{
	shared_ptr<SQLite::Database> readonlyDb(new SQLite::Database("MyDb.db", SQLite::OPEN_READONLY));
	shared_ptr<MyTable> readonlyMyTable(new MyTable(readonlyDb));

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	// Spawn 4 threads.
	thread worker1(readonlyMyTable, "db_temp1");
	thread worker2(readonlyMyTable, "db_temp2");
	thread worker3(readonlyMyTable, "db_temp3");
	thread worker4(readonlyMyTable, "db_temp4");

	worker1.start();
	worker2.start();
	worker3.start();
	worker4.start();

	worker1.wait();
	worker2.wait();
	worker3.wait();
	worker4.wait();

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	int elapsedInMiliSecs = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
	std::cout << "[" << __FUNCTION__ << "]" << "Time difference = " << elapsedInMiliSecs << std::endl;
}

void mergeDb()
{
	DeleteFileA("MyDb_temp.db");

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	shared_ptr<SQLite::Database> writeDb(new SQLite::Database("MyDb_temp.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE));
	SQLite::Statement createtable(*writeDb, "CREATE TABLE IF NOT EXISTS ResultTable (id INTEGER PRIMARY KEY, value STRING);");
	createtable.exec();

	SQLite::Statement attach1(*writeDb, "ATTACH DATABASE db_temp1 AS db1;");
	attach1.exec();
	SQLite::Statement attach2(*writeDb, "ATTACH DATABASE db_temp2 AS db2;");
	attach2.exec();
	SQLite::Statement attach3(*writeDb, "ATTACH DATABASE db_temp3 AS db3;");
	attach3.exec();
	SQLite::Statement attach4(*writeDb, "ATTACH DATABASE db_temp4 AS db4;");
	attach4.exec();

	SQLite::Statement mergeStat(*writeDb, "insert into ResultTable(id, value) select id, value from \
		  ( select id, value from db1.ResultTable \
		    union all \
	        select id, value from db2.ResultTable \
		    union all \
	        select id, value from db3.ResultTable \
		    union all \
	        select id, value from db4.ResultTable )"
	);

	mergeStat.exec();

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	int elapsedInMiliSecs = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
	std::cout << "[" << __FUNCTION__ << "]" << "Time difference = " << elapsedInMiliSecs << std::endl;
}

int main()
{
	AddDummyData(1000000);

	runSingleCore();

	runMultiThreads();
	mergeDb();

	return 0;
}

