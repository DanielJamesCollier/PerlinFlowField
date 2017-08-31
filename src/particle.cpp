#include "particle.hpp"
#include <cmath>

particle::particle(djc::math::vec2f position) 
:   current_position{position}
,   last_position{position}
,   velocity{std::cos(static_cast<float>(std::rand())), std::sin(static_cast<float>(std::rand()))}
,   acceleration{} {

}


