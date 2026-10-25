#pragma once
#include <vector>
#include <stdexcept>
#include <iterator>
#include <iostream>

using namespace std;

template <typename T>
class CircularBuffer
{
public:
    CircularBuffer(int capacity)
    {
        try
        {
            if (capacity > 0)
            {
                _capacity = size_t(capacity);
                _circularBuffer = vector<T>(capacity); // TODO: does it need to be dynamically allocated i.e. new vector?
                _readPtr = _circularBuffer.begin();
                _writePtr = _circularBuffer.begin();
            }
            else
            {
                throw invalid_argument("Capacity must be greater than 0");
            }
        }
        catch (const invalid_argument &e)
        {
            cerr << "Error: " << e.what() << endl;
        }
    }
    ~CircularBuffer()
    {
        // TODO does anything need to be deleted/cleaned up?
    }
    size_t capacity() const noexcept
    {
        return _capacity;
    }
    size_t size() const noexcept
    {
        return _size;
    }
    bool empty() const noexcept
    {
        if (_size < 1)
            return true;
        return false;
    }
    bool full() const noexcept
    {
        if (_size == _capacity)
            return true;
        return false;
    }
    void push(const T &value)
    {
        _pushImpl(value);
    }
    void push(T &&pageName)
    {
        _pushImpl(pageName);
    }
    T pop()
    {
        // only return something if values have been inputted ie size > 0
        try
        {
            if (_size > 0)
            {
                T tmp = *_readPtr;
                _readPtr++;
                if (_readPtr == _circularBuffer.end())
                    _readPtr = _circularBuffer.begin();
                _size--;
                return tmp;
            }
            else
            {
                throw underflow_error("Cannot pop from empty list");
            }
        }
        catch (const underflow_error &e)
        {
            cerr << "Error: " << e.what() << endl;
        }
    }
    void clear() noexcept
    {
        _size = 0;
        _writePtr = _readPtr; // TODO: is this an issue, will they move indpendently? do i have to use & or something
    };
    T &front()
    {
        try
        {

            if (_size > 0)
            {
                return *_readPtr;
            }
            else
            {
                throw underflow_error("Cannot call on empty buffer");
            }
        }
        catch (const underflow_error &e)
        {
            cerr << "Error: " << e.what() << endl;
        }
    }
    const T &front() const
    {
        try
        {

            if (_size > 0)
            {
                return *_readPtr;
            }
            else
            {
                throw underflow_error("Cannot call on empty buffer");
            }
        }
        catch (const underflow_error &e)
        {
            cerr << "Error: " << e.what() << endl;
        }
    }
    T &back()
    {
        // TODO: need to account for wrap around!
        try
        {

            if (_size > 0)
            {
                // check if we need to loop back
                if (_writePtr == _circularBuffer.begin())
                    return *prev(_circularBuffer.end());
                else
                    return *prev(_writePtr);
            }
            else
            {
                throw underflow_error("Cannot call on empty buffer");
            }
        }
        catch (const underflow_error &e)
        {
            cerr << "Error: " << e.what() << endl;
        }
    }
    const T &back() const
    {
        try
        {

            if (_size > 0)
            {
                // check if we need to loop back
                if (_writePtr == _circularBuffer.begin())
                    return *prev(_circularBuffer.end());
                else
                    return *prev(_writePtr);
            }
            else
            {
                throw underflow_error("Cannot call on empty buffer");
            }
        }
        catch (const underflow_error &e)
        {
            cerr << "Error: " << e.what() << endl;
        }
    }

private:
    void _pushImpl(T value)
    {
        *_writePtr = value;
        _writePtr++;
        // account for looping to the front, we only inc by one so we don't have to do modulo math
        if (_writePtr == _circularBuffer.end())
            _writePtr = _circularBuffer.begin();
        // only inc size if it's below capacity, otherwise if we continue adding at capacity the size will stay the same
        // with our current overwrite behavior
        if (_size < _capacity)
            _size++;
        // if we are overwriting previously valid data, we need to inc _readPtr as well
        else
        {
            _readPtr++;
        }
    }

    size_t _capacity;
    size_t _size = 0;
    // T array[_CAPACITY] myArray;
    vector<T> _circularBuffer;
    typename vector<T>::iterator _readPtr;
    typename vector<T>::iterator _writePtr;
};
