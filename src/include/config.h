#ifndef SRC_INCLUDE_CONFIG_H_
#define SRC_INCLUDE_CONFIG_H_

#include <string>

namespace Config {
    const std::string main_dir = "/home/liam/Documents/School/5607/final/";
    const std::string compute_shader_in  = main_dir + "config/compute.shader";
    const std::string compute_shader_out = main_dir + "shaders/compute.shader";
    const std::string vert_shader        = main_dir + "shaders/shader.vert";
    const std::string frag_shader        = main_dir + "shaders/shader.frag";

    const std::string scene_file         = main_dir + "scenes/env.scn";
}

#endif  // SRC_INCLUDE_CONFIG_H_
