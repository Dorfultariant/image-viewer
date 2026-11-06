#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3_image/SDL_image.h>

#include <cmath>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

struct ViewerState {
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;
  SDL_Texture *texture = nullptr;
  std::vector<std::string> images;
  std::size_t current_index = 0;
  float image_width = 0.0f;
  float image_height = 0.0f;
  bool fit_to_window = true;
  float zoom = 1.0f;
  float target_zoom = 1.0f;
  float pan_x = 0.0f;
  float pan_y = 0.0f;
  float rotation = 0.0f;
  bool flip_horizontal = false;
  bool flip_vertical = false;
};

static void reset_transforms(ViewerState &state) {
  state.pan_x = 0.0f;
  state.pan_y = 0.0f;
  state.zoom = 1.0f;
  state.target_zoom = 1.0f;
  state.rotation = 0.0f;
  state.flip_horizontal = false;
  state.flip_vertical = false;
}

static bool is_right_angle_rotation(float rotation) {
  int angle = static_cast<int>(std::round(rotation));

  angle %= 360;

  if (angle < 0) {
    angle += 360;
  }

  return angle == 90 || angle == 270;
}

static bool load_image(ViewerState &state) {
  if (state.images.empty()) {
    return false;
  }

  if (state.texture) {
    SDL_DestroyTexture(state.texture);
    state.texture = nullptr;
  }

  const std::string &path = state.images[state.current_index];

  state.texture = IMG_LoadTexture(state.renderer, path.c_str());

  if (!state.texture) {
    std::cerr << "Failed to load image: " << path << "\n"
              << SDL_GetError() << '\n';

    return false;
  }

  if (!SDL_GetTextureSize(state.texture, &state.image_width,
                          &state.image_height)) {
    std::cerr << "SDL_GetTextureSize failed: " << SDL_GetError() << '\n';

    SDL_DestroyTexture(state.texture);
    state.texture = nullptr;

    return false;
  }

  SDL_SetWindowTitle(state.window, path.c_str());

  return true;
}

static bool animation_active(const ViewerState &state) {
  return std::abs(state.target_zoom - state.zoom) > 0.001f;
}

static void next_image(ViewerState &state) {
  if (state.images.empty()) {
    return;
  }

  state.current_index = (state.current_index + 1) % state.images.size();

  load_image(state);

  reset_transforms(state);
}

static void previous_image(ViewerState &state) {
  if (state.images.empty()) {
    return;
  }

  if (state.current_index == 0) {
    state.current_index = state.images.size() - 1;
  } else {
    --state.current_index;
  }

  load_image(state);
  reset_transforms(state);
}

