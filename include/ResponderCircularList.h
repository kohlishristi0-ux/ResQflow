#ifndef RESPONDER_CIRCULAR_LIST_H
#define RESPONDER_CIRCULAR_LIST_H

class ResponderCircularList
{
private:
    struct Node
    {
        int responderId;
        Node* next;
        explicit Node(int id) : responderId(id), next(nullptr) {}
    };
    Node* tail;
    Node* cursor;
    int count;
public:
    ResponderCircularList();
    ~ResponderCircularList();
    ResponderCircularList(const ResponderCircularList&) = delete;
    ResponderCircularList& operator=(const ResponderCircularList&) = delete;
    bool addResponderId(int responderId);
    bool removeResponderId(int responderId);
    bool getNextResponderId(int& responderId);
    bool contains(int responderId) const;
    int getCount() const;
    bool isEmpty() const;
};

#endif
