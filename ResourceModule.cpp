#include "../include/ResourceModule.h"
#include <iostream>
#include <new>
#include <limits>

using namespace std;

Resource::Resource()
{
    resourceId = 0;
    resourceName = "";
    category = "";
    description = "";
    unit = "";
    minimumStockLevel = 0;
    active = true;
}

Resource::Resource(int id, string name, string category, string description, string unit,
                   int minimumStockLevel)
{
    resourceId = id;
    resourceName = name;
    this->category = category;
    this->description = description;
    this->unit = unit;
    this->minimumStockLevel = minimumStockLevel;
    active = true;
}

int Resource::getResourceId() const
{
    return resourceId;
}

string Resource::getResourceName() const
{
    return resourceName;
}

string Resource::getCategory() const
{
    return category;
}

string Resource::getDescription() const
{
    return description;
}

string Resource::getUnit() const
{
    return unit;
}

int Resource::getMinimumStockLevel() const
{
    return minimumStockLevel;
}

bool Resource::isActive() const
{
    return active;
}

void Resource::setResourceName(string name)
{
    resourceName = name;
}

void Resource::setCategory(string category)
{
    this->category = category;
}

void Resource::setDescription(string description)
{
    this->description = description;
}

void Resource::setUnit(string unit)
{
    this->unit = unit;
}

void Resource::setMinimumStockLevel(int level)
{
    if (level >= 0)
        minimumStockLevel = level;
}

void Resource::setStatus(bool status)
{
    active = status;
}

ResourceHashTable::ResourceHashTable(int initialCapacity)
{
    if (initialCapacity < 1)
        initialCapacity = 10;

    capacity = initialCapacity;
    count = 0;
    loadFactor = 0.0;
    table = new Entry*[capacity]();
}

ResourceHashTable::~ResourceHashTable()
{
    clear();
    delete[] table;
}

int ResourceHashTable::hashFunction(int resourceId) const
{
    return resourceId % capacity;
}

bool ResourceHashTable::resizeTable()
{
    if (capacity > (numeric_limits<int>::max() - 1) / 2)
        return false;

    int newCapacity = capacity * 2 + 1;

    Entry** newTable = new (nothrow) Entry*[newCapacity]();

    if (newTable == nullptr)
        return false;

    for (int i = 0; i < capacity; i++)
    {
        Entry* current = table[i];

        while (current != nullptr)
        {
            Entry* nextNode = current->next;
            int newIndex = current->resourceId % newCapacity;

            current->next = newTable[newIndex];
            newTable[newIndex] = current;

            current = nextNode;
        }
    }

    delete[] table;
    table = newTable;
    capacity = newCapacity;
    loadFactor = static_cast<double>(count) / capacity;

    return true;
}

bool ResourceHashTable::insert(int resourceId, Resource* resource)
{
    if (resourceId <= 0 || resource == nullptr)
        return false;

    if (resource->getResourceId() != resourceId)
        return false;

    if (search(resourceId) != nullptr)
        return false;

    if (static_cast<double>(count + 1) / capacity > 0.75)
    {
        if (!resizeTable())
            return false;
    }

    int index = hashFunction(resourceId);

    Entry* newEntry = new (nothrow) Entry(resourceId, resource);

    if (newEntry == nullptr)
        return false;

    newEntry->next = table[index];
    table[index] = newEntry;

    count++;
    loadFactor = static_cast<double>(count) / capacity;

    return true;
}

Resource* ResourceHashTable::search(int resourceId) const
{
    if (resourceId <= 0)
        return nullptr;

    int index = hashFunction(resourceId);
    Entry* current = table[index];

    while (current != nullptr)
    {
        if (current->resourceId == resourceId)
            return current->resource;

        current = current->next;
    }

    return nullptr;
}

bool ResourceHashTable::remove(int resourceId)
{
    if (resourceId <= 0)
        return false;

    int index = hashFunction(resourceId);
    Entry* current = table[index];
    Entry* previous = nullptr;

    while (current != nullptr)
    {
        if (current->resourceId == resourceId)
        {
            if (previous == nullptr)
                table[index] = current->next;
            else
                previous->next = current->next;

            delete current;
            count--;
            loadFactor = static_cast<double>(count) / capacity;

            return true;
        }

        previous = current;
        current = current->next;
    }

    return false;
}

