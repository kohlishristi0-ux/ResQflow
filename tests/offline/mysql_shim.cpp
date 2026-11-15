// OFFLINE TEST SHIM implementation (SQLite backend). See mysql.h and README.md.
#include "mysql.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

// ---- minimal SQLite prototypes (no sqlite3.h needed) ----
struct sqlite3; struct sqlite3_stmt; struct sqlite3_context; struct sqlite3_value;
extern "C" {
int sqlite3_open(const char*, sqlite3**); int sqlite3_close(sqlite3*);
int sqlite3_exec(sqlite3*, const char*, int(*)(void*,int,char**,char**), void*, char**);
int sqlite3_prepare_v2(sqlite3*, const char*, int, sqlite3_stmt**, const char**);
int sqlite3_step(sqlite3_stmt*); int sqlite3_finalize(sqlite3_stmt*); int sqlite3_reset(sqlite3_stmt*);
int sqlite3_bind_parameter_count(sqlite3_stmt*);
int sqlite3_bind_int64(sqlite3_stmt*, int, long long); int sqlite3_bind_double(sqlite3_stmt*, int, double);
int sqlite3_bind_text(sqlite3_stmt*, int, const char*, int, void(*)(void*)); int sqlite3_bind_null(sqlite3_stmt*, int);
int sqlite3_column_count(sqlite3_stmt*); int sqlite3_column_type(sqlite3_stmt*, int);
const unsigned char* sqlite3_column_text(sqlite3_stmt*, int); int sqlite3_column_bytes(sqlite3_stmt*, int);
const char* sqlite3_errmsg(sqlite3*); int sqlite3_changes(sqlite3*); long long sqlite3_last_insert_rowid(sqlite3*);
int sqlite3_create_function(sqlite3*, const char*, int, int, void*, void(*)(sqlite3_context*,int,sqlite3_value**), void*, void*);
long long sqlite3_value_int64(sqlite3_value*); int sqlite3_value_type(sqlite3_value*); const unsigned char* sqlite3_value_text(sqlite3_value*);
void sqlite3_result_int64(sqlite3_context*, long long); void sqlite3_result_text(sqlite3_context*, const char*, int, void(*)(void*)); void sqlite3_result_null(sqlite3_context*);
}
#define SQ_ROW 100
#define SQ_DONE 101
#define SQ_TRANSIENT ((void(*)(void*))-1)

struct MYSQL { sqlite3* db = nullptr; std::string err; my_ulonglong affected = 0, insertId = 0; };
struct MYSQL_RES { std::vector<std::vector<std::string>> rows; std::vector<std::vector<bool>> nulls; size_t pos = 0; std::vector<char*> cur; };
struct MYSQL_STMT { MYSQL* conn = nullptr; sqlite3_stmt* st = nullptr; std::string err; MYSQL_BIND* params = nullptr; MYSQL_BIND* results = nullptr;
  std::vector<std::vector<std::string>> rows; std::vector<std::vector<bool>> nulls; size_t pos = 0; bool stored = false; bool executed = false;
  my_ulonglong affected = 0; int ncols = 0; };

