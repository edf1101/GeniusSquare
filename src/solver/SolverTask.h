/*
 * Created by Ed Fillingham on 09/05/2026.
 *
 * Manages a background FreeRTOS solver task on Core 0.
 * Call start() when a valid arrangement is detected and stop() to cancel.
 */

#ifndef SOLVER_TASK_H
#define SOLVER_TASK_H

#include <vector>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "utils/math/maths.h"
#include "solver/Board.h"

class SolverTask {
public:
    SolverTask();
    ~SolverTask();

    /** @brief Cancel any running solve, allocate a new Board, and spawn the Core 0 task. */
    void start(std::vector<Coord> blockers);

    /** @brief Cancel the running solve and block until the task exits, then free the Board. */
    void stop();

    /** @brief True once the task has exited (solution found, timed out, or cancelled). */
    bool isDone() const;

    /** @brief True if the last solve found a solution. */
    bool hasSolution() const;

private:
    static void taskFn(void* param);

    Board*            _board          = nullptr;
    TaskHandle_t      _handle         = nullptr;  // Core 1 only — not accessed cross-core
    SemaphoreHandle_t _doneSem        = nullptr;
    std::atomic<bool> _done{false};
    std::atomic<bool> _foundSolution{false};
    bool              _active         = false;    // Core 1 only — not accessed cross-core

    static constexpr int   TASK_STACK_SIZE  = 8192;
    static constexpr int   TASK_PRIORITY    = 1;
    static constexpr int   TASK_CORE        = 0;
    static constexpr int   SOLUTION_LIMIT   = 1;
    static constexpr int   TIME_LIMIT_S     = 300;
};

#endif // SOLVER_TASK_H
