#ifndef particle_hpp
#define particle_hpp

// my 
#include "djc_math/djc_math.hpp"


struct particle {
    
    particle(djc::math::vec2f position);

    djc::math::vec2f current_position;
    djc::math::vec2f last_position;
    djc::math::vec2f velocity;
    djc::math::vec2f acceleration;
};

#endif // particle_hpp