static void fn_from_unixtime(sqlite3_context* c, int, sqlite3_value** v) {
  if (sqlite3_value_type(v[0]) == 5) { sqlite3_result_null(c); return; }
  time_t t = (time_t)sqlite3_value_int64(v[0]); struct tm tmv; gmtime_r(&t, &tmv); char b[32]; strftime(b, sizeof b, "%Y-%m-%d %H:%M:%S", &tmv);
  sqlite3_result_text(c, b, -1, SQ_TRANSIENT);
}
static void fn_unix_timestamp(sqlite3_context* c, int, sqlite3_value** v) {
  if (sqlite3_value_type(v[0]) == 5) { sqlite3_result_null(c); return; }
  const char* s = (const char*)sqlite3_value_text(v[0]); struct tm tmv; memset(&tmv, 0, sizeof tmv);
  if (!s || !strptime(s, "%Y-%m-%d %H:%M:%S", &tmv)) { sqlite3_result_null(c); return; }
  sqlite3_result_int64(c, (long long)timegm(&tmv));
}
static void fn_now(sqlite3_context* c, int, sqlite3_value**) {
  time_t t = time(nullptr); struct tm tmv; gmtime_r(&t, &tmv); char b[32]; strftime(b, sizeof b, "%Y-%m-%d %H:%M:%S", &tmv);
  sqlite3_result_text(c, b, -1, SQ_TRANSIENT);
}
// MySQL -> SQLite dialect differences that matter for ResQflow's statements.
static std::string translate(const char* q) {
  std::string s = q; size_t p;
  while ((p = s.find(" FOR UPDATE")) != std::string::npos) s.erase(p, 11);
  if (s == "START TRANSACTION") s = "BEGIN";
  return s;
}

MYSQL* mysql_init(MYSQL*) { return new MYSQL(); }
MYSQL* mysql_real_connect(MYSQL* m, const char*, const char*, const char*, const char*, unsigned int, const char*, unsigned long) {
  const char* path = getenv("RESQFLOW_SQLITE_DB"); if (!path) path = ":memory:";
  if (sqlite3_open(path, &m->db) != 0) return nullptr;
  sqlite3_create_function(m->db, "FROM_UNIXTIME", 1, 1, nullptr, fn_from_unixtime, nullptr, nullptr);
  sqlite3_create_function(m->db, "UNIX_TIMESTAMP", 1, 1, nullptr, fn_unix_timestamp, nullptr, nullptr);
  sqlite3_create_function(m->db, "NOW", 0, 1, nullptr, fn_now, nullptr, nullptr);
  sqlite3_exec(m->db, "PRAGMA foreign_keys=ON", nullptr, nullptr, nullptr);
  const char* schema = getenv("RESQFLOW_SQLITE_SCHEMA");
  if (schema) { std::ifstream f(schema); std::stringstream ss; ss << f.rdbuf(); char* e = nullptr;
    if (sqlite3_exec(m->db, ss.str().c_str(), nullptr, nullptr, &e) != 0) { std::cerr << "[shim] schema load failed: " << (e ? e : "?") << "\n"; return nullptr; } }
  return m;
}
void mysql_close(MYSQL* m) { if (!m) return; if (m->db) sqlite3_close(m->db); delete m; }
const char* mysql_error(MYSQL* m) { return m->err.c_str(); }
int mysql_set_character_set(MYSQL*, const char*) { return 0; }
static int execDirect(MYSQL* m, const std::string& sql) {
  char* e = nullptr; int rc = sqlite3_exec(m->db, sql.c_str(), nullptr, nullptr, &e);
  if (rc != 0) { m->err = e ? e : sqlite3_errmsg(m->db); return 1; } m->err.clear(); return 0; }
int mysql_commit(MYSQL* m) { return execDirect(m, "COMMIT"); }
int mysql_rollback(MYSQL* m) { return execDirect(m, "ROLLBACK"); }
my_ulonglong mysql_insert_id(MYSQL* m) { return m->insertId; }
my_ulonglong mysql_affected_rows(MYSQL* m) { return m->affected; }

