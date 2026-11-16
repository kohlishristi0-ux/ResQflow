#include <iostream>
#include <cstdlib>
#include <ctime>
#include "ResourceModule.h"

using namespace std;

int main()
{
    const char* password = getenv("RESQFLOW_DB_PASSWORD");

    if (password == nullptr)
    {
        cout << "ERROR: Database password not found." << endl;
        return 1;
    }

    DatabaseConnection db;

    if (!db.connect("127.0.0.1", "root", password, "resqflow", 3306))
    {
        cout << "CONNECTION FAILED: " << db.getLastError() << endl;
        return 1;
    }

    cout << "SUCCESS: Database connected!" << endl;

    ResourceRepository repository(db);
    ResourceLinkedList resourceList;

    int testId = 1000000000 +
                 static_cast<int>(time(nullptr) % 1000000000);

    if (repository.resourceExists(testId))
    {
        cout << "ERROR: Test resource ID already exists." << endl;
        return 1;
    }

    Resource resource(
        testId,
        "Repository Test Resource",
        "Testing",
        "Resource repository integration test",
        "Units",
        10
    );

    if (!repository.saveResource(resource))
    {
        cout << "FAILED: Resource insertion." << endl;
        return 1;
    }

    cout << "SUCCESS: Resource saved to MySQL!" << endl;

    if (!repository.resourceExists(testId))
    {
        cout << "FAILED: Resource existence check." << endl;
        return 1;
    }

    cout << "SUCCESS: Resource existence check passed!" << endl;

    if (!repository.loadActiveResources(resourceList))
    {
        cout << "FAILED: Loading active resources." << endl;
        return 1;
    }

    Resource* loadedResource = resourceList.searchResource(testId);

    if (loadedResource == nullptr ||
        loadedResource->getResourceName() != "Repository Test Resource")
    {
        cout << "FAILED: Resource loading into data structures." << endl;
        return 1;
    }

    cout << "SUCCESS: Resource loaded into linked list and hash table!" << endl;

    Resource updatedResource(
        testId,
        "Updated Repository Test Resource",
        "Testing",
        "Updated resource description",
        "Units",
        20
    );

    if (!repository.updateResource(updatedResource))
    {
        cout << "FAILED: Resource update." << endl;
        return 1;
    }

    cout << "SUCCESS: Resource updated in MySQL!" << endl;

    if (!repository.loadActiveResources(resourceList))
    {
        cout << "FAILED: Reloading updated resources." << endl;
        return 1;
    }

    loadedResource = resourceList.searchResource(testId);

    if (loadedResource == nullptr ||
        loadedResource->getResourceName() !=
        "Updated Repository Test Resource" ||
        loadedResource->getMinimumStockLevel() != 20)
    {
        cout << "FAILED: Updated resource verification." << endl;
        return 1;
    }

    cout << "SUCCESS: Updated resource verified!" << endl;

    if (!repository.deactivateResource(testId))
    {
        cout << "FAILED: Resource deactivation." << endl;
        return 1;
    }

    cout << "SUCCESS: Resource deactivated in MySQL!" << endl;

    if (!repository.loadActiveResources(resourceList))
    {
        cout << "FAILED: Reloading after deactivation." << endl;
        return 1;
    }

    if (resourceList.searchResource(testId) != nullptr)
    {
        cout << "FAILED: Inactive resource still exists in active list." << endl;
        return 1;
    }

    cout << "SUCCESS: Inactive resource excluded from active data structures!" << endl;

    if (!repository.resourceExists(testId))
    {
        cout << "FAILED: Inactive resource was not retained in database." << endl;
        return 1;
    }

    cout << "\nALL RESOURCE REPOSITORY TESTS PASSED!" << endl;

    db.disconnect();

    return 0;
}
