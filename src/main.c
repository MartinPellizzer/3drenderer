#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "matrix.h"
#include "light.h"
#include "texture.h"
#include "triangle.h"
#include "upng.h"

#define PI 3.141592

enum cull_method {
    CULL_NONE,
    CULL_BACKFACE
} cull_method;

enum render_method {
    RENDER_WIRE,
    RENDER_WIRE_VERTEX,
    RENDER_FILL_TRIANGLE,
    RENDER_FILL_TRIANGLE_WIRE,
    RENDER_TEXTURED,
    RENDER_TEXTURED_WIRE,
} render_method;

triangle_t *triangles_to_render = NULL;

bool is_running = false;
int previous_frame_time = 0;

vec3_t camera_position = { 0, 0, 0 };
mat4_t proj_matrix;

void setup()
{
    render_method = RENDER_TEXTURED_WIRE;
    cull_method = CULL_BACKFACE;

    color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);

    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );

    float fov = PI / 3.0; // 180/3 (60deg)
    float aspect = (float)window_height / (float)window_width;
    float znear = 0.1;
    float zfar = 100.0;

    proj_matrix = mat4_make_perspective(fov, aspect, znear, zfar);    

    // mesh_texture = (uint32_t*)REDBRICK_TEXTURE;
    // texture_width = 64;
    // texture_height = 64;

    // load_cube_mesh_data();
    load_obj_file_data("./assets/cube.obj");

    load_png_texture_data("./assets/cube.png");
}

void process_input()
{
    SDL_Event event;
    SDL_PollEvent(&event);
    switch (event.type)
    {
        case SDL_QUIT:
            is_running = false;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
                is_running = false;
            if (event.key.keysym.sym == SDLK_1)
                render_method = RENDER_WIRE_VERTEX;
            if (event.key.keysym.sym == SDLK_2)
                render_method = RENDER_WIRE;
            if (event.key.keysym.sym == SDLK_3)
                render_method = RENDER_FILL_TRIANGLE;
            if (event.key.keysym.sym == SDLK_4)
                render_method = RENDER_FILL_TRIANGLE_WIRE;
            if (event.key.keysym.sym == SDLK_5)
                render_method = RENDER_TEXTURED;
            if (event.key.keysym.sym == SDLK_6)
                render_method = RENDER_TEXTURED_WIRE;
            if (event.key.keysym.sym == SDLK_c)
                cull_method = CULL_BACKFACE;
            if (event.key.keysym.sym == SDLK_d)
                cull_method = CULL_NONE;
            break;
    }
}

