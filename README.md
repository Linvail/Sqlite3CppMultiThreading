# Sqlite3CppMultiThreading

This project is just a practice of SQLite3.

This project does the following things.

1. Create a database with a lot of dummy data (1000000 records).
2. In a single thread, read all records and do some dummy works, write all records to another database.
3. Measure how much time we spent in the previous step.
4. Do the same work as step 2 in multiple threads (4 threads).
5. Measure how much time we spent in the previous step.

In the step 4, 4 threads are created. They shared the same readonly database connection and read data
from it simultaneously.
Then, they write data to an independent temp database file.
Finally, in the main thread, I merge the above 4 temp database file into a single database file, which is
identical to the result of step 2.

This project uses SQLiteCpp as the C++ wrapper of SQLite3:
https://github.com/SRombauts/SQLiteCpp
