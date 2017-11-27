#include "include/parser.h"

Parser::Parser() {
    scene_filename_ = "scene.scn";
    Init();
}

Parser::Parser(string filename) {
    scene_filename_ = filename;
    Init();
}

void Parser::Init() {
    camera_ = Camera();
    ambient_light_ = AmbientLight();
    film_resolution_ = ivec2(640, 480);
}

bool Parser::Parse() {
    ifstream in(scene_filename_);
    if(in.fail()){
        cout << "Can't open the scene file '" << scene_filename_ << "'" << endl;
        return false;
    }

    string command;
    string line;
    Material current_material;
    while(in >> command) {
        if (command[0] == '#'){
            getline(in, line);
            continue;
        } else if (command == "camera") {
            vec3 pos, dir, up;
            in >> pos;
            in >> dir;
            in >> up;
            float ha;
            in >> ha;
            ha = ha * 2 * M_PI / 180;
            camera_ = Camera(pos, dir, up, ha);
        } else if (command == "film_resolution") {
            int w, h;
            in >> w >> h;
            film_resolution_ = ivec2(w, h);
        } else if (command == "sphere") {
            vec3 p;
            float r;
            in >> p >> r;
            spheres_.push_back(Sphere(p, r, current_material));
        } else if (command == "background") {
            vec3 c;
            in >> c;
            background_color_ = c;
        } else if (command == "material") {
            vec3 a, d, s, t;
            float ns, ior;
            in >> a >> d >> s >> ns >> t >> ior;
            current_material = Material(a, d, s, t, ns, ior);
        } else if (command == "directional_light"){
            vec3 c, d;
            in >> c >> d;
            directional_lights_.push_back(DirectionalLight(c, d));
        } else if (command == "point_light"){
            vec3 c, p;
            in >> c >> p;
            point_lights_.push_back(PointLight(c, p));
        } else if (command == "ambient_light"){
            vec3 c;
            in >> c;
            ambient_light_ = AmbientLight(c);
        } else {
            getline(in, line);
            cout << "WARNING. Do not know command: " << command << endl;
        }
    }
    in.close();
    return true;
}
