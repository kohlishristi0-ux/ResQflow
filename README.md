# ResQflow: Resource Planning and Emergency Coordination System

## Project overview

ResQflow is a terminal application written in **C++17** with a **MySQL 8** back end. It helps emergency coordinators answer four questions during a crisis: what has happened, how urgent is it, what resources do we have and where, and who can get them there.

A coordinator registers emergencies and the system ranks them by urgency. Resources are tracked across several warehouses. Stock is reserved and dispatched against each emergency, responders are assigned, and every change is recorded in an audit trail. Access is controlled by role.

The project is built around two goals:

- **Data structures from scratch.** All core structures are hand-written. The project uses no STL containers (`vector`, `map`, `queue`, and so on).
- **Consistency between memory and database.** Every change either succeeds in both the in-memory structures and MySQL, or in neither.

## Problems solved

| Problem in emergency response | How ResQflow handles it |
|---|---|
| Several emergencies compete for limited help, and it is unclear which comes first | A max-heap ranks emergencies by severity, number of people affected and waiting time |
| Stock levels are scattered across warehouses and go out of date | One inventory per warehouse, with reserved, available and dispatched quantities and low-stock alerts |
| The same stock gets promised to two emergencies | Allocation reserves stock first, and negative stock is rejected |
| Nobody can say where stock went | Every stock change writes a movement ledger row and an activity history entry |
| Responders are double-booked or forgotten after an emergency ends | Responders have a status, are assigned round-robin, and are released automatically when the emergency is resolved or cancelled |
| Stock must move between warehouses safely | Transfers are requested, then executed or cancelled, and execution updates both warehouses together |
| The fastest route is not the one with the fewest turns | Dijkstra finds the shortest route by distance over the road graph |
| Anyone can change anything | Role-based menus, hashed passwords and an admin approval step for new responders |

### Technical problems fixed during integration

- **Update failures.** The emergency details update used the wrong column name (`location` instead of `location_name`) and always failed. It now uses the correct column and reports the real MySQL error.
- **Resource add failure.** A failed save could leave a resource in memory that was missing from the database. Memory is now rolled back whenever the database save fails.
- **Memory and database drift.** Multi-step operations such as allocation, cancellation and transfers now run in a single database transaction with rollback and in-memory restore.
- **Strict start-up.** The application refuses to start if any table fails to load, instead of running with partial data.

## Features

| Module | What it does |
|---|---|
| Resource management | Add, search (by ID or name), update, deactivate and reactivate resources |
| Warehouse management | Register warehouses with city, area and full address; update and deactivate them |
| Inventory | Per-warehouse stock with reserve, release, dispatch, return, direct transfer and low-stock alerts. Every change is written to a stock-movement ledger. |
| Emergency management | Register emergencies, change severity and affected people, run the status lifecycle, and view the priority queue |
| Allocation | Reserve stock for an emergency, dispatch, return, complete, cancel, and auto-allocate across warehouses |
| Responders | Register, assign, round-robin assignment, release, set on or off duty, and a responder dashboard |
| Warehouse transfers | Request, execute and cancel transfers between warehouses |
| Location and routes | Locations and roads as a graph, with BFS, DFS and Dijkstra shortest route |
| Activity history | Audit timeline with filters by type, emergency, allocation and warehouse |
| Users and roles | PBKDF2-SHA256 password hashing, masked password input, role-based menus, responder registration with approval |

## How emergencies are prioritised

```
score = severity * 30
      + min(affected_people / 100, 10)
      + min(waiting_seconds / 1800, 10)
```

The highest score is served first. Ties go to the emergency registered earlier, then to the lower ID. The waiting-time part changes with the clock, so the heap is re-validated before each insert, extract or peek.

**Emergency lifecycle**

```
REPORTED -> VERIFIED -> IN_PROGRESS -> RESOLVED
Any non-final state -> CANCELLED
RESOLVED and CANCELLED are final.
```

Closing an emergency releases its responders. Cancelling also cancels its open allocations and returns the reserved stock.

## Data structures used

| Structure | Used for |
|---|---|
| Singly linked lists | Resources, warehouses, emergencies, inventory, allocations, transfers, responders |
| Doubly linked list | Activity history |
| Binary max-heap | Emergency priority queue |
| Hash tables with chaining and resizing | Fast resource and warehouse lookup |
| Circular linked list | Round-robin responder assignment |
| Stack and queue (array-based) | Supporting operations |
| Adjacency-list graph | Locations and roads; BFS, DFS and Dijkstra (O(V²)) |

## Architecture

### System architecture

```text
                    RESQFLOW APPLICATION
                           |
                           v
                    Main Application
                           |
             +-------------+-------------+
             |             |             |
             v             v             v
        Emergency      Resource      Responder
        Management     Management    Management
             |             |             |
             v             v             v
        Priority       Inventory     Responder
        Management     Management    Operations
             |             |
             v             v
        Allocation    Warehouse
        Management    Management
             |             |
             +------+------+
                    |
                    v
             Route Planning
                    |
                    v
             Activity History
                    |
                    v
             Database Layer
                    |
                    v
                 MySQL
```

### Layers

```
Console menus (main.cpp)
   -> input validation and role checks
   -> managers (in-memory data structures and business rules)
   -> repositories (prepared statements)
   -> MySQL
```

Each operation that changes data runs inside a database transaction. If saving fails, the transaction is rolled back and the in-memory state is restored, so memory and MySQL never disagree.

## Folder structure

```
main.cpp                    menus, role checks, transaction helpers
include/, src/              domain classes, managers, data structures
include/Database/,
src/Database/               repositories and the MySQL connection class
database/                   SQL schema and migration scripts
tests/                      module tests, data-structure stress test, repository tests
```

