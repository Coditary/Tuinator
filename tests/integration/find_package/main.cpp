#include <tuinator/tuinator.hpp>

int main() {
    tuinator::Application app;
    app.set_root(std::make_unique<tuinator::Label>("Tuinator library works"));
    return 0;
}
