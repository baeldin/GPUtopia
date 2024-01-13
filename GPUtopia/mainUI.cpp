#include <stdio.h>
#include "imgui.h"
#include "imgui_internal.h"
#include <windows.h>
#include <cmath>
#include <vector>

#include "GL\glew.h"
#include "GLFW\glfw3.h"

//// threading stuff
//#include <thread>
//#include <future>
//#include <mutex>


struct color
{
    float r;
    float g;
    float b;
    float a = 1.;
};

// allow fout
#pragma warning(disable : 4996)

void prepTexture(GLuint& texture)
{
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
}


void makeTexture(GLuint& texture, const int sizeX, const int sizeY, std::vector<color>& imgData)
{
    prepTexture(texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, sizeX, sizeY, 0, GL_RGBA, GL_FLOAT, imgData.data());
}

void refreshTexture(GLuint& texture, const int sizeX, const int sizeY, std::vector<color>& imgData)
{
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, sizeX, sizeY, 0, GL_RGBA, GL_FLOAT, imgData.data());
}

void drawBogusImg(std::vector<color>& img, int width, int height)
{
    // some blue/green thing
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            img[y * width + x].g = (float)x / width;
            img[y * width + x].b = (float)y / height;
        }
    }
}

static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}


namespace MainUI
{
    using real = double;

    void RenderMainUI()
    {
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
        static bool showGradientEditor = false;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }
        else
        {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", nullptr, window_flags);
        if (!opt_padding)
            ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }


        /*if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Options"))
            {
                // Disabling fullscreen would allow the window to be moved to the front of other windows,
                // which we can't undo at the moment without finer window depth/z control.
                ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
                ImGui::MenuItem("Padding", NULL, &opt_padding);
                ImGui::Separator();

                if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoSplit; }
                if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
                if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode; }
                if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
                if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
                ImGui::Separator();

            }
            if (ImGui::BeginMenu("Windows"))
            {
                if (ImGui::MenuItem("Gradient Editor", ""))
                {
                    showGradientEditor = (showGradientEditor) ? false : true;
                }
            }

            ImGui::EndMenuBar();
        }*/
        ImGui::Begin("MainView");
        static bool needFrameBuffer = true;
        static bool needTexture = true;
        static bool needBogusImg = true;
        static GLuint frameBufferID = 0;
        static GLuint textureID = 0;
        static ImVec2 mainViewportSize = ImGui::GetContentRegionAvail();
        static std::vector<color> textureColors((int)(mainViewportSize.x * mainViewportSize.y));
        if (mainViewportSize.x != ImGui::GetContentRegionAvail().x ||
            mainViewportSize.y != ImGui::GetContentRegionAvail().y) {
            mainViewportSize = ImGui::GetContentRegionAvail();
            // prevent x and y from going negative, this can happen when dragging violently
            mainViewportSize.x < 1 ? mainViewportSize.x = 1 : mainViewportSize.x;
            mainViewportSize.y < 1 ? mainViewportSize.y = 1 : mainViewportSize.y;
            needTexture = true;
            textureColors.resize((int)(mainViewportSize.x * mainViewportSize.y));
        }
        glViewport(0, 0, mainViewportSize.x, mainViewportSize.y);
        // TODO: fails due to lack of context maybe???
        if (needFrameBuffer) {
            glGenFramebuffers(1, &frameBufferID);
            glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
            needFrameBuffer = false;
        }
        if (needBogusImg) {
            drawBogusImg(textureColors, mainViewportSize.x, mainViewportSize.y);
        }
        if (needTexture) {
            glDeleteTextures(1, &textureID);
            makeTexture(textureID, mainViewportSize.x, mainViewportSize.y, textureColors);
            //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
        }
        ImVec2 vpPos = ImGui::GetCursorScreenPos();
        ImGui::GetWindowDrawList()->AddImage(
            (void*)textureID,
            ImVec2(vpPos.x, vpPos.y),
            ImVec2(vpPos.x + mainViewportSize.x, vpPos.y + mainViewportSize.y),
            ImVec2(0, 1),
            ImVec2(1, 0));
        ImGui::End();
        
        ImGui::ShowDemoWindow();

    }
}