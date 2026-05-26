#include "app.h"
#include "draw.h"
#include "graphics.h"
#include "entity.h"

struct Player : Entity {
    f32 speed;
    s32 hp;
};

struct Bullet : Entity {
    Vec3 velocity;
    f32 lifetime;
    s32 damage;
};

struct Enemy : Entity {
    f32 speed;
    s32 hp;
    Entity_Handle target;
};


#define ENTITY_IMPL
#include "entity.h"

// We create all the entities and their handles.
//Entity_Handle playerHandle =
//entity_create(Entity_Kind_Player);
//
void entity_create_many(Entity_Kind kind, s32 count, Entity_Handle* out)
{
    for (s32 i = 0; i < count; i++)
    {
        out[i] = entity_create(kind);
    }
}
//
//Entity_Handle enemies[10];
//entity_create_many(Entity_Kind_Enemy, 10, enemies);
//
//Entity_Handle bullets[10];
//entity_create_many(Entity_Kind_Bullet, 10, bullets);

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
    s32 curr_frame = 0;
    s32 anim_frames = 12;
    f32 frame_duration = 1.f / (f32)anim_frames;
    f32 frame_timer = 0.f;
    //s32 last_frame = frame_count - 1;*/

    entity_storage_init();

    // Crear enemigos

    Entity_Handle enemies[3];
    entity_create_many(Entity_Kind_Enemy, 3, enemies);

	// Inicializar datos de los enemigos

    for (int i = 0; i < 3; i++) {

        Enemy* e = EntityGet(Enemy, enemies[i]);

        e->tex = &monk_run_texture;

        e->frame_count =
            monk_run_texture.subtexs.count;

        e->frame_duration = frame_duration;

        e->pos = { (f32)i * 2.0f, 0, 0 };
    }

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

        for (int i = 0; i < 3; i++)
        {
            Enemy* e = EntityGet(Enemy, enemies[i]);

            draw_sprite(
                e->tex,
                curr_frame,
                e->tint,
                Mat4::transform(
                    e->pos,
                    e->rot,
                    e->scl
                )
            );
        }

        //draw_sprite(&monk_run_texture, curr_frame, Color.White, Mat4::transform(F32.Zero, F32.Zero, Vec3(F32.One) * 3.0f));
        os_swap_buffers();
    }

    entity_storage_done();
    texture_done(&monk_run_texture);
    draw_done();
    app_done();
}