#include "renderer.hpp"

namespace GUI
{
    void RenderMenu()
    {
        ImGui::Begin("Menu");
        ImGui::Checkbox("Render Visuals", &Cheat::RenderVisuals);
        ImGui::End();
    }
}