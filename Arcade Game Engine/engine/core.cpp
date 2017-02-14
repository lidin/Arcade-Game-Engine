//
//  core.cpp
//  Game Engine
//

#include "core.hpp"
#include <fstream>

/********************************
 * Sprite
 ********************************/
Sprite::Sprite(SDL_Renderer * renderer, SDL_Texture * texture)
  : _renderer(renderer)
  , _texture(texture)
{}

void Sprite::destroy()
{
  SDL_DestroyTexture(_texture);
}

void Sprite::draw(int x, int y, int w, int h, int scale)
{
  SDL_Rect rect {x*scale, y*scale, w*scale, h*scale};
  SDL_RenderCopy(_renderer, _texture, nullptr, &rect);
}

/********************************
 * Notifier
 ********************************/
void Notifier::addObserver(Observer * observer, const Event & event)
{
  observers()[event].push_back(observer);
}

void Notifier::removeObserver(Observer * observer, Event * event)
{
  if (event)
  {
    auto observers_for_event = observers()[*event];
    for (auto i = 0; i < observers_for_event.size(); i++)
    {
      if (observer == observers_for_event[i])
      {
        observers()[*event].erase(observers_for_event.begin()+i);
        return;
      }
    }
  }
  else
  {
    for (auto pair : observers())
    {
      for (auto i = 0; i < pair.second.size(); i++)
      {
        if (observer == pair.second[i])
        {
          observers()[pair.first].erase(pair.second.begin()+i);
          return;
        }
      }
    }
  }
}

void Notifier::notify(Entity & entity, Event event)
{
  for (auto observer : observers()[event]) observer->onNotify(entity, event);
}


/********************************
 * World
 ********************************/
World::World()
{
  scale(1);
}

bool World::init(const char * title,
                 Dimension2 dimensions,
                 RGBAColor background_color)
{
  // initialize SDL
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
  {
    SDL_Log("SDL_Init: %s\n", SDL_GetError());
    return false;
  }
  
  // initialize SDL_image
  if (IMG_Init(IMG_INIT_PNG) < 0)
  {
    SDL_Log("IMG_Init: %s\n", IMG_GetError());
    return false;
  }
  
  // create window
  const int w_pos_x = dimensions.w < 0 ? SDL_WINDOWPOS_UNDEFINED : dimensions.w;
  const int w_pos_y = dimensions.h < 0 ? SDL_WINDOWPOS_UNDEFINED : dimensions.h;
  view_dimensions({dimensions.w, dimensions.h});
  window(SDL_CreateWindow(title,
                          w_pos_x,
                          w_pos_y,
                          dimensions.w*scale(),
                          dimensions.h*scale(),
                          SDL_WINDOW_SHOWN));
  if (window() == nullptr)
  {
    SDL_Log("SDL_CreateWindow: %s\n", SDL_GetError());
    return false;
  }
  
  // create renderer for window
  renderer(SDL_CreateRenderer(window(), -1, SDL_RENDERER_ACCELERATED));
  if (renderer() == nullptr)
  {
    SDL_Log("SDL_CreateRenderer: %s\n", SDL_GetError());
    return false;
  }
  
  // clear screen
  SDL_SetRenderDrawColor(renderer(),
                         background_color.r,
                         background_color.g,
                         background_color.b,
                         background_color.a);
  SDL_RenderClear(renderer());
  
  // initialize member properties
  _keys.up = _keys.down = _keys.left = _keys.right = _keys.fire = false;
  _prev_time = 0;
  
  // initialize entities
  for (auto entity : entities())
  {
    entity->init(this);
  }
  
  return true;
}

void World::destroy()
{
  SDL_DestroyRenderer(renderer());
  SDL_DestroyWindow(window());
  SDL_Quit();
  _initialized = false;
}

void World::addEntity(Entity * entity)
{
  entities().push_back(entity);
  if (_initialized) entity->init(this);
}

void World::removeEntity(Entity * entity)
{
  for (auto i = 0; i < entities().size(); i++)
  {
    if (entity == entities()[i])
    {
      entities().erase(entities().begin()+i);
      return;
    }
  }
}

bool World::update()
{
  // record time
  double start_time = getElapsedTime();
  delta_time(start_time - _prev_time);
  _prev_time = start_time;
  
  // check if initialized
  if (_initialized)
  {
    // check user input
    SDL_Event event;
    bool should_continue = true;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
      {
        should_continue = false;
        break;
      }
      if (event.type == SDL_KEYDOWN)
      {
        switch (event.key.keysym.sym)
        {
          case SDLK_UP:
            _keys.up = true;
            break;
          case SDLK_DOWN:
            _keys.down = true;
            break;
          case SDLK_LEFT:
            _keys.left = true;
            break;
          case SDLK_RIGHT:
            _keys.right = true;
            break;
          case SDLK_SPACE:
            _keys.fire = true;
            break;
        }
      }
      if (event.type == SDL_KEYUP)
      {
        switch (event.key.keysym.sym)
        {
          case SDLK_UP:
            _keys.up = false;
            break;
          case SDLK_DOWN:
            _keys.down = false;
            break;
          case SDLK_LEFT:
            _keys.left = false;
            break;
          case SDLK_RIGHT:
            _keys.right = false;
            break;
          case SDLK_SPACE:
            _keys.fire = false;
            break;
          case SDLK_ESCAPE:
          case SDLK_q:
            should_continue = false;
            break;
        }
      }
    }
  
    // update entities
    for (auto entity : entities()) (entity->update(*this));
    
    // clear screen
    SDL_RenderPresent(renderer());
    SDL_RenderClear(renderer());
    
    return should_continue;
  }
  _initialized = true;
  
  return true;
}