bool ResourceHashTable::contains(int resourceId) const
{
    return search(resourceId) != nullptr;
}

int ResourceHashTable::getCount() const
{
    return count;
}

int ResourceHashTable::getCapacity() const
{
    return capacity;
}

double ResourceHashTable::getLoadFactor() const
{
    return loadFactor;
}

void ResourceHashTable::displayTable() const
{
    cout << "\n========== RESOURCE HASH TABLE ==========\n";

    for (int i = 0; i < capacity; i++)
    {
        cout << "Index " << i << ": ";

        Entry* current = table[i];

        if (current == nullptr)
        {
            cout << "EMPTY";
        }
        else
        {
            while (current != nullptr)
            {
                cout << "[" << current->resourceId << "]";

                if (current->next != nullptr)
                    cout << " -> ";

                current = current->next;
            }
        }

        cout << endl;
    }

    cout << "\nTotal Entries: " << count;
    cout << "\nCapacity: " << capacity;
    cout << "\nLoad Factor: " << loadFactor;
    cout << "\n=========================================\n";
}

void ResourceHashTable::clear()
{
    for (int i = 0; i < capacity; i++)
    {
        Entry* current = table[i];

        while (current != nullptr)
        {
            Entry* temp = current;
            current = current->next;
            delete temp;
        }

        table[i] = nullptr;
    }

    count = 0;
    loadFactor = 0.0;
}

ResourceLinkedList::ResourceLinkedList()
{
    head = nullptr;
    tail = nullptr;
    count = 0;
}

void ResourceLinkedList::clear()
{
    resourceIndex.clear();

    Node* current = head;

    while (current != nullptr)
    {
        Node* temp = current;
        current = current->next;
        delete temp;
    }

    head = nullptr;
    tail = nullptr;
    count = 0;
}

ResourceLinkedList::~ResourceLinkedList()
{
    clear();
}

bool ResourceLinkedList::isEmpty() const
{
    return head == nullptr;
}

bool ResourceLinkedList::isDuplicate(int resourceId) const
{
    return resourceIndex.contains(resourceId);
}

int ResourceLinkedList::getCount() const
{
    return count;
}

bool ResourceLinkedList::addResource(const Resource& resource)
{
    int resourceId = resource.getResourceId();

    if (resourceId <= 0 || isDuplicate(resourceId) || !resource.isActive())
        return false;

    Node* newNode = new Node(resource);

    if (!resourceIndex.insert(resourceId, &newNode->data))
    {
        delete newNode;
        return false;
    }

    if (head == nullptr)
    {
        head = newNode;
        tail = newNode;
    }
    else
    {
        tail->next = newNode;
        tail = newNode;
    }

    count++;
    return true;
}

bool ResourceLinkedList::deleteResource(int resourceId)
{
    if (head == nullptr)
        return false;

    Node* current = head;
    Node* previous = nullptr;

    while (current != nullptr && current->data.getResourceId() != resourceId)
    {
        previous = current;
        current = current->next;
    }

    if (current == nullptr)
        return false;

    if (!resourceIndex.remove(resourceId))
        return false;

    if (previous == nullptr)
        head = current->next;
    else
        previous->next = current->next;

    if (current == tail)
        tail = previous;

    delete current;
    count--;

    return true;
}

Resource* ResourceLinkedList::searchResource(int resourceId)
{
    return resourceIndex.search(resourceId);
}

const Resource* ResourceLinkedList::searchResource(int resourceId) const
{
    return resourceIndex.search(resourceId);
}

bool ResourceLinkedList::updateResource(int resourceId, string name, string category, string description,
                                        string unit, int minimumStockLevel)
{
    Resource* resource = searchResource(resourceId);

    if (resource == nullptr || minimumStockLevel < 0)
        return false;

    resource->setResourceName(name);
    resource->setCategory(category);
    resource->setDescription(description);
    resource->setUnit(unit);
    resource->setMinimumStockLevel(minimumStockLevel);

    return true;
}

