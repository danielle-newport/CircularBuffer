#include "headers/CircularBuffer.h"
#include <cassert>

using namespace std;

int main()
{
    CircularBuffer<int> cb(3);

    assert(cb.empty());
    assert(cb.capacity() == 3);

    cb.push(1);
    cb.push(2);
    cb.push(3);
    assert(cb.full());
    assert(cb.size() == 3);

    assert(cb.pop() == 1); // FIFO order
    assert(cb.size() == 2);

    cb.push(4); // wraps tail
    assert(cb.front() == 2);
    assert(cb.back() == 4);

    cb.push(5); // overwrite mode: evicts 2
    assert(cb.front() == 3);
    assert(cb.pop() == 3);

    cb.clear();
    assert(cb.empty());
    assert(cb.capacity() == 3); // capacity preserved
    cout << "oh hell yeah you passed the tests you go girl" << endl;
    return 0;
}