void World::getKeyStatus(World::KeyStatus & keys)
{
  keys.up    = this->_keys.up;
  keys.down  = this->_keys.down;
  keys.left  = this->_keys.left;
  keys.right = this->_keys.right;
  keys.fire  = this->_keys.fire;
}

double World::getElapsedTime()
{
  return SDL_GetTicks() / 1000.f;
}

/********************************
 * Entity
 ********************************/
Entity::Entity(InputComponent * input,
               AnimationComponent * animation,
               PhysicsComponent * physics,
               GraphicsComponent * graphics)
{
  this->input(input);
  this->animation(animation);
  this->physics(physics);
  this->graphics(graphics);
}

Entity::~Entity()
{
  if (input())     delete input();
  if (animation()) delete animation();
  if (physics())   delete physics();
  if (graphics())  delete graphics();
}

void Entity::init(World * owner)
{
  world(owner);
  if (input()) input()->init(this);
  if (animation()) animation()->init(this);
  if (physics()) physics()->init(this);
  if (graphics()) graphics()->init(this);
}

void Entity::update(World & world)
{
  if (input()) input()->update(world);
  if (animation()) animation()->update(world);
  if (physics()) physics()->update(world);
  if (graphics()) graphics()->update(world);
}

void Entity::moveTo(double x, double y)
{
  position().x = x;
  position().y = y;
}

void Entity::moveHorizontallyTo(double x)
{
  position().x = x;
}

void Entity::moveVerticallyTo(double y)
{
  position().y = y;
}

void Entity::moveBy(double dx, double dy)
{
  position().x += dx;
  position().y += dy;
}

void Entity::changeVelocityTo(double vx, double vy)
{
  velocity().x = vx;
  velocity().y = vy;
}

void Entity::changeHorizontalVelocityTo(double vx)
{
  velocity().x = vx;
}

void Entity::changeVerticalVelocityTo(double vy)
{
  velocity().y = vy;
}

void Entity::changeVelocityBy(double dvx, double dvy)
{
  velocity().x += dvx;
  velocity().y += dvy;
}


/********************************
 * Component
 ********************************/
void Component::init(Entity * owner)
{
  entity(owner);
}


/********************************
 * AnimationComponent
 ********************************/
AnimationComponent::AnimationComponent()
  : Component()
{
  animating(false);
}

bool AnimationComponent::loadAnimationFromFile(const char * filename,
                                               const char * animation_id,
                                               double duration)
{
  vector<Vector2> movements;
  ifstream file(filename);
  if (file.is_open())
  {
    double x, y;
    while (file >> x >> y)
    {
      movements.push_back({x, y});
    }
    if (movements.size() > 0)
    {
      _animation_paths[animation_id] = {movements, duration};
      return true;
    }
  }
  return false;
}

bool AnimationComponent::removeAnimation(const char * id)
{
  return _animation_paths.erase(id) == 1;
}

bool AnimationComponent::performAnimation(const char * id)
{
  if (!animating() && _animation_paths.find(id) != _animation_paths.end())
  {
    animating(true);
    _current_animation = _animation_paths[id];
    _current_movement_index = -1;
    _animation_start_time = entity()->world()->getElapsedTime();
    notify(*entity(), DidStartAnimating);
    return true;
  }
  return false;
}

void AnimationComponent::update(World & world)
{
  if (animating())
  {
    const double elapsed  = world.getElapsedTime() - _animation_start_time;
    const double fraction = elapsed / _current_animation.duration;
    const long max = _current_animation.movements.size();
    const long bound = fraction < 1 ? floor(fraction * max) + 1 : max;
    Vector2 total_movement;
    for (; _current_movement_index < bound; _current_movement_index++)
    {
      total_movement += _current_animation.movements[_current_movement_index];
    }
    entity()->moveBy(total_movement.x, total_movement.y);
    
    if (elapsed > _current_animation.duration)
    {
      animating(false);
      notify(*entity(), DidStopAnimating);
    }
  }
}


/********************************
 * GraphicsComponent
 ********************************/
GraphicsComponent::~GraphicsComponent()
{
  for (auto sprite : sprites())
  {
    sprite->destroy();
  }
}

void GraphicsComponent::initSprites(SDL_Renderer & renderer,
                                    vector<const char *> files,
                                    int current_sprite_index)
{
  for (auto file : files)
  {
    SDL_Surface * loaded_surface = IMG_Load(file);
    if (!loaded_surface)
    {
      SDL_Log("IMG_Load: %s\n", IMG_GetError());
    }
    else
    {
      SDL_Texture * player_texture =
      SDL_CreateTextureFromSurface(&renderer, loaded_surface);
      sprites().push_back(new Sprite(&renderer, player_texture));
      SDL_FreeSurface(loaded_surface);
    }
  }
  if (sprites().size() > 0) current_sprite(sprites()[current_sprite_index]);
}

void GraphicsComponent::offsetTo(int x, int y)
{
  bounds().pos.x = x;
  bounds().pos.y = y;
}

void GraphicsComponent::offsetBy(int dx, int dy)
{
  bounds().pos.x += dx;
  bounds().pos.y += dy;
}

void GraphicsComponent::resizeTo(int w, int h)
{
  bounds().dim.w = w;
  bounds().dim.h = h;
}

void GraphicsComponent::resizeBy(int dw, int dh)
{
  bounds().dim.w += dw;
  bounds().dim.h += dh;
}
