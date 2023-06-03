#include "shader.h"

GLuint shader_create(const char* path, GLenum shaderType) {
    FILE* file;
    long fsize;
    char* buffer;

    file = fopen(path, "r");

    fseek(file, 0, SEEK_END);
    fsize = ftell(file);
    rewind(file);

    buffer = NEW(char, fsize + 1);
    fread(buffer,1,fsize,file);
    buffer[fsize] = '\0';

    fclose(file);

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, (const GLchar** const)&buffer, NULL);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        char buffer[512];
        glGetShaderInfoLog(shader, 512, NULL, buffer);
        printf("Shader compile error: %s\n", buffer);
    }

    free(buffer);

    return shader;
}

GLuint shader_create_program(GLuint vertex_shader, GLuint fragment_shader) {
    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    GLint status;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        char buffer[512];
        glGetProgramInfoLog(shader_program, 512, NULL, buffer);
        printf("Program link error: %s\n", buffer);
    }

    return shader_program;
}

ShaderProgram3D* shader_program_3D_init(ShaderProgram3D* s) {
    ShaderProgram3D* shaderProgram3D = s ? s : NEW(ShaderProgram3D, 1);

    char* shader_vert_path = "src/shaders/3D.vert";
    char* shader_frag_path = "src/shaders/3D.frag";

    shaderProgram3D->shader_vert = shader_create(shader_vert_path, GL_VERTEX_SHADER);
    shaderProgram3D->shader_frag = shader_create(shader_frag_path, GL_FRAGMENT_SHADER);
    shaderProgram3D->shader_prog = shader_create_program(shaderProgram3D->shader_vert, shaderProgram3D->shader_frag);

    shaderProgram3D->attrib_position = glGetAttribLocation(shaderProgram3D->shader_prog, "position");
    shaderProgram3D->attrib_normal = glGetAttribLocation(shaderProgram3D->shader_prog, "normal");
    shaderProgram3D->unifrm_world_position = glGetUniformLocation(shaderProgram3D->shader_prog, "worldPosition");
    shaderProgram3D->unifrm_model = glGetUniformLocation(shaderProgram3D->shader_prog, "model");
    shaderProgram3D->unifrm_camera = glGetUniformLocation(shaderProgram3D->shader_prog, "camera");
    shaderProgram3D->unifrm_projection = glGetUniformLocation(shaderProgram3D->shader_prog, "projection");
    shaderProgram3D->unifrm_ambient = glGetUniformLocation(shaderProgram3D->shader_prog, "ambient");
    shaderProgram3D->unifrm_color = glGetUniformLocation(shaderProgram3D->shader_prog, "color");
    shaderProgram3D->unifrm_sun_position = glGetUniformLocation(shaderProgram3D->shader_prog, "sun_position");

    return shaderProgram3D;
}

void shader_program_3D_destroy(ShaderProgram3D* shaderProgram3D) {
    glDeleteProgram(shaderProgram3D->shader_prog);
    glDeleteShader(shaderProgram3D->shader_frag);
    glDeleteShader(shaderProgram3D->shader_vert);
}

void shader_program_3D_update_world_position(ShaderProgram3D* shaderProgram3D, float* position) {
    glUniform3f(shaderProgram3D->unifrm_world_position, position[0], position[1], position[2]);
}

void shader_program_3D_update_model(ShaderProgram3D* shaderProgram3D, float* mat4) {
    glUniformMatrix4fv(shaderProgram3D->unifrm_model, 1, GL_FALSE, mat4);
}

void shader_program_3D_update_camera(ShaderProgram3D* shaderProgram3D, float* mat4) {
    glUniformMatrix4fv(shaderProgram3D->unifrm_camera, 1, GL_FALSE, mat4);
}

void shader_program_3D_update_projection(ShaderProgram3D* shaderProgram3D, float* mat4) {
    glUniformMatrix4fv(shaderProgram3D->unifrm_projection, 1, GL_FALSE, mat4);
}

void shader_program_3D_update_color(ShaderProgram3D* shaderProgram3D, float r, float g, float b) {
    glUniform3f(shaderProgram3D->unifrm_color, r/255.0, g/255.0, b/255.0);
}

void shader_program_3D_update_ambient(ShaderProgram3D* shaderProgram3D, float a) {
    glUniform1f(shaderProgram3D->unifrm_ambient, a);
}

void shader_program_3D_update_sun_position(ShaderProgram3D* shaderProgram3D, float* position) {
    glUniform3f(shaderProgram3D->unifrm_sun_position, position[0], position[1], position[2]);
}

void shader_program_3D_use(ShaderProgram3D* shaderProgram3D) {
    glUseProgram(shaderProgram3D->shader_prog);
}
