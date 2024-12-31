#ifndef IMGUIWINDOW_H
#define IMGUIWINDOW_H

#include "Window/Window.h"
#include "OpenGL/Renderer.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"

namespace viz{
    class GUIWindow : public Window{
        public:
            GUIWindow() = delete;
            GUIWindow(const char* name, ImGuiWindowFlags windowFlags = 0) : Window(name), m_windowFlags(windowFlags) {}

            virtual void init() = 0;
            void render() override{
                preFrame();
                ImGui::Begin(m_name.c_str(), &m_isOpen, m_windowFlags);
                onRender();
                ImGui::End();
            }

            inline void setOpen(bool isOpen) {m_isOpen = isOpen;}

            virtual ~GUIWindow() = default;
        protected:
            virtual void onRender() = 0;
            virtual void preFrame() = 0;

            bool m_isOpen = true;
            ImGuiWindowFlags m_windowFlags = ImGuiWindowFlags_None;
    };

    class Dockspace : public GUIWindow{
        public:
            Dockspace() = delete;
            Dockspace(const char* name, ImGuiWindowFlags windowFlags = 0) : GUIWindow(name, windowFlags) {}

            void init() override;

            virtual ~Dockspace() = default;
        private:
            void onRender() override;
            void preFrame() override;
    };

    class SceneWindow : public GUIWindow {
        public:
            SceneWindow() = delete;
            SceneWindow(const char* name, std::shared_ptr<Renderer> renderer, ImGuiWindowFlags windowFlags = 0) : GUIWindow(name, windowFlags),  m_renderer(renderer) {};

            void init() override {}

            void attachRenderer(std::shared_ptr<Renderer> renderer) {m_renderer = renderer;}
            virtual ~SceneWindow() = default;
        private:
            std::shared_ptr<Renderer> m_renderer;

            void onRender() override;
            void preFrame() override {}
    };

    class ManagerWindow : public GUIWindow{
        public:
            float x = 0, y = 0, z = 0;
            float color[4] = {0, 0, 0, 1};

            ManagerWindow() = default;
            ManagerWindow(const char* name) : GUIWindow(name) {}

            void init() override {}

            virtual ~ManagerWindow() = default;
        private:
            void onRender() override;
            void preFrame() override {}
    };

    class ConsoleWindow : public GUIWindow{
        public:
            ConsoleWindow() = default;
            ConsoleWindow(const char* name) : GUIWindow(name) {}

            void init() {}

            virtual ~ConsoleWindow() = default;
        private:
            void onRender() override {}
            void preFrame() override {}
    };
}

#endif