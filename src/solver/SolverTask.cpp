/*
 * Created by Ed Fillingham on 09/05/2026.
 *
 * FreeRTOS task wrapper for Board::solve(). See SolverTask.h for API.
*/

#include "solver/SolverTask.h"
#include "Arduino.h"

SolverTask::SolverTask() {
    _doneSem = xSemaphoreCreateBinary();
}

SolverTask::~SolverTask() {
    stop();
    if (_doneSem) vSemaphoreDelete(_doneSem);
}

void SolverTask::start(std::vector<Coord> blockers) {
    stop();

    // Consume any leftover signal from a task that finished naturally before stop() was called.
    xSemaphoreTake(_doneSem, 0);

    _board = new Board(blockers, SOLUTION_LIMIT, TIME_LIMIT_S);
    _done.store(false);
    _foundSolution.store(false);
    _active        = true;

    BaseType_t created = xTaskCreatePinnedToCore(
        taskFn, "solver", TASK_STACK_SIZE,
        this, TASK_PRIORITY, &_handle, TASK_CORE);

    if (created != pdPASS) {
        _active = false;
        delete _board;
        _board = nullptr;
        Serial.println("[SolverTask] ERROR: task creation failed");
        return;
    }

    Serial.printf("[SolverTask] start() — task created on Core %d ptr=%p\n",
        TASK_CORE, (void*)_board);
}

void SolverTask::stop() {
    if (!_active) return;

    Serial.printf("[SolverTask] stop() — cancelling ptr=%p\n", (void*)_board);

    if (!_done.load()) _board->cancel();

    xSemaphoreTake(_doneSem, portMAX_DELAY); // blocks until taskFn signals completion

    _handle = nullptr; // task already deleted itself via vTaskDelete(NULL)
    delete _board;
    _board  = nullptr;
    _active = false;
    _done.store(false);

    Serial.println("[SolverTask] stop() — cleaned up");
}

bool SolverTask::isDone() const {
    return !_active || _done.load(std::memory_order_acquire);
}

bool SolverTask::hasSolution() const {
    return _foundSolution.load();
}

void SolverTask::taskFn(void* param) {
    SolverTask* self = static_cast<SolverTask*>(param);

    Serial.printf("[SolverTask] task running on Core %d\n", xPortGetCoreID());

    bool solved = self->_board->solve();
    self->_foundSolution.store(solved);

    if (solved) self->_board->printSolution();

    self->_done.store(true, std::memory_order_release);
    xSemaphoreGive(self->_doneSem);
    vTaskDelete(NULL); // FreeRTOS tasks must not return
}
