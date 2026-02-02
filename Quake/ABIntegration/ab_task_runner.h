#pragma once

// STL
#include <functional>
#include <mutex>
#include <queue>
#include <utility>

class ABTaskRunner {
public:
    void execute_task_queue() noexcept;

    template<typename FnT, typename... ArgsT>
    void queue_task(FnT&& callable, ArgsT&&... args) noexcept
    {
        tasks_.emplace([f = std::forward<FnT>(callable), ... captured_args = std::forward<ArgsT>(args)]() {
            std::invoke(f, std::move(captured_args)...);
        });
    }

private:
    std::queue<std::function<void()>> tasks_;
    std::mutex task_mtx_;
};
