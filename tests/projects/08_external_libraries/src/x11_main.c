#include <stdio.h>
#include <X11/Xlib.h>

int main(void)
{
    Display* display = XOpenDisplay(NULL);
    if (display == NULL)
    {
        fprintf(stderr, "Failed to initialize display with XOpenDisplay. \n");
        return -1;
    }

    XCloseDisplay(display);

    return 0;
}