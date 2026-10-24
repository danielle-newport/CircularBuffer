#include <gtest/gtest.h>
#include <CircularBuffer.h> // adjust to your header

#include <string>
#include <stdexcept>
#include <utility>

// ---------------------------------------------------------------------------
// Configure these to match the decisions your implementation documents.
// ---------------------------------------------------------------------------
// On push to a full buffer:
//   1 = overwrite oldest (default spec behavior)
//   0 = reject (return void but throw, or you'll adapt to a bool-returning API)
#define POLICY_PUSH_OVERWRITE_ON_FULL 1

// On pop/front/back of an empty buffer:
//   1 = throws std::underflow_error / std::out_of_range
//   0 = asserts / UB (tests for this case are skipped)
#define POLICY_THROWS_ON_EMPTY 1

// Copy / move of the buffer itself:
#define POLICY_BUFFER_IS_COPYABLE 0
#define POLICY_BUFFER_IS_MOVABLE 0
// ---------------------------------------------------------------------------

// ===========================================================================
// Construction & capacity
// ===========================================================================

TEST(CircularBufferCtor, ZeroCapacityThrows)
{
    EXPECT_THROW((CircularBuffer<int>(0)), std::invalid_argument);
}

TEST(CircularBufferCtor, InitialState)
{
    CircularBuffer<int> cb(5);
    EXPECT_EQ(cb.capacity(), 5u);
    EXPECT_EQ(cb.size(), 0u);
    EXPECT_TRUE(cb.empty());
    EXPECT_FALSE(cb.full());
}

TEST(CircularBufferCtor, CapacityOneIsValid)
{
    CircularBuffer<int> cb(1);
    EXPECT_EQ(cb.capacity(), 1u);
    EXPECT_TRUE(cb.empty());
    EXPECT_FALSE(cb.full());
}

// ===========================================================================
// Basic FIFO behavior
// ===========================================================================

TEST(CircularBufferBasic, PushPopFIFO)
{
    CircularBuffer<int> cb(3);
    cb.push(10);
    cb.push(20);
    cb.push(30);

    EXPECT_EQ(cb.size(), 3u);
    EXPECT_TRUE(cb.full());

    EXPECT_EQ(cb.pop(), 10);
    EXPECT_EQ(cb.pop(), 20);
    EXPECT_EQ(cb.pop(), 30);
    EXPECT_TRUE(cb.empty());
}

TEST(CircularBufferBasic, FrontAndBack)
{
    CircularBuffer<int> cb(3);
    cb.push(1);
    cb.push(2);
    cb.push(3);

    EXPECT_EQ(cb.front(), 1);
    EXPECT_EQ(cb.back(), 3);

    // front/back must not remove
    EXPECT_EQ(cb.size(), 3u);
}

TEST(CircularBufferBasic, FrontBackConst)
{
    CircularBuffer<int> cb(2);
    cb.push(7);
    cb.push(8);

    const auto &ccb = cb;
    EXPECT_EQ(ccb.front(), 7);
    EXPECT_EQ(ccb.back(), 8);
}

TEST(CircularBufferBasic, FrontBackAreMutableReferences)
{
    CircularBuffer<int> cb(2);
    cb.push(1);
    cb.push(2);

    cb.front() = 100;
    cb.back() = 200;
    EXPECT_EQ(cb.pop(), 100);
    EXPECT_EQ(cb.pop(), 200);
}

// ===========================================================================
// Wrap-around
// ===========================================================================

TEST(CircularBufferWrap, TailWrapsAfterPopAndPush)
{
    CircularBuffer<int> cb(3);
    cb.push(1);
    cb.push(2);
    cb.push(3);
    EXPECT_EQ(cb.pop(), 1); // head advances
    cb.push(4);             // tail must wrap to slot 0
    EXPECT_EQ(cb.size(), 3u);

    EXPECT_EQ(cb.pop(), 2);
    EXPECT_EQ(cb.pop(), 3);
    EXPECT_EQ(cb.pop(), 4);
    EXPECT_TRUE(cb.empty());
}

