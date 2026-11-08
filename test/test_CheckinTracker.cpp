/// @file test_CheckinTracker.cpp
/// @brief Host unit tests for CheckinTracker (the target independent task monitor logic).

#include "CheckinTracker.hpp"

#include "unity.h"

#include <functional>
#include <vector>

namespace
{
    using Tracker = CheckinTracker<int>;

    constexpr int TASK_A = 1;   ///< ID of the first test task
    constexpr int TASK_B = 2;   ///< ID of the second test task

    int checkinCallsA = 0;      ///< Number of checkin requests sent to TASK_A
    int checkinCallsB = 0;      ///< Number of checkin requests sent to TASK_B

    /// @brief Checkin callback for TASK_A
    void CheckinA() { checkinCallsA++; }

    /// @brief Checkin callback for TASK_B
    void CheckinB() { checkinCallsB++; }

    /// @brief Records the violations reported by Tracker::Update()
    struct ViolationLog
    {
        std::vector<int> ids;               ///< Task IDs reported, in order
        std::vector<uint32_t> elapsed;      ///< elapsedWaitingForResponse of each report

        /// @brief Violation handler passed to Tracker::Update()
        void operator()(int id, const Tracker::Info& info)
        {
            ids.push_back(id);
            elapsed.push_back(info.elapsedWaitingForResponse);
        }
    };

    /// @brief Get the monitoring state of a registered task
    const Tracker::Info& InfoOf(const Tracker& tracker, int id)
    {
        return tracker.Tasks().at(id);
    }
}

/// @brief Reset the callback counters before each test
void setUp(void)
{
    checkinCallsA = 0;
    checkinCallsB = 0;
}

/// @brief Nothing to clean up after each test
void tearDown(void) {}

/// @brief A new tracker has no tasks, and Update() does nothing
static void test_Empty(void)
{
    Tracker tracker;
    ViolationLog log;

    tracker.Update(1000, std::ref(log));

    TEST_ASSERT_TRUE(tracker.Tasks().empty());
    TEST_ASSERT_TRUE(log.ids.empty());
}

/// @brief Register() sets the timeout and callback and clears all times
static void test_RegisterInitialState(void)
{
    Tracker tracker;
    tracker.Register(TASK_A, 100, &CheckinA);

    const Tracker::Info& info = InfoOf(tracker, TASK_A);
    TEST_ASSERT_EQUAL_UINT32(100, info.timeout);
    TEST_ASSERT_EQUAL_UINT32(0, info.elapsedSinceLastCheckin);
    TEST_ASSERT_EQUAL_UINT32(0, info.elapsedWaitingForResponse);
    TEST_ASSERT_EQUAL_UINT32(0, info.maxWaitingForResponse);
    TEST_ASSERT_FALSE(info.waitingOnResponse);
    TEST_ASSERT_EQUAL_PTR(&CheckinA, info.checkinCall);
}

/// @brief A checkin from a task that isn't registered is rejected
static void test_CheckinUnregistered(void)
{
    Tracker tracker;
    tracker.Register(TASK_A, 100, &CheckinA);

    TEST_ASSERT_FALSE(tracker.Checkin(TASK_B));
    TEST_ASSERT_TRUE(tracker.Checkin(TASK_A));
    TEST_ASSERT_EQUAL(1, tracker.Tasks().size());
}

/// @brief No checkin request is sent until strictly more than the timeout has elapsed
static void test_RequestOnlyAfterTimeout(void)
{
    Tracker tracker;
    ViolationLog log;
    tracker.Register(TASK_A, 100, &CheckinA);

    tracker.Update(60, std::ref(log));
    tracker.Update(40, std::ref(log));     // exactly the timeout
    TEST_ASSERT_EQUAL(0, checkinCallsA);
    TEST_ASSERT_FALSE(InfoOf(tracker, TASK_A).waitingOnResponse);

    tracker.Update(1, std::ref(log));
    TEST_ASSERT_EQUAL(1, checkinCallsA);
    TEST_ASSERT_TRUE(InfoOf(tracker, TASK_A).waitingOnResponse);
    TEST_ASSERT_EQUAL_UINT32(0, InfoOf(tracker, TASK_A).elapsedWaitingForResponse);
    TEST_ASSERT_TRUE(log.ids.empty());
}

