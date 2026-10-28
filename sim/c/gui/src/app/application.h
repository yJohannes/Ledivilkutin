#pragma once

#include "platform/window.h"
#include "ui/simulation_view.h"
#include "ui/config_view.h"


namespace ledv::gui
{
    class Application
    {
    public:
        Application(Window& window);
        int Run();

    private:
        Window& window_;
        SimulationView main_view_;
        ConfigView cfg_view_;
    };
}
