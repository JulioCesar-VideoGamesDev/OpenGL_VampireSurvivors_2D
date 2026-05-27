#include <iostream>
#include <cmath>
#include <random>

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

float enemySpawningRadius{ 3.f };

// We create all the entities and their handles.
//Entity_Handle playerHandle =
//entity_create(Entity_Kind_Player);
//
void entity_create_many(Entity_Kind kind, s32 count, Entity_Handle* out)
{
    for (s32 i = 0; i < count; i++)
    {
        out[i] = entity_create(kind);
        out->length++;
    }
}
//
//Entity_Handle enemies[10];
//entity_create_many(Entity_Kind_Enemy, 10, enemies);
//
//Entity_Handle bullets[10];
//entity_create_many(Entity_Kind_Bullet, 10, bullets);

Vec2 GetRandomPointOnCircle(const Vec2& center, float radius)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());

    // Ángulo aleatorio entre 0 y 2π
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.1415);

    float angle = angleDist(gen);

    Vec2 point;
    point.x = center.x + radius * std::cos(angle);
    point.y = center.y + radius * std::sin(angle);

    return point;
}

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

    /*Entity_Handle playerHandle[1];
    playerHandle[1] = entity_create(Entity_Kind_Player);*/

    // Crear enemigos

    Entity_Handle enemiesHandle[3];
    entity_create_many(Entity_Kind_Enemy, 3, enemiesHandle);

	// Update enemies.

    for (int i = 0; i < enemiesHandle->length; i++) {

        Enemy* e = EntityGet(Enemy, enemiesHandle[i]);

        e->enabled = false;

        e->tex = &monk_run_texture;

        e->tint = Color.Red;

        e->frame_count = monk_run_texture.subtexs.count;

        e->frame_duration = frame_duration;

        Vec2 newPosition{};

        if (!e->enabled) // Position around player
        {
            newPosition = GetRandomPointOnCircle(Vec2(0, 0), enemySpawningRadius);
            e->enabled = true;
        }
        else // Move to player
        {
            //Player* p = EntityGet(Player, playerHandle[1]);
            //newPosition = (p->pos - e->pos) * (f32)os_delta_time();
        }

        

        e->pos = { newPosition.x, newPosition.y, 0 }; // If the enemy is disable, then enable it and place it in a random position of a circumference around the player. If not then move to the player.
    }

    while(app_running()) {
       
        frame_timer += os_delta_time();
        
        while(frame_timer >= frame_duration) { // Instead of having one current frame, update the current frame of all entities.
            frame_timer -= frame_duration;
            curr_frame++;
            if (curr_frame >= frame_count) {
                curr_frame = 0;
            }
        }

        clear_back_buffer();

        for (int i = 0; i < 3; i++)
        {
            Enemy* e = EntityGet(Enemy, enemiesHandle[i]);

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