void ResourceLinkedList::displayAllResources() const
{
    if (isEmpty())
    {
        cout << "\nNo resources available.\n";
        return;
    }

    Node* current = head;

    while (current != nullptr)
    {
        cout << "\n-----------------------------";
        cout << "\nResource ID: " << current->data.getResourceId();
        cout << "\nName: " << current->data.getResourceName();
        cout << "\nCategory: " << current->data.getCategory();
        cout << "\nDescription: " << current->data.getDescription();
        cout << "\nUnit: " << current->data.getUnit();
        cout << "\nMinimum Stock Level: " << current->data.getMinimumStockLevel();
        cout << "\nStatus: " << (current->data.isActive() ? "Active" : "Inactive");
        cout << "\n-----------------------------\n";

        current = current->next;
    }
}

namespace
{
string toLowerCopy(const string& text)
{
    string lowered = text;
    for (size_t i = 0; i < lowered.size(); ++i)
    {
        if (lowered[i] >= 'A' && lowered[i] <= 'Z')
            lowered[i] = static_cast<char>(lowered[i] - 'A' + 'a');
    }
    return lowered;
}
}

int ResourceLinkedList::displayMatchingName(const string& fragment) const
{
    if (fragment.empty())
        return 0;

    string needle = toLowerCopy(fragment);
    int matches = 0;

    for (Node* current = head; current != nullptr; current = current->next)
    {
        if (toLowerCopy(current->data.getResourceName()).find(needle) == string::npos)
            continue;

        cout << "ID: " << current->data.getResourceId() << " | " << current->data.getResourceName() << " | "
             << current->data.getCategory() << " | Unit: " << current->data.getUnit()
             << " | Min stock: " << current->data.getMinimumStockLevel() << "\n";
        ++matches;
    }

    return matches;
}

ResourceRepository::ResourceRepository(DatabaseConnection& db) : database(db)
{
}

bool ResourceRepository::saveResource(const Resource& resource)
{
    if (!database.isConnected() || resource.getResourceId() <= 0 || !resource.isActive() ||
        resource.getMinimumStockLevel() < 0)
        return false;

    const char* query = "INSERT INTO resources "
                        "(resource_id, resource_name, category, description, unit, minimum_stock_level) "
                        "VALUES (?, ?, ?, ?, ?, ?)";

    MYSQL_STMT* statement = database.prepareStatement(query);

    if (statement == nullptr)
        return false;

    int id = resource.getResourceId();
    int minimumStock = resource.getMinimumStockLevel();

    std::string name = resource.getResourceName();
    std::string category = resource.getCategory();
    std::string description = resource.getDescription();
    std::string unit = resource.getUnit();

    unsigned long nameLength = static_cast<unsigned long>(name.length());
    unsigned long categoryLength = static_cast<unsigned long>(category.length());
    unsigned long descriptionLength = static_cast<unsigned long>(description.length());
    unsigned long unitLength = static_cast<unsigned long>(unit.length());

    MYSQL_BIND parameters[6]{};

    bindInt(parameters[0], id);

    bindString(parameters[1], name, nameLength);

    bindString(parameters[2], category, categoryLength);

    bindString(parameters[3], description, descriptionLength);

    bindString(parameters[4], unit, unitLength);

    bindInt(parameters[5], minimumStock);

    bool success = mysql_stmt_bind_param(statement, parameters) == 0 && mysql_stmt_execute(statement) == 0;

    mysql_stmt_close(statement);

    return success;
}

bool ResourceRepository::loadActiveResources(ResourceLinkedList& resourceList)
{
    if (!database.isConnected())
        return false;

    const char* query = "SELECT resource_id, resource_name, category, description, "
                        "unit, minimum_stock_level "
                        "FROM resources WHERE is_active = TRUE ORDER BY resource_id";

    MYSQL* connection = database.getConnection();

    if (mysql_query(connection, query) != 0)
        return false;

    MYSQL_RES* result = mysql_store_result(connection);

    if (result == nullptr)
        return false;

    resourceList.clear();

    MYSQL_ROW row;
    bool success = true;

    while ((row = mysql_fetch_row(result)) != nullptr)
    {
        int id = std::stoi(row[0]);
        std::string name = row[1] ? row[1] : "";
        std::string category = row[2] ? row[2] : "";
        std::string description = row[3] ? row[3] : "";
        std::string unit = row[4] ? row[4] : "";
        int minimumStock = std::stoi(row[5]);

        Resource resource(id, name, category, description, unit, minimumStock);

        if (!resourceList.addResource(resource))
        {
            success = false;
            break;
        }
    }

    mysql_free_result(result);

    if (!success)
        resourceList.clear();

    return success;
}

