#include <cstdlib>

#include "window.h"
#include "application.h"


int main()
{
    ledv::gui::Window window;

    if (!window.Initialize())
        return EXIT_FAILURE;

    ledv::gui::Application app(window);
    return app.Run();
}
