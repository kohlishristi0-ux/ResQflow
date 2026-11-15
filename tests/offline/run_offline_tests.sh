#!/bin/bash
# Offline test run (Linux / WSL / MSYS2 with sqlite3 + OpenSSL installed). NO MySQL server needed.
# Builds the REAL ResQflow sources against tests/offline/mysql_shim.cpp (a SQLite-backed look-alike of the
# MariaDB C API) and drives the finished program through every menu. See tests/offline/README.md.
set -e
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="${TMPDIR:-/tmp}/resqflow_offline"
mkdir -p "$OUT"
cd "$ROOT"
python3 tests/offline/translate_schema.py database/ResQflow_Database_Schema.sql "$OUT/schema.sql"
SQLITE_LIB="-lsqlite3"; [ -e /usr/lib/x86_64-linux-gnu/libsqlite3.so.0 ] && [ ! -e /usr/lib/x86_64-linux-gnu/libsqlite3.so ] && SQLITE_LIB="-l:libsqlite3.so.0"
CRYPTO_LIB="-lcrypto"; [ -e /usr/lib/x86_64-linux-gnu/libcrypto.so.3 ] && [ ! -e /usr/lib/x86_64-linux-gnu/libcrypto.so ] && CRYPTO_LIB="-l:libcrypto.so.3"
echo "Compiling..."
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude -Itests/offline -idirafter tests/offline/stub \
    main.cpp src/*.cpp src/Database/*.cpp tests/offline/mysql_shim.cpp $SQLITE_LIB $CRYPTO_LIB -o "$OUT/resqflow_offline"
echo "Running end-to-end suites..."
RQ_BIN="$OUT/resqflow_offline" RQ_SCHEMA="$OUT/schema.sql" RQ_DB="$OUT/e2e.db" python3 tests/offline/e2e/run_all.py
