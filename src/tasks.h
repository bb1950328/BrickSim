#pragma once

#include <functional>
#include <optional>
#include <string>
#include <future>

namespace bricksim {
    class RunningTask {
        std::future<std::chrono::microseconds> future;

    public:
        bool isDone() const;
        std::chrono::microseconds finish();
        RunningTask(const std::function<void(float*)>& func, float* progress, const std::string& name);
    };

    class Task {
    public:
        Task(std::string name, std::function<void(float*)> taskFunction, bool autostart = false);
        Task(std::string name, const std::function<void()>& taskFunctionNoProgress, bool autostart = false);
        Task(const Task& other) = delete;
        Task(Task&& other) noexcept;
        Task& operator=(const Task& other) = delete;
        Task& operator=(Task&& other) noexcept;
        virtual ~Task();
        [[nodiscard]] const std::string& getName() const;
        [[nodiscard]] float getProgress() const;
        [[nodiscard]] const float* getProgressPtr() const;
        void startThread();
        void joinThread();
        [[nodiscard]] bool isStarted() const;
        [[nodiscard]] bool isDone() const;

    private:
        std::string name;
        std::function<void(float*)> function;
        std::optional<RunningTask> runningTask;
        //std::atomic<bool> is_done{false};
        //std::atomic<long> duration_us{0};
        float progress = 0;
    };
}
