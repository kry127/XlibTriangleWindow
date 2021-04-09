#include <X11/Xlib.h>
#include <unistd.h>
#include <exception>

int main() {
    Display *d = XOpenDisplay(nullptr);

    if (d) {
        // Create the window
        Window w = XCreateWindow(d, DefaultRootWindow(d), 0, 0, 300,
                                 200, 0, CopyFromParent, CopyFromParent,
                                 CopyFromParent, 0, nullptr);

        // Show the window
        XMapWindow(d, w);
        XFlush(d);

        XGCValues values;
        values.background = 1;
        GC gc = XCreateGC(d, w, GCBackground, &values);
        if (gc == nullptr) {
            throw std::exception();
        }
        XSetBackground(d, gc, 0xFFFFFFFF);
        XDrawLine(d, w, gc,
                  20, 20,
                  30, 30);

        // Sleep long enough to see the window.
        for (int i = 0; i < 11; i++) {
            XBell(d, (i - 5) * 20);
            XFlush(d);
            sleep(1);
        }
    }
    return 0;
}
