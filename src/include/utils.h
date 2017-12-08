#ifndef SRC_INCLUDE_UTILS_H_
#define SRC_INCLUDE_UTILS_H_

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

#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <vector>

using namespace glm;
using namespace std;

enum Event {
    NO_EVENT = 0,
    QUIT,
    L_LEFT_DOWN,
    L_RIGHT_DOWN,
    L_FORWARDS_DOWN,
    L_BACKWARDS_DOWN,
    L_UP_DOWN,
    L_DOWN_DOWN,
    L_LEFT_UP,
    L_RIGHT_UP,
    L_FORWARDS_UP,
    L_BACKWARDS_UP,
    L_UP_UP,
    L_DOWN_UP,
    R_LEFT_DOWN,
    R_RIGHT_DOWN,
    R_FORWARDS_DOWN,
    R_BACKWARDS_DOWN,
    R_LEFT_UP,
    R_RIGHT_UP,
    R_FORWARDS_UP,
    R_BACKWARDS_UP,
};

inline ostream& operator <<(ostream& out, const vec3& v) {
    out << v.x << " " << v.y << " " << v.z;
    return out;
}

inline ostream& operator <<(ostream& out, const mat4& m) {
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            out << m[r][c] << " ";
        }
        out << endl;
    }
    return out;
}

inline istream& operator >>(istream& in, vec3& v) {
    in >> v.x >> v.y >> v.z;
    return in;
}

inline GLuint LoadComputeShader(string compute_shader_path) {
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

    // Check Vertex Shader
    glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(compute_shader, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        vector<char> ComputeShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(compute_shader, InfoLogLength, NULL, &ComputeShaderErrorMessage[0]);
        printf("%s\n", &ComputeShaderErrorMessage[0]);
    }

    // Link the program
    // printf("Linking program\n");
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

inline GLuint LoadShaders(string vertex_file_path, string fragment_file_path) {
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
    // printf("Linking program\n");
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

#endif  // SRC_INCLUDE_UTILS_H_
