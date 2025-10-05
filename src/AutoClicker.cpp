#include "AutoClicker.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/XTest.h>

using namespace std;

AutoClicker::AutoClicker() {
    worker = thread(&AutoClicker::run, this);
}

AutoClicker::~AutoClicker() {
    stop();
    if (worker.joinable()) {
        worker.join();
    }
}

void AutoClicker::start() {
    running = true;
}

void AutoClicker::stop() {
    running = false;
    alive = false;
}

bool AutoClicker::setup_xi2(Display *dpy, Window root) {
    int event, error;
    if (!XQueryExtension(dpy, "XInputExtension", &xi_opcode, &event, &error)) {
        fprintf(stderr, "Error: XInput2 not available.\n");
        return false;
    }
    int major = 2, minor = 3;
    if (XIQueryVersion(dpy, &major, &minor) != Success) {
        fprintf(stderr, "Error: XI2 version not compatible\n");
        return false;
    }

    XIEventMask mask;
    unsigned char mask_data[(XI_LASTEVENT + 7) / 8] = {0};
    mask.deviceid = XIAllMasterDevices;
    mask.mask_len = sizeof(mask_data);
    mask.mask = mask_data;
    XISetMask(mask.mask, XI_RawButtonPress);
    XISetMask(mask.mask, XI_RawButtonRelease);

    XISelectEvents(dpy, root, &mask, 1);
    XFlush(dpy);
    return true;
}

void AutoClicker::run() {
    Display *dpy = XOpenDisplay(nullptr);
    if (!dpy) {
        fprintf(stderr, "Error: Could not open display.\n");
        return;
    }

    int ev, err, maj, min;
    if (!XTestQueryExtension(dpy, &ev, &err, &maj, &min)) {
        fprintf(stderr, "Error: XTest extension not available.\n");
        return;
    }

    Window root = DefaultRootWindow(dpy);
    if (!setup_xi2(dpy, root))
        return;

    const unsigned int TOGGLE_BUTTON = 9;
    bool active = false;
    bool btn_down = false;
    auto last_click = chrono::steady_clock::now();

    puts("Autoclicker: Front page button (Button9) toggles globally. Ctrl+C exits.");

    while (alive) {
        while (XPending(dpy)) {
            XEvent evnt;
            XNextEvent(dpy, &evnt);

            if (evnt.type == GenericEvent && evnt.xcookie.extension == xi_opcode) {
                if (XGetEventData(dpy, &evnt.xcookie)) {
                    if (evnt.xcookie.evtype == XI_RawButtonPress ||
                        evnt.xcookie.evtype == XI_RawButtonRelease) {
                        XIRawEvent *raw = reinterpret_cast<XIRawEvent *>(evnt.xcookie.data);
                        unsigned int btn = raw->detail;

                        if (evnt.xcookie.evtype == XI_RawButtonPress && btn == TOGGLE_BUTTON) {
                            if (!btn_down) {
                                active = !active;
                                printf("Auto-Click: %s\n", active ? "ON" : "OFF");
                                fflush(stdout);
                            }
                            btn_down = true;
                        } else if (evnt.xcookie.evtype == XI_RawButtonRelease && btn == TOGGLE_BUTTON) {
                            btn_down = false;
                        }
                    }
                    XFreeEventData(dpy, &evnt.xcookie);
                }
            }
        }

        auto now = chrono::steady_clock::now();
        if (active && now - last_click >= chrono::milliseconds(delay)) {
            if (mode == "l") {
                XTestFakeButtonEvent(dpy, 1, True, CurrentTime);
                XTestFakeButtonEvent(dpy, 1, False, CurrentTime);
            } else {
                XTestFakeButtonEvent(dpy, 3, True, CurrentTime);
                XTestFakeButtonEvent(dpy, 3, False, CurrentTime);
            }
            XFlush(dpy);
            last_click = now;
        }

        this_thread::sleep_for(chrono::milliseconds(5));
    }

    XCloseDisplay(dpy);
}

void AutoClicker::shutdown() {
    running = false;
    alive = false;
}

bool AutoClicker::isRunning() const {
    return running;
}

bool AutoClicker::isAlive() const {
    return alive;
}

void AutoClicker::setAlive(const bool &value) {
    alive = value;
}
