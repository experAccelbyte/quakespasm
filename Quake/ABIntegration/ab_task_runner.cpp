#include "ab_task_runner.h"

#include <mutex>

void ABTaskRunner::execute_task_queue() noexcept
{
    std::lock_guard lk{task_mtx_};
    while (!tasks_.empty()) {
        const auto& task = tasks_.front();
        task();
        tasks_.pop();
    }
}
