//
//  animation.cpp
//  Arcade Game Engine
//

#include "core.hpp"
#include <fstream>


//
// MARK: - AnimationComponent
//

// MARK: Property functions

string AnimationComponent::trait() { return "animation"; }

// MARK: Member functions

void AnimationComponent::reset()
{
  animating(false);
  _updateVelocity = false;
}

void AnimationComponent::addSegment(string id, vec3 position, vec3 velocity)
{
  _curves[id].push_back({position, velocity});
}

void AnimationComponent::removeCurve(string id)
{
  _curves.erase(id);
}

void AnimationComponent::performAnimation(string id,
                                          double duration,
                                          bool updateVelocity)
{
  if (!animating() && _curves.find(id) != _curves.end())
  {
    animating(true);
    _currentCurve = _curves[id];
    entity()->localPosition(_startPosition);
    _startTime = entity()->core()->effectiveElapsedTime();
    _duration = duration;
    _updateVelocity = updateVelocity;
    NotificationCenter::notify(DidStartAnimating, *this);
  }
}

void AnimationComponent::update(Core & world)
{
  if (animating())
  {
    const double dt = _duration / (_currentCurve.size() - 1);
    const double elapsed  = world.effectiveElapsedTime() - _startTime;
    if (elapsed < _duration)
    {
      const int i = (int)floor(elapsed / dt);
      const double t = fmod(elapsed, dt) / dt;
      const double t2 = t*t;
      const double t3 = t2*t;
      const double two_t3 = 2*t3;
      const double three_t2 = 3*t2;
      const float cp0 = two_t3 - three_t2 + 1;
      const float cm0 = t3 - 2*t2 + t;
      const float cm1 = t3 - t2;
      const float cp1 = three_t2 - two_t3;
      const auto s0 = _currentCurve[i];
      const auto s1 = _currentCurve[i+1];
      const vec3 p = s0.first*cp0 + s0.second*cm0 +
                     s1.first*cp1 + s1.second*cm1;
      
      entity()->setPosition(_startPosition.x + p.x,
                            _startPosition.y + p.y,
                            _startPosition.z + p.z);
    }
    else
    {
      auto lastHalfSpline = _currentCurve.back();
      entity()->setPosition(_startPosition.x + lastHalfSpline.first.x,
                            _startPosition.y + lastHalfSpline.first.y,
                            _startPosition.z + lastHalfSpline.first.z);
      if (_updateVelocity)
      {
        entity()->pVelocity() = lastHalfSpline.second/(float)_duration;
      }
      animating(false);
      NotificationCenter::notify(DidStopAnimating, *this);
    }
  }
}
