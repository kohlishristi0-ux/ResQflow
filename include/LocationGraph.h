#ifndef LOCATION_GRAPH_H
#define LOCATION_GRAPH_H
#include "Location.h"
class LocationGraph
{
private:
    struct Edge
    {
        int destinationId;
        double distance;
        Edge* next;
        Edge(int id, double dist) : destinationId(id), distance(dist), next(nullptr) {}
    };
    struct Vertex
    {
        Location data;
        Edge* adjacencyList;
        Vertex* next;
        explicit Vertex(const Location& location) : data(location), adjacencyList(nullptr), next(nullptr) {}
    };
    Vertex* head;
    Vertex* tail;
    int vertexCount;
    Vertex* findVertex(int locationId) const;
    bool hasEdge(int sourceId, int destinationId) const;
    void clearEdges(Edge* edge);
public:
    LocationGraph();
    ~LocationGraph();
    LocationGraph(const LocationGraph&) = delete;
    LocationGraph& operator=(const LocationGraph&) = delete;
    bool addLocation(const Location& location);
    bool removeLocation(int locationId);
    Location* findLocation(int locationId);
    bool addConnection(int sourceId, int destinationId, double distance);
    bool removeConnection(int sourceId, int destinationId);
    bool hasConnection(int sourceId, int destinationId) const;
    void displayGraph() const;
    void breadthFirstSearch(int startLocationId) const;
    void depthFirstSearch(int startLocationId) const;
    void findShortestPath(int sourceId, int destinationId) const;
    int getLocationCount() const;
    bool isEmpty() const;
};
#endif
