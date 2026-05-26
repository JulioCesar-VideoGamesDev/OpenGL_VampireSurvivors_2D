#include "draw.h"
#include "graphics.h"
#include "io_image.h"

struct {
    Vertex_Buffer vbo;
    Shader shader;
} quad;

struct {
    Global_Buffer gbo;
    Texture white;

    struct {
        Mat4 projection = Mat.Identity4;
        Mat4 transform = Mat.Identity4;
        Vec2 uv_size = F32.One;
        Vec2 uv_offset = F32.Zero;
        Vec4 tint = Color.White;
        s32  tex_unit = 0;
    } global_data;
} scene;

fn draw_init() -> void {

    // Quad init.
    {
        constexpr f32 verts[] = {
            -0.5f, -0.5f,  0.0f, 0.0f, 
            +0.5f, -0.5f,  1.0f, 0.0f,
            +0.5f, +0.5f,  1.0f, 1.0f,
            -0.5f, +0.5f,  0.0f, 1.0f,
        };
        constexpr u32 elems[] = {
            0u, 1u, 2u, // Triangle 1
            2u, 3u, 0u, // Triangle 2
        };
        constexpr Data_Type attrs[] = {
            Data_Type::Float2,
            Data_Type::Float2,
        };

        Vertex_Buffer_Def vbo_def = {
            { verts, sizeof(verts), /* vertex count */ 4 },
            { elems, /* element count */ 6 },
            { attrs, /* attribute count * */ 2 },
        };
        vertex_buffer_init(&quad.vbo, vbo_def);
        
        const char* shader_filename = "shader_sprite.glsl";
        shader_init(&quad.shader, {shader_filename});

        global_buffer_init(&scene.gbo, { sizeof(scene.global_data) });
    }

    Texture_Def def;
    def.image = io_image_white();
    texture_init(&scene.white, def);

    f32 aspect = 16.0f / 9.0f;
    f32 zoom = 3.0f;
    f32 nearpl = -1.0f;
    f32 farpl = +1.0f;
    scene.global_data.projection = Mat4::transpose(Mat4::orthographic(aspect, zoom, nearpl, farpl));
}

fn draw_sprite(const Texture* tex, s32 frame, Vec4 tint, const Mat4& transform) -> void {

    ser_blend_enabled();
    
    if (tex && tex->subtexs.count) {
        if (!ensuref(frame < tex->subtexs.count, "Error! subtex %i does not exist!", 20)) {
            return;
        }
    
        auto& subtex = tex->subtexs.data[frame];

        scene.global_data.uv_size = { 
            (f32) subtex.width  / (f32) tex->width, 
            (f32) subtex.height / (f32) tex->height 
        }; 
        
        scene.global_data.uv_offset = { 
            (f32) subtex.x / (f32) tex->width, 
            1.0f - ((f32)subtex.y + subtex.height) / tex->height
        };
    }

    scene.global_data.transform = Mat4::transpose(transform);
    scene.global_data.tint = tint;

    texture_use(scene.white, 0u);

    if (tex) {
        scene.global_data.tex_unit = 1;
        texture_use(*tex, /* unit */ 1u);
    } else {
        scene.global_data.tex_unit = 0;
    }

    shader_use(quad.shader);

    s32 samplers[32];
    for (s32 i = 0; i < 32; ++i) {
        samplers[i] = i;
    }

    shader_set_param(quad.shader, "u_samplers", samplers, 32);

    global_buffer_update(scene.gbo, &scene.global_data);
    global_buffer_use(scene.gbo);

    vertex_buffer_draw(quad.vbo);
}

fn draw_sprite(Vec4 tint, const Mat4& transform) -> void {
    draw_sprite(nullptr, 0, tint, transform);
}

fn draw_done() -> void {
    global_buffer_done(&scene.gbo);
    vertex_buffer_done(&quad.vbo);
    shader_done(&quad.shader);
}