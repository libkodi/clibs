#ifndef _CLIBS_SQLITE3_H_
#define _CLIBS_SQLITE3_H_ 1

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sqlite3.h>
#include "clibs/array.hpp"

namespace clibs {
    namespace db {
        /**
         * 数据库句柄
         */
        typedef struct {
            sqlite3* sqlite;
            bool opened;
        } sqlite3_t;

        /**
         * sqlite3_get_table的结果数据
         */
        typedef struct {
            char** data;
            int rows;
            int columns;
        } sqlite3_table_t;

        /** 预处理的游标 */
        typedef struct {
            sqlite3_stmt* stmt;
        } sqlite3_cursor_t;

        /**
         * 打开数据库
         * @param  db_file 文件名
         * @param  db      
         * @return         true/false
         */
        bool sqlite3_open(const char* db_file, sqlite3_t* db) {
            if (sqlite3_open(db_file, &db->sqlite) == SQLITE_OK) {
                db->opened = true;
            } else {
                db->opened = false;
            }

            return db->opened;
        }

        /** 关闭数据库 */
        bool sqlite3_close(sqlite3_t* db) {
            if (db->opened) {
                sqlite3_close(db->sqlite);
                db->opened = false;
            }
        }

        /** 获取错误信息 */
        const char* sqlite3_error(sqlite3_t* db) {
            if (!db->opened) {
                return "The database is not open.";
            }

            return sqlite3_errmsg(db->sqlite);
        }

        /** 获取错误码 */
        int sqlite3_errno(sqlite3_t* db) {
            if (!db->opened) {
                return SQLITE_ERROR;
            }

            return sqlite3_errcode(db->sqlite);
        }

        /**
         * 执行语句
         * @param  db       
         * @param  sql_text sql语句
         * @param  table    结果的输出
         * @return          true/false
         */
        bool sqlite3_execute(sqlite3_t* db, const char* sql_text, sqlite3_table_t* table) {
            if (table == NULL) {
                return sqlite3_exec(db->sqlite, sql_text, NULL, NULL, NULL) == SQLITE_OK;
            } else {
                bool result = sqlite3_get_table(db->sqlite, sql_text, &table->data, &table->rows, &table->columns, NULL) == SQLITE_OK;

                if (!result) {
                    table->data = NULL;
                }

                return result;
            }
        }

        /** 无返回结果执行语句 */
        bool sqlite3_execute(sqlite3_t* db, const char* sql_text) {
            return sqlite3_execute(db, sql_text, NULL);
        }

        /** 释放sqlite3_get_table返回数据的内存 */
        void sqlite3_table_free(sqlite3_table_t* table) {
            if (table->data != NULL) {
                sqlite3_free_table(table->data);
            }
        }

        /** 开启事务 */
        bool sqlite3_begin(sqlite3_t* db) {
            return sqlite3_execute(db, "BEGIN");
        }

        /** 关闭事务 */
        bool sqlite3_commit(sqlite3_t* db) {
            return sqlite3_execute(db, "COMMIT");
        }

        /** 回滚事务 */
        bool sqlite3_rollback(sqlite3_t* db) {
            return sqlite3_execute(db, "ROLLBACK");
        }

        /** 获取表头列 */
        const char* sqlite3_table_header(sqlite3_table_t* table, int index) {
            if (index < table->columns && index >= 0) {
                return table->data[index];
            } else {
                return NULL;
            }
        }

        /**
         * 获取查询出来的table的值
         * @param  table 结果集
         * @param  row   第几行
         * @param  col   第几列
         * @return       const char*
         */
        const char* sqlite3_table_value(sqlite3_table_t* table, int row, int col) {
            if (row >= 0 && row < table->rows && col >= 0 && col < table->columns) {
                return table->data[table->columns + ((row * table->columns) + col)];
            } else {
                return NULL;
            }
        }

        /** 创建一条预处理语句 */
        bool sqlite3_prepare(sqlite3_t* db, const char* sql_text, sqlite3_cursor_t* cursor) {
            bool result = sqlite3_prepare(db->sqlite, sql_text, -1, &cursor->stmt, NULL) == SQLITE_OK;

            if (!result) {
                cursor->stmt = NULL;
            }

            return result;
        }

