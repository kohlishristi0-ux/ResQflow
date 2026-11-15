// Stress / invariant tests for ResQflow's custom data structures (no database needed).
// Build (from the project root):
//   g++ -std=c++17 -Iinclude tests/DataStructureStressTest.cpp src/Emergency.cpp src/EmergencyMaxHeap.cpp src/Resource.cpp src/ResourceHashTable.cpp src/ResourceLinkedList.cpp src/Location.cpp src/LocationGraph.cpp src/Stack.cpp -o DataStructureStressTest
#include <iostream>
#include <sstream>
#include <cstdlib>
#include "../include/EmergencyMaxHeap.h"
#include "../include/ResourceLinkedList.h"
#include "../include/ResourceHashTable.h"
#include "../include/LocationGraph.h"

using namespace std;

static int failures = 0;
static void check(const char* name, bool ok)
{
    cout << (ok ? "PASS  " : "FAIL  ") << name << "\n";
    if (!ok) ++failures;
}

int main()
{
    srand(12345);

    // ---------- max-heap ----------
    {
        EmergencyMaxHeap heap;
        time_t now = time(nullptr);
        bool valid = true;
        const int N = 2000;                                  // forces several capacity doublings (10 -> 2560)
        for (int i = 1; i <= N; ++i)
        {
            Emergency e(i, "T", "L", "d", 1 + rand() % 5, 0, 0, i, 1 + rand() % 5000, now);
            if (!heap.insertEmergency(e)) valid = false;
            if (i % 97 == 0 && !heap.isValidHeap()) valid = false;
        }
        check("heap: 2000 inserts with resizing keep the heap property", valid && heap.getSize() == N && heap.isValidHeap());
        check("heap: duplicate ID rejected", !heap.insertEmergency(Emergency(5, "T", "L", "d", 3, 0, 0, 9999, 10, now)));

        for (int i = 0; i < 300; ++i)                        // random priority updates
            heap.updatePriority(1 + rand() % N, 1 + rand() % 5, 1 + rand() % 5000);
        check("heap: priority updates keep the heap property", heap.isValidHeap());

        int removed = 0;
        for (int i = 0; i < 300; ++i)                        // random removals
            if (heap.removeEmergency(1 + rand() % N)) ++removed;
        check("heap: removals keep the heap property", heap.isValidHeap() && heap.getSize() == N - removed);
        check("heap: removing a missing ID is safe", !heap.removeEmergency(999999));

        Emergency previous, current;
        bool ordered = heap.extractMax(previous);
        int extracted = 1;
        while (heap.extractMax(current))
        {
            // scores never increase; on equal score the earlier registration must come first
            double a = previous.calculatePriorityScore(), b = current.calculatePriorityScore();
            if (b > a + 1e-9) ordered = false;
            if (b == a && current.getRegistrationOrder() < previous.getRegistrationOrder()) ordered = false;
            previous = current;
            ++extracted;
        }
        check("heap: extraction order is non-increasing priority", ordered && extracted == N - removed && heap.isEmpty());
        check("heap: extract/peek on empty heap are safe", !heap.extractMax(current) && !heap.peekMax(current));

        EmergencyMaxHeap ties;                               // equal priority -> FIFO by registration order
        for (int i = 1; i <= 50; ++i) ties.insertEmergency(Emergency(i, "T", "L", "d", 3, 0, 0, i, 100, now));
        bool fifo = true; Emergency e;
        for (int i = 1; i <= 50; ++i) { ties.extractMax(e); if (e.getEmergencyId() != i) fifo = false; }
        check("heap: equal priorities leave in registration order", fifo);
    }

    // ---------- hash table + linked list ----------
    {
        ResourceHashTable table;
        const int N = 5000;
        Resource* store = new Resource[N];
        bool ok = true;
        for (int i = 0; i < N; ++i)
        {
            store[i] = Resource(i * 10 + 1, "R", "C", "D", "U", 0);   // ids share residues -> collisions
            if (!table.insert(store[i].getResourceId(), &store[i])) ok = false;
        }
        for (int i = 0; i < N; ++i)
            if (table.search(i * 10 + 1) != &store[i]) ok = false;
        check("hash: 5000 inserts survive collisions and resizing", ok && table.getCount() == N && table.getCapacity() > 10);
        check("hash: duplicate rejected, missing key not found", !table.insert(1, &store[0]) && table.search(2) == nullptr);
        for (int i = 0; i < N; i += 2) if (!table.remove(i * 10 + 1)) ok = false;
        bool rest = true;
        for (int i = 0; i < N; ++i)
            if ((table.search(i * 10 + 1) != nullptr) != (i % 2 == 1)) rest = false;
        check("hash: deletes leave the other entries intact", ok && rest && table.getCount() == N / 2);
        delete[] store;

        ResourceLinkedList list;
        bool listOk = true;
        for (int i = 1; i <= 3000; ++i) if (!list.addResource(Resource(i, "R", "C", "D", "U", 1))) listOk = false;
        check("list: 3000 inserts, duplicates rejected", listOk && !list.addResource(Resource(5, "x", "c", "d", "u", 1)) && list.getCount() == 3000);
        for (int i = 1; i <= 3000; i += 3) if (!list.deleteResource(i)) listOk = false;   // head, middle and tail deletions
        check("list: deletions (head/middle/tail) keep search consistent",
              listOk && list.searchResource(1) == nullptr && list.searchResource(2) != nullptr && list.getCount() == 2000);
        check("list: update + invalid update", list.updateResource(2, "N", "C", "D", "U", 7) &&
              list.searchResource(2)->getMinimumStockLevel() == 7 && !list.updateResource(2, "N", "C", "D", "U", -1));
        list.clear();
        check("list: clear empties it and it stays usable", list.isEmpty() && list.addResource(Resource(1, "R", "C", "D", "U", 1)));
    }

    // ---------- graph: Dijkstra / BFS / DFS ----------
    {
        LocationGraph g;
        const int N = 200;
        for (int i = 1; i <= N; ++i) g.addLocation(Location(i, "V" + to_string(i), 10.0 + i * 0.01, 20.0));
        for (int i = 1; i < N; ++i) g.addConnection(i, i + 1, 2.0);         // chain: distance (j - i) * 2
        g.addConnection(1, N, 1000.0);                                       // long shortcut must NOT be used
        stringstream captured; streambuf* old = cout.rdbuf(captured.rdbuf());
        g.findShortestPath(1, N);
        cout.rdbuf(old);
        check("graph: Dijkstra picks the chain (398 km) over the 1000 km shortcut", captured.str().find("Total Distance: 398 km") != string::npos);

        g.addLocation(Location(500, "Island", 50.0, 50.0));
        captured.str(""); old = cout.rdbuf(captured.rdbuf());
        g.findShortestPath(1, 500);
        g.findShortestPath(1, 12345);
        g.breadthFirstSearch(1); g.depthFirstSearch(1);
        cout.rdbuf(old);
        string text = captured.str();
        check("graph: disconnected and unknown vertices are reported", text.find("No path exists") != string::npos &&
              text.find("not found") != string::npos);
        check("graph: BFS/DFS visit every connected vertex but not the island", text.find("V200") != string::npos && text.find("Island ") == string::npos);
        check("graph: removing a vertex removes its edges", g.removeLocation(100) && !g.hasConnection(99, 100) && !g.hasConnection(101, 100));
        captured.str(""); old = cout.rdbuf(captured.rdbuf());
        g.findShortestPath(1, 150);
        cout.rdbuf(old);
        check("graph: path re-routes through the 1000 km shortcut once the chain is cut (1000 + 100 = 1100 km)",
              captured.str().find("Total Distance: 1100 km") != string::npos);
        check("graph: invalid edges rejected", !g.addConnection(1, 1, 5) && !g.addConnection(1, 2, -1) && !g.addConnection(1, 999, 3) && !g.addConnection(1, 2, 3));
    }

    cout << (failures == 0 ? "\nALL DATA-STRUCTURE STRESS TESTS PASSED\n" : "\nSOME DATA-STRUCTURE STRESS TESTS FAILED\n");
    return failures == 0 ? 0 : 1;
}
