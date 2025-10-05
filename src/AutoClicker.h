#ifndef AUTOCLICKER_AUTOCLICKER_H
#define AUTOCLICKER_AUTOCLICKER_H
#include <atomic>
#include <string>
#include <thread>
#include <X11/X.h>
#include <X11/Xlib.h>

class AutoClicker {
    std::atomic<bool> running{true};
    bool alive = true;
    int delay = 100;
    int xi_opcode = 0;
    std::thread worker;
    std::string mode = "l";

public:
    AutoClicker();
    ~AutoClicker();

    void start();

    void stop();

    bool setup_xi2(Display *dpy, Window root);

    void shutdown();

    bool isRunning() const;

    bool isAlive() const;

    void setAlive(const bool &value);

    void setDelay(const int &value) {
        delay = value;
    }

    void setMode(const std::string &value) {
        mode = value;
    }

private:
    void run();
};

#endif //AUTOCLICKER_AUTOCLICKER_H
