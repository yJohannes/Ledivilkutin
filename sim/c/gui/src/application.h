#pragma once

#include <memory>

#include <spdlog/logger.h>

#include <thread>
#include <sstream>
#include <istream>
#include <ostream>

#include <wce/engine.h>
#include <wce/cli/cli.h>
#include <wce/uci/uci.h>

#include "app_state.h"
#include "platform/window.h"
#include "ui/console.h"
#include "ui/main_view.h"


namespace ledv::gui
{
    class Application
    {
    public:
        Application(Window& window);
        int Run();

    private:
        AppState app_state_;

        Window& window_;
        MainView main_view_;

        std::stringbuf cli_in_buf_;
        std::stringbuf cli_out_buf_;
        std::stringbuf uci_in_buf_;
        std::stringbuf uci_out_buf_;

        std::istream cli_in_{&cli_in_buf_};
        std::ostream cli_out_{&cli_out_buf_};
        std::istream uci_in_{&uci_in_buf_};
        std::ostream uci_out_{&uci_out_buf_};

        Console console_cio_;
        Console console_cerr_;

        std::shared_ptr<spdlog::logger> logger_;

        Engine engine_;
        cli::Cli cli_;
        uci::Uci uci_;

        std::jthread cli_thread_;
        std::jthread uci_thread_;
    };
}
