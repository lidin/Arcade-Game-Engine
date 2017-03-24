//
//  main.cpp
//  Arcade Game Engine
//

#include "core.hpp"
#include <glm/gtx/transform.hpp>

const Event DidPressKey("DidPressKey");
const Event DidReleaseKey("DidReleaseKey");

class CubeInputComponent
  : public InputComponent
{
  
public:
  void update(Core & core)
  {
    vec3 localRight(1.0f, 0.0f, 0.0f);
    vec3 localUp(0.0f, 1.0f, 0.0f);
    float angle = 3.0f * core.deltaTime();
    
    Core::KeyStatus keys;
    core.keyStatus(keys);
    
    if (keys.up)    entity()->rotate(-angle, localRight);
    if (keys.down)  entity()->rotate(angle,  localRight);
    if (keys.left)  entity()->rotate(-angle, localUp);
    if (keys.right) entity()->rotate(angle,  localUp);
  }
  
};

class CubeGraphicsComponent
  : public GraphicsComponent
{
  
public:
  CubeGraphicsComponent()
    : GraphicsComponent()
  {
    vector<vec3> positions {
      {-0.5f, -0.5f, -0.5f},
      { 0.5f, -0.5f, -0.5f},
      {-0.5f,  0.5f, -0.5f},
      { 0.5f,  0.5f, -0.5f},
      {-0.5f, -0.5f,  0.5f},
      { 0.5f, -0.5f,  0.5f},
      {-0.5f,  0.5f,  0.5f},
      { 0.5f,  0.5f,  0.5f}
    };
    vector<vec3> colors {
      {0.0f, 0.0f, 0.0f},
      {1.0f, 0.0f, 0.0f},
      {0.0f, 1.0f, 0.0f},
      {1.0f, 1.0f, 0.0f},
      {0.0f, 0.0f, 1.0f},
      {1.0f, 0.0f, 1.0f},
      {0.0f, 1.0f, 1.0f},
      {1.0f, 1.0f, 1.0f}
    };
    vector<int> indices {
      0, 6, 2,
      0, 4, 6,
      1, 3, 7,
      1, 7, 5,
      2, 1, 0,
      2, 3, 1,
      3, 6, 7,
      3, 2, 6,
      4, 0, 1,
      4, 1, 5,
      5, 6, 4,
      5, 7, 6
    };
    
    attachMesh(positions, colors, indices);
    attachShader("shaders/simple.vert", "shaders/simple.frag");
  }
  
};

int main(int argc, char *  argv[])
{
  Core core;
  CoreOptions options {"Demo", 800, 700};
  core.pBackgroundColor().r = 0.2f;
  core.pBackgroundColor().g = 0.2f;
  core.pBackgroundColor().b = 0.2f;
  
  Entity * cube = core.createEntity("cube");
  cube->translate(0.0f, 0.0f, -2.5f);
  cube->pInput(new CubeInputComponent);
  cube->pGraphics(new CubeGraphicsComponent);
  
  if (core.init(options))
  {
    while (core.update());
    core.destroy();
  }
  return 0;
}
