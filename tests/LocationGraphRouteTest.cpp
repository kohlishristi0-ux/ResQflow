#include "../include/LocationGraph.h"
#include "../include/RoutePlanner.h"
#include "../include/Stack.h"
#include <iostream>
#include <sstream>
#include <limits>
#include <cmath>
using namespace std;
static int passed = 0;
static bool check(bool condition, const char* name)
{
    cout << name << ": " << (condition ? "PASSED" : "FAILED") << "\n";
    if (condition) ++passed;
    return condition;
}
static string captureTraversal(LocationGraph& graph, bool bfs, int id)
{
    ostringstream output; streambuf* old = cout.rdbuf(output.rdbuf());
    if (bfs) graph.breadthFirstSearch(id); else graph.depthFirstSearch(id);
    cout.rdbuf(old); return output.str();
}
int main()
{
    cout << "========== RESQFLOW LOCATION & ROUTING TESTS ==========\n";
    Stack stack(2); int value = 0;
    bool stackOK = stack.isEmpty() && stack.push(10) && stack.push(20) && !stack.push(30) && stack.peek(value) && value == 20 && stack.pop(value) && value == 20 && stack.pop(value) && value == 10 && !stack.pop(value);
    check(stackOK, "Custom Stack operations and boundary validation");

    LocationGraph graph;
    bool addOK = graph.addLocation(Location(1, "Hospital", 28.60, 77.20)) &&
                 graph.addLocation(Location(2, "Warehouse", 28.61, 77.21)) &&
                 graph.addLocation(Location(3, "Shelter", 28.62, 77.22)) &&
                 graph.addLocation(Location(4, "Clinic", 28.63, 77.23));
    check(addOK && graph.getLocationCount() == 4 && !graph.addLocation(Location(1, "Duplicate", 0, 0)) &&
          !graph.addLocation(Location(-1, "Bad", 0, 0)) && !graph.addLocation(Location(5, "", 0, 0)), "Location insertion and duplicate/invalid validation");

    bool edgeOK = graph.addConnection(1, 2, 8.0) && graph.addConnection(2, 3, 3.0) &&
                  graph.addConnection(1, 3, 20.0) && graph.addConnection(3, 4, 2.0);
    check(edgeOK && !graph.addConnection(1, 2, 4.0) && !graph.addConnection(1, 1, 1.0) &&
          !graph.addConnection(1, 4, 0.0) && !graph.addConnection(1, 4, numeric_limits<double>::infinity()) &&
          !graph.addConnection(1, 99, 1.0), "Undirected edge creation and connection validation");

    string bfs = captureTraversal(graph, true, 1);
    string dfs = captureTraversal(graph, false, 1);
    check(bfs.find("Hospital") != string::npos && bfs.find("Warehouse") != string::npos && bfs.find("Shelter") != string::npos &&
          dfs.find("Hospital") != string::npos && dfs.find("Warehouse") != string::npos && dfs.find("Clinic") != string::npos,
          "BFS and iterative DFS reach connected locations");

    ostringstream route; streambuf* old = cout.rdbuf(route.rdbuf()); graph.findShortestPath(1, 4); cout.rdbuf(old);
    check(route.str().find("Hospital -> Warehouse -> Shelter -> Clinic") != string::npos &&
          route.str().find("13") != string::npos, "Dijkstra selects the minimum-distance route");

    ostringstream same; old = cout.rdbuf(same.rdbuf()); graph.findShortestPath(2, 2); cout.rdbuf(old);
    check(same.str().find("Total Distance: 0") != string::npos, "Dijkstra handles source equal to destination");

    graph.addLocation(Location(5, "Remote", 28.70, 77.30));
    ostringstream disconnected; old = cout.rdbuf(disconnected.rdbuf()); graph.findShortestPath(1, 5); cout.rdbuf(old);
    check(disconnected.str().find("No path exists") != string::npos, "Disconnected destination is reported safely");

    Location* clinic = graph.findLocation(4);
    check(clinic != nullptr && clinic->setCoordinates(90.0, 180.0) && !clinic->setCoordinates(numeric_limits<double>::quiet_NaN(), 0.0) &&
          !clinic->setCoordinates(91.0, 0.0), "Coordinate range and NaN validation");

    check(graph.removeConnection(1, 3) && !graph.removeConnection(1, 3) && !graph.removeConnection(1, 99), "Connection removal and invalid removal");
    check(graph.removeLocation(2) && graph.getLocationCount() == 4 && graph.findLocation(2) == nullptr && !graph.removeLocation(2), "Location removal cleans connected edges");

    LocationGraph empty;
    ostringstream emptyOutput; old = cout.rdbuf(emptyOutput.rdbuf()); empty.breadthFirstSearch(1); empty.depthFirstSearch(1); empty.findShortestPath(1, 2); cout.rdbuf(old);
    check(empty.isEmpty() && empty.getLocationCount() == 0, "Empty graph boundary handling");

    LocationGraph inactive;
    inactive.addLocation(Location(10, "Active", 0, 0));
    Location inactiveLocation(11, "Inactive", 0, 0); inactiveLocation.setActive(false);
    check(!inactive.addLocation(inactiveLocation), "Inactive locations cannot enter the routing graph");

    LocationGraph routeGraph;
    routeGraph.addLocation(Location(20, "A", 0, 0)); routeGraph.addLocation(Location(21, "B", 0, 0));
    RoutePlanner planner(&routeGraph);
    ostringstream plannerOutput; old = cout.rdbuf(plannerOutput.rdbuf()); planner.planRoute(20, 21); planner.setGraph(nullptr); planner.planRoute(20, 21); cout.rdbuf(old);
    check(plannerOutput.str().find("No path exists") != string::npos && plannerOutput.str().find("not connected") != string::npos,
          "RoutePlanner handles disconnected routes and missing graph");

    cout << "\nPassed test groups: " << passed << "/13\n";
    if (passed == 13) cout << "\nALL LOCATION AND ROUTING TESTS PASSED SUCCESSFULLY!\n";
    else cout << "\nLOCATION AND ROUTING TESTS FAILED.\n";
    return passed == 13 ? 0 : 1;
}
