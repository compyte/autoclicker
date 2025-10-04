#include <iostream>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/extensions/XInput2.h>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <thread>

static volatile sig_atomic_t keep_running = 1;
void handle_sigint(int) { keep_running = 0; }

static int xi_opcode = 0;

bool setup_xi2(Display *dpy, Window root)
{
    int event, error;
    if (!XQueryExtension(dpy, "XInputExtension", &xi_opcode, &event, &error))
    {
        std::fprintf(stderr, "Error: XInput2 not available.\n");
        return false;
    }
    int major = 2, minor = 3;
    if (XIQueryVersion(dpy, &major, &minor) != Success)
    {
        std::fprintf(stderr, "Error: XI2 version not compatible\n");
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

void printHelp()
{
    std::cout << "Usage:\n"
              << "  -i <interval> -m <l|r>\n";
}

int main(int argc, char *argv[])
{
    std::signal(SIGINT, handle_sigint);

    int interval = 0;
    std::string mode;
    bool hasI = false;
    bool hasM = false;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "-i" && i + 1 < argc)
        {
            interval = std::atoi(argv[++i]);
            hasI = true;
        }
        else if (arg == "-m" && i + 1 < argc)
        {
            mode = argv[++i];
            hasM = true;
        }
    }

    if (!hasI || !hasM)
    {
        printHelp();
        return 1;
    }

    if (mode != "l" && mode != "r")
    {
        std::cerr << "Error: Mode must be 'l' or 'r'.\n";
        return 1;
    }

    Display *dpy = XOpenDisplay(nullptr);
    if (!dpy)
    {
        std::fprintf(stderr, "Error: Could not open display.\n");
        return 1;
    }

    int ev, err, maj, min;
    if (!XTestQueryExtension(dpy, &ev, &err, &maj, &min))
    {
        std::fprintf(stderr, "Error: XTest extension not available.\n");
        return 1;
    }

    Window root = DefaultRootWindow(dpy);
    if (!setup_xi2(dpy, root))
        return 1;

    const unsigned int TOGGLE_BUTTON = 9;
    bool active = false;
    bool btn_down = false;
    auto last_click = std::chrono::steady_clock::now();

    std::puts("Autoclicker: Front page button (Button9) toggles globally. Ctrl+C exits.");

    while (keep_running)
    {
        while (XPending(dpy))
        {
            XEvent evnt;
            XNextEvent(dpy, &evnt);

            if (evnt.type == GenericEvent && evnt.xcookie.extension == xi_opcode)
            {
                if (XGetEventData(dpy, &evnt.xcookie))
                {
                    if (evnt.xcookie.evtype == XI_RawButtonPress ||
                        evnt.xcookie.evtype == XI_RawButtonRelease)
                    {

                        XIRawEvent *raw = reinterpret_cast<XIRawEvent *>(evnt.xcookie.data);
                        unsigned int btn = raw->detail;

                        if (evnt.xcookie.evtype == XI_RawButtonPress && btn == TOGGLE_BUTTON)
                        {
                            if (!btn_down)
                            {
                                active = !active;
                                std::printf("Auto-Click: %s\n", active ? "AN" : "AUS");
                                std::fflush(stdout);
                            }
                            btn_down = true;
                        }
                        else if (evnt.xcookie.evtype == XI_RawButtonRelease && btn == TOGGLE_BUTTON)
                        {
                            btn_down = false;
                        }
                    }
                    XFreeEventData(dpy, &evnt.xcookie);
                }
            }
        }

        auto now = std::chrono::steady_clock::now();
        if (active && now - last_click >= std::chrono::milliseconds(interval))
        {
            if (mode == "l")
            {
                XTestFakeButtonEvent(dpy, 1, True, CurrentTime);
                XTestFakeButtonEvent(dpy, 1, False, CurrentTime);
            }
            else
            {
                XTestFakeButtonEvent(dpy, 3, True, CurrentTime);
                XTestFakeButtonEvent(dpy, 3, False, CurrentTime);
            }
            XFlush(dpy);
            last_click = now;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    XCloseDisplay(dpy);
    return 0;
}
