// std
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <cstdint>
#include <algorithm>

// my
#include "djc_math/djc_math.hpp"
#include "sdl_module.hpp"
#include "particle.hpp"

// dependancies
#include "SDL2/SDL.h"

djc::math::vec2f limit(djc::math::vec2f const & vec, float limit) {
    auto length = vec.length();

    if (length > limit) {
        return djc::math::normalise(vec) * limit;
    }

    return vec;
}

int main(int argc, char *argv[]) {
    
    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL Could not be initialised: %s", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }
    
    window_spec main_window{640, 460, 30};

    if (main_window.init() < 0) {
        SDL_Quit();     
        return EXIT_FAILURE; 
    }
           
    djc::math::perlin<double> noisy(227);
    std::vector<std::uint32_t> perlin_pixel_buffer(main_window.perlin_grid_width * main_window.perlin_grid_height, 0);
    std::vector<djc::math::vec2f> perlin_flow_field(main_window.perlin_grid_width * main_window.perlin_grid_height, djc::math::vec2f(0, 0));
    std::vector<particle> particles(10000, djc::math::vec2f(0,0));
       
    // give the particles random initial positions
    for (particle & p : particles) {
        p = djc::math::vec2f(std::rand() % main_window.renderer_width, std::rand() % main_window.renderer_height);
    }

    SDL_Event event;
    int current_frame_buffer = 0; // keeps track of the frame buffer to draw
    double acc = 0.0;
    double zstep = 0.0;
    bool running = true;

    while (running) {

        // check for input events 
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                // "x" in the window was pressed, brake out of this while loop
                SDL_Log("SDL_QUIT");
                running = false;
                break; 
            }

            if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_SPACE) {
                    // if "space" key is pressed cycle to to next frame buffer
                    current_frame_buffer += 1;
                    
                    if (current_frame_buffer == 3) {
                        current_frame_buffer = 0;
                    }
                }            

                if (event.key.keysym.sym == SDLK_c) {
                    // if the current buffer is the gost buffer and "c" key is pressed - clear it 
                    if (current_frame_buffer == 2) { 
                        SDL_SetRenderTarget(main_window.sdl_renderer, main_window.sdl_gost_texture);
                        SDL_SetRenderDrawColor(main_window.sdl_renderer, 255, 255, 255, 255);
                        SDL_RenderClear(main_window.sdl_renderer);
                    }
                }
            }
        }
        
        // update all the particles
        for (particle & p : particles) {

            // make sure particles do screen wrapping
            if (p.current_position.x < 0) p.current_position.x = main_window.renderer_width;
            if (p.current_position.x > main_window.renderer_width) p.current_position.x = 0;
            if (p.current_position.y < 0) p.current_position.y = main_window.renderer_height;
            if (p.current_position.y > main_window.renderer_height) p.current_position.y = 0;
            
            // get the particle position in the perlin grid
            int grid_x = static_cast<int>(std::floor(p.current_position.x / main_window.perlin_grid_width)); 
            int grid_y = static_cast<int>(std::floor(p.current_position.y / main_window.perlin_grid_height));
            int index  = grid_y * main_window.perlin_grid_width + grid_x;  
            
            // update the particle using the perlin grid 
            p.last_position = p.current_position;
            p.acceleration += perlin_flow_field[index] * 0.01f; 
            p.velocity += p.acceleration;
            p.velocity = limit(p.velocity, 4);
            p.current_position += p.velocity;
            p.acceleration *= 0.0f; // reset 
        }        

        // begin render -- clear the screen to white
        //---------------------------------------------------------------------
        SDL_SetRenderDrawColor(main_window.sdl_renderer, 255, 255, 255, 255);
        SDL_RenderClear(main_window.sdl_renderer);

        // draw perlin background into texture
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
        SDL_UpdateTexture(main_window.sdl_perlin_texture, NULL, perlin_pixel_buffer.data(), sizeof(std::uint32_t) * main_window.perlin_grid_width); 
        
        // draw flow field into texture
        //---------------------------------------------------------------------
        SDL_SetRenderTarget(main_window.sdl_renderer, main_window.sdl_flow_field_texture);
        SDL_RenderClear(main_window.sdl_renderer);
        SDL_SetRenderDrawColor(main_window.sdl_renderer, 0, 0, 0, 255);

        float xstep = (float)main_window.renderer_width / (float)main_window.perlin_grid_width; 
        float ystep = (float)main_window.renderer_height / (float)main_window.perlin_grid_height; 
        float x_pos = 0.0f;
        float y_pos = 0.0f;

        for (int y = 0; y < main_window.perlin_grid_height; y++) {
            for (int x = 0; x < main_window.perlin_grid_width; x++) {
               // render perlin flow lines
               int index = y * main_window.perlin_grid_width + x;

               int x1 = x_pos - xstep / 2;
               int y1 = y_pos - ystep / 2;
               int x2 = x1 + perlin_flow_field[index].x;
               int y2 = y1 + perlin_flow_field[index].y;
                
               SDL_RenderDrawLine(main_window.sdl_renderer, x1, y1, x2, y2); 
               x_pos += xstep; 
            }

            x_pos = 0.0f;
            y_pos += ystep;
        }
        //..
  

        // draw flow field affected effect 
        //---------------------------------------------------------------------
        SDL_SetRenderTarget(main_window.sdl_renderer, main_window.sdl_gost_texture);
        SDL_SetRenderDrawColor(main_window.sdl_renderer, 0, 0, 0, 10);
        SDL_SetRenderDrawBlendMode(main_window.sdl_renderer, SDL_BLENDMODE_BLEND);
        
        for(particle & p: particles) {
            SDL_RenderDrawLine(main_window.sdl_renderer, p.last_position.x, p.last_position.y, p.current_position.x, p.current_position.y);
        }

        // end render
        //---------------------------------------------------------------------
        {
            SDL_SetRenderTarget(main_window.sdl_renderer, NULL);
                  
            // perlin background animation - copy into back buffer
            if (current_frame_buffer == 0) {
                SDL_RenderCopy(main_window.sdl_renderer, main_window.sdl_perlin_texture, NULL, NULL);
            }
            
            // flow field lines - copy into backbuffer 
            if (current_frame_buffer == 1) {
                SDL_RenderCopy(main_window.sdl_renderer, main_window.sdl_flow_field_texture, NULL, NULL);
            }
            
            // gosting line effect - copy into back buffer 
            if (current_frame_buffer == 2) {
                SDL_RenderCopy(main_window.sdl_renderer, main_window.sdl_gost_texture, NULL, NULL);
            }
            
            SDL_RenderPresent(main_window.sdl_renderer); // swap back bufer to front
        }
        
        // step the accumilators 
        //---------------------------------------------------------------------
        zstep+= 0.005f;
        acc += .005;
    }

    SDL_Quit();
    return EXIT_SUCCESS;
}
