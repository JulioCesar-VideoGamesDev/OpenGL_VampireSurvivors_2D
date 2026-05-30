#include <iostream>
#include <cmath>
#include <random>

#include "app.h"
#include "draw.h"
#include "graphics.h"
#include "entity.h"

struct Player : Entity {
    f32 speed = 1.f;
    u32 hp = 1;

    AABB boxCollision2D{};
};

struct Bullet : Entity {
    Vec3 velocity = {0.f, 0.f, 0.f};
    f32 lifetime = 5;
    u32 damage = 1;

    AABB boxCollision2D{};
};

struct Enemy : Entity {
    f32 speed;
    u32 hp;
    Entity_Handle target_handle;
    Player* target;

    AABB boxCollision2D{};
};


#define ENTITY_IMPL
#include "entity.h"

// General Variables
f32 enemySpawningRadius{ 3.f };
const int enemyPoolNumber{ 20 }; // Max amount of enemies at the same time.
f32 enemySpeed{ 1.f };
u32 enemyHp{ 1 };

// Function to create multiple entities of the same type.
void entity_create_many(Entity_Kind kind, u32 count, Entity_Handle* out)
{
    for (u32 i = 0; i < count; i++)
    {
        out[i] = entity_create(kind);
        out->length++;
    }
}

Vec2 GetRandomPointOnCircle(const Vec2& center, float radius)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());

    // Random angle between 0 and 2*PI
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.1415);
    float angle = angleDist(gen);

    Vec2 point;
    point.x = center.x + radius * std::cos(angle);
    point.y = center.y + radius * std::sin(angle);

    return point;
}

Enemy* updateEnemy(Enemy* e)
{
    if (e->enabled)
    {
        // Check overlap
        if (e->boxCollision2D.overlap(e->boxCollision2D, e->target->boxCollision2D))
        {
            e->enabled = false;
            //printf("NO");
            return e;
        }

        e->pos += Vec3(e->target->pos - e->pos).normalized() * e->speed * os_delta_time();

        e->boxCollision2D = updateBoxCollisionPosition2D(e->pos, e->boxCollision2D);
    }
    else
    {
        Vec2 newPosition = GetRandomPointOnCircle({ e->target->pos.x, e->target->pos.y }, enemySpawningRadius);

        e->pos = { newPosition.x, newPosition.y, 0 }; // If the enemy is disable, then enable it and place it in a random position of a circumference around the player. If not then move to the player.
    }
    return e;
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
    //s32 last_frame = frame_count - 1;

    entity_storage_init();

    // Create Player
    Entity_Handle playerHandle[1];
    playerHandle[0] = entity_create(Entity_Kind_Player);

    Player* p = EntityGet(Player, playerHandle[0]);
    p->enabled = true;
    p->tex = &monk_run_texture;
    p->tint = Color.White;
    p->frame_count = monk_run_texture.subtexs.count;
    p->pos = { 0.f, 0.f, 0.f };
    p->scl = { 1.f, 1.f, 1.f };

    p->boxCollision2D = setBoxCollisionSize2D(p->boxCollision2D, 0.1f, 0.1f /* I should not be  hard coding this but I will leave it like this for now */, p->scl);
    p->boxCollision2D = updateBoxCollisionPosition2D(p->pos, p->boxCollision2D);

    // Create Enemies
    Entity_Handle enemiesHandle[enemyPoolNumber];
    entity_create_many(Entity_Kind_Enemy, enemyPoolNumber, enemiesHandle);

    for (int i = 0; i < enemiesHandle->length; i++) {

        Enemy* e = EntityGet(Enemy, enemiesHandle[i]);

        e->enabled = true;

        e->tex = &monk_run_texture;

        e->tint = Color.Red;

        e->frame_count = monk_run_texture.subtexs.count;

        e->frame_duration = frame_duration;

        e->target_handle = playerHandle[0];

        e->target = p;

        Vec2 newPosition{};

        newPosition = GetRandomPointOnCircle({ e->target->pos.x, e->target->pos.y }, enemySpawningRadius);

        e->pos = { newPosition.x, newPosition.y, 0 }; // If the enemy is disable, then enable it and place it in a random position of a circumference around the player. If not then move to the player.
    
        e->scl = Vec3{ 1.f, 1.f, 1.f };

        e->boxCollision2D = setBoxCollisionSize2D(e->boxCollision2D, 0.5f, 0.5f, e->scl);
        e->boxCollision2D = updateBoxCollisionPosition2D(e->pos, e->boxCollision2D);

        e->speed = enemySpeed;
        e->hp = enemyHp;
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

        // UpdateEnemies
        for (int i = 0; i < enemiesHandle->length; i++)
        {
            Enemy* e = EntityGet(Enemy, enemiesHandle[i]);

            e = updateEnemy(e);

            if (e->enabled)
            {
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
        }

        draw_sprite(p->tex, curr_frame, p->tint, Mat4::transform(p->pos, p->rot, p->scl));

        //draw_sprite(&monk_run_texture, curr_frame, Color.White, Mat4::transform(F32.Zero, F32.Zero, Vec3(F32.One) * 3.0f));
        os_swap_buffers();
    }

    entity_storage_done();
    texture_done(&monk_run_texture);
    draw_done();
    app_done();
 }