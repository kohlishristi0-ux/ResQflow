#ifndef RESQFLOW_STACK_H
#define RESQFLOW_STACK_H
class Stack
{
private:
    int* data;
    int capacity;
    int topIndex;
public:
    explicit Stack(int maxSize);
    ~Stack();
    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;
    bool isEmpty() const;
    bool isFull() const;
    bool push(int value);
    bool pop(int& value);
    bool peek(int& value) const;
};
#endif
