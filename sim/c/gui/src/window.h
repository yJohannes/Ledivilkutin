#pragma once


// Forward declare
struct GLFWwindow;


namespace ledv::gui
{
    /// @brief The platform native window and rendering interface
    class Window
    {
    public:
        Window() = default;
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
        Window(Window&&) = delete;
        Window& operator=(Window&&) = delete;
        ~Window();

        bool Initialize();

        [[nodiscard]] bool ShouldClose() const;
        void PollEvents();

        void BeginFrame();
        void EndFrame();

        [[nodiscard]] float GetDpiScale() const;
        [[nodiscard]] float GetWindowDevicePixelRatio() const;

    private:
        GLFWwindow* handle_ = nullptr;
    };
}
