// background_task.h
// Created by bab21 on 05.11.20.
//

#ifndef BRICKSIM_BACKGROUND_TASK_H
#define BRICKSIM_BACKGROUND_TASK_H

#include <functional>
#include <thread>
#include <atomic>

class BackgroundTask {
public:
    BackgroundTask(unsigned int id, std::string name, const std::function<void()>&  taskFunction);
    [[nodiscard]] const std::string &getTaskName() const;
    [[nodiscard]] unsigned int getId() const;
    void joinThread();
    bool isDone() const;
private:

    std::string taskName;
    std::thread thread;
    unsigned int id;
    std::atomic<bool> is_done{false};
};

#endif //BRICKSIM_BACKGROUND_TASK_H
