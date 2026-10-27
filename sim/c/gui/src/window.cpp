#include "window.h"

#include <fstream>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <imgui.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <misc/cpp/imgui_stdlib.h>


namespace ledv::gui
{
    namespace
    {
        struct WindowState
        {
            int x = 100;
            int y = 100;
            int width = 1280;
            int height = 800;
        };

        constexpr const char* window_state_file = "window.ini";

        WindowState LoadWindowState()
        {
            WindowState state;
            std::ifstream file(window_state_file);

            if (file)
            {
                file >> state.x >> state.y
                    >> state.width >> state.height;
            }

            // Reject invalid or corrupted dimensions.
            if (state.width < 320 || state.height < 240)
                return {};

            return state;
        }

        void SaveWindowState(GLFWwindow* window)
        {
            WindowState state;

            glfwGetWindowPos(window, &state.x, &state.y);
            glfwGetWindowSize(window, &state.width, &state.height);

            std::ofstream file(window_state_file);
            if (file)
            {
                file << state.x << ' ' << state.y << ' '
                    << state.width << ' ' << state.height << '\n';
            }
        }
    }


    bool Window::Initialize()
    {
        if (!glfwInit())
            return false;

        const auto state = LoadWindowState();


        handle_ = glfwCreateWindow(
            state.width,
            state.height,
            "Chess Client",
            nullptr, nullptr
        );

        if (!handle_)
        {
            glfwTerminate();
            return false;
        }

        glfwSetWindowPos(handle_, state.x, state.y);

        glfwMakeContextCurrent(handle_);

        if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
        {
            // Other stuff is cleaned up in destructor
            return false;
        }

        glfwSwapInterval(1);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();

        // Content scaling
        constexpr float font_size = 28.0;
        const float window_device_pixel_ratio = GetWindowDevicePixelRatio();
        const float scaled_font_size = font_size * window_device_pixel_ratio;

        io.Fonts->AddFontFromFileTTF(
            paths::FontInter.string().c_str(),
            scaled_font_size);

        io.FontGlobalScale = 1.f / window_device_pixel_ratio;

        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;


        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(handle_, true);
        ImGui_ImplOpenGL3_Init("#version 330");

        return true;
    }

    Window::~Window()
    {
        if (handle_)
        {
            SaveWindowState(handle_);

            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();

            glfwDestroyWindow(handle_);
            glfwTerminate();
        }
    }

    bool Window::ShouldClose() const
    {
        return glfwWindowShouldClose(handle_);
    }

    void Window::PollEvents()
    {
        glfwPollEvents();
    }

    void Window::BeginFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Window::EndFrame()
    {
        ImGui::Render();

        int width, height;
        glfwGetFramebufferSize(handle_, &width, &height);

        glViewport(0, 0, width, height);
        glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(handle_);
    }

    /// @brief Returns the DPI scaling factor.
    ///
    /// The implementation is questionable to avoid
    // pulling Windows headers to the project but
    /// we ignore it as of now.
    float Window::GetDpiScale() const
    {
        float x_scale, y_scale;

        glfwGetWindowContentScale(
            this->handle_,
            &x_scale,
            &y_scale);

        return x_scale;
    }

    float Window::GetWindowDevicePixelRatio() const
    {
        int windowWidth, windowHeight;
        int framebufferWidth, framebufferHeight;

        glfwGetWindowSize(this->handle_, &windowWidth, &windowHeight);
        glfwGetFramebufferSize(this->handle_, &framebufferWidth, &framebufferHeight);

        if (windowWidth <= 0 || windowHeight <= 0)
            return 1.0f;

        return static_cast<float>(framebufferWidth) / windowWidth;
    }
}
