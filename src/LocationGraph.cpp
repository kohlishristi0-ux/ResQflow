#include "../include/LocationGraph.h"
#include "../include/Stack.h"
#include <iostream>
#include <limits>
#include <cmath>
#include <new>
using namespace std;
LocationGraph::LocationGraph() : head(nullptr), tail(nullptr), vertexCount(0) {}
LocationGraph::~LocationGraph()
{
    Vertex* current = head;
    while (current != nullptr)
    {
        Vertex* next = current->next;
        clearEdges(current->adjacencyList);
        delete current;
        current = next;
    }
}
void LocationGraph::clearEdges(Edge* edge)
{
    while (edge != nullptr) { Edge* next = edge->next; delete edge; edge = next; }
}
LocationGraph::Vertex* LocationGraph::findVertex(int id) const
{
    Vertex* current = head;
    while (current != nullptr) { if (current->data.getLocationId() == id) return current; current = current->next; }
    return nullptr;
}
bool LocationGraph::hasEdge(int sourceId, int destinationId) const
{
    Vertex* source = findVertex(sourceId);
    if (!source) return false;
    Edge* edge = source->adjacencyList;
    while (edge) { if (edge->destinationId == destinationId) return true; edge = edge->next; }
    return false;
}
bool LocationGraph::addLocation(const Location& location)
{
    if (location.getLocationId() <= 0 || location.getLocationName().empty() || !location.isActive() ||
        !isfinite(location.getLatitude()) || !isfinite(location.getLongitude()) ||
        location.getLatitude() < -90.0 || location.getLatitude() > 90.0 ||
        location.getLongitude() < -180.0 || location.getLongitude() > 180.0 || findVertex(location.getLocationId())) return false;
    Vertex* node = new (nothrow) Vertex(location);
    if (!node) return false;
    if (!head) head = tail = node;
    else { tail->next = node; tail = node; }
    ++vertexCount;
    return true;
}
bool LocationGraph::removeLocation(int id)
{
    Vertex* current = head; Vertex* previous = nullptr;
    while (current && current->data.getLocationId() != id) { previous = current; current = current->next; }
    if (!current) return false;
    if (previous) previous->next = current->next; else head = current->next;
    if (tail == current) tail = previous;
    clearEdges(current->adjacencyList); delete current;
    for (Vertex* vertex = head; vertex; vertex = vertex->next)
    {
        Edge* edge = vertex->adjacencyList; Edge* before = nullptr;
        while (edge)
        {
            if (edge->destinationId == id)
            {
                Edge* doomed = edge; edge = edge->next;
                if (before) before->next = edge; else vertex->adjacencyList = edge;
                delete doomed;
            }
            else { before = edge; edge = edge->next; }
        }
    }
    --vertexCount;
    if (!head) tail = nullptr;
    return true;
}
Location* LocationGraph::findLocation(int id)
{
    Vertex* vertex = findVertex(id);
    return vertex ? &vertex->data : nullptr;
}
bool LocationGraph::addConnection(int sourceId, int destinationId, double distance)
{
    Vertex* source = findVertex(sourceId); Vertex* destination = findVertex(destinationId);
    if (sourceId == destinationId || !isfinite(distance) || distance <= 0.0 || !source || !destination ||
        !source->data.isActive() || !destination->data.isActive() || hasEdge(sourceId, destinationId)) return false;
    Edge* first = new (nothrow) Edge(destinationId, distance);
    if (!first) return false;
    Edge* second = new (nothrow) Edge(sourceId, distance);
    if (!second) { delete first; return false; }
    first->next = source->adjacencyList; source->adjacencyList = first;
    second->next = destination->adjacencyList; destination->adjacencyList = second;
    return true;
}
bool LocationGraph::removeConnection(int sourceId, int destinationId)
{
    if (!hasEdge(sourceId, destinationId) || !hasEdge(destinationId, sourceId)) return false;
    int ids[2] = {sourceId, destinationId};
    for (int i = 0; i < 2; ++i)
    {
        Vertex* vertex = findVertex(ids[i]); Edge* edge = vertex->adjacencyList; Edge* previous = nullptr;
        while (edge && edge->destinationId != ids[1-i]) { previous = edge; edge = edge->next; }
        if (!edge) return false;
        if (previous) previous->next = edge->next; else vertex->adjacencyList = edge->next;
        delete edge;
    }
    return true;
}
void LocationGraph::displayGraph() const
{
    if (!head) { cout << "\nGraph is empty.\n"; return; }
    for (Vertex* vertex = head; vertex; vertex = vertex->next)
    {
        cout << "\n" << vertex->data.getLocationName() << " (" << vertex->data.getLocationId() << ") -> ";
        Edge* edge = vertex->adjacencyList;
        if (!edge) cout << "No connections";
        while (edge)
        {
            Vertex* destination = findVertex(edge->destinationId);
            if (destination) cout << destination->data.getLocationName() << " [" << edge->distance << " km]";
            if (edge->next) cout << " -> ";
            edge = edge->next;
        }
        cout << "\n";
    }
}
void LocationGraph::breadthFirstSearch(int startId) const
{
    if (!findVertex(startId)) { cout << "\nStarting location not found.\n"; return; }
    int* ids = new (nothrow) int[vertexCount]; bool* visited = new (nothrow) bool[vertexCount];
    int* queue = new (nothrow) int[vertexCount];
    if (!ids || !visited || !queue) { delete[] ids; delete[] visited; delete[] queue; cout << "\nBFS memory allocation failed.\n"; return; }
    int i = 0; for (Vertex* v = head; v; v = v->next) { ids[i] = v->data.getLocationId(); visited[i++] = false; }
    int startIndex = 0; while (ids[startIndex] != startId) ++startIndex;
    int front = 0, rear = 0; queue[rear++] = startId; visited[startIndex] = true;
    cout << "\nBFS Traversal: ";
    while (front < rear)
    {
        int id = queue[front++]; Vertex* vertex = findVertex(id);
        if (!vertex) continue;
        cout << vertex->data.getLocationName() << " ";
        for (Edge* edge = vertex->adjacencyList; edge; edge = edge->next)
        {
            int index = 0; while (index < vertexCount && ids[index] != edge->destinationId) ++index;
            Vertex* target = findVertex(edge->destinationId);
            if (index < vertexCount && target && target->data.isActive() && !visited[index])
            { visited[index] = true; queue[rear++] = edge->destinationId; }
        }
    }
    cout << "\n"; delete[] ids; delete[] visited; delete[] queue;
}
void LocationGraph::depthFirstSearch(int startId) const
{
    if (!findVertex(startId)) { cout << "\nStarting location not found.\n"; return; }
    int* ids = new (nothrow) int[vertexCount]; bool* visited = new (nothrow) bool[vertexCount];
    if (!ids || !visited) { delete[] ids; delete[] visited; cout << "\nDFS memory allocation failed.\n"; return; }
    int i = 0; for (Vertex* v = head; v; v = v->next) { ids[i] = v->data.getLocationId(); visited[i++] = false; }
    Stack stack(vertexCount);
    int startIndex = 0; while (ids[startIndex] != startId) ++startIndex;
    if (!stack.push(startId)) { delete[] ids; delete[] visited; cout << "\nDFS stack allocation failed.\n"; return; }
    visited[startIndex] = true;
    cout << "\nDFS Traversal: ";
    int id;
    while (stack.pop(id))
    {
        Vertex* vertex = findVertex(id); if (!vertex) continue;
        cout << vertex->data.getLocationName() << " ";
        for (Edge* edge = vertex->adjacencyList; edge; edge = edge->next)
        {
            int index = 0; while (index < vertexCount && ids[index] != edge->destinationId) ++index;
            Vertex* target = findVertex(edge->destinationId);
            if (index < vertexCount && target && target->data.isActive() && !visited[index])
            { visited[index] = true; if (!stack.push(edge->destinationId)) { cout << "\nDFS stack capacity error.\n"; break; } }
        }
    }
    cout << "\n"; delete[] ids; delete[] visited;
}
void LocationGraph::findShortestPath(int sourceId, int destinationId) const
{
    if (!findVertex(sourceId) || !findVertex(destinationId)) { cout << "\nSource or destination location not found.\n"; return; }
    Vertex* sourceVertex = findVertex(sourceId); Vertex* destinationVertex = findVertex(destinationId);
    if (!sourceVertex->data.isActive() || !destinationVertex->data.isActive()) { cout << "\nSource or destination location is inactive.\n"; return; }
    int n = vertexCount;
    double* distances = new (nothrow) double[n]; bool* visited = new (nothrow) bool[n];
    int* previous = new (nothrow) int[n]; int* ids = new (nothrow) int[n];
    if (!distances || !visited || !previous || !ids)
    { delete[] distances; delete[] visited; delete[] previous; delete[] ids; cout << "\nRoute calculation memory allocation failed.\n"; return; }
    int index = 0; for (Vertex* v = head; v; v = v->next)
    { ids[index] = v->data.getLocationId(); distances[index] = numeric_limits<double>::max(); visited[index] = false; previous[index] = -1; ++index; }
    int sourceIndex = 0; while (ids[sourceIndex] != sourceId) ++sourceIndex;
    distances[sourceIndex] = 0.0;
    for (int count = 0; count < n; ++count)
    {
        int minIndex = -1;
        for (int i = 0; i < n; ++i)
            if (!visited[i] && (minIndex == -1 || distances[i] < distances[minIndex])) minIndex = i;
        if (minIndex == -1 || distances[minIndex] == numeric_limits<double>::max()) break;
        visited[minIndex] = true;
        Vertex* vertex = findVertex(ids[minIndex]);
        for (Edge* edge = vertex->adjacencyList; edge; edge = edge->next)
        {
            int targetIndex = 0; while (targetIndex < n && ids[targetIndex] != edge->destinationId) ++targetIndex;
            Vertex* target = findVertex(edge->destinationId);
            if (targetIndex < n && target && target->data.isActive() && !visited[targetIndex] &&
                distances[minIndex] <= numeric_limits<double>::max() - edge->distance)
            {
                double candidate = distances[minIndex] + edge->distance;
                if (candidate < distances[targetIndex]) { distances[targetIndex] = candidate; previous[targetIndex] = minIndex; }
            }
        }
    }
    int destinationIndex = 0; while (ids[destinationIndex] != destinationId) ++destinationIndex;
    if (distances[destinationIndex] == numeric_limits<double>::max()) cout << "\nNo path exists between these locations.\n";
    else
    {
        int* path = new (nothrow) int[n];
        if (!path) cout << "\nRoute path memory allocation failed.\n";
        else
        {
            int pathSize = 0, current = destinationIndex;
            while (current != -1 && pathSize < n) { path[pathSize++] = current; current = previous[current]; }
            cout << "\nShortest Path: ";
            for (int i = pathSize - 1; i >= 0; --i)
            { Vertex* location = findVertex(ids[path[i]]); cout << (location ? location->data.getLocationName() : "Unknown"); if (i > 0) cout << " -> "; }
            cout << "\nTotal Distance: " << distances[destinationIndex] << " km\n";
            delete[] path;
        }
    }
    delete[] distances; delete[] visited; delete[] previous; delete[] ids;
}
int LocationGraph::getLocationCount() const { return vertexCount; }
bool LocationGraph::isEmpty() const { return head == nullptr; }

bool LocationGraph::hasConnection(int sourceId, int destinationId) const
{
    return hasEdge(sourceId, destinationId);
}