void update()
{
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
    {
        SDL_Delay(time_to_wait);
    }
    previous_frame_time = SDL_GetTicks();

    triangles_to_render = NULL;
    
    // mesh.rotation.x += 0.01;
    mesh.rotation.y += 0.01;
    // mesh.rotation.z += 0.01;

    // mesh.scale.x += 0.002;
    // mesh.scale.y += 0.001;

    // mesh.translation.x += 0.01;
    mesh.translation.z = 5;

    mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
    mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);

    int num_faces = array_length(mesh.faces);
    for (int i = 0; i < num_faces; i++)
    {
        face_t mesh_face = mesh.faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a - 1];
        face_vertices[1] = mesh.vertices[mesh_face.b - 1];
        face_vertices[2] = mesh.vertices[mesh_face.c - 1];

        vec4_t transformed_vertices[3];

        for (int j = 0; j < 3; j++)
        {
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);
            
            mat4_t world_matrix = mat4_identity();
            world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
            world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

            transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);
            
            transformed_vertices[j] = transformed_vertex;
        }
        
        // backface culling
        vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]);
        vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]);
        vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]);

        vec3_t vector_ab = vec3_sub(vector_b, vector_a);
        vec3_t vector_ac = vec3_sub(vector_c, vector_a);
        vec3_normalize(&vector_ab);
        vec3_normalize(&vector_ac);

        vec3_t normal = vec3_cross(vector_ab, vector_ac);
        vec3_normalize(&normal);

        vec3_t camera_ray = vec3_sub(camera_position, vector_a);

        float dot_normal_camera = vec3_dot(normal, camera_ray);

        if (cull_method == CULL_BACKFACE)
        {
            if (dot_normal_camera < 0)
            {
                continue;
            }
        }

        vec4_t projected_points[3];

        // project
        for (int j = 0; j < 3; j++)
        {
            projected_points[j] = mat4_mul_vec4_project(proj_matrix, transformed_vertices[j]);

            // scale into the view
            projected_points[j].x *= (window_width / 2.0);
            projected_points[j].y *= (window_height / 2.0);

            projected_points[j].y *= -1;

            // translate to middle of screen
            projected_points[j].x += (window_width / 2.0); 
            projected_points[j].y += (window_height / 2.0);
        }
        
        /// avg depth
        float avg_depth = (transformed_vertices[0].z + transformed_vertices[1].z + transformed_vertices[2].z) / 3.0;

        /// shading
        float light_intensity_factor = -vec3_dot(normal, light.direction);
        uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);
        

        triangle_t projected_triangle = {
            .points = {
                {projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w},
                {projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w},
                {projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w},
            },
            .texcoords = {
                { mesh_face.a_uv.u, mesh_face.a_uv.v },
                { mesh_face.b_uv.u, mesh_face.b_uv.v },
                { mesh_face.c_uv.u, mesh_face.c_uv.v },
            },
            .color = triangle_color,
            .avg_depth = avg_depth,
        };

        array_push(triangles_to_render, projected_triangle);
    }

    // sort by depth
    int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles; i++)
    {
        for (int j = i; j < num_triangles; j++)
        {
            if (triangles_to_render[i].avg_depth < triangles_to_render[j].avg_depth)
            {
                triangle_t temp = triangles_to_render[i];
                triangles_to_render[i] = triangles_to_render[j];
                triangles_to_render[j] = temp;
            }
        }
    }
}

void render()
{
    clear_color_buffer(0xFF111111);

    draw_grid_dots(0xFF888888, 10);
    // draw_rect(100, 300, 800, 500, 0xFF0000FF);

    int num_triangles = array_length(triangles_to_render);   
    for (int i = 0; i < num_triangles; i++)
    {
        triangle_t triangle = triangles_to_render[i];

        // draw triangle
        int x0 = triangle.points[0].x;
        int y0 = triangle.points[0].y;
        int x1 = triangle.points[1].x;
        int y1 = triangle.points[1].y;
        int x2 = triangle.points[2].x;
        int y2 = triangle.points[2].y;

        // draw fill
        if(render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE)
        {
            draw_filled_triangle(x0, y0, x1, y1, x2, y2, triangle.color);
        }

        // draw texture
        if(render_method == RENDER_TEXTURED || render_method == RENDER_TEXTURED_WIRE)
        {
            draw_textured_triangle(
                triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.texcoords[0].u, triangle.texcoords[0].v,
                triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.texcoords[1].u, triangle.texcoords[1].v,
                triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.texcoords[2].u, triangle.texcoords[2].v,
                mesh_texture
            );
        }

        // draw wire
        if(render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX || render_method == RENDER_FILL_TRIANGLE_WIRE || render_method == RENDER_TEXTURED_WIRE)
        {
            draw_triangle(x0, y0, x1, y1, x2, y2, 0xFFFFFFFF);
        }

        // draw vertex
        if(render_method == RENDER_WIRE_VERTEX)
        {
            draw_rect(triangle.points[0].x, triangle.points[0].y, 3, 3, 0xFFFF0000);
            draw_rect(triangle.points[1].x, triangle.points[1].y, 3, 3, 0xFFFF0000);
            draw_rect(triangle.points[2].x, triangle.points[2].y, 3, 3, 0xFFFF0000);
        }

    }

    // draw_filled_triangle(300, 100, 50, 400, 500, 700, 0xFF00FF00);

    array_free(triangles_to_render);

    render_color_buffer();

    SDL_RenderPresent(renderer);
}

void free_resources()
{
    free(color_buffer);
    upng_free(png_texture);
    array_free(mesh.faces);
    array_free(mesh.vertices);
}

int main(void)
{
    is_running = initialize_window(); 

    setup();

    if (1)
    {
        while (is_running)
        {
            process_input();
            update();
            render();
        }
    }

    destroy_window();
    free_resources();

    return 0;
}

