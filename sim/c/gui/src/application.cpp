#include "application.h"

#include <cstdlib>
#include <cassert>
#include <filesystem>
#include <string_view>
#include <vector>
#include <thread>
#include <iostream>

#include <imgui.h>

#include <wce/position.h>
#include <wce/piece.h>


#include "common/paths.h"
#include "common/log.h"
#include "platform/window.h"
#include "ui/position_info.h"
#include "ui/bitboard_inspector.h"
#include "ui/console.h"
#include "wce/engine.h"


namespace ledv::gui
{
    namespace
    {
    }

    Application::Application(Window& window)
        : window_(window)
        , console_cio_("Standard I/O", std::cin, std::cout)
        , console_cerr_("Standard Error", std::cerr)
        , logger_(CreateGuiLogger())
        , engine_(CreateLogger("engine"))
        , cli_(engine_, cli_in_, cli_out_, CreateLogger("cli"))
        , uci_(engine_, uci_in_, uci_out_, CreateLogger("uci"))    {
        logger_->info("Created loggers");
        logger_->info("Current working directory: {}", std::filesystem::current_path().string());
    }

    int Application::Run()
    {
        cli_thread_ = std::jthread([this]
        {
            cli_.Run();
        });

        uci_thread_ = std::jthread([this]
        {
            uci_.Run();
        });

        while (!window_.ShouldClose())
        {
            window_.PollEvents();

            window_.BeginFrame();

            ImGui::DockSpaceOverViewport();

            DrawPositionInfo(app_state_, engine_);

            const Position& p = engine_.GetPosition();

            // In your application's ImGui frame:
            console_cio_.Render();
            console_cerr_.Render();

            const std::vector<BitboardEntry> bitboards = {
                {"Total Occupancy", p.Pieces()},
                {"White Occupancy", p.Pieces(White)},
                {"Black Occupancy", p.Pieces(Black)},
                {"White Pawns", p.Pieces(White, Pawn)},
                {"White Knights", p.Pieces(White, Knight)},
                {"White Bishops", p.Pieces(White, Bishop)},
                {"White Rooks", p.Pieces(White, Rook)},
                {"White Queens", p.Pieces(White, Queen)},
                {"White King", p.Pieces(White, King)},
                {"Black Pawns", p.Pieces(Black, Pawn)},
                {"Black Knights", p.Pieces(Black, Knight)},
                {"Black Bishops", p.Pieces(Black, Bishop)},
                {"Black Rooks", p.Pieces(Black, Rook)},
                {"Black Queens", p.Pieces(Black, Queen)},
                {"Black King", p.Pieces(Black, King)},
            };

            DrawBitboardInspector(bitboards);

            main_view_.Render(engine_);

            window_.EndFrame();
        }

        return EXIT_SUCCESS;
    }
}
