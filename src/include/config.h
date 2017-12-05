#ifndef SRC_INCLUDE_CONFIG_H_
#define SRC_INCLUDE_CONFIG_H_

#include <string>

namespace Config {
    const std::string main_dir = "/home/liam/Documents/School/5607/final/";
    const std::string vert_shader    = main_dir + "shaders/shader.vert";
    const std::string frag_shader    = main_dir + "shaders/shader.frag";
    const std::string compute_shader = main_dir + "shaders/compute2.shader";
    const std::string scene_file     = main_dir + "scenes/test_scene.scn";
}

#endif  // SRC_INCLUDE_CONFIG_H_