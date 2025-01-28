#include "32blit.hpp"
#include "3d-demo.hpp"

#include "camera.hpp"
#include "model.hpp"
#include "render-3d.hpp"
#include "vec4.hpp"

#include "assets.hpp"

using namespace blit;
using namespace std;

static Surface *main_texture;
static Surface *boots_texture;
static Surface *eye_texture;
static Surface *mouth_texture;
static Surface *handopen_texture;
static Surface *bracelet_texture;
static Surface *glove_texture;
static Surface *sheath_texture;

static bool draw_models = true;
static bool draw_lines = false;
static bool draw_vertices = false;

static ModelShaderParams shader_params;
static Model link_model(link_obj);
static bool mesh_visible[10];

static Render3D r3d;

static Camera cam{
  Vec3(0.0f, 0.0f, -2.3f),
  Vec3(0.0f, 0.0f, -3.0f),
  Vec3(0.0f, 1.0f, 0.0f)
};

static ScreenMode screen_mode = hires;

void init() {
  set_screen_mode(hires, PixelFormat::RGB565);

  r3d.set_projection(Camera::perspective_matrix(0.1f, 10.0f, blit::pi / 4.0f, float(screen.bounds.w) / screen.bounds.h));

  r3d.set_shader_params(&shader_params);

  main_texture = Surface::load(main_texture_packed);
  boots_texture = Surface::load(boots_texture_packed);
  eye_texture = Surface::load(eye_texture_packed);
  mouth_texture = Surface::load(mouth_texture_packed);
  handopen_texture = Surface::load(handopen_texture_packed);
  bracelet_texture = Surface::load(bracelet_texture_packed);
  glove_texture = Surface::load(glove_texture_packed);
  sheath_texture = Surface::load(sheath_texture_packed);

  r3d.set_texture(sheath_texture, 0);
  r3d.set_texture(main_texture, 1);
  r3d.set_texture(boots_texture, 2);
  r3d.set_texture(eye_texture, 3);
  r3d.set_texture(mouth_texture, 4);
  r3d.set_texture(handopen_texture, 5);
  r3d.set_texture(bracelet_texture, 6);
  r3d.set_texture(glove_texture, 7);

  for(auto &visible : mesh_visible)
    visible = true;

  mesh_visible[0] = false; // hide glitched eyebrows TODO: fix?

  mesh_visible[9] = false; // hide links "second" pair of hands...
}

void render(uint32_t time_ms) {

  uint32_t ms_start = now();

  Vec3 light(1.0f, 0.0f, 0.0f);

  // link object

  // world to screen transformation matrix
  Mat4 camera_transformation = cam.get_look_matrix();

  // object transformation matrix
  Mat4 object_transformation = Mat4::identity();
  object_transformation *= Mat4::translation(Vec3(0.0f, 0.0f, -3.0f));

  // rotate around y axis based on time
  object_transformation *= Mat4::rotation(time_ms / 20.0f, Vec3(0.0f, 1.0f, 0.0f));
  //object_transformation *= Mat4::rotation(joystick.x * 45.0f, Vec3(0.0f, 1.0f, 0.0f));

  // scale link to be 1 world unit wide
  float link_scale = 1.0f / 15.7f;
  //link_scale *= 10.0f;
  object_transformation *= Mat4::scale(Vec3(link_scale, link_scale, link_scale));
  // links feet are anchored at y = 0 so let's offset him by half his height
  // because it's easier to position him for testing when 0, 0, 0 is bang in the centre
  float y_offset = (16.05f) / 2.0f;
  object_transformation *= Mat4::translation(Vec3(0.0f, -y_offset, 0.0f));

  // combine transformations
  Mat4 transformation = camera_transformation * object_transformation;

  r3d.set_model_view(transformation);

  shader_params.light_direction = Vec3(camera_transformation * Vec4(light, 0.0f));
  shader_params.light_direction.normalize();

  // draw
  r3d.set_position_shader(fixed32_mvp_pos_shader);
  r3d.set_vertex_shader(model_lit_shader);

  if(draw_models) {
    for(unsigned i = 0; i < link_model.get_num_meshes(); i++) {
      if(mesh_visible[i])
        link_model.draw_mesh(i, r3d);
    }
  }

  auto vertex_count = r3d.get_transformed_vertex_count();

  // we can't mix filled and non-filled...
  r3d.set_fill_triangles(!draw_lines);

  r3d.set_clear_colour({20, 30, 40});
  r3d.rasterise();

  uint32_t ms_end = now();

  screen.pen = Pen(255, 255, 255);
  screen.text(std::to_string(vertex_count), minimal_font, Rect(2, screen.bounds.h - 33, 50, 10));

  // draw FPS meter*
  screen.pen = Pen(255, 255, 255);
  std::string fms = std::to_string(ms_end - ms_start);
  screen.text(fms, minimal_font, Rect(2, screen.bounds.h - 13, 50, 10));

  int block_size = 4;
  for (uint32_t i = 0; i < (ms_end - ms_start); i++) {
    screen.pen = Pen(i * 5, 255 - (i * 5), 0);
    screen.rectangle(Rect(i * (block_size + 1) + 1, screen.bounds.h - block_size - 1, block_size, block_size));
  }

  screen.watermark();
}

void update(uint32_t time) {
  auto cam_dir = (cam.look_at - cam.position);
  cam_dir.normalize();
  cam.position += cam_dir * joystick.y * 0.1f;

  if(buttons.pressed & Button::A) {
    mesh_visible[7] = !mesh_visible[7];
    mesh_visible[9] = !mesh_visible[9];
  }

  if(buttons.pressed & Button::B) {
    draw_models = !draw_models;
  }

  if(buttons.pressed & Button::X) {
    draw_lines = !draw_lines;
  }

  if(buttons.pressed & Button::Y) {
    draw_vertices = !draw_vertices;
  }

  if(buttons.pressed & Button::MENU) {
    screen_mode = screen_mode == hires ? lores : hires;
    set_screen_mode(screen_mode, PixelFormat::RGB565);
  }
}