bool ResourceRepository::updateResource(const Resource& resource)
{
    if (!database.isConnected() || resource.getResourceId() <= 0 || resource.getMinimumStockLevel() < 0)
        return false;

    const char* query = "UPDATE resources SET resource_name = ?, category = ?, "
                        "description = ?, unit = ?, minimum_stock_level = ? "
                        "WHERE resource_id = ?";

    MYSQL_STMT* statement = database.prepareStatement(query);

    if (statement == nullptr)
        return false;

    std::string name = resource.getResourceName();
    std::string category = resource.getCategory();
    std::string description = resource.getDescription();
    std::string unit = resource.getUnit();

    int minimumStock = resource.getMinimumStockLevel();
    int id = resource.getResourceId();

    unsigned long nameLength = static_cast<unsigned long>(name.length());
    unsigned long categoryLength = static_cast<unsigned long>(category.length());
    unsigned long descriptionLength = static_cast<unsigned long>(description.length());
    unsigned long unitLength = static_cast<unsigned long>(unit.length());

    MYSQL_BIND parameters[6]{};

    bindString(parameters[0], name, nameLength);

    bindString(parameters[1], category, categoryLength);

    bindString(parameters[2], description, descriptionLength);

    bindString(parameters[3], unit, unitLength);

    bindInt(parameters[4], minimumStock);

    bindInt(parameters[5], id);

    bool success = mysql_stmt_bind_param(statement, parameters) == 0 && mysql_stmt_execute(statement) == 0;

    mysql_stmt_close(statement);

    return success;
}

bool ResourceRepository::deactivateResource(int resourceId)
{
    if (!database.isConnected() || resourceId <= 0)
        return false;

    const char* query = "UPDATE resources SET is_active = FALSE WHERE resource_id = ?";

    MYSQL_STMT* statement = database.prepareStatement(query);

    if (statement == nullptr)
        return false;

    MYSQL_BIND parameter{};
    bindInt(parameter, resourceId);

    bool success = mysql_stmt_bind_param(statement, &parameter) == 0 && mysql_stmt_execute(statement) == 0 &&
                   mysql_stmt_affected_rows(statement) == 1;

    mysql_stmt_close(statement);

    return success;
}

bool ResourceRepository::resourceExists(int resourceId)
{
    if (!database.isConnected() || resourceId <= 0)
        return false;

    const char* query = "SELECT resource_id FROM resources WHERE resource_id = ? LIMIT 1";

    MYSQL_STMT* statement = database.prepareStatement(query);

    if (statement == nullptr)
        return false;

    MYSQL_BIND parameter{};
    bindInt(parameter, resourceId);

    bool success = mysql_stmt_bind_param(statement, &parameter) == 0 && mysql_stmt_execute(statement) == 0 &&
                   mysql_stmt_store_result(statement) == 0;

    bool exists = false;

    if (success)
        exists = mysql_stmt_num_rows(statement) > 0;

    mysql_stmt_close(statement);

    return exists;
}

bool ResourceRepository::reactivateResource(int resourceId)
{
    if (!database.isConnected() || resourceId <= 0)
        return false;

    MYSQL_STMT* statement = database.prepareStatement(
            "UPDATE resources SET is_active = TRUE WHERE resource_id = ? AND is_active = FALSE");

    if (statement == nullptr)
        return false;

    MYSQL_BIND parameter{};
    bindInt(parameter, resourceId);

    bool success = mysql_stmt_bind_param(statement, &parameter) == 0 && mysql_stmt_execute(statement) == 0 &&
                   mysql_stmt_affected_rows(statement) == 1;

    mysql_stmt_close(statement);
    return success;
}

bool ResourceRepository::isResourceActive(int resourceId)
{
    if (!database.isConnected() || resourceId <= 0)
        return false;

    MYSQL_STMT* statement = database.prepareStatement(
            "SELECT resource_id FROM resources WHERE resource_id = ? AND is_active = TRUE");

    if (statement == nullptr)
        return false;

    MYSQL_BIND parameter{};
    bindInt(parameter, resourceId);

    bool ok = mysql_stmt_bind_param(statement, &parameter) == 0 && mysql_stmt_execute(statement) == 0 &&
              mysql_stmt_store_result(statement) == 0;

    bool active = ok && mysql_stmt_num_rows(statement) > 0;
    mysql_stmt_close(statement);
    return active;
}