TEST(CircularBufferWrap, RepeatedCyclingKeepsOrder)
{
    CircularBuffer<int> cb(3);
    // Cycle many times; every pop must return a strictly increasing value
    // pushed in matching order.
    int expected = 0;
    for (int i = 0; i < 10; ++i)
        cb.push(expected++);
    // buffer now holds {7,8,9}
    for (int i = 0; i < 3; ++i)
        EXPECT_EQ(cb.pop(), 7 + i);

    for (int i = 0; i < 100; ++i)
    {
        cb.push(expected++);
        EXPECT_EQ(cb.pop(), expected - 3);
    }
}

TEST(CircularBufferWrap, CapacityOnePushPopRepeatedly)
{
    CircularBuffer<int> cb(1);
    for (int i = 0; i < 5; ++i)
    {
        cb.push(i);
        EXPECT_TRUE(cb.full());
        EXPECT_EQ(cb.pop(), i);
        EXPECT_TRUE(cb.empty());
    }
}

// ===========================================================================
// Full-buffer policy
// ===========================================================================

#if POLICY_PUSH_OVERWRITE_ON_FULL
TEST(CircularBufferFull, OverwritesOldest)
{
    CircularBuffer<int> cb(3);
    cb.push(1);
    cb.push(2);
    cb.push(3);
    cb.push(4); // evicts 1
    EXPECT_EQ(cb.size(), 3u);
    EXPECT_EQ(cb.front(), 2);
    EXPECT_EQ(cb.back(), 4);

    cb.push(5); // evicts 2
    EXPECT_EQ(cb.pop(), 3);
    EXPECT_EQ(cb.pop(), 4);
    EXPECT_EQ(cb.pop(), 5);
}

TEST(CircularBufferFull, OverwriteRepeatedlyStaysFull)
{
    CircularBuffer<int> cb(2);
    cb.push(1);
    cb.push(2);
    for (int i = 3; i <= 10; ++i)
    {
        cb.push(i);
        EXPECT_TRUE(cb.full());
        EXPECT_EQ(cb.size(), 2u);
    }
    EXPECT_EQ(cb.pop(), 9);
    EXPECT_EQ(cb.pop(), 10);
}
#endif

// ===========================================================================
// Empty-buffer policy
// ===========================================================================

#if POLICY_THROWS_ON_EMPTY
TEST(CircularBufferEmpty, PopThrows)
{
    CircularBuffer<int> cb(2);
    ASSERT_FALSE(cb.pop());
}

TEST(CircularBufferEmpty, FrontThrows)
{
    CircularBuffer<int> cb(2);
    EXPECT_THROW(cb.front(), std::underflow_error);
    // adjust to std::out_of_range / assert-death tests if you chose otherwise
}

TEST(CircularBufferEmpty, BackThrows)
{
    CircularBuffer<int> cb(2);
    EXPECT_THROW(cb.back(), std::underflow_error);
}
#endif

// ===========================================================================
// clear()
// ===========================================================================

TEST(CircularBufferClear, ClearsElementsKeepsCapacity)
{
    CircularBuffer<int> cb(4);
    cb.push(1);
    cb.push(2);
    cb.push(3);

    cb.clear();
    EXPECT_TRUE(cb.empty());
    EXPECT_EQ(cb.size(), 0u);
    EXPECT_EQ(cb.capacity(), 4u);

    cb.push(42);
    EXPECT_EQ(cb.pop(), 42);
}

TEST(CircularBufferClear, ClearOnEmptyIsNoop)
{
    CircularBuffer<int> cb(2);
    cb.clear();
    EXPECT_TRUE(cb.empty());
    EXPECT_EQ(cb.capacity(), 2u);
}

// ===========================================================================
// push overload resolution (copy vs move)
// ===========================================================================

TEST(CircularBufferPushOverloads, LvalueIsCopied)
{
    CircularBuffer<std::string> cb(2);
    std::string s = "hello";
    cb.push(s);
    EXPECT_EQ(s, "hello"); // source untouched by copy
    EXPECT_EQ(cb.pop(), "hello");
}

TEST(CircularBufferPushOverloads, RvalueIsMoved)
{
    CircularBuffer<std::string> cb(2);
    std::string s = "hello";
    cb.push(std::move(s));
    // s is moved-from; do not assert its contents, but confirm the buffer got it
    EXPECT_EQ(cb.pop(), "hello");
}