## Technology stack

| Layer | Technology |
|---|---|
| Language | C++17 (object-oriented, no STL containers) |
| Compiler | g++ (MSYS2 UCRT64) |
| Database | MySQL 8.0 |
| Database access | MariaDB Connector/C, with prepared statements |
| Security | OpenSSL (PBKDF2-SHA256 password hashing) |
| Data structures | Hand-written linked lists, max-heap, hash tables, circular list, stack, queue and graph |
| Interface | Terminal (console menus) |
| Version control | Git and GitHub |

## Requirements

- Windows with **MSYS2 UCRT64** (or any environment with g++ supporting C++17)
- **MySQL 8.0** server
- **MariaDB Connector/C** (`pacman -S mingw-w64-ucrt-x86_64-libmariadbclient`)
- **OpenSSL** (`pacman -S mingw-w64-ucrt-x86_64-openssl`), used for password hashing

## Setup

### 1. Create the database

New installation:

```
mysql -u root -p < database/ResQflow_Database_Schema.sql
```

This creates the `resqflow` database and all tables.

If you already have an older ResQflow database, run `database/Migration_001_Phase3_Alignment.sql` once instead. It is safe to run twice.

### 2. Set the database password

The program reads its database settings from environment variables. Nothing is stored in the source code.

| Variable | Required | Default |
|---|---|---|
| `RESQFLOW_DB_PASSWORD` | yes | none |
| `RESQFLOW_DB_HOST` | no | `localhost` |
| `RESQFLOW_DB_USER` | no | `root` |
| `RESQFLOW_DB_NAME` | no | `resqflow` |
| `RESQFLOW_DB_PORT` | no | `3306` |

In the MSYS2 UCRT64 terminal:

```
export RESQFLOW_DB_PASSWORD='your-mysql-password'
```

This lasts for the current terminal window only. Never commit your password.

To avoid typing it every time, create your own local file, for example `resqflow_env.sh`, next to `main.cpp`:

```
export RESQFLOW_DB_PASSWORD='your-mysql-password'
export RESQFLOW_DB_USER='root'
export RESQFLOW_DB_NAME='resqflow'
```

Load it with `source resqflow_env.sh` before running the program. Add this file to your `.gitignore` so the password never reaches GitHub.

On Windows Command Prompt, use `set RESQFLOW_DB_PASSWORD=your-mysql-password` instead of `export`.

If the program prints `Database connection failed`, check that the MySQL service is running, that the password is correct, and that the `resqflow` database exists.

### 3. Build

From the project folder (the one that contains `main.cpp`):

```
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude -IC:/msys64/ucrt64/include/mariadb main.cpp src/*.cpp src/Database/*.cpp -LC:/msys64/ucrt64/lib -lmariadb -lcrypto -o ResQflow.exe
```

### 4. Run

```
./ResQflow.exe
```

On start-up the program loads all tables and prints one `[OK] ... loaded.` line for each. If any table fails to load, it stops instead of running with partial data.

The first time you run it with an empty `users` table, it asks you to create the first **ADMIN** account. The password must be at least 10 characters.

## Roles

| Role | Access |
|---|---|
| ADMIN | All modules, activity history, user management, responder approval |
| COORDINATOR | Resources, warehouses, inventory, emergencies, allocations, responders, transfers, routes |
| RESPONDER | Own dashboard only: availability and progress of the emergency assigned to them |

Responders register themselves. An admin must approve the registration before that responder can log in.

## Tests

- **Module tests** (`tests/*.cpp`): data-structure and manager tests that need no database. Each file has its build command in its header comment.
- **Data-structure stress test** (`tests/DataStructureStressTest.cpp`).
- **Repository tests** (`tests/Database/*.cpp`): need a running MySQL with the schema loaded.
- **Offline end-to-end tests** (`tests/offline/`): run the real program against a SQLite stand-in for the MariaDB API. They check program logic and SQL structure but do not replace testing against MySQL.

## Design notes

- Passwords are hashed with PBKDF2-SHA256 (210,000 iterations) and are read without echo.
- Database credentials come only from environment variables.
- All queries that take user input use prepared statements.
- Records are deactivated, not deleted, so history stays intact in MySQL.
- Stock can never go negative, and deactivation is blocked while stock, active allocations or pending transfers exist.
- There is no live GPS. Routes use stored or typed coordinates.

## Team and work division

| Member | Responsibility | Main work |
|---|---|---|
| SHRISTI KOHLI | Core architecture and integration | Overall C++ architecture, resource management, warehouse management, inventory, project integration and final release |
| SATYAM SHARMA | Emergency management | Emergency lifecycle, priority calculation, max heap, emergency testing |
| SHAURYA PAL| Allocation and resource coordination | Allocation workflows, stock validation, warehouse transfers, responder management |
| VAISHNAVY CHAUHAN | Database and supporting modules | MySQL schema and testing, location and route planning, activity history, user management |

Replace "Member 1" to "Member 4" with the real names before publishing.

## Future enhancements

The current system establishes a foundation for further development. Possible future improvements include:

- A graphical user interface.
- A web-based emergency coordination dashboard.
- Automated notifications for critical emergencies.
- More advanced resource-shortage alerts.
- Improved route optimisation.
- Enhanced analytics and reporting.
- More detailed operational monitoring.
- Additional database-backed reporting.
- Integration with external emergency information systems.

## Known limitations

- The application is single-user. Several instances running at once are not coordinated.
- Heap re-validation is O(n) per operation, because the priority score depends on waiting time.
- Warehouses have no coordinates, so they are not vertices of the route graph.
- Terminal interface only.
