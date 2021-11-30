#include <iostream>
#include "clibs/db/sqlite3.hpp"

int main(int argc, char const *argv[])
{
    clibs::db::sqlite3_t db;
    clibs::db::sqlite3_table_t table;
    clibs::db::sqlite3_cursor_t cursor;

    if (!clibs::db::sqlite3_open("./test.db", &db)) {
        std::cout << clibs::db::sqlite3_error(&db) << std::endl;
    }

    if (!clibs::db::sqlite3_execute(&db, "select * from abc", &table)) {
        std::cout << clibs::db::sqlite3_error(&db) << std::endl;
    } else {
        std::cout << clibs::db::sqlite3_table_header(&table, 0) << std::endl;
    }

    for (int i = 0; i < table.rows; i ++) {
        std::cout << clibs::db::sqlite3_table_value(&table, i, 0) << std::endl;
    }

    clibs::db::sqlite3_table_free(&table);

    if (clibs::db::sqlite3_prepare(&db, "select * from abc", &cursor)) {
        int id;
        
        while(clibs::db::sqlite3_prepare_fetch(&cursor)) {
            clibs::db::sqlite3_prepare_column_value(&cursor, 0, id);

            std::cout << clibs::db::sqlite3_prepare_column_name(&cursor, 0) << ":" << id << std::endl;
        }

        clibs::db::sqlite3_prepare_finalize(&cursor);
    }

    clibs::db::sqlite3_close(&db);

    return 0;
}