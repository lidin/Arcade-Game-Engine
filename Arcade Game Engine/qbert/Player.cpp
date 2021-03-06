//
//  Player.cpp
//  Game Engine
//

#include "Player.hpp"
#include "Board.hpp"
#include "HUD.hpp"


//
// MARK: - PlayerInputComponent
//

// MARK: Member functions

void PlayerInputComponent::init(Entity * entity)
{
  CharacterInputComponent::init(entity);
  
  auto did_clear_board = [this](Event) { _did_clear_board = true; };
  auto did_collide_with_enemy = [this, entity](Event event)
  {
    if (airborn())
    {
      auto character = (Character*)entity;
      character->board_position(character->previous_board_position());
      character->order(character->previous_order());
    }
  };

  auto physics = entity->physics();
  NotificationCenter::observe(did_clear_board, DidClearBoard);
  NotificationCenter::observe(did_collide_with_enemy,
                              DidCollideWithEnemy,
                              physics);
}

void PlayerInputComponent::reset()
{
  CharacterInputComponent::reset();
  
  _did_clear_board = false;
}

CharacterDirection PlayerInputComponent::update_direction(Core & core)
{
  if (!_did_clear_board)
  {
    Core::KeyStatus keys;
    core.keyStatus(keys);
    
    if (keys.up)
      return UP;
    if (keys.down)
      return DOWN;
    if (keys.left)
      return LEFT;
    if (keys.right)
      return RIGHT;
  }
  return NONE;
}

double PlayerInputComponent::animation_ending_delay()
{
  return 0.15;
}

vector<pair<int, int>> PlayerInputComponent::board_position_changes()
{
  return {
    {-1,  0},
    { 1,  0},
    {-1, -1},
    { 1,  1}
  };
}


//
// MARK: - PlayerAnimationComponent
//

// MARK: Member functions

vector<Vector2> PlayerAnimationComponent::end_points()
{
  return {
    { 16, -24},
    {-16,  24},
    {-16, -24},
    { 16,  24}
  };
}

double PlayerAnimationComponent::animation_speed() { return 0.3; }


//
// MARK: - PlayerAudioComponent
//

void PlayerAudioComponent::init(Entity * entity)
{
  AudioComponent::init(entity);
  
  _did_jump_off = false;
  
#ifdef __APPLE__
  synthesizer().load("synthesizer/land.synth");
  synthesizer().load("synthesizer/gibberish.synth");
  synthesizer().load("synthesizer/fall_off.synth");
#elif defined(_WIN32)
  synthesizer().load("synthesizer\\land.synth");
  synthesizer().load("synthesizer\\gibberish.synth");
  synthesizer().load("synthesizer\\fall_off.synth");
#endif

  
  auto did_jump_off           = [this](Event) { _did_jump_off = true; };
  auto did_collide_with_enemy = [this](Event)
  {
    playSound("gibberish", 0.66);
  };
  auto did_stop_animating = [this](Event)
  {
    if (_did_jump_off) playSound("fall_off", 1.5, 0, 1);
    else               playSound("land", 0.1);
  };
  
  auto player_input     = entity->input();
  auto player_physics   = entity->physics();
  auto player_animation = entity->animation();
  NotificationCenter::observe(did_jump_off,
                              DidJumpOff,
                              player_input);
  NotificationCenter::observe(did_collide_with_enemy,
                              DidCollideWithEnemy,
                              player_physics);
  NotificationCenter::observe(did_stop_animating,
                              DidStopAnimating,
                              player_animation);
}

void PlayerAudioComponent::reset()
{
  CharacterAudioComponent::reset();
  
  _did_jump_off = false;
}


//
// MARK: - PlayerPhysicsComponent
//

// MARK: Member functions

PlayerPhysicsComponent::PlayerPhysicsComponent()
  : CharacterPhysicsComponent()
{
  collision_bounds({7, 4, 2, 12});
}

void PlayerPhysicsComponent::init(Entity * entity)
{
  CharacterPhysicsComponent::init(entity);

  auto did_move_out_of_view = [entity](Event)
  {
    entity->core()->pause();
    entity->core()->reset(1.5);
  };
  
  NotificationCenter::observe(did_move_out_of_view, DidMoveOutOfView, this);
}

void PlayerPhysicsComponent::collision_with_block(Block * block)
{
  block->touch();
}

void PlayerPhysicsComponent::collision_with_entity(Entity * entity)
{
  string id = entity->id();
  if (id.compare(0, 5, "enemy") == 0)
  {
    NotificationCenter::notify(DidCollideWithEnemy, *this);
    entity->core()->pause();
    entity->core()->reset(1.5);
  }
}


//
// MARK: - Player
//

Player::Player(string id)
  : Character(id, 11)
{
  addInput(new PlayerInputComponent());
  addAnimation(new PlayerAnimationComponent());
  addPhysics(new PlayerPhysicsComponent());
  addAudio(new PlayerAudioComponent());
  addGraphics(new PlayerGraphicsComponent());
}

void Player::init(Core * core)
{
  Character::init(core);
  
  _should_revert = false;
  
  auto should_revert = [this](Event) { _should_revert = true; };
  
  NotificationCenter::observe(should_revert, DidClearBoard);
  NotificationCenter::observe(should_revert,
                              DidMoveOutOfView,
                              physics());
  NotificationCenter::observe(should_revert, DidDie);
}

void Player::reset()
{
  Character::reset();
  
  if (_should_revert)
  {
    _should_revert = false;
    board_position(previous_board_position());
    order(previous_order());
  }
  
  const Dimension2 view_dimensions = core()->view_dimensions();
  const int row = board_position().first;
  const int column = board_position().second;
  const double x_pos = view_dimensions.x/2 - 8   - 16*row + 32*column;
  const double y_pos = view_dimensions.y   - 200 + 24*row;
  moveTo(x_pos, y_pos);
}

string Player::prefix_standing()                { return "qbert_standing"; }
string Player::prefix_jumping()                 { return "qbert_jumping";  }
int Player::direction_mask()                    { return 0b1111;           }
pair<int, int> Player::default_board_position() { return {0, 0};           }
int Player::default_order()                     { return 25;               }
CharacterDirection Player::default_direction() { return DOWN;             }
