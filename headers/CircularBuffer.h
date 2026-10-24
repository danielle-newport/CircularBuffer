#pragma once
#include <vector>

using namespace std;

template <typename T>
class CircularBuffer
{
public:
    CircularBuffer(int capacity)
    {
        _CAPACITY = capacity;
        _circularBuffer = vector<T>(size = capacity); // TODO: does it need to be dynamically allocated i.e. new vector?
    }
    ~CircularBuffer()
    {
        // TODO does anything need to be deleted/cleaned up?
    }
    size_t capacity() const noexcept
    {
        return _CAPACITY;
    }
    size_t size() const noexcept
    {
        return _size;
    }
    bool empty() const noexcept
    {
        _size < 1 ? return True : return False;
    }
    bool full() const noexcept
    {
        _size == _CAPACITY ? return True : return False;
    }
    void push(const T &value)
    {
        _pushImpl(value);
    }
    void push(T &&pageName)
    {
        _pushImpl(move(pageName));
    }
    T pop()
    {
        // only return something if values have been inputted ie size > 0
        if (_size > 0)
        {
            T tmp = *_readPtr;
            _readPtr++;
            if (_readPtr == _circularBuffer.end())
                _readPtr = _circularBuffer.begin();
            _size--;
            return tmp;
        }
        return NULL;
    }
    T &front()
    {
        return _readPtr;
    }
    const T &front() const
    {
        return _readPtr;
    }
    T &back()
    {
        return prev(_writePtr);
    }
    const T &back() const
    {
        return prev(_writePtr);
    }

private:
    void _pushImpl(T value)
    {
        _circularBuffer.insert(_writePtr, value);
        _writePtr++;
        // account for looping to the front, we only inc by one so we don't have to do modulo math
        if (_writePtr == _circularBuffer.end())
            _writePtr = _circularBuffer.begin();
        // only inc size if it's below capacity, otherwise if we continue adding at capacity the size will stay the same
        // with our current overwrite behavior
        if (_size < _CAPACITY)
            _size++;
    }

    vector<T>::iterator _readPtr;
    vector<T>::iterator _writePtr;
    const int _CAPACITY;
    int _size = 0;
    // T array[_CAPACITY] myArray;
    vector<T> _circularBuffer;
};
