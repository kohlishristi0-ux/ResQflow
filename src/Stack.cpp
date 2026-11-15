#include "../include/Stack.h"
#include <new>
Stack::Stack(int maxSize) : data(nullptr), capacity(maxSize > 0 ? maxSize : 0), topIndex(-1)
{
    if (capacity > 0) data = new (std::nothrow) int[capacity];
    if (data == nullptr) capacity = 0;
}
Stack::~Stack() { delete[] data; }
bool Stack::isEmpty() const { return topIndex < 0; }
bool Stack::isFull() const { return capacity == 0 || topIndex + 1 >= capacity; }
bool Stack::push(int value)
{
    if (isFull()) return false;
    data[++topIndex] = value; return true;
}
bool Stack::pop(int& value)
{
    if (isEmpty()) return false;
    value = data[topIndex--]; return true;
}
bool Stack::peek(int& value) const
{
    if (isEmpty()) return false;
    value = data[topIndex]; return true;
}