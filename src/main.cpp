#include <iostream>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <cstdint>
#include <algorithm>

#include "SDL2/SDL.h"
#include "djc_math/djc_math.hpp"


struct window_spec {
    window_spec(int dpi_unscaled_width, int dpi_unscaled_height, int perlin_grid_divisor = 20, Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI) noexcept(false)
    :   sdl_window{nullptr}
    ,   sdl_renderer{nullptr} 
    ,   sdl_gost_texture{nullptr}
    ,   x{SDL_WINDOWPOS_CENTERED}
    ,   y{SDL_WINDOWPOS_CENTERED}
    ,   dpi_unscaled_width{dpi_unscaled_width}
    ,   dpi_unscaled_height{dpi_unscaled_height}
    ,   perlin_grid_divisor{perlin_grid_divisor}
    ,   dpi_scaled_width{dpi_unscaled_width}
    ,   dpi_scaled_height{dpi_unscaled_height}
    ,   renderer_width{dpi_unscaled_height}
    ,   renderer_height{dpi_unscaled_height}
    ,   flags{flags}
    {
        float dpi;
        float default_dpi;

        my_sdl_get_display_dpi(0, &dpi, &default_dpi);

        dpi_scaled_width = int(dpi_unscaled_width * dpi / default_dpi);
        dpi_scaled_height = int(dpi_unscaled_height * dpi / default_dpi); 

        if ((sdl_window = SDL_CreateWindow("PerlinFlowField", x, y, dpi_scaled_width, dpi_scaled_height, flags)) == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
            throw "SDL_CreateWindow() failed"; 
        }

        if ((sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE)) == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
            throw "SDL_CreateRenderer() failed";
        }

        if (SDL_GetRendererOutputSize(sdl_renderer, &renderer_width, &renderer_height) < 0) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
            throw "SDL_GetRendeerOutputSize() failed"; 
        }
        
        perlin_grid_width = renderer_width / perlin_grid_divisor;
        perlin_grid_height = renderer_height / perlin_grid_divisor;

        if((sdl_gost_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, renderer_width, renderer_height)) == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
            throw "SDL_CreteTexture() failed";
        }

        if((sdl_perlin_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, perlin_grid_width, perlin_grid_height)) == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
            throw "SDL_CreteTexture() failed";
        }

        if((sdl_flow_field_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, renderer_width, renderer_height)) == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
            throw "SDL_CreteTexture() failed";
        }
        
        std::cout << "\n------- window info ------\n";
        std::cout << "dpi unscaled dim: " << dpi_unscaled_width << " x " << dpi_unscaled_height << '\n';
        std::cout << "dpi scaled dim:   " << dpi_scaled_width << " x " << dpi_scaled_height << '\n';
        std::cout << "renderer dim:     " << renderer_width << " x " << renderer_height << '\n';

        if (dpi_scaled_width != renderer_width && dpi_scaled_height != renderer_height) {
            std::cout << "High DPI monitor: true" << '\n';
        } else {
            std::cout << "High DPI monitor: false" << '\n';
        }

        std::cout << "perlin dim:       " << perlin_grid_width << " x " << perlin_grid_height << '\n';
        std::cout << "--------------------------\n";
        
                auto xstep =  renderer_width / perlin_grid_width;
                auto ystep = renderer_height / perlin_grid_height; 

                std::cout << "xstep: " << xstep << std::endl;
                std::cout << "ystep: " << ystep << std::endl;
//        SDL_SetRenderDrawBlendMode(sdl_renderer,SDL_BLENDMODE_ADD);  
//        SDL_SetTextureBlendMode(sdl_, SDL_BLENDMODE_ADD);
        SDL_SetRenderTarget(sdl_renderer, sdl_gost_texture);
        SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
        SDL_RenderClear(sdl_renderer);
    }

    ~window_spec() {
         SDL_DestroyTexture(sdl_flow_field_texture);
         SDL_DestroyTexture(sdl_perlin_texture); 
         SDL_DestroyTexture(sdl_gost_texture);
         SDL_RenderClear(sdl_renderer);
         SDL_DestroyRenderer(sdl_renderer);
         SDL_DestroyWindow(sdl_window);
    } 

    void my_sdl_get_display_dpi(int displayIndex, float* dpi, float* defaultDpi) {
    const float kSysDefaultDpi =
        #ifdef __APPLE__
            72.0f;
        #elif defined(_WIN32)
             96.0f;
        #else
            static_assert(false, "No system default DPI set for this platform.");
        #endif

        if (SDL_GetDisplayDPI(displayIndex, NULL, dpi, NULL) != 0)
        {
            // Failed to get DPI, so just return the default value.
            if (dpi) *dpi = kSysDefaultDpi;
        }

        if (defaultDpi) *defaultDpi = kSysDefaultDpi;
    }

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
};


