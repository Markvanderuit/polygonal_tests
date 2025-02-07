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

#include <cstdlib>
#include <exception>
#include <core/imgui.hpp>
#include <core/math.hpp>
#include <core/mesh.hpp>
#include <core/utility.hpp>
#include <small_gl/array.hpp>
#include <small_gl/buffer.hpp>
#include <small_gl/framebuffer.hpp>
#include <small_gl/texture.hpp>
#include <small_gl/utility.hpp>
#include <small_gl/window.hpp>

namespace prg {
  // List of default flags to have a simple, antialiased, movable/sizeable window
  constexpr auto window_flags 
    = gl::WindowFlags::eVisible   | gl::WindowFlags::eFocused 
    | gl::WindowFlags::eDecorated | gl::WindowFlags::eResizable 
    | gl::WindowFlags::eMSAA prg_debug_insert(| gl::WindowFlags::eDebug); 
  
  // Initial polygonal data layout
  std::vector<eig::Vector2f> verts = {
    eig::Array2f { .25, .5 },
    eig::Array2f { .5, .25 },
    eig::Array2f { .75, .5 },
    eig::Array2f { .5, .75 }
  };
  std::vector<eig::AlArray3f> colrs = {
    eig::AlArray3f { 1, 0, 0 },
    eig::AlArray3f { 0, 1, 0 },
    eig::AlArray3f { 0, 0, 1 },
    eig::AlArray3f { 1, 1, 0 }
  };


  // Method settings flags
  enum class Method : uint {
    eBarycentric     = 0,
    eMeanValueCoords = 1
  };

  // Unnamed settings object, pushed to shaders through uniform data
  struct {
    alignas(16) eig::Matrix4f projection;
    alignas(16) bool          draw_lines  = false;
    alignas(16) Method        draw_method = Method::eBarycentric;
  } settings;
  
  // Draw objects
  gl::Window  window;
  gl::Array   default_array; // Empty VAO as we'll be doing vertex pulling
  gl::Program polygon_program;
  gl::Program mvc_program;
  gl::Program bary_program;
  
  // Vertex selection/editing data 
  std::optional<uint> vert_mouseover;
  std::optional<uint> vert_selected;
  ImGui::Gizmo2D      vert_gizmo;

  void init_mean_value_coordinates() {
    // Load VAO; leave empty for now and just do vertex pulling
    default_array = {{}};

    // Load shader programs
    polygon_program = {{ .type       = gl::ShaderType::eVertex,
                         .glsl_path  = "shaders/draw_polygon.vert",
                         .cross_path = "shaders/draw_polygon.vert.json" },
                       { .type       = gl::ShaderType::eFragment,
                         .glsl_path  = "shaders/draw_polygon.frag",
                         .cross_path = "shaders/draw_polygon.frag.json" }};
    mvc_program = {{ .type       = gl::ShaderType::eVertex,
                     .glsl_path  = "shaders/draw_mvc.vert",
                     .cross_path = "shaders/draw_mvc.vert.json" },
                   { .type       = gl::ShaderType::eFragment,
                     .glsl_path  = "shaders/draw_mvc.frag",
                     .cross_path = "shaders/draw_mvc.frag.json" }};
    bary_program = {{ .type       = gl::ShaderType::eVertex,
                      .glsl_path  = "shaders/draw_bary.vert",
                      .cross_path = "shaders/draw_bary.vert.json" },
                    { .type       = gl::ShaderType::eFragment,
                      .glsl_path  = "shaders/draw_bary.frag",
                      .cross_path = "shaders/draw_bary.frag.json" }};
  }