        /** 结束一条预处理 */
        void sqlite3_prepare_finalize(sqlite3_cursor_t* cursor) {
            sqlite3_finalize(cursor->stmt);
        }

        /** 重置一条预处理 */
        bool sqlite3_prepare_reset(sqlite3_cursor_t* cursor) {
            return sqlite3_reset(cursor->stmt) == SQLITE_OK;
        }

        /** 清除一条预处理语句上的参数绑定 */
        bool sqlite3_prepare_clear_bindings(sqlite3_cursor_t* cursor) {
            return sqlite3_clear_bindings(cursor->stmt) == SQLITE_OK;
        }

        /**
         * 参数绑定处理
         */
        bool __sqlite3_bind_values(sqlite3_cursor_t* cursor, size_t index) {
            return sqlite3_bind_null(cursor->stmt, index) == SQLITE_OK;
        }

        bool __sqlite3_bind_values(sqlite3_cursor_t* cursor, size_t index, double value) {
            return sqlite3_bind_double(cursor->stmt, index, value) == SQLITE_OK;
        }

        bool __sqlite3_bind_values(sqlite3_cursor_t* cursor, size_t index, int value) {
            return sqlite3_bind_int(cursor->stmt, index, value) == SQLITE_OK;
        }

        bool __sqlite3_bind_values(sqlite3_cursor_t* cursor, size_t index, sqlite3_int64 value) {
            return sqlite3_bind_int64(cursor->stmt, index, value) == SQLITE_OK;
        }

        bool __sqlite3_bind_values(sqlite3_cursor_t* cursor, size_t index, sqlite3_value* value) {
            return sqlite3_bind_value(cursor->stmt, index, value) == SQLITE_OK;
        }

        /** 绑定预处理语句中的绑定 */
        template<class... Args>
        bool sqlite3_prepare_bind_values(sqlite3_cursor_t* cursor, Args&&... args) {
            bool result = false;

            foreach_args([&result, &cursor](size_t index, auto val) -> bool {
                std::string type = typeid(val).name();
                    size_t i = index + 1;

                    if (type == typeid(NULL).name()) {
                        result = __sqlite3_bind_values(cursor, i);
                    } else {
                        result = __sqlite3_bind_values(cursor, i, val);
                    }

                    return result;
            }, args...);

            return result;
        }

        /** 执行一次查询 */
        bool sqlite3_prepare_fetch(sqlite3_cursor_t* cursor) {
            return sqlite3_step(cursor->stmt) == SQLITE_ROW;
        }

        /** 获取返回结果的列数 */
        int sqlite3_prepare_column_count(sqlite3_cursor_t* cursor) {
            return sqlite3_column_count(cursor->stmt);
        }

        /** 获取指定结果列的数据类型 */
        int sqlite3_prepare_column_type(sqlite3_cursor_t* cursor, int index) {
            return sqlite3_column_type(cursor->stmt, index);
        }

        /** 获取指定结果列的列名 */
        const char* sqlite3_prepare_column_name(sqlite3_cursor_t* cursor, int index) {
            return sqlite3_column_name(cursor->stmt, index);
        }

        /**
         * 获取指定结果列的值
         */
        void sqlite3_prepare_column_value(sqlite3_cursor_t* cursor, int index, std::string& value) {
            value = (const char*)sqlite3_column_text(cursor->stmt, index);
        }

        void sqlite3_prepare_column_value(sqlite3_cursor_t* cursor, int index, double& value) {
            value = sqlite3_column_double(cursor->stmt, index);
        }

        void sqlite3_prepare_column_value(sqlite3_cursor_t* cursor, int index, int& value) {
            value = sqlite3_column_int(cursor->stmt, index);
        }

        void sqlite3_prepare_column_value(sqlite3_cursor_t* cursor, int index, sqlite3_int64& value) {
            value = sqlite3_column_int64(cursor->stmt, index);
        }
    }
}

#endif