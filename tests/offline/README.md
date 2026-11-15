# Offline test harness (no MySQL server required)

`mysql.h` + `mysql_shim.cpp` re-implement the small part of the MariaDB C API that ResQflow uses on top of SQLite,
and `translate_schema.py` converts `database/ResQflow_Database_Schema.sql` into SQLite DDL. The **unchanged** ResQflow
sources are compiled against them, and `e2e/*.py` drive the finished program through its real menus and then inspect
the database rows.

```
bash tests/offline/run_offline_tests.sh        # needs g++, python3, libsqlite3, libcrypto
```

What it proves: column names, parameter counts/types, bind lifetimes, transaction/rollback behaviour, menu logic,
role checks, restart/reload consistency.
What it cannot prove: MySQL-specific behaviour (strict typing, `ENUM`/`CHECK` enforcement details, "affected rows =
changed rows", time zones, locking). Run the smoke test from `docs/TEST_REPORT.md` on your real MySQL.

This folder is test tooling only – it is not part of the ResQflow build command.
