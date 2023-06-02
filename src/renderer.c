#include "renderer.h"
#include "internal/renderer.h"

void render_mesh(void* ptr, void* rendererPtr) {
    Mesh* mesh = (Mesh*)ptr;
    Renderer* renderer = (Renderer*)rendererPtr;

    float color[3];
    block_color_rgb(mesh->color, color);
    renderer_3D_update_color(renderer, color[0], color[1], color[2]);
    renderer_render_mesh(renderer, mesh, MESH_FILL);
}

void render_world_chunk(void* worldChunkPtr, void* rendererPtr) {
    WorldChunk* worldChunk = (WorldChunk*)worldChunkPtr;
    Renderer* renderer = (Renderer*)rendererPtr;
    GLfloat position[3] = {
        worldChunk->id.x * WORLD_CHUNK_LENGTH,
        worldChunk->id.y * WORLD_CHUNK_LENGTH,
        worldChunk->id.z * WORLD_CHUNK_LENGTH
    };

    if (worldChunk->chunk->meshes.size)
        renderer_render_chunk(renderer, worldChunk->chunk, position);
}

void render_panel(void* panelPtr, void* rendererPtr) {
    Panel* panel = (Panel*)panelPtr;
    Renderer* renderer = (Renderer*)rendererPtr;
    renderer_render_panel(renderer, panel);
}

void renderer_render_panel(Renderer* renderer, Panel* panel) {
    glBindVertexArray(panel->vao);
    renderer_2D_use(renderer);
    glBindBuffer(GL_ARRAY_BUFFER, panel->vbo);

    glEnableVertexAttribArray(renderer->shaderProgram2D.attrib_position);
    glVertexAttribPointer(renderer->shaderProgram2D.attrib_position, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), 0);
    glEnableVertexAttribArray(renderer->shaderProgram2D.attrib_texcoord);
    glVertexAttribPointer(renderer->shaderProgram2D.attrib_texcoord, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));

    panel->drawCallback(panel->owner);
    panel_texture(panel);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Renderer* renderer_init(Renderer* r) {
    Renderer* renderer = r ? r : NEW(Renderer, 1);

    shader_program_3D_init(&renderer->shaderProgram3D);

    float mat4[16];
    renderer_3D_update_model(renderer, mat4_identity(mat4));
    renderer_3D_update_ambient(renderer, 0.4);

    float sunPosition[] = { 100, 100, 100 };
    renderer_3D_update_sun_position(renderer, sunPosition);

    shader_program_2D_init(&renderer->shaderProgram2D);

    glActiveTexture(GL_TEXTURE0);
    renderer_2D_update_sampler(renderer, 0);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.50f, 0.75f, 0.86f, 1.0f);
    glClearDepthf(1);
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glLineWidth(2);

    return renderer;
}

void renderer_destroy(Renderer* renderer) {
    shader_program_2D_destroy(&renderer->shaderProgram2D);
    shader_program_3D_destroy(&renderer->shaderProgram3D);
}

void renderer_clear(Renderer* renderer) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void renderer_resize(Renderer* renderer, int width, int height, Camera* camera) {
    glViewport(0, 0, width, height);

    GLfloat mat[16];
    mat4_orthographic(mat, 0, width, 0, height);
    renderer_2D_update_projection(renderer, mat);
    renderer_apply_camera(renderer, camera);
}

void renderer_3D_update_world_position(Renderer* renderer, float* position) {
    shader_program_3D_use(&renderer->shaderProgram3D);
    shader_program_3D_update_world_position(&renderer->shaderProgram3D, position);
}

void renderer_3D_update_model(Renderer* renderer, float* mat4) {
    shader_program_3D_use(&renderer->shaderProgram3D);
    shader_program_3D_update_model(&renderer->shaderProgram3D, mat4);
}

void renderer_3D_update_camera(Renderer* renderer, float* mat4) {
    shader_program_3D_use(&renderer->shaderProgram3D);
    shader_program_3D_update_camera(&renderer->shaderProgram3D, mat4);
}

void renderer_3D_update_projection(Renderer* renderer, float* mat4) {
    shader_program_3D_use(&renderer->shaderProgram3D);
    shader_program_3D_update_projection(&renderer->shaderProgram3D, mat4);
}

void renderer_3D_update_color(Renderer* renderer, float r, float g, float b) {
    shader_program_3D_use(&renderer->shaderProgram3D);
    shader_program_3D_update_color(&renderer->shaderProgram3D, r, g, b);
}

void renderer_3D_update_ambient(Renderer* renderer, float a) {
    shader_program_3D_use(&renderer->shaderProgram3D);
    shader_program_3D_update_ambient(&renderer->shaderProgram3D, a);
}

