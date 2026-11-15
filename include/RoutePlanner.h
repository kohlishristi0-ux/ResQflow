#ifndef ROUTE_PLANNER_H
#define ROUTE_PLANNER_H
#include "LocationGraph.h"
class RoutePlanner
{
private:
    LocationGraph* graph;
public:
    explicit RoutePlanner(LocationGraph* locationGraph);
    void setGraph(LocationGraph* locationGraph);
    void planRoute(int sourceId, int destinationId) const;
};
#endif
