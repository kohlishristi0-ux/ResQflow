#include <iostream>
#include <sstream>
#include <limits>
#include "../include/ActivityHistory.h"

using namespace std;

static int passed = 0;
static void report(const char* name, bool ok)
{
    cout << name << ": " << (ok ? "PASSED" : "FAILED") << endl;
    if (ok) ++passed;
}

int main()
{
    cout << "========== RESQFLOW ACTIVITY HISTORY TESTS ==========" << endl;
    ActivityHistory history;
    report("Empty history and initial count", history.isEmpty() && history.getCount() == 0);

    bool added = history.addActivity(ActivityType::EMERGENCY_REPORTED, "Emergency reported", "Coordinator", 12);
    added = added && history.addActivity(ActivityType::STOCK_RESERVED, "Stock reserved", "System", 12, 4, 7, 2, -1, -1, 15);
    added = added && history.addActivity(ActivityType::WAREHOUSE_TRANSFERRED, "Transfer completed", "Admin", -1, -1, 7, 2, 3, -1, 5);
    report("Append records and automatic IDs", added && history.getCount() == 3 &&
           history.searchByActivityId(1) != nullptr && history.searchByActivityId(3) != nullptr);

    const ActivityRecord* first = history.searchByActivityId(1);
    const ActivityRecord* second = history.searchByActivityId(2);
    report("Record fields and timestamp capture", first != nullptr && second != nullptr &&
           first->getType() == ActivityType::EMERGENCY_REPORTED && first->getEmergencyId() == 12 &&
           first->getTimestamp() > 0 && second->getQuantity() == 15 && second->getAllocationId() == 4);

    report("Invalid activity data rejected", !history.addActivity(ActivityType::STOCK_ADDED, "") &&
           !history.addActivity(ActivityType::STOCK_ADDED, "Bad quantity", "Admin", -1, -1, -1, -1, -1, -1, -2) &&
           !history.addActivity(static_cast<ActivityType>(999), "Bad type"));

    report("Search missing activity safely", history.searchByActivityId(999) == nullptr);
    report("Delete middle node", history.deleteActivity(2) && history.getCount() == 2 &&
           history.searchByActivityId(2) == nullptr && history.searchByActivityId(3) != nullptr);
    report("Delete head and tail nodes", history.deleteActivity(1) && history.deleteActivity(3) &&
           history.isEmpty() && history.getCount() == 0);
    report("Delete missing node safely", !history.deleteActivity(500));

    history.addActivity(ActivityType::STOCK_ADDED, "Stock added", "Admin", -1, -1, 9, 5, -1, -1, 20);
    history.addActivity(ActivityType::WAREHOUSE_TRANSFERRED, "Moved stock", "Admin", -1, -1, 9, 5, 6, -1, 8);
    const ActivityRecord* transfer = history.searchByActivityId(5);
    report("Warehouse transfer stores source and destination IDs", transfer != nullptr &&
           transfer->getWarehouseId() == 5 && transfer->getDestinationWarehouseId() == 6 &&
           history.getCount() == 2);

    history.clear();
    report("Clear releases all records", history.isEmpty() && history.getCount() == 0);
    history.addActivity(ActivityType::RESPONDER_ASSIGNED, "Responder assigned", "Coordinator", 22, -1, -1, -1, -1, 31);
    report("History remains usable after clear", history.getCount() == 1 && history.searchByActivityId(6) != nullptr);

    cout << "\nPassed test groups: " << passed << "/11" << endl;
    if (passed == 11) cout << "\nALL ACTIVITY HISTORY TESTS PASSED SUCCESSFULLY!" << endl;
    else cout << "\nACTIVITY HISTORY TESTS FAILED." << endl;
    return passed == 11 ? 0 : 1;
}
