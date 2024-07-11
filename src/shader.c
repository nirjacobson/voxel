#include "shader.h"

extern const char __3D_vert_glsl[];
extern const char __3D_frag_glsl[];

extern const char __2D_vert_glsl[];
extern const char __2D_frag_glsl[];

GLuint shader_create(const char* src, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, (const GLchar** const)&src, NULL);
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        char buffer[512];
        glGetShaderInfoLog(shader, 512, NULL, buffer);
        printf("Shader compile error: %s\n", buffer);
    }

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

    shaderProgram3D->shader_vert = shader_create(__3D_vert_glsl, GL_VERTEX_SHADER);
    shaderProgram3D->shader_frag = shader_create(__3D_frag_glsl, GL_FRAGMENT_SHADER);
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

ShaderProgram2D* shader_program_2D_init(ShaderProgram2D* s) {
    ShaderProgram2D* shaderProgram2D = s ? s : NEW(ShaderProgram2D, 1);

    shaderProgram2D->shader_vert = shader_create(__2D_vert_glsl, GL_VERTEX_SHADER);
    shaderProgram2D->shader_frag = shader_create(__2D_frag_glsl, GL_FRAGMENT_SHADER);
    shaderProgram2D->shader_prog = shader_create_program(shaderProgram2D->shader_vert, shaderProgram2D->shader_frag);

    shaderProgram2D->attrib_position = glGetAttribLocation(shaderProgram2D->shader_prog, "position");
    shaderProgram2D->attrib_texcoord = glGetAttribLocation(shaderProgram2D->shader_prog, "texcoord");

    shaderProgram2D->unifrm_projection = glGetUniformLocation(shaderProgram2D->shader_prog, "projection");
    shaderProgram2D->unifrm_sampler = glGetUniformLocation(shaderProgram2D->shader_prog, "sampler");

    return shaderProgram2D;
}

void shader_program_2D_destroy(ShaderProgram2D* shaderProgram2D) {
    glDeleteProgram(shaderProgram2D->shader_prog);
    glDeleteShader(shaderProgram2D->shader_frag);
    glDeleteShader(shaderProgram2D->shader_vert);
}

void shader_program_2D_update_projection(ShaderProgram2D* shaderProgram2D, float* mat4) {
    glUniformMatrix4fv(shaderProgram2D->unifrm_projection, 1, GL_FALSE, mat4);
}

void shader_program_2D_update_sampler(ShaderProgram2D* shaderProgram2D, GLint sampler) {
    glUniform1i(shaderProgram2D->unifrm_sampler, sampler);
}

void shader_program_2D_use(ShaderProgram2D* shaderProgram2D) {
    glUseProgram(shaderProgram2D->shader_prog);
}
