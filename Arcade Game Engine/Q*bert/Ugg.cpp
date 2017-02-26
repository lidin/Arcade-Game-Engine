//
//  Ugg.cpp
//  Game Engine
//

#include "Ugg.hpp"


//
// MARK: - UggInputComponent
//

// MARK: Member functions

ControllerDirection UggInputComponent::update_direction(Core & core)
{
  return arc4random_uniform(2)*2;
}

double UggInputComponent::animation_ending_delay()
{
  return 0.2;
};

vector<pair<int, int>> UggInputComponent::board_position_changes()
{
  return {
    {-1, -1},
    {      },
    { 0, -1},
    {      }
  };
}

//
// MARK: - UggAnimationComponent
//

// MARK: Member functions

vector<Vector2> UggAnimationComponent::end_points()
{
  return {
    {-16, -24},
    {        },
    {-32,   0},
    {        }
  };
}

double UggAnimationComponent::animation_speed() { return 0.7; };


//
// MARK: - UggPhysicsComponent
//

// MARK: Member functions

UggPhysicsComponent::UggPhysicsComponent()
  : ControllerPhysicsComponent()
{
  this->gravity({-1.417, -0.818});
}

void UggPhysicsComponent::init(Entity * entity)
{
  ControllerPhysicsComponent::init(entity);
  
  auto did_move_out_of_view = [entity](Event)
  {
    entity->enabled(false);
    entity->reset();
  };
  
  NotificationCenter::observe(did_move_out_of_view, DidMoveOutOfView, this);
}


//
// MARK: - Ugg
//

Ugg::Ugg()
  : Controller("enemy_ugg", default_order())
{
  addInput(new UggInputComponent());
  addAnimation(new UggAnimationComponent());
  addPhysics(new UggPhysicsComponent());
  addGraphics(new UggGraphicsComponent());
}

void Ugg::reset()
{
  Controller::reset();
  
  enabled(false);
  board_position(default_board_position());
  order(default_order());
  direction(default_direction());
  
  core()->createEffectiveTimer(arc4random_uniform(3)+2, [this]
                               {
                                 enabled(true);
                               });
  
  const Dimension2 view_dimensions = core()->view_dimensions();
  moveTo(view_dimensions.x/2 + 102, view_dimensions.y-30);
}

string Ugg::prefix_standing()
{
  return "enemy_ugg_standing";
}

string Ugg::prefix_jumping()
{
  return "enemy_ugg_jumping";
}

int Ugg::direction_mask()
{
  return 0b1010;
}

pair<int, int> Ugg::default_board_position()
{
  return {6, 6};
}

int Ugg::default_order()
{
  return 91;
}

ControllerDirection Ugg::default_direction()
{
  return UP;
}
