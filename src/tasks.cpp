//
// Created by Bader on 09.12.2020.
//

#include <iostream>
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
    std::cout << "starting " << name << std::endl;
    thread = std::thread([this](){
        std::cout << name << "started" << std::endl;
        function(&progress);
        std::cout << name << "finished" << std::endl;
        progress=1;
        is_done.store(true);
    });
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
