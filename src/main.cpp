#include "include/rayTracer.h"

/*
   } else if (e.type == SDL_MOUSEMOTION) {
   float x = e.motion.xrel;
   float y = e.motion.yrel;
   x = -radians(x) / 4;
   y = -radians(y) / 4;
   camera_rot.x = fmin(radians(80.0f), fmax(radians(-80.0f), camera_rot.x + y));
   camera_rot.y += x;
   mat4 r(1.0f);
   r = rotate(r, camera_rot.y, vec3(0, 1, 0));
   r = rotate(r, camera_rot.x, vec3(1, 0, 0));
   rotated_camera_dir = vec3(r*vec4(camera.dir, 0));
   rotated_camera_up = vec3(r*vec4(camera.up, 0));
   rotated_dx = cross(rotated_camera_dir, rotated_camera_up);
   rotated_dy = -rotated_camera_up;
   ul = camera.pos + dist_to_plane * rotated_camera_dir
   - .5*(SW - 1)*rotated_dx - .5*(SH - 1)*rotated_dy;
    view = lookAt(
    camera.pos,
    camera.pos + rotated_camera_dir,
    rotated_camera_up);
    }
*/
int SW = 640;
int SH = 480;

int main() {
    SDL_Window* window = InitAndWindow("Real Time RayTracer", 100, 100, SW, SH);

    RayTracer* RT = new RayTracer();
    RT->SetUp();

    // SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_Event e;
    bool quit = false;
    unsigned int frameCounter = 0;
    while (!quit) {
        RT->setCurrentTime(SDL_GetTicks() / 1000.0f);
        while (SDL_PollEvent(&e)) {
            Event pe = NO_EVENT;
            if (e.type == SDL_QUIT) {
                pe = QUIT;
            } else if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                switch(e.key.keysym.sym) {
                    case SDLK_a:
                        pe = L_LEFT_DOWN;
                        break;
                    case SDLK_d:
                        pe = L_RIGHT_DOWN;
                        break;
                    case SDLK_s:
                        pe = L_DOWN_DOWN;
                        break;
                    case SDLK_w:
                        pe = L_UP_DOWN;
                        break;
                }
            } else if (e.type == SDL_KEYUP) {
                switch(e.key.keysym.sym) {
                    case SDLK_a:
                        pe = L_LEFT_UP;
                        break;
                    case SDLK_d:
                        pe = L_RIGHT_UP;
                        break;
                    case SDLK_s:
                        pe = L_DOWN_UP;
                        break;
                    case SDLK_w:
                        pe = L_UP_UP;
                        break;
                }
            }
            quit = RT->ParseEvent(pe);
        }

        mat4 view = lookAt(RT->getCameraPos(), RT->getCameraDir(), RT->getCameraUp());
        mat4 proj = perspective(RT->getFov(), RT->getAspectRatio(), .1f, 400.0f);

        RT->Render(view, proj);
        SDL_GL_SwapWindow(window);
    }

    delete RT;
    SDL_Quit();

    return 0;
}
