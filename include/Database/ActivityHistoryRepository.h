#ifndef ACTIVITY_HISTORY_REPOSITORY_H
#define ACTIVITY_HISTORY_REPOSITORY_H

#include "DatabaseConnection.h"
#include "../ActivityHistory.h"

class ActivityHistoryRepository
{
private:
    DatabaseConnection& database;

public:
    explicit ActivityHistoryRepository(DatabaseConnection& db);

    bool saveActivity(const ActivityRecord& activity);

    bool loadActivityHistory(ActivityHistory& history);

    bool activityExists(int activityId);
};

#endif
