// OFFLINE TEST SHIM: minimal MariaDB C API look-alike backed by SQLite.
// Used ONLY to run ResQflow without a MySQL server (see README.md here). Never used for the real build.
#pragma once
#include <cstddef>
typedef char my_bool;
struct MYSQL; struct MYSQL_RES; struct MYSQL_STMT;
typedef char** MYSQL_ROW;
typedef unsigned long long my_ulonglong;
enum enum_field_types { MYSQL_TYPE_DECIMAL, MYSQL_TYPE_TINY, MYSQL_TYPE_SHORT, MYSQL_TYPE_LONG, MYSQL_TYPE_FLOAT, MYSQL_TYPE_DOUBLE,
  MYSQL_TYPE_NULL, MYSQL_TYPE_TIMESTAMP, MYSQL_TYPE_LONGLONG, MYSQL_TYPE_INT24, MYSQL_TYPE_DATE, MYSQL_TYPE_TIME, MYSQL_TYPE_DATETIME,
  MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_BLOB, MYSQL_TYPE_VAR_STRING, MYSQL_TYPE_STRING };
struct MYSQL_BIND { unsigned long* length; my_bool* is_null; void* buffer; my_bool* error; enum_field_types buffer_type; unsigned long buffer_length; my_bool is_unsigned; };
#define MYSQL_PORT 3306
#define MYSQL_DATA_TRUNCATED 101
extern "C++" {
MYSQL* mysql_init(MYSQL*);
MYSQL* mysql_real_connect(MYSQL*,const char*,const char*,const char*,const char*,unsigned int,const char*,unsigned long);
void mysql_close(MYSQL*); int mysql_query(MYSQL*,const char*); const char* mysql_error(MYSQL*);
int mysql_commit(MYSQL*); int mysql_rollback(MYSQL*);
MYSQL_RES* mysql_store_result(MYSQL*); MYSQL_ROW mysql_fetch_row(MYSQL_RES*); void mysql_free_result(MYSQL_RES*); my_ulonglong mysql_num_rows(MYSQL_RES*);
my_ulonglong mysql_insert_id(MYSQL*); my_ulonglong mysql_affected_rows(MYSQL*);
unsigned long mysql_real_escape_string(MYSQL*,char*,const char*,unsigned long);
int mysql_set_character_set(MYSQL*,const char*);
MYSQL_STMT* mysql_stmt_init(MYSQL*); int mysql_stmt_prepare(MYSQL_STMT*,const char*,unsigned long);
my_bool mysql_stmt_bind_param(MYSQL_STMT*,MYSQL_BIND*); my_bool mysql_stmt_bind_result(MYSQL_STMT*,MYSQL_BIND*);
int mysql_stmt_execute(MYSQL_STMT*); int mysql_stmt_store_result(MYSQL_STMT*); int mysql_stmt_fetch(MYSQL_STMT*);
my_ulonglong mysql_stmt_num_rows(MYSQL_STMT*); my_ulonglong mysql_stmt_affected_rows(MYSQL_STMT*); my_ulonglong mysql_stmt_insert_id(MYSQL_STMT*);
const char* mysql_stmt_error(MYSQL_STMT*); my_bool mysql_stmt_close(MYSQL_STMT*);
}
