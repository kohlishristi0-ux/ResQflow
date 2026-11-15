#ifndef RESOURCE_REPOSITORY_H
#define RESOURCE_REPOSITORY_H

#include "DatabaseConnection.h"
#include "../Resource.h"
#include "../ResourceLinkedList.h"

class ResourceRepository
{
private:
    DatabaseConnection& database;

public:
    explicit ResourceRepository(DatabaseConnection& db);

    bool saveResource(const Resource& resource);

    bool loadActiveResources(ResourceLinkedList& resourceList);

    bool updateResource(const Resource& resource);

    bool deactivateResource(int resourceId);

    bool resourceExists(int resourceId);

    // Re-enables a previously deactivated resource row (keeps its ID and history).
    bool reactivateResource(int resourceId);
    bool isResourceActive(int resourceId);
};

#endif