static void render(ViewerState &state) {
  int window_width = 0;
  int window_height = 0;
  if (std::abs(state.target_zoom - state.zoom) > 0.001f) {
    state.zoom += (state.target_zoom - state.zoom) * 0.15f;
  } else {
    state.zoom = state.target_zoom;
  }

  SDL_GetWindowSize(state.window, &window_width, &window_height);

  SDL_SetRenderDrawColor(state.renderer, 40, 40, 40, 255);

  SDL_RenderClear(state.renderer);

  if (state.texture) {
    float fit_width = state.image_width;
    float fit_height = state.image_height;

    if (is_right_angle_rotation(state.rotation)) {
      std::swap(fit_width, fit_height);
    }

    float fit_scale = std::min(static_cast<float>(window_width) / fit_width,
                               static_cast<float>(window_height) / fit_height);

    if (fit_scale > 1.0f) {
      fit_scale = 1.0f;
    }

    float scale;

    if (state.fit_to_window) {
      scale = fit_scale * state.zoom;
    } else {
      scale = state.zoom;
    }
    SDL_FRect dst;

    dst.w = state.image_width * scale;

    dst.h = state.image_height * scale;

    dst.x = (window_width - dst.w) * 0.5f + state.pan_x;
    dst.y = (window_height - dst.h) * 0.5f + state.pan_y;

    SDL_FlipMode flip = SDL_FLIP_NONE;

    if (state.flip_horizontal && state.flip_vertical) {
      flip = static_cast<SDL_FlipMode>(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
    } else if (state.flip_horizontal) {
      flip = SDL_FLIP_HORIZONTAL;
    } else if (state.flip_vertical) {
      flip = SDL_FLIP_VERTICAL;
    }

    SDL_RenderTextureRotated(state.renderer, state.texture, nullptr, &dst,
                             state.rotation, nullptr, flip);
  }

  SDL_RenderPresent(state.renderer);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage:\n"
              << "  iv image1.png image2.jpg ...\n";

    return 1;
  }

  ViewerState state;

  for (int i = 1; i < argc; ++i) {
    state.images.push_back(argv[i]);
  }

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "SDL_Init failed:\n" << SDL_GetError() << '\n';

    return 1;
  }

  state.window = SDL_CreateWindow("iv", 1280, 900, SDL_WINDOW_RESIZABLE);

  if (!state.window) {
    std::cerr << "SDL_CreateWindow failed:\n" << SDL_GetError() << '\n';

    SDL_Quit();
    return 1;
  }

  state.renderer = SDL_CreateRenderer(state.window, nullptr);

  if (!state.renderer) {
    std::cerr << "SDL_CreateRenderer failed:\n" << SDL_GetError() << '\n';

    SDL_DestroyWindow(state.window);

    SDL_Quit();
    return 1;
  }

  if (!load_image(state)) {
    SDL_DestroyRenderer(state.renderer);

    SDL_DestroyWindow(state.window);

    SDL_Quit();

    return 1;
  }

  bool running = true;

  while (running) {
    SDL_Event event;

    bool got_event;

    if (animation_active(state)) {
      got_event = SDL_WaitEventTimeout(&event, 16);
    } else {
      got_event = SDL_WaitEvent(&event);
    }

    if (!got_event) {
      render(state);
      continue;
    }

    /* Image movement within the window */
    int window_width = 0;
    int window_height = 0;

    SDL_GetWindowSize(state.window, &window_width, &window_height);

    float fit_scale =
        std::min(static_cast<float>(window_width) / state.image_width,

                 static_cast<float>(window_height) / state.image_height);

    if (fit_scale > 1.0f) {
      fit_scale = 1.0f;
    }

    switch (event.type) {
    case SDL_EVENT_QUIT:
      running = false;
      break;

    case SDL_EVENT_KEY_DOWN: {
      /* Image movement event handling */
      SDL_Keymod mods = SDL_GetModState();
      bool shift = (mods & SDL_KMOD_SHIFT);
      bool ctrl = (mods & SDL_KMOD_CTRL);

      if (ctrl) {
        switch (event.key.key) {
        case SDLK_H:
        case SDLK_LEFT:
          state.rotation -= 90.0f;
          break;

        case SDLK_L:
        case SDLK_RIGHT:
          state.rotation += 90.0f;
          break;

        default:
          break;
        }

        break;
      }

      if (shift) {
        float render_scale =
            state.fit_to_window ? fit_scale * state.zoom : state.zoom;

        float move_step_x = state.image_width * render_scale * 0.05f;

        float move_step_y = state.image_height * render_scale * 0.05f;

        switch (event.key.key) {
        case SDLK_H:
        case SDLK_LEFT:
          state.pan_x -= move_step_x;
          break;
        case SDLK_L:
        case SDLK_RIGHT:
          state.pan_x += move_step_x;
          break;
        case SDLK_K:
        case SDLK_UP:
          state.pan_y -= move_step_y;
          break;
        case SDLK_J:
        case SDLK_DOWN:
          state.pan_y += move_step_y;
          break;
        case SDLK_M:
          state.flip_horizontal = !state.flip_horizontal;
          break;
        default:
          break;
        }
      } else { // Non-modifier keys
        switch (event.key.key) {
        case SDLK_Q:
        case SDLK_ESCAPE:
          running = false;
          break;

        case SDLK_M:
          state.flip_vertical = !state.flip_vertical;
          break;

        case SDLK_LEFT:
        case SDLK_H:
          previous_image(state);
          render(state);
          state.pan_x = 0.0f;
          state.pan_y = 0.0f;
          break;

        case SDLK_RIGHT:
        case SDLK_L:
          next_image(state);
          render(state);
          state.pan_x = 0.0f;
          state.pan_y = 0.0f;
          break;

        case SDLK_UP:
        case SDLK_K:
          state.target_zoom *= 1.2f;
          if (state.target_zoom > 50.0f) {
            state.target_zoom = 50.0f;
          }
          break;

        case SDLK_DOWN:
        case SDLK_J:
          state.target_zoom /= 1.2f;
          if (state.target_zoom < 0.02f) {
            state.target_zoom = 0.02f;
          }
          break;

        case SDLK_F:
          state.fit_to_window = true;
          state.zoom = 1.0f;
          state.target_zoom = 1.0f;
          state.pan_x = 0.0f;
          state.pan_y = 0.0f;
          break;

        case SDLK_1:
          state.fit_to_window = false;
          state.zoom = 1.0f;
          state.target_zoom = 1.0f;
          state.pan_x = 0.0f;
          state.pan_y = 0.0f;
          break;

          // RESET TRANSFORMS TO DEFAULTS
        case SDLK_0:
          reset_transforms(state);
          break;
        }
      }
      break;
    }
    default:
      break;
    }

    render(state);
    // SDL_Delay(16);
  }

  if (state.texture) {
    SDL_DestroyTexture(state.texture);
  }

  SDL_DestroyRenderer(state.renderer);

  SDL_DestroyWindow(state.window);

  SDL_Quit();

  return 0;
}
