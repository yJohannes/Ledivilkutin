
#pragma once

#include <Windows.h>

#include <cstdint>
#include <string>
#include <string_view>

namespace ledv::gui
{
    class SerialController
    {
    public:
        SerialController(std::string_view port = "COM4", DWORD baudrate = CBR_38400);
        ~SerialController();

        // Returns 1 if a valid acceleration line was read, otherwise 0.
        // On failure, ax and ay are set to 0.
        bool ReadAcceleration(int16_t& ax, int16_t& ay);

        void Close();

    private:
        HANDLE serial = INVALID_HANDLE_VALUE;
        std::string buffer;

        bool ReadLine(std::string& line);
    };
} // namespace ledv::gui

