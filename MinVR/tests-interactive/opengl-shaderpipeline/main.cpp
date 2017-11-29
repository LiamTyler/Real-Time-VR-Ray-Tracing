#include <iostream>

#include "GL/glew.h"
#ifdef _WIN32
#include "GL/wglew.h"
#elif (!defined(__APPLE__))
#include "GL/glxew.h"
#endif

// OpenGL Headers
#if defined(WIN32)
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#elif defined(__APPLE__)
#define GL_GLEXT_PROTOTYPES
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif

// MinVR header
#include <api/MinVR.h>
using namespace MinVR;

// Just included for some simple Matrix math used below
// This is not required for use of MinVR in general
#include <math/VRMath.h>

#include "include/utils.h"
#include "include/parser.h"
#include "include/camera.h"

string main_dir = "/home/liam/Documents/School/5607/final/";
string vert_shader = main_dir + "shaders/shader.vert";
string frag_shader = main_dir + "shaders/shader.frag";
string compute_shader = main_dir + "shaders/compute2.shader";
string scene_file = main_dir + "scenes/test_scene.scn";

/*
    SDL_SetRelativeMouseMode(SDL_TRUE);
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
        }
    }
}
*/



/**
 * MyVRApp is an example of a modern OpenGL using VBOs, VAOs, and shaders.  MyVRApp inherits
 * from VRGraphicsApp, which allows you to override onVREvent to get input events, onRenderContext
 * to setup context sepecific objects, and onRenderScene that renders to each viewport.
 */
class MyVRApp : public VRApp {
public:
	MyVRApp(int argc, char** argv) : VRApp(argc, argv) {
        vec3 camera_vel = vec3(0, 0, 0);
        vec3 camera_rot = vec3(0, 0, 0);
        speed_ = 150;
        fpsTime_ = currentTime_ = lastTime_ = 0;
    }

