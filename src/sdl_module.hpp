#ifndef sdl_module_hpp
#define sdl_module_hpp

// std
#include <iostream>

// dependancies
#include "SDL2/SDL.h"

struct window_spec {
    SDL_Window *sdl_window;
    SDL_Renderer *sdl_renderer;
    SDL_Texture *sdl_gost_texture;
    SDL_Texture *sdl_perlin_texture;
    SDL_Texture * sdl_flow_field_texture;
    int x;
    int y;
    int dpi_unscaled_width;
    int dpi_unscaled_height; 
    int perlin_grid_divisor;
    int dpi_scaled_width;
    int dpi_scaled_height;
    int renderer_width;
    int renderer_height;
    int perlin_grid_width;
    int perlin_grid_height;
    Uint32 flags; 
    
    window_spec(int dpi_unscaled_width, int dpi_unscaled_height, int perlin_grid_divisor = 20, Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI) noexcept(false);

    ~window_spec(); 

   int init(); 
    
private:
    void my_sdl_get_display_dpi(int display_index, float *dpi, float *defaultDpi);

};

#endif // sdl_module_hpp
