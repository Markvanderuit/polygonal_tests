// Copyright (c) 2025 Mark van de Ruit, Delft University of Technology

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <core/imgui.hpp>
#include <core/utility.hpp>
#include <small_gl/window.hpp>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

namespace ImGui {
  void Initialize(const gl::Window &window) {
    // Initialize ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // Pass context to ImGuizmo as they are separate libraries
    ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());

    auto &io = ImGui::GetIO();

    // Enable docking mode
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    // Handle dpi scaling
    ImGui::GetStyle().ScaleAllSizes(window.content_scale());

    // Initialize ImGui platform specific bindings
    ImGui_ImplOpenGL3_Init();
    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow *) window.object(), true);
  }

  void Destroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }

  void BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::SetOrthographic(true);
    ImGuizmo::BeginFrame();
  }

  void DrawFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }
  
  bool Gizmo2D::begin_delta(const gl::Window &window, trf init_trf) {
    // Reset internal state
    if (!m_is_active) {
      m_init_trf  = init_trf;
      m_delta_trf = trf::Identity();
    }

    float aspect = static_cast<float>(window.window_size().x())
                 / static_cast<float>(window.window_size().y());
    fmt::print("aspect {}\n", aspect);

    // Specify ImGuizmo settings for current viewport and insert the gizmo
    ImGuizmo::SetRect(0, 0, window.window_size().x(), window.window_size().y());
    ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

    // Helper data
    Eigen::Affine3f view = trf::Identity();
    view = view * Eigen::Translation3f(Eigen::Array3f { -aspect, -1.f, 0.f });
    view = view * Eigen::Scaling(2.f * aspect, 2.f, 1.f);
    auto operation = ImGuizmo::OPERATION::TRANSLATE_X | ImGuizmo::OPERATION::TRANSLATE_Y;
    auto orthogrph = Eigen::ortho(-aspect, aspect, -1.f, 1.f, -1, 1);
    
    ImGuizmo::Manipulate(view.data(), orthogrph.data(), 
      operation, ImGuizmo::MODE::LOCAL, m_init_trf.data(), m_delta_trf.data());

    guard(!m_is_active && ImGuizmo::IsUsing(), false);
    m_is_active = true;
    return true;
  }

  std::pair<bool, Gizmo2D::trf> Gizmo2D::eval_delta() {
    guard(m_is_active && ImGuizmo::IsUsing(), {false, trf::Identity()});
    return { true, m_delta_trf };
  }

  bool Gizmo2D::end_delta() {
    guard(m_is_active && !ImGuizmo::IsUsing(), false);
    m_is_active = false;
    return true;
  }

  bool Gizmo2D::is_over() const {
    return ImGuizmo::IsOver();
  }
} // namespace imgui