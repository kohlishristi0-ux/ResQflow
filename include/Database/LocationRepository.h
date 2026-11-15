#ifndef LOCATION_REPOSITORY_H
#define LOCATION_REPOSITORY_H

#include "DatabaseConnection.h"
#include "../Location.h"
#include "../LocationGraph.h"

class LocationRepository
{
private:
    DatabaseConnection& database;

public:
    explicit LocationRepository(DatabaseConnection& db);

    bool saveLocation(const Location& location);

    bool loadLocations(LocationGraph& graph);

    bool updateLocation(const Location& location);

    bool deactivateLocation(int locationId);

    bool locationExists(int locationId);

    bool saveConnection(int sourceId, int destinationId, double distance);

    bool loadConnections(LocationGraph& graph);

    bool connectionExists(int sourceId, int destinationId);

    bool deactivateConnection(int sourceId, int destinationId);
};

#endif