void renderer_3D_update_sun_position(Renderer* renderer, float* position) {
    shader_program_3D_use(&renderer->shaderProgram3D);
    shader_program_3D_update_sun_position(&renderer->shaderProgram3D, position);
}

void renderer_apply_camera(Renderer* renderer, Camera* camera) {
    float mat[16];
    mat4_multiply(mat, camera->mat_view, camera->mat_model);
    mat4_inverse(mat, mat);
    renderer_3D_update_camera(renderer, mat);

    mat4_perspective(camera->mat_proj, camera->fov, camera->aspect, camera->near, camera->far);
    mat4_inverse(camera->mat_proj_inv, camera->mat_proj);
    renderer_3D_update_projection(renderer, camera->mat_proj);
}

void renderer_3D_use(Renderer* renderer) {
    shader_program_3D_use(&renderer->shaderProgram3D);
}

void renderer_2D_update_projection(Renderer* renderer, float* mat4) {
    shader_program_2D_use(&renderer->shaderProgram2D);
    shader_program_2D_update_projection(&renderer->shaderProgram2D, mat4);
}

void renderer_2D_update_sampler(Renderer* renderer, GLint sampler) {
    shader_program_2D_use(&renderer->shaderProgram2D);
    shader_program_2D_update_sampler(&renderer->shaderProgram2D, sampler);
}

void renderer_2D_use(Renderer* renderer) {
    shader_program_2D_use(&renderer->shaderProgram2D);
}

void renderer_render_ground(Renderer* renderer, Ground* ground, Camera* camera) {
    float worldPosition[] = {
        camera->mat_view[12],
        0,
        camera->mat_view[14],
    };
    renderer_3D_update_world_position(renderer, worldPosition);
    renderer_3D_update_color(renderer, 192, 192, 192);
    renderer_render_mesh(renderer, &ground->mesh, MESH_FILL);
}

void renderer_render_chunk(Renderer* renderer, Chunk* chunk, float* position) {
    renderer_3D_update_world_position(renderer, position);
    linked_list_foreach(&chunk->meshes, render_mesh, renderer);
}

void renderer_render_mesh(Renderer* renderer, Mesh* mesh, char mode) {
    glBindVertexArray(mesh->vao);
    renderer_3D_use(renderer);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);

    glEnableVertexAttribArray(renderer->shaderProgram3D.attrib_position);
    glVertexAttribPointer(renderer->shaderProgram3D.attrib_position, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), 0);
    glEnableVertexAttribArray(renderer->shaderProgram3D.attrib_normal);
    glVertexAttribPointer(renderer->shaderProgram3D.attrib_normal, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));

    for (int q=0; q<mesh->quads.size; q++)
        glDrawElements(mode == MESH_FILL ? GL_TRIANGLE_STRIP : GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, (GLvoid*) (4*q*sizeof(GLushort)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void renderer_render_world(Renderer* renderer, World* world, Camera* camera) {
    renderer_render_ground(renderer, &world->ground, camera);

    linked_list_foreach(&world->chunks, render_world_chunk, renderer);
}

void renderer_render_picker(Renderer* renderer, Picker* picker) {
    GLfloat mat[16];
    GLfloat vec[3];

    if (picker->selection.present) {
        mat4_rotate(mat, NULL, (M_PI/2) * picker->selection.rotation, Y);
        switch (picker->selection.rotation) {
            case 0:
                break;
            case 1:
                vec[0] = 0;
                vec[1] = 0;
                vec[2] = picker->selection.box.width;
                mat4_translate(mat, mat, vec);
                break;
            case 2:
                vec[0] = picker->selection.box.width;
                vec[1] = 0;
                vec[2] = picker->selection.box.length;
                mat4_translate(mat, mat, vec);
                break;
            case 3:
                vec[0] = picker->selection.box.length;
                vec[1] = 0;
                vec[2] = 0;
                mat4_translate(mat, mat, vec);
                break;
            default:
                break;
        }
        renderer_3D_update_model(renderer, mat);
        renderer_3D_update_world_position(renderer, picker->selection.box.position);
        renderer_3D_update_color(renderer, 0,255,255);
        renderer_render_mesh(renderer, &picker->selection.mesh, MESH_LINE);
    }

    if (picker->selection.model) {
        renderer_render_chunk(renderer, picker->selection.model, picker->box.position);
    } else {
        renderer_3D_update_world_position(renderer, picker->box.position);
        renderer_3D_update_color(renderer, 255,255,0);
        renderer_render_mesh(renderer, &picker->mesh, MESH_LINE);
    }

    mat4_identity(mat);
    renderer_3D_update_model(renderer, mat);
}


void renderer_render_panels(Renderer* renderer, LinkedList* panels) {
    linked_list_foreach(panels, render_panel, renderer);
}