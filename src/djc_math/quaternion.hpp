#include "config.hpp"

namespace djc::math {

template<typename T>
class quaternion {

public:
    quaternion(T x, T y, T z, T theta);


  
#if DJC_MATH_VEC_DATA == DJC_MATH_EXPLICIT
    T x;
    T y;
    T z;
    T w;
    
#if DJC_MATH_VEC_DATA == DJC_MATH_ARRAY
    T data[4];

#elif DJC_MATH_VEC_DATA == DJC_MATH_UNIONi
    union {
       
        T data[4];

        struct {
            T x;
            T y;
            T z;
            T w;
        };
    };
#endif

};

} // djc::math