	/// onVREvent is called when a new intput event happens.
	void onVREvent(const VREvent &event) {
        event.print();
        
		if (event.getName() == "FrameStart") {
            currentTime_ = event.getDataAsFloat("ElapsedSeconds");
		} else {
            string name = event.getName();
            bool down = name[name.find('_') + 1] == 'D';
            if (down) {
                if (name == "KbdEsc_Down") {
                    shutdown();
                } else if (name == "KbdW_Down") {
                    camera_vel_.z = 1;
                } else if (name == "KbdS_Down") {
                    camera_vel_.z = -1;
                } else if (name == "KbdA_Down") {
                    camera_vel_.x = -1;
                } else if (name == "KbdD_Down") {
                    camera_vel_.x = 1;
                }
            } else {
                if (name == "KbdW_Up" || name == "KbdS_Up") {
                    cout << "w / s up" << endl;
                    string s;
                    cin >> s;
                    camera_vel_.z = 0;
                } else if (name == "KbdA_Up" || name == "KbdD_Up") {
                    cout << "a / d up" << endl;
                    string s;
                    cin >> s;
                    camera_vel_.x = 0;
                }
            }
        }
	}

    
	/// onVRRenderContext is the override which allows users to setup context specific
	/// variables like VBO's, VAO's, textures, framebuffers, and shader programs.
	void onVRRenderGraphicsContext(const VRGraphicsState &renderState) {
		// If this is the inital call, initialize context variables
		if (renderState.isInitialRenderCall()) {
			glewExperimental = GL_TRUE;
			GLenum err = glewInit();
			if (GLEW_OK != err)
			{
				std::cout << "Error initializing GLEW." << std::endl;
			}

			// Init GL
			glEnable(GL_DEPTH_TEST);
			glClearDepth(1.0f);
			glDepthFunc(GL_LEQUAL);
			glClearColor(0, 0, 0, 1);

            const float verts[] = {
                -1, 1, 0,
                -1, -1, 0,
                1, -1, 0,
                1, -1, 0,
                1, 1, 0,
                -1, 1, 0,
            };

            const float uv[] = {
                0, 1,
                0, 0,
                1, 0,
                1, 0,
                1, 1,
                0, 1,
            };

            // Parse the scene file
            parser_ = Parser(scene_file);
            if (!parser_.Parse()) {
                cout << "Could not parse scene" << endl;
                return;
            }
            SW_ = parser_.film_resolution.x;
            SH_ = parser_.film_resolution.y;

            cout << "vendor: " << glGetString(GL_VENDOR) << endl;
            cout << "renderer: " << glGetString(GL_RENDERER) << endl;
            cout << "version: " << glGetString(GL_VERSION) << endl;
            ivec3 work_group_cnt;
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_group_cnt[0]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_group_cnt[1]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_group_cnt[2]);

            cout << "max global work group size: " << work_group_cnt.x << ", " << work_group_cnt.y
                << ", " << work_group_cnt.z << endl;

            ivec3 work_group_size;
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_group_size[0]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_group_size[1]);
            glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_group_size[2]);

            cout << "max global work group size: " << work_group_size.x << ", " << work_group_size.y
                << ", " << work_group_size.z << endl;

            int work_group_invocations;
            glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_group_invocations);
            cout << "max local work group invocations: " << work_group_invocations << endl;

            compute_program_ = LoadComputeShader(compute_shader);
            render_program_ = LoadShaders(vert_shader, frag_shader);
            glUseProgram(render_program_);

            tex_output_;
            glGenTextures(1, &tex_output_);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex_output_);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SW_, SH_, 0, GL_RGBA, GL_FLOAT, NULL);
            glBindImageTexture(0, tex_output_, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

            quad_vao_;
            glGenVertexArrays(1, &quad_vao_);
            glBindVertexArray(quad_vao_);

            quad_verts_vbo_;
            glGenBuffers(1, &quad_verts_vbo_);
            glBindBuffer(GL_ARRAY_BUFFER, quad_verts_vbo_);
            glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
            GLint posAttrib = glGetAttribLocation(render_program_, "pos");
            glEnableVertexAttribArray(posAttrib);
            glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

            quad_uv_vbo_;
            glGenBuffers(1, &quad_uv_vbo_);
            glBindBuffer(GL_ARRAY_BUFFER, quad_uv_vbo_);
            glBufferData(GL_ARRAY_BUFFER, sizeof(uv), uv, GL_STATIC_DRAW);
            GLint texAttrib = glGetAttribLocation(render_program_, "inTexCoord");
            glEnableVertexAttribArray(texAttrib);
            glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

            camera_ = parser_.camera;
            vec3 dx = normalize(cross(camera_.dir, camera_.up));
            vec3 dy = -camera_.up;
            rotated_dx_ = dx;
            rotated_dy_ = dy;
            rotated_camera_dir_ = camera_.dir;
            rotated_camera_up_ = camera_.up;

            glUseProgram(compute_program_);
            ivec2 ss(SW_, SH_);
            glUniform2iv(glGetUniformLocation(compute_program_, "img_size"), 1, &ss[0]);
            glUniform3fv(glGetUniformLocation(compute_program_, "camera_pos"), 1, &camera_.pos[0]);

            // Send spheres to the GPU
            glGenBuffers(1, &spheres_ssbo_);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, spheres_ssbo_);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Sphere) * parser_.spheres.size(), &parser_.spheres[0], GL_STATIC_COPY);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, spheres_ssbo_);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            glUniform1i(glGetUniformLocation(compute_program_, "num_spheres"), parser_.spheres.size());

            // send lights to the GPU
            // ambient
            glUniform3fv(glGetUniformLocation(compute_program_, "ambient_light"),
                         1, (GLfloat*) &parser_.ambient_light);
            // directional
            glGenBuffers(1, &dir_lights_ssbo_);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, dir_lights_ssbo_);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DirectionalLight) * parser_.directional_lights.size(), &parser_.directional_lights[0], GL_STATIC_COPY);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, dir_lights_ssbo_);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            glUniform1i(glGetUniformLocation(compute_program_, "num_dir_lights"), parser_.directional_lights.size());
            // point
            glGenBuffers(1, &point_lights_ssbo_);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, point_lights_ssbo_);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PointLight) * parser_.point_lights.size(), &parser_.point_lights[0], GL_STATIC_COPY);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, point_lights_ssbo_);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            glUniform1i(glGetUniformLocation(compute_program_, "num_point_lights"), parser_.point_lights.size());
            // send other variables to the GPU
            glUniform4fv(glGetUniformLocation(compute_program_, "background_color"),
                         1, &parser_.background_color[0]);

		}

		// Destroy context items if the program is no longer running
		if (!isRunning()) {
			glDeleteBuffers(1, &quad_verts_vbo_);
			glDeleteBuffers(1, &quad_uv_vbo_);
			glDeleteVertexArrays(1, &quad_vao_);
			glDeleteProgram(compute_program_);
			glDeleteProgram(render_program_);
			return;
		}
	}

	/// onVRRenderScene will run draw calls on each viewport inside a context.
	void onVRRenderGraphics(const VRGraphicsState &renderState) {
		// Only draw if the application is still running.
		if (isRunning()) {
			// clear screen
            // update shit
            /*
            float dt = currentTime - lastTime;
            vec3 inc = speed * dt * (camera_vel.z*rotated_camera_dir + camera_vel.x*rotated_dx);
            camera.pos += inc;
            ul += inc;
            glUseProgram(compute_program);
            glUniform3fv(glGetUniformLocation(compute_program, "camera_pos"), 1, &camera_.pos[0]);
            glUniform3fv(glGetUniformLocation(compute_program, "camera_ul"), 1, &ul[0]);
            view = lookAt(
                    camera.pos,
                    camera.pos + rotated_camera_dir,
                    rotated_camera_up);
            */
            glUseProgram(compute_program_);
			GLint loc = glGetUniformLocation(compute_program_, "proj");
			glUniformMatrix4fv(loc, 1, GL_FALSE, renderState.getProjectionMatrix());
			loc = glGetUniformLocation(compute_program_, "view");
			glUniformMatrix4fv(loc, 1, GL_FALSE, renderState.getViewMatrix());

            glDispatchCompute((GLuint)SW_, (GLuint)SH_, 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            glClear(GL_COLOR_BUFFER_BIT);
			// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            glUseProgram(render_program_);
            glBindVertexArray(quad_vao_);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex_output_);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            frameCounter_++;
            lastTime_ = currentTime_;
            if (currentTime_ > fpsTime_ + 1) {
                cout << "FPS: " << frameCounter_ << endl;
                fpsTime_ = currentTime_;
                frameCounter_ = 0;
            }

			glUseProgram(0);
		}
	}

    GLuint LoadComputeShader(string compute_shader_path) {
        ifstream in(compute_shader_path);
        if (in.fail()) {
            cout << "Failed to open the compute shader file: " << compute_shader_path << endl;
            return 0;
        }
        string file, line;
        while (getline(in, line))
            file += line + '\n';
        in.close();

        GLuint compute_shader = glCreateShader(GL_COMPUTE_SHADER);
        const char * ComputePointer = file.c_str();
        glShaderSource(compute_shader, 1, &ComputePointer, NULL);
        glCompileShader(compute_shader);

        GLint Result = GL_FALSE;
        int InfoLogLength;

        glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(compute_shader, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 ){
            vector<char> ComputeShaderErrorMessage(InfoLogLength+1);
            glGetShaderInfoLog(compute_shader, InfoLogLength, NULL, &ComputeShaderErrorMessage[0]);
            printf("%s\n", &ComputeShaderErrorMessage[0]);
        }

        // Link the program
        GLuint ProgramID = glCreateProgram();
        glAttachShader(ProgramID, compute_shader);
        glLinkProgram(ProgramID);

        // Check the program
        glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 ){
            vector<char> ProgramErrorMessage(InfoLogLength+1);
            glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
            printf("%s\n", &ProgramErrorMessage[0]);
        }

        glDetachShader(ProgramID, compute_shader);
        glDeleteShader(compute_shader);

        return ProgramID;
    }

    GLuint LoadShaders(string vertex_file_path, string fragment_file_path) {
        // Create the shaders
        GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
        GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

        // Read the Vertex Shader code from the file
        string VertexShaderCode;
        ifstream VertexShaderStream(vertex_file_path, ios::in);
        if(VertexShaderStream.is_open()){
            string Line = "";
            while(getline(VertexShaderStream, Line))
                VertexShaderCode += "\n" + Line;
            VertexShaderStream.close();
        } else {
            cerr << "Could not open vertex shader: " << vertex_file_path << endl;
            exit(1);
        }

        // Read the Fragment Shader code from the file
        string FragmentShaderCode;
        ifstream FragmentShaderStream(fragment_file_path, ios::in);
        if(FragmentShaderStream.is_open()){
            string Line = "";
            while(getline(FragmentShaderStream, Line))
                FragmentShaderCode += "\n" + Line;
            FragmentShaderStream.close();
        } else {
            cerr << "Could not open fragment shader: " << fragment_file_path << endl;
            exit(1);
        }

        GLint Result = GL_FALSE;
        int InfoLogLength;

        // Compile Vertex Shader
        // printf("Compiling shader : %s\n", vertex_file_path);
        char const * VertexSourcePointer = VertexShaderCode.c_str();
        glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
        glCompileShader(VertexShaderID);

        // Check Vertex Shader
        glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 ){
            vector<char> VertexShaderErrorMessage(InfoLogLength+1);
            glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
            printf("%s\n", &VertexShaderErrorMessage[0]);
        }

        // Compile Fragment Shader
        // printf("Compiling shader : %s\n", fragment_file_path);
        char const * FragmentSourcePointer = FragmentShaderCode.c_str();
        glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
        glCompileShader(FragmentShaderID);

        // Check Fragment Shader
        glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 ){
            vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
            glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
            printf("%s\n", &FragmentShaderErrorMessage[0]);
        }

        // Link the program
        GLuint ProgramID = glCreateProgram();
        glAttachShader(ProgramID, VertexShaderID);
        glAttachShader(ProgramID, FragmentShaderID);
        glLinkProgram(ProgramID);

        // Check the program
        glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if ( InfoLogLength > 0 ){
            vector<char> ProgramErrorMessage(InfoLogLength+1);
            glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
            printf("%s\n", &ProgramErrorMessage[0]);
        }


        glDetachShader(ProgramID, VertexShaderID);
        glDetachShader(ProgramID, FragmentShaderID);

        glDeleteShader(VertexShaderID);
        glDeleteShader(FragmentShaderID);

        return ProgramID;
    }


private:
    GLuint quad_vao_;
    GLuint quad_verts_vbo_;
    GLuint quad_uv_vbo_;
    GLuint tex_output_;
    GLuint render_program_;
    GLuint compute_program_;


    GLuint spheres_ssbo_;
    GLuint dir_lights_ssbo_;
    GLuint point_lights_ssbo_;

    int SW_;
    int SH_;
    Parser parser_;
    Camera camera_;
    float dist_to_plane_;
    vec3 camera_vel_;
    vec3 camera_rot_;
    vec3 rotated_camera_dir_;
    vec3 rotated_camera_up_;
    vec3 rotated_dx_;
    vec3 rotated_dy_;
    float speed_;
    float lastTime_;
    float currentTime_;
    float fpsTime_;
    unsigned int frameCounter_;

};

/// Main method which creates and calls application
int main(int argc, char **argv) {
	MyVRApp app(argc, argv);
	app.run();
	return 0;
}
