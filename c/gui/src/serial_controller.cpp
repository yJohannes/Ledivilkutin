
#include "serial_controller.h"

#include <sstream>
#include <iostream>
#include <string_view>


namespace ledv::gui
{
    SerialController::SerialController(std::string_view port, DWORD baudrate)
    {
        std::string device = "\\\\.\\" + std::string(port);

        serial = ::CreateFileA(
            device.c_str(),
            GENERIC_READ,
            0,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr
        );

        if (serial == INVALID_HANDLE_VALUE)
        {
            std::cerr << "Failed to open serial port: " << port << '\n';
            return;
        }

        // Configure serial port
        DCB dcb = {};
        dcb.DCBlength = sizeof(dcb);

        if (!::GetCommState(serial, &dcb))
        {
            Close();
            return;
        }

        dcb.BaudRate = baudrate;
        dcb.ByteSize = 8;
        dcb.StopBits = ONESTOPBIT;
        dcb.Parity = NOPARITY;

        if (!::SetCommState(serial, &dcb))
        {
            Close();
            return;
        }

        // Non-blocking reads
        COMMTIMEOUTS timeouts = {};
        timeouts.ReadIntervalTimeout = MAXDWORD;
        timeouts.ReadTotalTimeoutConstant = 0;
        timeouts.ReadTotalTimeoutMultiplier = 0;

        ::SetCommTimeouts(serial, &timeouts);
    }

    SerialController::~SerialController()
    {
        Close();
    }

    void SerialController::Close()
    {
        if (serial != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(serial);
            serial = INVALID_HANDLE_VALUE;
        }
    }

    bool SerialController::ReadLine(std::string& line)
    {
        if (serial == INVALID_HANDLE_VALUE)
            return false;

        char temp[256];
        DWORD bytesRead = 0;

        // Read all currently available bytes
        DWORD available = 0;

        if (!::ClearCommError(serial, nullptr, nullptr))
            return false;

        COMSTAT status;

        if (!::ClearCommError(serial, nullptr, &status))
            return false;

        available = status.cbInQue;

        if (available > 0)
        {
            DWORD toRead = (available < sizeof(temp))
                ? available
                : sizeof(temp);

            if (!::ReadFile(serial, temp, toRead, &bytesRead, nullptr))
                return false;

            buffer.append(temp, bytesRead);
        }

        // Find complete line
        size_t pos = buffer.find('\n');

        if (pos == std::string::npos)
            return false;

        line = buffer.substr(0, pos);
        buffer.erase(0, pos + 1);

        // Remove CR from CRLF
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        return true;
    }

    bool SerialController::ReadAcceleration(int16_t& ax, int16_t& ay)
    {
        ax = 0;
        ay = 0;

        std::string line;
        std::string latest;

        // Consume all available complete lines
        while (ReadLine(line))
        {
            if (!line.empty() && line != "0")
                latest = line;
        }

        if (latest.empty())
            return false;

        int x, y;
        char comma;

        std::stringstream ss(latest);

        if (!(ss >> x >> comma >> y) || comma != ',')
            return 0;

        // Validate int16 range
        if (x < INT16_MIN || x > INT16_MAX ||
            y < INT16_MIN || y > INT16_MAX)
        {
            return false;
        }

        ax = static_cast<int16_t>(x);
        ay = static_cast<int16_t>(y);

        return true;
    }
} // namespace ledv::gui
