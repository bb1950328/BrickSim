#include "tasks.h"
#include "config.h"
#include <spdlog/spdlog.h>

namespace bricksim {
    RunningTask::RunningTask(const std::function<void(float*)>& func, float* progress, const std::string& name) :
        future(std::async(std::launch::async, [&name, progress, &func]() {
            const auto plName = fmt::format("Tasks/", name);
            util::setThreadName(plName.c_str());

            spdlog::debug("Task \"{}\" started", name);
            const auto before = std::chrono::high_resolution_clock::now();
            func(progress);
            const auto after = std::chrono::high_resolution_clock::now();
            spdlog::debug("Task \"{}\" finished", name);

            *progress = 1.f;
            return std::chrono::duration_cast<std::chrono::microseconds>(after - before);
        })) {
    }

    bool RunningTask::isDone() const {
        return future.wait_for(std::chrono::microseconds(0)) == std::future_status::ready;
    }

    std::chrono::microseconds RunningTask::finish() {
        return future.get();
    }

    Task::Task(std::string name, const std::function<void()>& taskFunctionNoProgress, bool autostart) :
        Task(
                std::move(name),
                [taskFunctionNoProgress](float* ignore) {
                    taskFunctionNoProgress();
                },
                autostart) {
        progress = 0.5;
    }

    Task::Task(std::string name, std::function<void(float*)> taskFunction, bool autostart) :
        name(std::move(name)),
        function(std::move(taskFunction)) {
        if (autostart) {
            startThread();
        }
    }

    const std::string& Task::getName() const {
        return name;
    }

    void Task::startThread() {
        runningTask = std::make_optional<RunningTask>(function, &progress, name);

        if (!config::get(config::THREADING_ENABLED)) {
            joinThread();
        }
    }

    void Task::joinThread() {
        try {
            const auto duration = runningTask->finish();
            spdlog::info("thread of task {} joined. task used {}ms.", name, static_cast<float>(duration.count()) / 1000.f);
        } catch (...) {
            spdlog::critical("Exception thrown in task {}", name);
            throw;
        }
    }

    bool Task::isStarted() const {
        return runningTask.has_value();
    }

    bool Task::isDone() const {
        return runningTask->isDone();
    }

    float Task::getProgress() const {
        return progress;
    }

    const float* Task::getProgressPtr() const {
        return &progress;
    }

    Task::Task(Task&& other) noexcept :
        name(std::move(other.name)),
        function(std::move(other.function)),
        runningTask(std::move(other.runningTask)),
        progress(other.progress) {}

    Task& Task::operator=(Task&& other) noexcept {
        name = std::move(other.name);
        function = std::move(other.function);
        runningTask = std::move(other.runningTask);
        progress = other.progress;
        return *this;
    }
    Task::~Task() = default;
}