  void update_mean_value_coordinates() {
    const auto &io          = ImGui::GetIO();
    eig::Vector2f mouse_pos = io.MousePos;

    // Spawn small settings/vertex config window
    if (ImGui::Begin("ImGui")) {
      ImGui::SeparatorText("Settings");

      bool is_mvc = (settings.draw_method == Method::eMeanValueCoords);
      ImGui::Checkbox("Draw mean value coords", &is_mvc);
      if (is_mvc)
        ImGui::Checkbox("Draw grid lines", &settings.draw_lines);
      settings.draw_method = is_mvc ? Method::eMeanValueCoords : Method::eBarycentric;
      
      ImGui::SeparatorText("Vertices");

      if (ImGui::Button("Add vertex")) {
        // Search for longest edge
        float edge_l = (verts[1] - verts[0]).norm();
        uint  vert_i = 0;
        for (uint i = 0; i < verts.size(); ++i) {
          float edge_l_ = (verts[(i + 1) % verts.size()] - verts[i]).norm();
          guard_continue(edge_l_ > edge_l);
          edge_l = edge_l_;
          vert_i = i;
        }
        fmt::print("{}\n", vert_i);
        
        // Generate spliting vertex on longest edge
        auto vert = (.5f * verts[vert_i].array() + .5f * verts[(vert_i + 1) % verts.size()].array()).eval();
        auto colr = (.5f * colrs[vert_i].array() + .5f * colrs[(vert_i + 1) % verts.size()].array()).eval();
        
        // Insert splitting vertex in between vertices of longest edge
        verts.insert(verts.begin() + vert_i + 1, vert);
        colrs.insert(colrs.begin() + vert_i + 1, colr);
      }
      
      // List of vertex color data
      if (ImGui::BeginTable("'##data_table", 3, ImGuiTableFlags_SizingStretchProp)) {
        // Setup table header
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0); ImGui::Text("Position");
        ImGui::TableSetColumnIndex(1); ImGui::Text("Color");

        for (uint i = 0; i < verts.size(); ++i) {
          ImGui::PushID(i);
          ImGui::TableNextRow();

          // Position column
          ImGui::TableSetColumnIndex(0);
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          ImGui::DragFloat2("##data_vert", verts[i].data(), .05f);

          // Color column
          ImGui::TableSetColumnIndex(1);
          ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
          ImGui::ColorEdit3("##data_colr", colrs[i].data(), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_InputRGB);

          ImGui::TableSetColumnIndex(2);
          if (ImGui::Button("X")) {
          // Delete button (exits mesh early)
            verts.erase(verts.begin() + i);
            colrs.erase(colrs.begin() + i);
            ImGui::PopID();
            break;
          }

          ImGui::PopID();
        }
        ImGui::EndTable();
      }
    } 
    ImGui::End();

