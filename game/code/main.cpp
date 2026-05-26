#include "app.h"
#include "draw.h"
#include "graphics.h"

fn main() -> s32 {
    
    App_Desc desc;
    desc.window.title = L"Survive 2D";
    app_init(desc);
    draw_init();
    
    Texture monk_run_texture;
    {
        Texture_Def def;
        def.kind = Texture_Kind_Multiple;
        def.subtex_size = 192;
        def.filename = "sprites/Units/Blue Units/Monk/Run.png";
        texture_init(&monk_run_texture, def);
    }

    s32 frame_count = monk_run_texture.subtexs.count;
    s32 anim_frames = 12;
    s32 curr_frame = 0;
    s32 last_frame = frame_count - 1;
    f32 frame_duration = 1.0f / (f32) anim_frames; 
    f32 frame_timer = 0.0f;

    while(app_running()) {
       
        frame_timer += os_delta_time();
        
        while(frame_timer >= frame_duration) {
            frame_timer -= frame_duration;
            curr_frame++;
            if (curr_frame >= frame_count) {
                curr_frame = 0;
            }
        }

        clear_back_buffer();
        draw_sprite(&monk_run_texture, curr_frame, Color.White, Mat4::transform(F32.Zero, F32.Zero, Vec3(F32.One) * 3.0f));
        os_swap_buffers();
    }

    texture_done(&monk_run_texture);
    draw_done();
    app_done();
}