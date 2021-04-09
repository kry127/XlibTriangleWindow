#include <X11/Xlib.h>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <cerrno>
#include <exception>
#include <unistd.h>

extern int errno;

typedef struct {
    bool buttonPressed, wasCreated;
    int triangleX, triangleY;
    XColor foregroundColor;
} SimpleGuiState;

const int InitialWindowWidth = 640;
const int InitialWindowHeight = 480;

struct timeval tv;
int x11_fd;
fd_set in_fds;

class IOException : std::exception {

};

// dirty hacks
// https://stackoverflow.com/questions/53396936/xlib-disconnection-callback
// https://github.com/carlodoro88/lxdm/blob/d33391ae1594dd3a9c36e86bb39d6d5be74054a1/src/xconn.c
// https://cboard.cprogramming.com/c-programming/177009-xlib-program-does-not-want-change-background-color-post1283079.html
static int CatchIOErrors(Display *dpy)
{
    // закрываем файловый дескриптор
    close(ConnectionNumber(dpy));
    // не даём библиотеке завершить приложение с ненулевым кодом возврата
    throw IOException();
}

void drawContent(Display *d, Window w, SimpleGuiState& state) {
    int s = XDefaultScreen(d);
//    XClearWindow(d, w);
    XClearArea(d, w, 0, 0, 0, 0, False); // DO NOT SET TRUE BECAUSE EXPOSURE IS FIRED!

    short triangleSize = 30;

    /* Если кнопка мыши была нажата хотя бы раз, рисовать треугольник */
    if (state.wasCreated) {
        XSetForeground(d, DefaultGC(d, s), state.foregroundColor.pixel);
        XPoint x[4];
        x[0].x = state.triangleX;
        x[0].y = state.triangleY;
        x[1].x = state.triangleX - triangleSize;
        x[1].y = state.triangleY + triangleSize * 2;
        x[2].x = state.triangleX + triangleSize;
        x[2].y = state.triangleY + triangleSize * 2;
        x[3].x = state.triangleX;
        x[3].y = state.triangleY;
        XDrawLines(d, w, DefaultGC(d, s), x, 4, CoordModeOrigin);
        XFillPolygon(d, w, DefaultGC(d, s), x, 4, Convex, CoordModeOrigin);
    }
}

int main() {
    Display *d;
    Window w;
    XEvent e;
    int s;

    /* Соединиться с X сервером, если X сервер на удаленной машине
     * следует разрешить на машине, где запущен X Server
     * удаленные соединения командой xhost+ (см. man xhost)
     */
    if ((d = XOpenDisplay(nullptr)) == nullptr) {
        std::cout << "Can't connect X server:" << strerror(errno) << std::endl;
        exit(1);
    }

    /* Ставим собственный обработчик для ошибок IO (например, закрытия окна)
     * Это предотвратит exit(1) со стороны библиотеки, когда окно закроется,
     * И мы сможем выйти с нулевым кодом возврата
     */
    XSetIOErrorHandler(CatchIOErrors);

    /* Получаем экран по умолчанию */
    s = XDefaultScreen(d);

    try {

    /* Создать фоновый цвет окна */
    char bg_color[] = "#25854B";
    XColor color;

    XParseColor(d, DefaultColormap(d, s), bg_color, &color);
    XAllocColor(d, DefaultColormap(d, s), &color);

    /* Создаём состояние GUI */
    SimpleGuiState state;
    state.wasCreated = false;
    state.buttonPressed = false;
    state.foregroundColor;

    /* Создать окно */
    w = XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, InitialWindowWidth, InitialWindowHeight, 1,
                            state.foregroundColor.pixel, color.pixel);


    /* На какие события будем реагировать */
    XSelectInput(d, w, ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

    /* Вывести окно на экран */
    XMapWindow(d, w);
    XFlush(d);

    /* получаем файловый дескриптор дисплея X11 */
    x11_fd = ConnectionNumber(d);

    /* Бесконечный цикл обработки событий */
    bool working = true;
    while (working) {
        // Create a File Description Set containing x11_fd
        FD_ZERO(&in_fds);
        FD_SET(x11_fd, &in_fds);

        // Set our timer.  One second sounds good.
        tv.tv_usec = 300000; // 1 000 000 = 1 second
        tv.tv_sec = 0;

        // Wait for X Event or a Timer
        int num_ready_fds = select(x11_fd + 1, &in_fds, nullptr, nullptr, &tv);
        if (num_ready_fds > 0) {
            // OK! we have events at file descriptor
        } else if (num_ready_fds == 0) {
            // Handle timer here
//            std::cout << "Timer fired!" << std::endl;
            state.foregroundColor.red += 2;
            state.foregroundColor.green += 5;
            state.foregroundColor.blue += 13;
            state.foregroundColor.red %= 255;
            state.foregroundColor.green %= 255;
            state.foregroundColor.blue %= 255;
            drawContent(d, w, state);
            continue;
        } else {
            std::cout << "Select error!" << std::endl;
        }

        // Handle XEvents and flush the input
        while(XPending(d)) {
            XNextEvent(d, &e);

            switch (e.type) {
                case Expose:
                    /* Перерисовать окно */
                    drawContent(d, w, state);
                    break;
                case MotionNotify:
                    /* Если нажата кнопка, рисовать треугольник в новом месте */
                    if (state.buttonPressed) {
                        state.triangleX = e.xmotion.x;
                        state.triangleY = e.xmotion.y;
                    }
                    drawContent(d, w, state);
                    break;
                case ButtonPress:
                    /* Запоминаем, что кнопка была зажата и рисуем треугольник */
                    state.wasCreated = true;
                    state.buttonPressed = true;
                    state.triangleX = e.xbutton.x;
                    state.triangleY = e.xbutton.y;
                    drawContent(d, w, state);
                    break;
                case ButtonRelease:
                    /* Перестаём рисовать треугольник */
                    state.buttonPressed = false;
                    break;
            }
            /* При нажатии кнопки-выход */
            if (e.type == KeyPress) {
                working = false;
                break;
            }
        }

    }
    } catch (IOException &iex) {
        // скорее всего, приложение закрыли по крестику
        std::cout << "Oh, application was probably closed :)" << std::endl;
    }

    /* Закрыть соединение с X сервером */
    XCloseDisplay(d);

    return 0;
}