/// @brief Only one request is sent while waiting for the response
static void test_RequestSentOnce(void)
{
    Tracker tracker;
    ViolationLog log;
    tracker.Register(TASK_A, 100, &CheckinA);

    tracker.Update(101, std::ref(log));
    for (int i = 0; i < 10; i++)
    {
        tracker.Update(10, std::ref(log));
    }

    TEST_ASSERT_EQUAL(1, checkinCallsA);
    TEST_ASSERT_TRUE(log.ids.empty());
}

/// @brief A task is in violation once it has waited strictly more than its timeout for a response
static void test_ViolationAfterTimeout(void)
{
    Tracker tracker;
    ViolationLog log;
    tracker.Register(TASK_A, 100, &CheckinA);

    tracker.Update(101, std::ref(log));    // request sent
    tracker.Update(100, std::ref(log));    // waited exactly the timeout
    TEST_ASSERT_TRUE(log.ids.empty());

    tracker.Update(1, std::ref(log));
    TEST_ASSERT_EQUAL(1, log.ids.size());
    TEST_ASSERT_EQUAL(TASK_A, log.ids[0]);
    TEST_ASSERT_EQUAL_UINT32(101, log.elapsed[0]);
}

/// @brief The violation is reported on every Update() until the task checks in
static void test_ViolationRepeatsUntilCheckin(void)
{
    Tracker tracker;
    ViolationLog log;
    tracker.Register(TASK_A, 100, &CheckinA);

    tracker.Update(101, std::ref(log));
    tracker.Update(101, std::ref(log));
    tracker.Update(10, std::ref(log));
    TEST_ASSERT_EQUAL(2, log.ids.size());
    TEST_ASSERT_EQUAL_UINT32(111, log.elapsed[1]);

    TEST_ASSERT_TRUE(tracker.Checkin(TASK_A));
    tracker.Update(10, std::ref(log));
    TEST_ASSERT_EQUAL(2, log.ids.size());
    TEST_ASSERT_EQUAL(1, checkinCallsA);   // no request sent while in violation
}

/// @brief Responding to a request clears the wait, and the next request comes a full timeout later
static void test_ResponseRestartsPeriod(void)
{
    Tracker tracker;
    ViolationLog log;
    tracker.Register(TASK_A, 100, &CheckinA);

    tracker.Update(101, std::ref(log));
    tracker.Update(30, std::ref(log));
    TEST_ASSERT_TRUE(tracker.Checkin(TASK_A));

    const Tracker::Info& info = InfoOf(tracker, TASK_A);
    TEST_ASSERT_FALSE(info.waitingOnResponse);
    TEST_ASSERT_EQUAL_UINT32(0, info.elapsedSinceLastCheckin);
    TEST_ASSERT_EQUAL_UINT32(0, info.elapsedWaitingForResponse);

    tracker.Update(100, std::ref(log));
    TEST_ASSERT_EQUAL(1, checkinCallsA);
    tracker.Update(1, std::ref(log));
    TEST_ASSERT_EQUAL(2, checkinCallsA);
    TEST_ASSERT_TRUE(log.ids.empty());
}

/// @brief Unsolicited checkins more often than the timeout mean no request is ever sent
static void test_UnsolicitedCheckinsPreventRequests(void)
{
    Tracker tracker;
    ViolationLog log;
    tracker.Register(TASK_A, 100, &CheckinA);

    for (int i = 0; i < 100; i++)
    {
        tracker.Update(90, std::ref(log));
        TEST_ASSERT_TRUE(tracker.Checkin(TASK_A));
    }

    TEST_ASSERT_EQUAL(0, checkinCallsA);
    TEST_ASSERT_TRUE(log.ids.empty());
}