    // Handle gizmo input to move vertices
    {
      // Projection matrix
      float aspect = static_cast<float>(window.framebuffer_size().x())
                  / static_cast<float>(window.framebuffer_size().y());
      auto proj = eig::ortho(-aspect, aspect, -1.f, 1.f, -1.f, 1.f);

      // Iterate polygon vertices; find closest mouseover candidate
      vert_mouseover = { };
      for (uint i = 0; i < verts.size(); ++i) {
        auto v = (eig::Vector3f() << verts[i].array() * 2.f - 1.f, 0).finished();
        auto p = eig::world_to_window_space(v, proj, { 0, 0 }, window.window_size().cast<float>().eval());
        guard_continue((p - mouse_pos).norm() <= 8.f);
        vert_mouseover = i;
      }

      // On mouseclick and no near vertex/gizmo, deselect and kill gizmo
      if (io.MouseClicked[0] && !vert_gizmo.is_over() && !vert_mouseover) {
        vert_selected = { };
        vert_gizmo.set_active(false);
      }

      // On mouseclick and near vertex and no gizmo, set as selected
      if (io.MouseClicked[0] && vert_mouseover) {
        vert_selected = vert_mouseover;
      }

      // Register gizmo use start, do nothing else
      guard(vert_selected);
      auto vert = verts[*vert_selected];
      eig::Affine3f trf_init(eig::Translation3f((eig::Array3f() << vert, 0).finished()));
      if (vert_gizmo.begin_delta(window, trf_init)) { /* ... */ }

      // Register continuous gizmo use; apply transform to vertex
      if (auto [active, delta] = vert_gizmo.eval_delta(); active) {
        auto &vert = verts[*vert_selected];
        vert = (delta * (eig::Vector3f() << vert, 0).finished()).head<2>();      
      }

      // Register gizmo use end; do nothing else
      if (vert_gizmo.end_delta()) { /* ... */ }
    }
  }

  void draw_mean_value_coordinates() {
    // Triangulate the polygon on the fly; not a good or stable triangulation, but whatever
    auto elems = triangulate_polygon(cnt_span<const eig::Vector2f>(verts));

    // Generate aligned block of color data
    std::vector<eig::AlArray3f> colrs_aligned(range_iter(colrs));
    
    // Update projection matrix
    float aspect = static_cast<float>(window.framebuffer_size().x())
                 / static_cast<float>(window.framebuffer_size().y());
    settings.projection = eig::ortho(-aspect, aspect, -1.f, 1.f, -1.f, 1.f).matrix();

    // Push fresh vertex/element/color/settings data to new buffers;
    // we're eating the cost of per-frame buffer creation for simplicity
    gl::Buffer polygon_elems   = {{ .data = cnt_span<const std::byte>(elems)         }};
    gl::Buffer polygon_verts   = {{ .data = cnt_span<const std::byte>(verts)         }};
    gl::Buffer polygon_colrs   = {{ .data = cnt_span<const std::byte>(colrs_aligned) }};
    gl::Buffer settings_buffer = {{ .data = obj_span<const std::byte>(settings)      }};

    // Declare fresh VAO assembling polygon buffers;
    // we're eating the cost of per-frame VAO creation for simplicity
    gl::Array polygon_array = {{
      .buffers  = {{ .buffer = &polygon_verts, .index = 0, .stride = sizeof(eig::Vector2f)  },
                   { .buffer = &polygon_colrs, .index = 1, .stride = sizeof(eig::Vector4f)  }},
      .attribs  = {{ .attrib_index = 0, .buffer_index = 0, .size = gl::VertexAttribSize::e2 },
                   { .attrib_index = 1, .buffer_index = 1, .size = gl::VertexAttribSize::e3 }},
      .elements = &polygon_elems
    }};

    // Set draw state; we'll be drawing to the default framebuffer directly,
    // no funny business whatsoever
    gl::state::set(gl::DrawCapability::eMSAA,       true);
    gl::state::set(gl::DrawCapability::eLineSmooth, true);
    gl::state::set(gl::DrawCapability::eCullOp,     false);
    gl::state::set(gl::DrawCapability::eDepthClamp, false);
    gl::state::set(gl::DrawCapability::eDepthTest,  false);
    gl::state::set_viewport(window.framebuffer_size());
    gl::state::set_line_width(2.f);

    // Draw fullscreen quad, which generates the MVC background
    if (settings.draw_method == Method::eBarycentric) {
      // Bind relevant resources using program names
      bary_program.bind("b_buffer_settings", settings_buffer);

      // Submit draw info
      gl::dispatch_draw({ 
        .type             = gl::PrimitiveType::eTriangles, 
        .vertex_count     = static_cast<uint>(elems.size()) * 3,
        .draw_op          = gl::DrawOp::eFill,
        .bindable_array   = &polygon_array,
        .bindable_program = &bary_program
      });
    } else if (settings.draw_method == Method::eMeanValueCoords) {
      // Bind relevant resources using program names
      mvc_program.bind("b_buffer_verts",    polygon_verts);
      mvc_program.bind("b_buffer_colrs",    polygon_colrs);
      mvc_program.bind("b_buffer_settings", settings_buffer);

      // Submit draw info
      gl::dispatch_draw({ 
        .type             = gl::PrimitiveType::eTriangleStrip, 
        .vertex_count     = static_cast<uint>(elems.size()) * 3,
        .draw_op          = gl::DrawOp::eFill,
        .bindable_array   = &polygon_array,
        .bindable_program = &mvc_program
      });
    } else {
      /* ... */
    }

    // Draw polygon lines over background
    {
      // Bind relevant resources using program names
      polygon_program.bind("b_buffer_settings", settings_buffer);

      // Submit draw info
      gl::dispatch_draw({ 
        .type             = gl::PrimitiveType::eTriangles, 
        .vertex_count     = static_cast<uint>(elems.size()) * 3,
        .draw_op          = gl::DrawOp::eLine,
        .bindable_array   = &polygon_array,
        .bindable_program = &polygon_program,
      });
    }
  }

  // Application main code
  void create_window_loop() {
    // Spawn OpenGL context, GLFW window
    window = {{
      .size  = { 1024, 768 },
      .title = "Mean value coordinates test",
      .flags = window_flags
    }};

    // Enable OpenGL debug callback messages, if requested
    if constexpr (prg_enable_debug) {
      gl::debug::enable_messages(gl::DebugMessageSeverity::eLow, gl::DebugMessageTypeFlags::eAll);
      gl::debug::insert_message("OpenGL messages enabled", gl::DebugMessageSeverity::eLow);
    }

    // Setup ImGui
    ImGui::Initialize(window);

    // Init components for draw
    init_mean_value_coordinates();

    while (!window.should_close()) { 
      window.poll_events();
    // Primary window mesh
      ImGui::BeginFrame();
      // First window mesh components

      // Get handle to default FBO and clear frame
      auto default_framebuffer = gl::Framebuffer::make_default();
      default_framebuffer.clear(gl::FramebufferType::eColor, eig::Array4f(0, 0, 0, 1));

      // Primary code components go here 
      update_mean_value_coordinates();
      draw_mean_value_coordinates();

      ImGui::DrawFrame();
      window.swap_buffers();
      // Last window mesh components
    }

    // Tear down ImGui before window destruction
    ImGui::Destroy();
  }
} // namespace prg

// Application entry point
int main() {
  try {
    prg::create_window_loop();
  } catch (const std::exception &e) {
    fmt::print(stderr, "{}\n", e.what());
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}