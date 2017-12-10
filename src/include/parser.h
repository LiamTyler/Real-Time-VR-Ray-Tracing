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
        int max_depth;
        string scene_filename;
        Camera camera;
        ivec2 film_resolution;

        vector<Sphere> spheres;

        vector<PointLight> point_lights;
        vector<DirectionalLight> directional_lights;
        AmbientLight ambient_light;
        
        vec4 background_color;
        string env_map;
};

#endif  // SRC_INCLUDE_PARSER_H_

