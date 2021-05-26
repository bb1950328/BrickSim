namespace controller {
    int run();
}

int main() {
    static_assert(false, "just to test ccache");
    return controller::run();
}
