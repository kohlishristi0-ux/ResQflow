#ifndef ALLOCATION_REPOSITORY_H
#define ALLOCATION_REPOSITORY_H

#include "DatabaseConnection.h"
#include "../Allocation.h"
#include "../AllocationManager.h"

class AllocationRepository
{
private:
    DatabaseConnection& database;

public:
    explicit AllocationRepository(DatabaseConnection& db);

    bool saveAllocation(const Allocation& allocation);

    bool loadAllocations(AllocationManager& allocationManager);

    bool updateAllocation(const Allocation& allocation);

    bool allocationExists(int allocationId);

    // Recomputes emergency_requirements for (emergency, resource) from its non-cancelled allocations.
    bool syncRequirement(int emergencyId, int resourceId);
    // Prints required/fulfilled quantities per resource for one emergency.
    bool displayRequirements(int emergencyId);
};

#endif