# ResQflow – Resource Planning and Emergency Coordination System (Phase 3)

Terminal application written in C++17 with **custom data structures** (no STL containers) and a **MySQL** back end.
It coordinates emergency response: resources, multi-warehouse inventory, emergencies with a priority queue,
allocations, responders, warehouse transfers, location/route planning, activity history and role-based login.

## Folder structure

```
main.cpp                      menus, role checks, transaction/rollback helpers
include/  src/                domain classes, managers, DSA implementations
include/Database/ src/Database/   one repository per table group (prepared statements) + DatabaseConnection
database/                     ResQflow_Database_Schema.sql (fresh install), Migration_001_Phase3_Alignment.sql (old DB)
config/                       environment-variable templates (no passwords)
tests/                        module tests, DSA stress test, tests/Database (needs MySQL), tests/offline (no MySQL needed)
```

## 1. Database setup (MySQL 8.0)

Fresh install:

```
mysql -u root -p < database/ResQflow_Database_Schema.sql
```

Existing Phase 3 database created from the old SQL: run `database/Migration_001_Phase3_Alignment.sql` once
(adds the responder columns the code needs; safe to run twice).

## 2. Configuration (environment variables – nothing is stored in the source)

| Variable | Required | Default |
|---|---|---|
| `RESQFLOW_DB_PASSWORD` | yes | – |
| `RESQFLOW_DB_HOST` | no | `localhost` |
| `RESQFLOW_DB_USER` | no | `root` |
| `RESQFLOW_DB_NAME` | no | `resqflow` |
| `RESQFLOW_DB_PORT` | no | `3306` |

**Windows Command Prompt:**
1. Copy `config\\resqflow_env_template.bat` to `config\\resqflow_env.bat`.
2. Open the copied file and replace `CHANGE_ME` with your MySQL password.
3. From the project directory, run `call config\\resqflow_env.bat`.
4. Start the application using `ResQflow.exe`.

**MSYS2 UCRT64 shell:**
1. Copy `config/resqflow_env_template.sh` to `config/resqflow_env.sh`.
2. Replace `CHANGE_ME` with your MySQL password.
3. Run `source config/resqflow_env.sh`.
4. Start the application using `./ResQflow.exe`.

Never commit the filled-in configuration files. They are excluded by `.gitignore`.

## 3. Build and run (MSYS2 UCRT64)

Your original command is correct – it already compiles `main.cpp`, every `src/*.cpp` and every `src/Database/*.cpp`
and links `mariadb` + `crypto` (OpenSSL is needed by `PasswordHasher`):

```
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude -IC:/msys64/ucrt64/include/mariadb main.cpp src/*.cpp src/Database/*.cpp -LC:/msys64/ucrt64/lib -lmariadb -lcrypto -o ResQflow.exe
./ResQflow.exe
```

The first start (empty `users` table) asks you to create the first ADMIN account (password ≥ 10 characters, hashed with PBKDF2-SHA256).

## 4. Roles

| Role | Access |
|---|---|
| ADMIN | all modules, activity history, user management, responder approval |
| COORDINATOR | resources, warehouses, inventory, emergencies, allocations, responders, transfers, routes |
| RESPONDER | own dashboard only: availability, progress of the emergency assigned to them |

## 5. Tests

* **Data-structure tests (no DB):** see `tests/DataStructureStressTest.cpp` and the module tests in `tests/*.cpp`.
* **Repository tests (need MySQL):** `tests/Database/*.cpp` – each file has its own build line in the header comment.
* **Offline end-to-end tests (no MySQL):** `tests/offline/run_offline_tests.sh` (Linux/WSL) – see `tests/offline/README.md`.
  These use a SQLite stand-in for the MariaDB API, so they prove the program logic and SQL structure, **not** MySQL itself.
