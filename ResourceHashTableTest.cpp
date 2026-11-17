#include <iostream>
#include "../include/ResourceModule.h"

using namespace std;

int main()
{
    ResourceLinkedList resources;

    cout << "\n========== RESQFLOW RESOURCE TEST ==========\n";

    for (int i = 1; i <= 20; i++)
    {
        Resource r(i, "Resource" + to_string(i), "Medical", "Test Resource", "Units", 5);

        if (!resources.addResource(r))
        {
            cout << "Insertion failed for ID: " << i << endl;
            return 1;
        }
    }

    cout << "\nTEST 1 PASSED: 20 resources inserted.";

    Resource* found = resources.searchResource(10);

    if (found == nullptr || found->getResourceId() != 10)
    {
        cout << "\nTEST 2 FAILED: Search.";
        return 1;
    }

    cout << "\nTEST 2 PASSED: Resource ID 10 found.";

    Resource duplicate(10, "Duplicate", "Medical", "Duplicate Test", "Units", 5);

    if (resources.addResource(duplicate))
    {
        cout << "\nTEST 3 FAILED: Duplicate accepted.";
        return 1;
    }

    cout << "\nTEST 3 PASSED: Duplicate rejected.";

    if (!resources.deleteResource(10))
    {
        cout << "\nTEST 4 FAILED: Deletion.";
        return 1;
    }

    if (resources.searchResource(10) != nullptr)
    {
        cout << "\nTEST 4 FAILED: Deleted resource still found.";
        return 1;
    }

    cout << "\nTEST 4 PASSED: Resource deleted from both structures.";

    if (resources.getCount() != 19)
    {
        cout << "\nTEST 5 FAILED: Incorrect resource count.";
        return 1;
    }

    cout << "\nTEST 5 PASSED: Resource count is correct.";

    ResourceHashTable testTable(5);

    Resource testResources[10] =
    {
        Resource(101, "R1", "Medical", "Test", "Units", 1),
        Resource(102, "R2", "Medical", "Test", "Units", 1),
        Resource(103, "R3", "Medical", "Test", "Units", 1),
        Resource(104, "R4", "Medical", "Test", "Units", 1),
        Resource(105, "R5", "Medical", "Test", "Units", 1),
        Resource(106, "R6", "Medical", "Test", "Units", 1),
        Resource(107, "R7", "Medical", "Test", "Units", 1),
        Resource(108, "R8", "Medical", "Test", "Units", 1),
        Resource(109, "R9", "Medical", "Test", "Units", 1),
        Resource(110, "R10", "Medical", "Test", "Units", 1)
    };

    for (int i = 0; i < 10; i++)
    {
        if (!testTable.insert(testResources[i].getResourceId(), &testResources[i]))
        {
            cout << "\nTEST 6 FAILED: Hash insertion.";
            return 1;
        }
    }

    if (testTable.getCapacity() <= 5)
    {
        cout << "\nTEST 6 FAILED: Hash table did not resize.";
        return 1;
    }

    cout << "\nTEST 6 PASSED: Hash table resized successfully.";

    cout << "\n\nALL TESTS PASSED SUCCESSFULLY!";
    cout << "\n===========================================\n";

    return 0;
}