static MYSQL_RES* g_pending = nullptr;
int mysql_query(MYSQL* m, const char* q) {
  std::string sql = translate(q); sqlite3_stmt* st = nullptr;
  if (sqlite3_prepare_v2(m->db, sql.c_str(), -1, &st, nullptr) != 0) { m->err = sqlite3_errmsg(m->db); return 1; }
  MYSQL_RES* res = nullptr; int nc = sqlite3_column_count(st); if (nc > 0) res = new MYSQL_RES();
  int rc;
  while ((rc = sqlite3_step(st)) == SQ_ROW) {
    if (!res) continue;
    std::vector<std::string> row; std::vector<bool> nul;
    for (int i = 0; i < nc; ++i) {
      if (sqlite3_column_type(st, i) == 5) { row.push_back(""); nul.push_back(true); }
      else { const unsigned char* t = sqlite3_column_text(st, i); row.push_back(std::string((const char*)t, sqlite3_column_bytes(st, i))); nul.push_back(false); } }
    res->rows.push_back(row); res->nulls.push_back(nul); }
  if (rc != SQ_DONE) { m->err = sqlite3_errmsg(m->db); sqlite3_finalize(st); delete res; return 1; }
  sqlite3_finalize(st); m->affected = nc > 0 ? 0 : (my_ulonglong)sqlite3_changes(m->db);
  m->insertId = (my_ulonglong)sqlite3_last_insert_rowid(m->db); m->err.clear();
  delete g_pending; g_pending = res; return 0;
}
MYSQL_RES* mysql_store_result(MYSQL*) { MYSQL_RES* r = g_pending; g_pending = nullptr; return r; }
my_ulonglong mysql_num_rows(MYSQL_RES* r) { return r->rows.size(); }
MYSQL_ROW mysql_fetch_row(MYSQL_RES* r) {
  if (r->pos >= r->rows.size()) return nullptr;
  auto& row = r->rows[r->pos]; auto& nul = r->nulls[r->pos]; ++r->pos;
  r->cur.assign(row.size() + 1, nullptr);
  for (size_t i = 0; i < row.size(); ++i) r->cur[i] = nul[i] ? nullptr : const_cast<char*>(row[i].c_str());
  return r->cur.data();
}
void mysql_free_result(MYSQL_RES* r) { delete r; }
// MySQL-style literal escaping adapted for SQLite: quotes are doubled, nothing else is special.
unsigned long mysql_real_escape_string(MYSQL*, char* to, const char* from, unsigned long n) {
  unsigned long o = 0; for (unsigned long i = 0; i < n; ++i) { if (from[i] == '\'') to[o++] = '\''; to[o++] = from[i]; } to[o] = 0; return o; }

MYSQL_STMT* mysql_stmt_init(MYSQL* m) { auto* s = new MYSQL_STMT(); s->conn = m; return s; }
int mysql_stmt_prepare(MYSQL_STMT* s, const char* q, unsigned long len) {
  std::string sql = translate(std::string(q, len).c_str());
  if (sqlite3_prepare_v2(s->conn->db, sql.c_str(), -1, &s->st, nullptr) != 0) { s->err = sqlite3_errmsg(s->conn->db); s->conn->err = s->err; return 1; }
  s->ncols = sqlite3_column_count(s->st); return 0; }
