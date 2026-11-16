#include "../include/ResponderModule.h"
#include <iostream>
#include <limits>
using namespace std;
int main()
{
    int passed = 0;
    ResponderManager manager;
    if (manager.addResponder(Responder(1, "Asha", "Medic", "9876543210", "First Aid", 28.6, 77.2))
    && manager.addResponder(Responder(2, "Ravi", "Driver", "9876543211", "Transport", 28.7, 77.3))
    && manager.addResponder(Responder(3, "Neha", "Rescue", "9876543212", "Search", 28.8, 77.4))
    && manager.getResponderCount() == 3) {
        cout << "Registration and count: PASSED\n"; ++passed;
    }
    else
     cout << "Registration and count: FAILED\n";
    if (!manager.addResponder(Responder(1, "Duplicate", "Medic", "123", "First Aid", 0, 0))
    && !manager.addResponder(Responder(4, "", "Medic", "123", "First Aid", 0, 0))
    && !manager.addResponder(Responder(5, "Bad", "Medic", "123", "First Aid", numeric_limits<double>::quiet_NaN(), 0))) {
         cout << "Duplicate, empty-field and coordinate validation: PASSED\n"; ++passed;
    }
    else
     cout << "Duplicate, empty-field and coordinate validation: FAILED\n";
    int assigned = -1;
    if (manager.assignNextAvailableResponder(100, assigned) && assigned == 1 && manager.findResponder(1)->getAssignedEmergencyId() == 100) { cout << "Round-robin automatic assignment: PASSED\n"; ++passed; } else cout << "Round-robin automatic assignment: FAILED\n";
    if (manager.assignNextAvailableResponder(101, assigned) && assigned == 2 && manager.assignNextAvailableResponder(102, assigned) && assigned == 3 && !manager.assignNextAvailableResponder(103, assigned)) { cout << "Round-robin selection and no-availability handling: PASSED\n"; ++passed; } else cout << "Round-robin selection and no-availability handling: FAILED\n";
    if (manager.releaseResponder(2) && manager.getAvailableCount() == 1 && manager.assignNextAvailableResponder(104, assigned) && assigned == 2) { cout << "Release and scheduler cycling: PASSED\n"; ++passed; } else cout << "Release and scheduler cycling: FAILED\n";
    if (!manager.setResponderOffDuty(2) && manager.releaseResponder(2) && manager.setResponderOffDuty(2) && !manager.assignResponder(2, 105) && manager.setResponderAvailable(2)) { cout << "Status transitions and assignment guards: PASSED\n"; ++passed; } else cout << "Status transitions and assignment guards: FAILED\n";
    if (!manager.removeResponder(1) && manager.releaseResponder(1) && manager.removeResponder(1) && manager.getResponderCount() == 2 && manager.findResponder(1) == nullptr) { cout << "Safe removal and linked-list count: PASSED\n"; ++passed; } else cout << "Safe removal and linked-list count: FAILED\n";
    if (manager.addResponder(Responder(4, "Kabir", "Medic", "9876543213", "First Aid", 0, 0)) && manager.getResponderCount() == 3) { cout << "Tail insertion after removal: PASSED\n"; ++passed; } else cout << "Tail insertion after removal: FAILED\n";
    cout << "\n" << passed << "/8 test groups passed.\n";
    if (passed == 8) { cout << "ALL RESPONDER MODULE TESTS PASSED SUCCESSFULLY!\n"; return 0; }
    cout << "RESPONDER MODULE TESTS FAILED.\n"; return 1;
}
