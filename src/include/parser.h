#ifndef SRC_INCLUDE_PARSER_H_
#define SRC_INCLUDE_PARSER_H_

#include "include/utils.h"
#include "include/shapes.h"
#include "include/camera.h"
#include "include/lights.h"

class Parser {
    public:
        Parser();

        // filename of the scene to parse
        Parser(string filename);

        // sets all the variables to their default values
        void Init();

        // Parses the file and updates the variables
        bool Parse();

    // private:
        string scene_filename_;
        Camera camera_;
        ivec2 film_resolution_;

        vector<Sphere> spheres_;

        vector<PointLight> point_lights_;
        vector<DirectionalLight> directional_lights_;
        AmbientLight ambient_light_;
        
        vec3 background_color_;
};

#endif  // SRC_INCLUDE_PARSER_H_