djc::math::vec2f limit(djc::math::vec2f const & vec, float limit) {
    auto length = vec.length();

    if (length > limit) {
        return djc::math::normalise(vec) * limit;
    }

    return vec;
}

struct particle {
    
    particle(djc::math::vec2f position) 
    :   current_position{position}
    ,   last_position{position}
    ,   velocity{(float)std::cos(std::rand()),(float)std::sin(std::rand())} 
    ,   acceleration{} {

    }

    void update() {
        velocity += acceleration;
        velocity = limit(velocity, 4);
        current_position += velocity;
        acceleration *= 0.0f;
        last_position = current_position;
    }

    djc::math::vec2f current_position;
    djc::math::vec2f last_position;
    djc::math::vec2f velocity;
    djc::math::vec2f acceleration;
};

int main(int argc, char *argv[]) {
    
    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL Could not be initialised: %s", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }
    
    try {
        window_spec main_window{640, 460};
           
        djc::math::perlin<double> noisy(227);
        
        std::vector<std::uint32_t> perlin_pixel_buffer(main_window.perlin_grid_width * main_window.perlin_grid_height, 0);
        std::vector<djc::math::vec2f> perlin_flow_field(main_window.perlin_grid_width * main_window.perlin_grid_height, djc::math::vec2f(0, 0));
        particle p1(djc::math::vec2f(main_window.renderer_width / 2, main_window.renderer_height / 2));

        std::vector<particle> particles(10, djc::math::vec2f(0,0));
        
        // give the particles random initial positions
        for (particle & p : particles) {
            p = djc::math::vec2f(std::rand() % main_window.renderer_width, std::rand() % main_window.renderer_height);
        }

        SDL_Event event;
        double acc = 0.0;
        double zstep = 0.0;
        bool running = true;

        while (running) {

            // check for input events 
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    SDL_Log("SDL_QUIT");
                    running = false;
                    break; 
                }
            }

            // update
            {
                for (particle & p : particles) {
                    // make sure particles do screen wrapping
                    if (p.current_position.x < 0) p.current_position.x = main_window.renderer_width;
                    if (p.current_position.x > main_window.renderer_width) p.current_position.x = 0;
                    if (p.current_position.y < 0) p.current_position.y = main_window.renderer_height;
                    if (p.current_position.y > main_window.renderer_height) p.current_position.y = 0;

                    // update particle position using flow field 
                    auto grid_x = std::floor(p.current_position.x / main_window.perlin_grid_width); 
                    auto grid_y = std::floor(p.current_position.y / main_window.perlin_grid_height);
                    int index = grid_y * main_window.perlin_grid_width + grid_x;  
                    p.acceleration += perlin_flow_field[index] * 0.01f; 
                
                    p.update();
                }        
            }

            // begin render
            {
                SDL_SetRenderDrawColor(main_window.sdl_renderer, 0, 0, 0, 255);
                SDL_RenderClear(main_window.sdl_renderer);
            }

            // draw perlin background
            { 
                for (int y = 0; y < main_window.perlin_grid_height; y++) {
                    for (int x = 0; x < main_window.perlin_grid_width; x++) {
                        double X = (double)x / (double)main_window.perlin_grid_width;
                        double Y = (double)y / (double)main_window.perlin_grid_height;
                      
                        float angle = noisy.noise(X * 5 ,Y * 5 , zstep); 
                        std::uint8_t noise = angle * 255; 
                        int index = main_window.perlin_grid_width * y + x;
                        
                        perlin_pixel_buffer[index] = (255 << 24) + (noise << 16) + (noise << 8) + noise; 

                        perlin_flow_field[index] = djc::math::vec2f(std::cos(angle * djc::math::tau<float>), std::sin(angle * djc::math::tau<float>)) * 20.0f;
                    }
                }
                SDL_UpdateTexture(main_window.sdl_perlin_texture, NULL, perlin_pixel_buffer.data(), sizeof(std::uint32_t) * main_window.perlin_grid_width); //Copy entire array only once
            }

            // draw flow field 
            {
                SDL_SetRenderTarget(main_window.sdl_renderer, main_window.sdl_flow_field_texture);
                SDL_RenderClear(main_window.sdl_renderer);
                SDL_SetRenderDrawColor(main_window.sdl_renderer, 255, 255, 255, 255);
                
                // draw flow field
                float xstep = (float)main_window.renderer_width / (float)main_window.perlin_grid_width; 
                float ystep = (float)main_window.renderer_height / (float)main_window.perlin_grid_height; 
                float x_pos = 0.0f;
                float y_pos = 0.0f;

                for (int y = 0; y < main_window.perlin_grid_height; y++) {
                    for (int x = 0; x < main_window.perlin_grid_width; x++) {
                       // render grid corner points
                       SDL_SetRenderDrawColor(main_window.sdl_renderer, 255, 255, 255, 255);
                       SDL_RenderDrawPoint(main_window.sdl_renderer,x_pos, y_pos);

                       // render perlin flow lines
                       int index = y * main_window.perlin_grid_width + x;

                       int x1 = x_pos - xstep / 2;
                       int y1 = y_pos - ystep / 2;
                       int x2 = x1 + perlin_flow_field[index].x;
                       int y2 = y1 + perlin_flow_field[index].y;
                        
                       SDL_SetRenderDrawColor(main_window.sdl_renderer, 60, 70, 86, 200);
                       SDL_RenderDrawLine(main_window.sdl_renderer, x1, y1, x2, y2); 

                       x_pos += xstep; 
                    }

                    x_pos = 0.0f;
                    y_pos += ystep;
                }
            }
                
            
            // draw flow field affected effect 
            {
                SDL_SetRenderTarget(main_window.sdl_renderer, main_window.sdl_gost_texture);
                SDL_SetRenderDrawColor(main_window.sdl_renderer, 255, 255, 255, 50);
                
                
                for(particle & p: particles) {
                    //SDL_RenderDrawPoint(main_window.sdl_renderer, p.current_position.x, p.current_position.y);
                    SDL_RenderDrawLine(main_window.sdl_renderer, p.last_position.x, p.last_position.y, p.current_position.x, p.current_position.y);
                }

                p1.last_position = p1.current_position;
            }

            // step
            {
                zstep+= 0.005f;
                acc += .005;
            }

            // end render
            {
                SDL_SetRenderTarget(main_window.sdl_renderer, NULL);
                 
                // perlin background animation - copy into back buffer
                    //SDL_SetTextureBlendMode(main_window.sdl_perlin_texture, SDL_BLENDMODE_ADD);
                    //SDL_SetTextureAlphaMod(main_window.sdl_perlin_texture, 100);
                //SDL_RenderCopy(main_window.sdl_renderer, main_window.sdl_perlin_texture, NULL, NULL);
                
                
                
                // flow field lines - copy into backbuffer 
                    //SDL_SetTextureBlendMode(main_window.sdl_flow_field_texture, SDL_BLENDMODE_ADD);
                    //SDL_SetTextureAlphaMod(main_window.sdl_gost_texture, 255);
                //SDL_RenderCopy(main_window.sdl_renderer, main_window.sdl_flow_field_texture, NULL, NULL);
                
                // gosting line effect - copy into back buffer 
                    SDL_SetTextureBlendMode(main_window.sdl_gost_texture, SDL_BLENDMODE_BLEND);
//                    SDL_SetTextureAlphaMod(main_window.sdl_gost_texture, 255);
                SDL_RenderCopy(main_window.sdl_renderer, main_window.sdl_gost_texture, NULL, NULL);

                // swap back buffer to front
                SDL_RenderPresent(main_window.sdl_renderer);
            }
        }
    } catch (const char *msg) {
        std::cout << msg << std::endl;
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Quit();
    return EXIT_SUCCESS;
}