/// @brief The longest response time is kept across checkins
static void test_MaxResponseTime(void)
{
    Tracker tracker;
    ViolationLog log;
    tracker.Register(TASK_A, 100, &CheckinA);

    tracker.Update(101, std::ref(log));    // request 1
    tracker.Update(40, std::ref(log));
    tracker.Checkin(TASK_A);
    TEST_ASSERT_EQUAL_UINT32(40, InfoOf(tracker, TASK_A).maxWaitingForResponse);

    tracker.Update(101, std::ref(log));    // request 2
    tracker.Update(20, std::ref(log));
    tracker.Checkin(TASK_A);
    TEST_ASSERT_EQUAL_UINT32(40, InfoOf(tracker, TASK_A).maxWaitingForResponse);

    tracker.Update(101, std::ref(log));    // request 3
    tracker.Update(30, std::ref(log));
    tracker.Update(30, std::ref(log));
    tracker.Checkin(TASK_A);
    TEST_ASSERT_EQUAL_UINT32(60, InfoOf(tracker, TASK_A).maxWaitingForResponse);
}

/// @brief Registering again resets a task's state, including a violation
static void test_ReRegisterResets(void)
{
    Tracker tracker;
    ViolationLog log;
    tracker.Register(TASK_A, 100, &CheckinA);

    tracker.Update(101, std::ref(log));
    tracker.Update(101, std::ref(log));
    TEST_ASSERT_EQUAL(1, log.ids.size());

    tracker.Register(TASK_A, 500, &CheckinB);
    const Tracker::Info& info = InfoOf(tracker, TASK_A);
    TEST_ASSERT_EQUAL_UINT32(500, info.timeout);
    TEST_ASSERT_FALSE(info.waitingOnResponse);
    TEST_ASSERT_EQUAL_UINT32(0, info.maxWaitingForResponse);

    tracker.Update(501, std::ref(log));
    TEST_ASSERT_EQUAL(1, log.ids.size());
    TEST_ASSERT_EQUAL(1, checkinCallsA);
    TEST_ASSERT_EQUAL(1, checkinCallsB);   // new callback used
}

/// @brief Tasks with different timeouts are tracked independently
static void test_IndependentTasks(void)
{
    Tracker tracker;
    ViolationLog log;
    tracker.Register(TASK_A, 100, &CheckinA);
    tracker.Register(TASK_B, 1000, &CheckinB);

    tracker.Update(101, std::ref(log));
    TEST_ASSERT_EQUAL(1, checkinCallsA);
    TEST_ASSERT_EQUAL(0, checkinCallsB);

    tracker.Update(101, std::ref(log));    // A in violation, B still fine
    TEST_ASSERT_EQUAL(1, log.ids.size());
    TEST_ASSERT_EQUAL(TASK_A, log.ids[0]);
    TEST_ASSERT_EQUAL_UINT32(202, InfoOf(tracker, TASK_B).elapsedSinceLastCheckin);

    TEST_ASSERT_TRUE(tracker.Checkin(TASK_A));
    TEST_ASSERT_TRUE(InfoOf(tracker, TASK_A).elapsedSinceLastCheckin == 0);
    TEST_ASSERT_EQUAL_UINT32(202, InfoOf(tracker, TASK_B).elapsedSinceLastCheckin);

    tracker.Update(799, std::ref(log));
    TEST_ASSERT_EQUAL(1, checkinCallsB);
}

/// @brief Run all tests
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_Empty);
    RUN_TEST(test_RegisterInitialState);
    RUN_TEST(test_CheckinUnregistered);
    RUN_TEST(test_RequestOnlyAfterTimeout);
    RUN_TEST(test_RequestSentOnce);
    RUN_TEST(test_ViolationAfterTimeout);
    RUN_TEST(test_ViolationRepeatsUntilCheckin);
    RUN_TEST(test_ResponseRestartsPeriod);
    RUN_TEST(test_UnsolicitedCheckinsPreventRequests);
    RUN_TEST(test_MaxResponseTime);
    RUN_TEST(test_ReRegisterResets);
    RUN_TEST(test_IndependentTasks);
    return UNITY_END();
}
