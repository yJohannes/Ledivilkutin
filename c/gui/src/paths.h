#pragma once

#include <filesystem>


namespace ledv::gui::paths
{
    namespace fs = std::filesystem;

    inline const fs::path Assets = GUI_ASSETS_DIR;
    inline const fs::path Fonts = Assets / "fonts";
    inline const fs::path FontInter = Fonts / "Inter.ttf";
}
