//
// Created by Bader on 09.12.2020.
//

#include <iostream>
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
        function(&progress);
        progress=1;
        spdlog::debug("thread of task {} finishing", name);
        is_done.store(true);
    });
    spdlog::info("task {} finished", name);
}

void Task::joinThread() {
    thread->join();
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
