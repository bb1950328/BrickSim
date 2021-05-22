#include <spdlog/spdlog.h>
#include "tasks.h"


Task::Task(std::string name, const std::function<void()>& taskFunctionNoProgress, bool autostart):
    Task(std::move(name), [taskFunctionNoProgress](float *ignore){taskFunctionNoProgress();}, autostart)  {
    progress=0.5;
}

Task::Task(std::string name, std::function<void(float *)> taskFunction, bool autostart):
    name(std::move(name)), function(std::move(taskFunction)) {
        is_done = false;
        if (autostart) {
            startThread();
        }
}

const std::string &Task::getName() const {
    return name;
}

void Task::startThread() {
    spdlog::info("starting task {}", name);
    thread = std::thread([this](){
        spdlog::debug("thread of task {} started", name);

        auto before = std::chrono::high_resolution_clock::now();
        function(&progress);
        auto after = std::chrono::high_resolution_clock::now();

        progress=1;
        duration_us.store(std::chrono::duration_cast<std::chrono::microseconds>(after - before).count());
        is_done.store(true);
    });
}

void Task::joinThread() {
    thread->join();
    spdlog::info("thread of task {} joined. task used {}ms.", name, duration_us/1000.0f);
}

bool Task::isStarted() const {
    return thread.has_value();
}

bool Task::isDone() const {
    return is_done;
}

float Task::getProgress() const {
    return progress;
}
