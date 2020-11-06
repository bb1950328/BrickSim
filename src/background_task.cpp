// background_task.cpp
// Created by bab21 on 05.11.20.
//

#include <utility>
#include <iostream>
#include "background_task.h"
#include "controller.h"

BackgroundTask::BackgroundTask(unsigned int id, std::string name, const std::function<void()>&  taskFunction): id(id), taskName(std::move(name)) {
    //todo maybe also make a constructor for a function which gets the background task passes so it can report progress
    std::cout << "starting " << taskName << std::endl;
    thread = std::thread([this, taskFunction]() {
        std::chrono::seconds timespan(1);
        std::this_thread::sleep_for(timespan);
        //std::cout << taskName << " started." << std::endl;
        taskFunction();
        //std::cout << taskName << " finished." << std::endl;
        is_done.store(true);
    });
}

const std::string &BackgroundTask::getTaskName() const {
    return taskName;
}

unsigned int BackgroundTask::getId() const {
    return id;
}

void BackgroundTask::joinThread() {
    thread.join();
}

bool BackgroundTask::isDone() const {
    return is_done.load();
}
