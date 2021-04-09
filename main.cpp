#include <X11/Xlib.h>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <cerrno>

extern int errno;

struct {

} SimpleGuiState;

const int InitialWindowWidth = 640;
const int InitialWindowHeight = 480;


int main() {
    Display *d;
    Window w;
    XEvent e;
    std::string msg = "Hello, World!";
    int s;

    /* Соединиться с X сервером, если X сервер на удаленной машине
     * следует разрешить на машине, где запущен X Server
     * удаленные соединения командой xhost+ (см. man xhost)
     */
    if ((d = XOpenDisplay(nullptr)) == nullptr) {
        std::cout << "Can't connect X server:" << strerror(errno) << std::endl;
        exit(1);
    }

    s = XDefaultScreen(d);

    /* Создать фоновый цвет окна */
    char bg_color[] = "#25854B";
    XColor color;

    XParseColor(d, DefaultColormap(d, s), bg_color, &color);
    XAllocColor(d, DefaultColormap(d, s), &color);

    /* Создать окно */
    w = XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, InitialWindowWidth, InitialWindowHeight, 1,
                            XBlackPixel(d, s), color.pixel);


    /* На какие события будем реагировать */
    XSelectInput(d, w, ExposureMask | ButtonPressMask | ButtonReleaseMask | MotionNotify);

    /* Вывести окно на экран */
    XMapWindow(d, w);

    /* Бесконечный цикл обработки событий */
    while (true) {
        std::cout << "Before NextEvent" << std::endl;
        XNextEvent(d, &e);
        std::cout << "Event received:" << e.type  << std::endl;

        /* Перерисовать окно */
        if (e.type == Expose) {
            XFillRectangle(d, w, DefaultGC(d, s), 20, 20, 10, 10);
            XDrawString(d, w, DefaultGC(d, s), 50, 50, msg.c_str(), msg.length());
        }
        if (e.type == ButtonRelease) {
            XDrawString(d, w, DefaultGC(d, s), 100, 100, msg.c_str(), msg.length());
        }
        /* При нажатии кнопки-выход */
        if (e.type == KeyPress)
            break;
    }

    /* Закрыть соединение с X сервером */
    XCloseDisplay(d);

    return 0;
}
