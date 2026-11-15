#include "../include/RoutePlanner.h"
#include <iostream>
using namespace std;
RoutePlanner::RoutePlanner(LocationGraph* locationGraph) : graph(locationGraph) {}
void RoutePlanner::setGraph(LocationGraph* locationGraph) { graph = locationGraph; }
void RoutePlanner::planRoute(int sourceId, int destinationId) const
{
    if (!graph) { cout << "\nLocation graph is not connected.\n"; return; }
    Location* source = graph->findLocation(sourceId); Location* destination = graph->findLocation(destinationId);
    if (!source || !destination) { cout << "\nInvalid source or destination location.\n"; return; }
    cout << "\n========== ROUTE PLANNING ==========\nSource: " << source->getLocationName()
         << "\nDestination: " << destination->getLocationName() << "\n";
    graph->findShortestPath(sourceId, destinationId);
    cout << "====================================\n";
}
