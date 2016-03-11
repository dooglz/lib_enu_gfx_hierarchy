#include <graphics_framework.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

using namespace std;
using namespace graphics_framework;
using namespace glm;

mesh m;
effect eff;
target_camera cam;

#define NUM_RINGS 3

static mesh rings[NUM_RINGS];
static mesh cubes[NUM_RINGS];
bool load_content() {
  // Note: I've hacked graphics library to output weird colours from meshbuilder for clarity.
  for (size_t i = 0; i < NUM_RINGS; ++i) {
    rings[i] = mesh(geometry_builder::create_torus(20, 20, 0.5f, NUM_RINGS - i));
    rings[i].get_material().set_diffuse(vec4((i % 3) / 3.0f, ((i + 1) % 3) / 3.0f, ((i + 2) % 3) / 3.0f, 1.0f));

    cubes[i] = mesh(geometry_builder::create_box());
    cubes[i].get_transform().translate(vec3(0, i - 2.0f, 0));
    cubes[i].get_material().set_diffuse(rings[i].get_material().get_diffuse());
  }

  // ***********************
  // Create mesh object here
  // ***********************

  // Load in shaders
  eff.add_shader("basic.vert",        // filename
                 GL_VERTEX_SHADER);   // type
  eff.add_shader("basic.frag",        // filename
                 GL_FRAGMENT_SHADER); // type
  // Build effect
  eff.build();

  // Set camera properties
  cam.set_position(vec3(0.0f, 0.0f, 20.0f));
  cam.set_target(vec3(0.0f, 0.0f, 0.0f));
  auto aspect = static_cast<float>(renderer::get_screen_width()) / static_cast<float>(renderer::get_screen_height());
  cam.set_projection(quarter_pi<float>(), aspect, 2.414f, 1000.0f);
  return true;
}

bool update(float delta_time) {

  for (unsigned int i = 0; i < NUM_RINGS; i++) {
    glm::vec3 rot;
    switch (i % 3) {
    case (0):
      rot = glm::vec3(0, 0, 1);
      break;
    case (2):
      rot = glm::vec3(0, 0, -1);
      break;
    case (1):
      rot = glm::vec3(1, 0, 0);
      break;
    }
    rings[i].get_transform().rotate(delta_time * 0.3f * rot);
  }

  static float runtime = 0;
  runtime += delta_time;

  // the center parent cube
  cubes[2].get_transform().rotate(delta_time * 0.3f * vec3(0, 0, 1));
  // notice I'm setting the position directly, not using translate(),
  // which would add to the existing position
  cubes[2].get_transform().position = (vec3((sin(runtime) * 2.0f) - 1.0f, 0, 0));

  // midle cube
  cubes[1].get_transform().rotate(delta_time * 0.3f * vec3(0, 1, 0));

  // ouermost cube
  cubes[0].get_transform().rotate(delta_time * 0.3f * vec3(1, 0, 0));

  // Update the camera
  cam.update(delta_time);
  return true;
}

bool render() {
  // Bind effect
  renderer::bind(eff);
  // *************************************
  // Get the model transform from the mesh
  //**************************************

  // Create MVP matrix
  auto V = cam.get_view();
  auto P = cam.get_projection();
  auto shift = glm::translate(vec3(6.0f, -4.0f, 0));

  // Gimbal on the right = No parenting
  for (auto &e : rings) {
    mat4 M = e.get_transform().get_transform_matrix();
    auto MVP = P * V * shift * M;
    glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));

    glUniform3fv(eff.get_uniform_location("overrideColour"), 1, &e.get_material().get_diffuse()[0]);
    renderer::render(e);
  }

  shift = glm::translate(vec3(6.0f, 4.0f, 0));
  for (auto &e : cubes) {
    mat4 M = e.get_transform().get_transform_matrix();
    auto MVP = P * V * shift * M;
    glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));

    glUniform3fv(eff.get_uniform_location("overrideColour"), 1, &e.get_material().get_diffuse()[0]);
    renderer::render(e);
  }

  // Gimbal on the left = parenting
  shift = glm::translate(vec3(-6.0f, -4.0f, 0));
  for (size_t i = 0; i < NUM_RINGS; ++i) {
    mat4 M = rings[i].get_transform().get_transform_matrix();

    // i'm not using pointers for parenting, just walking up the array.
    // so ring 0 is a child of ring[1], and ring[1] is a child of ring[2]
    for (size_t j = i + 1; j < NUM_RINGS; ++j) {
      // super slow,as we have to calucalte this matrix loads of times.
      // The best way is to have some logic inside the get_transform_matrix() function
      // to only recalulate if something  has changed.
      // You have to re-write this funciton anyway for the courseowrk it seems.
      M = rings[j].get_transform().get_transform_matrix() * M;
    }
    auto MVP = P * V * shift * M;
    glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));

    glUniform3fv(eff.get_uniform_location("overrideColour"), 1, &rings[i].get_material().get_diffuse()[0]);
    renderer::render(rings[i]);
  }

  // Cubes on the left = parenting

  shift = glm::translate(vec3(-6.0f, 4.0f, 0));
  for (size_t i = 0; i < NUM_RINGS; ++i) {
    mat4 M = cubes[i].get_transform().get_transform_matrix();
    for (size_t j = i + 1; j < NUM_RINGS; ++j) {
      M = cubes[j].get_transform().get_transform_matrix() * M;
    }
    auto MVP = P * V * shift * M;
    glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));

    glUniform3fv(eff.get_uniform_location("overrideColour"), 1, &cubes[i].get_material().get_diffuse()[0]);
    renderer::render(cubes[i]);
  }

  return true;
}

void main() {
  // Create application
  app application;
  // Set load content, update and render methods
  application.set_load_content(load_content);
  application.set_update(update);
  application.set_render(render);
  // Run application
  application.run();
}