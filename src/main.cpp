#include <zconf.h>
#include "controller.h"

int main() {
    chdir("/home/bab21/CLionProjects/BrickSim");
    return controller::run();
}
