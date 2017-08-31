#include "sdl_module.hpp"

window_spec::window_spec(int dpi_unscaled_width, int dpi_unscaled_height, int perlin_grid_divisor, Uint32 flags) noexcept(false) 
:   sdl_window{nullptr}
,   sdl_renderer{nullptr}
,   sdl_gost_texture{nullptr}
,   sdl_perlin_texture{nullptr}
,   sdl_flow_field_texture{nullptr}
,   x{SDL_WINDOWPOS_CENTERED}
,   y{SDL_WINDOWPOS_CENTERED}
,   dpi_unscaled_width{dpi_unscaled_width}
,   dpi_unscaled_height{dpi_unscaled_height}
,   perlin_grid_divisor{perlin_grid_divisor}
,   dpi_scaled_width{dpi_unscaled_width}
,   dpi_scaled_height{dpi_unscaled_height}
,   renderer_width{dpi_unscaled_width}
,   renderer_height{dpi_unscaled_width}
,   flags{flags} {

}

window_spec::~window_spec() {
    SDL_DestroyTexture(sdl_flow_field_texture);
    SDL_DestroyTexture(sdl_perlin_texture); 
    SDL_DestroyTexture(sdl_gost_texture);
    SDL_RenderClear(sdl_renderer);
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(sdl_window);
}

int window_spec::init() {
    float dpi;
    float default_dpi;

    my_sdl_get_display_dpi(0, &dpi, &default_dpi);

    dpi_scaled_width = static_cast<int>(dpi_unscaled_width * dpi / default_dpi);
    dpi_scaled_height = static_cast<int>(dpi_unscaled_height * dpi / default_dpi); 

    if ((sdl_window = SDL_CreateWindow("PerlinFlowField", x, y, dpi_scaled_width, dpi_scaled_height, flags)) == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return -1;
    }

    if ((sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE)) == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return -1;
    }

    if (SDL_GetRendererOutputSize(sdl_renderer, &renderer_width, &renderer_height) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return -1;
    }
    
    perlin_grid_width = renderer_width / perlin_grid_divisor;
    perlin_grid_height = renderer_height / perlin_grid_divisor;

    if((sdl_gost_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, renderer_width, renderer_height)) == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return -1;
    }

    if((sdl_perlin_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, perlin_grid_width, perlin_grid_height)) == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return -1;
    }

    if((sdl_flow_field_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, renderer_width, renderer_height)) == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return -1;
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

    SDL_SetRenderTarget(sdl_renderer, sdl_gost_texture);
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
    SDL_RenderClear(sdl_renderer);
    return 0;
}

void window_spec::my_sdl_get_display_dpi(int display_index, float *dpi, float *default_dpi) {
    const float system_default_dpi =
    #ifdef __APPLE__
        72.0f;
    #elif defined(_WIN32)
         96.0f;
    #else
        static_assert(false, "No system default DPI set for this platform.");
    #endif

    if (SDL_GetDisplayDPI(display_index, NULL, dpi, NULL) != 0) {
        if (dpi) *dpi = system_default_dpi;
    }

    if (default_dpi) {
        *default_dpi = system_default_dpi;
    }
}
