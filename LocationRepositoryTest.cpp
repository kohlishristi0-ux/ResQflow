#include "ResponderModule.h"
#include <mysql.h>
#include <iostream>
#include <cstdlib>
#include <string>

using namespace std;

int main()
{
    DatabaseConnection database;
    const char* password = getenv("RESQFLOW_DB_PASSWORD");

    if (!password)
    {
        cout << "ERROR: Database password not set." << endl;
        return 1;
    }

    if (!database.connect("localhost", "root", password, "resqflow", 3306))
    {
        cout << "Database connection failed: "
             << database.getLastError() << endl;
        return 1;
    }

    cout << "Database connected successfully." << endl;

    LocationRepository repository(database);
    MYSQL* connection = database.getConnection();

    if (mysql_query(connection,
        "SELECT COALESCE(MAX(location_id), 0) + 1 FROM locations") != 0)
        return 1;

    MYSQL_RES* result = mysql_store_result(connection);
    MYSQL_ROW row = mysql_fetch_row(result);

    int firstId = stoi(row[0]);
    mysql_free_result(result);

    int secondId = firstId + 1;

    Location first(firstId, "Repository Test Location A", 30.3165, 78.0322);
    Location second(secondId, "Repository Test Location B", 30.3250, 78.0400);

    if (!repository.saveLocation(first) ||
        !repository.saveLocation(second))
    {
        cout << "FAIL: Location saving failed." << endl;
        return 1;
    }

    cout << "PASS: Locations saved." << endl;

    if (!repository.locationExists(firstId) ||
        !repository.locationExists(secondId))
    {
        cout << "FAIL: Location existence check failed." << endl;
        return 1;
    }

    cout << "PASS: Location existence verified." << endl;

    if (!repository.saveConnection(firstId, secondId, 5.5))
    {
        cout << "FAIL: Route connection saving failed." << endl;
        return 1;
    }

    cout << "PASS: Route connection saved." << endl;

    if (!repository.connectionExists(firstId, secondId))
    {
        cout << "FAIL: Connection existence check failed." << endl;
        return 1;
    }

    cout << "PASS: Connection existence verified." << endl;

    LocationGraph graph;

    if (!repository.loadLocations(graph))
    {
        cout << "FAIL: Location loading failed." << endl;
        return 1;
    }

    if (!repository.loadConnections(graph))
    {
        cout << "FAIL: Connection loading failed." << endl;
        return 1;
    }

    if (graph.getLocationCount() < 2)
    {
        cout << "FAIL: Graph location count incorrect." << endl;
        return 1;
    }

    cout << "PASS: Locations and connections loaded into graph." << endl;

    string deleteEdges =
        "DELETE FROM route_edges WHERE source_location_id IN (" +
        to_string(firstId) + "," + to_string(secondId) +
        ") OR destination_location_id IN (" +
        to_string(firstId) + "," + to_string(secondId) + ")";

    string deleteLocations =
        "DELETE FROM locations WHERE location_id IN (" +
        to_string(firstId) + "," + to_string(secondId) + ")";

    if (!database.executeQuery(deleteEdges.c_str()))
        cout << "WARNING: Route cleanup failed." << endl;

    if (!database.executeQuery(deleteLocations.c_str()))
        cout << "WARNING: Location cleanup failed." << endl;

    cout << "\nLOCATION REPOSITORY TESTS COMPLETED." << endl;

    return 0;
}