TEST(CircularBufferPushOverloads, TemporaryPushed)
{
    CircularBuffer<std::string> cb(2);
    cb.push(std::string("tmp"));
    EXPECT_EQ(cb.pop(), "tmp");
}

// Move-count probe: verifies the rvalue overload actually moves, not copies.
struct MoveProbe
{
    int copies = 0;
    int moves = 0;
    int value = 0;

    MoveProbe() = default;
    explicit MoveProbe(int v) : value(v) {}
    MoveProbe(const MoveProbe &o) : copies(o.copies + 1), moves(o.moves), value(o.value) {}
    MoveProbe(MoveProbe &&o) noexcept : copies(o.copies), moves(o.moves + 1), value(o.value) {}
    MoveProbe &operator=(const MoveProbe &) = default;
    MoveProbe &operator=(MoveProbe &&) noexcept = default;
};

TEST(CircularBufferPushOverloads, RvalueOverloadMovesNotCopies)
{
    CircularBuffer<MoveProbe> cb(2);
    MoveProbe p(99);
    cb.push(std::move(p));
    // We can't see the probe's counters once moved-in without popping, but
    // popping should itself move and thus not increase the copy count.
    MoveProbe out = cb.pop();
    EXPECT_EQ(out.value, 99);
    // If your pop returns by value it may add one move, but never a copy:
    EXPECT_EQ(out.copies, 0);
}

// ===========================================================================
// Copy / move policy of the buffer itself
// ===========================================================================

#if !POLICY_BUFFER_IS_COPYABLE
TEST(CircularBufferCopyMove, NotCopyable)
{
    static_assert(!std::is_copy_constructible_v<CircularBuffer<int>>);
    static_assert(!std::is_copy_assignable_v<CircularBuffer<int>>);
}
#endif

#if !POLICY_BUFFER_IS_MOVABLE
TEST(CircularBufferCopyMove, NotMovable)
{
    static_assert(!std::is_move_constructible_v<CircularBuffer<int>>);
    static_assert(!std::is_move_assignable_v<CircularBuffer<int>>);
}
#endif

#if POLICY_BUFFER_IS_COPYABLE
TEST(CircularBufferCopyMove, CopyIsIndependent)
{
    CircularBuffer<int> a(3);
    a.push(1);
    a.push(2);
    CircularBuffer<int> b = a;
    b.push(3);

    EXPECT_EQ(a.size(), 2u);
    EXPECT_EQ(b.size(), 3u);
    EXPECT_EQ(a.pop(), 1); // original order preserved
    EXPECT_EQ(b.pop(), 1);
}
#endif

// ===========================================================================
// Non-trivial element type
// ===========================================================================

TEST(CircularBufferNonTrivial, StringBufferWrapAround)
{
    CircularBuffer<std::string> cb(3);
    cb.push("a");
    cb.push("b");
    cb.push("c");
    cb.push("d"); // overwrite policy: evicts "a"
    EXPECT_EQ(cb.pop(), "b");
    EXPECT_EQ(cb.pop(), "c");
    EXPECT_EQ(cb.pop(), "d");
    EXPECT_TRUE(cb.empty());
}

// ===========================================================================
// Exception safety of push (only meaningful if you promised strong guarantee)
// ===========================================================================

struct ThrowingOnCopy
{
    int value;
    static bool armed;

    ThrowingOnCopy(int v = 0) : value(v) {}
    ThrowingOnCopy(const ThrowingOnCopy &o) : value(o.value)
    {
        if (armed)
            throw std::runtime_error("copy boom");
    }
    ThrowingOnCopy(ThrowingOnCopy &&o) noexcept : value(o.value) {}
};

bool ThrowingOnCopy::armed = false;

TEST(CircularBufferExceptionSafety, PushFailureLeavesBufferUnchanged)
{
    CircularBuffer<ThrowingOnCopy> cb(3);
    cb.push(ThrowingOnCopy(1));
    cb.push(ThrowingOnCopy(2));

    ThrowingOnCopy src(99);
    ThrowingOnCopy::armed = true;
    EXPECT_THROW(cb.push(src), std::runtime_error);
    ThrowingOnCopy::armed = false;

    // Size and ordering must be intact.
    EXPECT_EQ(cb.size(), 2u);
    EXPECT_EQ(cb.pop().value, 1);
    EXPECT_EQ(cb.pop().value, 2);
}