my_bool mysql_stmt_bind_param(MYSQL_STMT* s, MYSQL_BIND* b) { s->params = b; return 0; }
my_bool mysql_stmt_bind_result(MYSQL_STMT* s, MYSQL_BIND* b) { s->results = b; return 0; }
int mysql_stmt_execute(MYSQL_STMT* s) {
  sqlite3_stmt* st = s->st;
  sqlite3_reset(st);
  int np = sqlite3_bind_parameter_count(st);
  if (np > 0 && !s->params) { s->err = "no parameters bound"; return 1; }
  for (int i = 0; i < np; ++i) { MYSQL_BIND& b = s->params[i];
    if (b.is_null && *b.is_null) { sqlite3_bind_null(st, i + 1); continue; }
    switch (b.buffer_type) {
      case MYSQL_TYPE_LONG: sqlite3_bind_int64(st, i + 1, *(int*)b.buffer); break;
      case MYSQL_TYPE_LONGLONG: sqlite3_bind_int64(st, i + 1, *(long long*)b.buffer); break;
      case MYSQL_TYPE_TINY: sqlite3_bind_int64(st, i + 1, *(signed char*)b.buffer); break;
      case MYSQL_TYPE_DOUBLE: sqlite3_bind_double(st, i + 1, *(double*)b.buffer); break;
      case MYSQL_TYPE_STRING: case MYSQL_TYPE_VAR_STRING: { unsigned long n = b.length ? *b.length : b.buffer_length;
        sqlite3_bind_text(st, i + 1, (const char*)b.buffer, (int)n, SQ_TRANSIENT); break; }
      default: s->err = "unsupported bind type"; return 1; } }
  s->rows.clear(); s->nulls.clear(); s->pos = 0; s->stored = false;
  int rc;
  while ((rc = sqlite3_step(st)) == SQ_ROW) {
    std::vector<std::string> row; std::vector<bool> nul;
    for (int i = 0; i < s->ncols; ++i) {
      if (sqlite3_column_type(st, i) == 5) { row.push_back(""); nul.push_back(true); }
      else { const unsigned char* t = sqlite3_column_text(st, i); row.push_back(std::string((const char*)t, sqlite3_column_bytes(st, i))); nul.push_back(false); } }
    s->rows.push_back(row); s->nulls.push_back(nul); }
  if (rc != SQ_DONE) { s->err = sqlite3_errmsg(s->conn->db); s->conn->err = s->err; sqlite3_reset(st); return 1; }
  sqlite3_reset(st);
  s->affected = s->ncols > 0 ? (my_ulonglong)s->rows.size() : (my_ulonglong)sqlite3_changes(s->conn->db);
  s->conn->insertId = (my_ulonglong)sqlite3_last_insert_rowid(s->conn->db); s->conn->affected = s->affected; s->executed = true; s->err.clear(); return 0; }
int mysql_stmt_store_result(MYSQL_STMT* s) { if (!s->executed) return 1; s->stored = true; return 0; }
my_ulonglong mysql_stmt_num_rows(MYSQL_STMT* s) { return s->stored ? s->rows.size() : 0; }  // like the real API: 0 unless stored
my_ulonglong mysql_stmt_affected_rows(MYSQL_STMT* s) { return s->affected; }
my_ulonglong mysql_stmt_insert_id(MYSQL_STMT* s) { return s->conn->insertId; }
const char* mysql_stmt_error(MYSQL_STMT* s) { return s->err.c_str(); }
int mysql_stmt_fetch(MYSQL_STMT* s) {
  if (s->pos >= s->rows.size()) return 100;  // MYSQL_NO_DATA
  if (!s->results) { ++s->pos; return 0; }
  auto& row = s->rows[s->pos]; auto& nul = s->nulls[s->pos]; ++s->pos; bool trunc = false;
  for (int i = 0; i < s->ncols; ++i) { MYSQL_BIND& b = s->results[i];
    if (nul[i]) { if (b.is_null) *b.is_null = 1; if (b.length) *b.length = 0; continue; }
    if (b.is_null) *b.is_null = 0;
    switch (b.buffer_type) {
      case MYSQL_TYPE_LONG: *(int*)b.buffer = (int)strtoll(row[i].c_str(), nullptr, 10); break;
      case MYSQL_TYPE_LONGLONG: *(long long*)b.buffer = strtoll(row[i].c_str(), nullptr, 10); break;
      case MYSQL_TYPE_TINY: *(signed char*)b.buffer = (signed char)strtol(row[i].c_str(), nullptr, 10); break;
      case MYSQL_TYPE_DOUBLE: *(double*)b.buffer = strtod(row[i].c_str(), nullptr); break;
      case MYSQL_TYPE_STRING: case MYSQL_TYPE_VAR_STRING: { unsigned long len = row[i].size(); unsigned long n = len < b.buffer_length ? len : b.buffer_length;
        memcpy(b.buffer, row[i].data(), n); if (b.length) *b.length = len; if (len > b.buffer_length) trunc = true; break; }
      default: break; } }
  return trunc ? MYSQL_DATA_TRUNCATED : 0; }
my_bool mysql_stmt_close(MYSQL_STMT* s) { if (s->st) sqlite3_finalize(s->st); delete s; return 0; }
