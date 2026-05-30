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

    Vec2 dir = { 0.f, 0.f };
    Vec2 lastDir = { 1.f, 0.f };  // Última dirección válida

    f32 shootCooldown = 0.5f;
    f32 shootTimer = 0.f;

    AABB boxCollision2D{};
};

struct Bullet : Entity {
    f32 speed;
    Vec2 dir = {0.f, 0.f};
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

const u32 enemyPoolNumber{ 10 }; // Max amount of enemies at the same time.
f32 enemySpeed{ 0.5f };

const u32 bulletPoolNumber{ 100 }; // Max amount of enemies at the same time.
f32 bulletSpeed{ 4.f };
f32 bulletLifetime{ 0.5f };

// Function to create multiple entities of the same type.
void entity_create_many(Entity_Kind kind, u32 count, Entity_Handle* out)
{
    for (u32 i = 0; i < count; i++)
    {
        out[i] = entity_create(kind);
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

        Vec3 delta = e->target->pos - e->pos;
        if (delta.lenght() > 0.0001f)
        {
            e->pos += delta.normalized() * e->speed * os_delta_time();
        }

        e->boxCollision2D = updateBoxCollisionPosition2D(e->pos, e->boxCollision2D);
    }
    else
    {
        Vec2 newPosition = GetRandomPointOnCircle({ e->target->pos.x, e->target->pos.y }, enemySpawningRadius);

        e->pos = { newPosition.x, newPosition.y, 0 }; // If the enemy is disable, then enable it and place it in a random position of a circumference around the player. If not then move to the player.
        e->boxCollision2D = updateBoxCollisionPosition2D(e->pos, e->boxCollision2D);

        e->enabled = true;
    }
    return e;
}

void ShootBullet(Player* p, Entity_Handle* bulletHandle, u32 bulletPoolNumber)
{
    for (u32 i = 0; i < bulletPoolNumber; i++)
    {
        Bullet* b = EntityGet(Bullet, bulletHandle[i]);

        if (!b->enabled)
        {
            b->enabled = true;

            b->pos = p->pos;

            b->dir = p->lastDir;

            b->lifetime = 3.f;

            b->boxCollision2D =
                updateBoxCollisionPosition2D(
                    b->pos,
                    b->boxCollision2D
                );

            break;
        }
    }
}

Bullet* UpdateBullet(Bullet* b)
{
    if (!b->enabled) return b;

    b->pos += {b->dir.x* b->speed* os_delta_time(), b->dir.y* b->speed* os_delta_time(), 0};

    b->lifetime -= os_delta_time();

    b->boxCollision2D = updateBoxCollisionPosition2D(b->pos, b->boxCollision2D);

    if (b->lifetime <= 0.f)
    {
        b->enabled = false;
    }

    return b;
}

void CheckBulletEnemyCollision(
    Entity_Handle* bulletsHandle,
    u32 bulletCount,
    Entity_Handle* enemiesHandle,
    u32 enemyCount)
{
    for (u32 i = 0; i < bulletCount; i++)
    {
        Bullet* b = EntityGet(Bullet, bulletsHandle[i]);

        if (!b->enabled)
            continue;

        for (u32 j = 0; j < enemyCount; j++)
        {
            Enemy* e = EntityGet(Enemy, enemiesHandle[j]);

            if (!e->enabled)
                continue;

            if (AABB::overlap(
                b->boxCollision2D,
                e->boxCollision2D))
            {
                b->enabled = false;
                e->enabled = false;

                break;
            }
        }
    }
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

    p->boxCollision2D = setBoxCollisionSize2D(p->boxCollision2D, 0.15f, 0.15f /* I should not be  hard coding this but I will leave it like this for now */, p->scl);
    p->boxCollision2D = updateBoxCollisionPosition2D(p->pos, p->boxCollision2D);

    // Create Bullets
    Entity_Handle bulletHandle[bulletPoolNumber];
    entity_create_many(Entity_Kind_Bullet, bulletPoolNumber, bulletHandle);

    for (int i = 0; i < bulletPoolNumber; i++)
    {

        Bullet* b = EntityGet(Bullet, bulletHandle[i]);

        b->enabled = false;

        b->tex = &monk_run_texture;

        b->tint = Color.Blue;

        b->frame_count = monk_run_texture.subtexs.count;

        b->frame_duration = frame_duration;

        b->pos = { p->pos.x, p->pos.y, 0 }; // If the enemy is disable, then enable it and place it in a random position of a circumference around the player. If not then move to the player.

        b->scl = Vec3{ 1.f, 1.f, 1.f };

        b->boxCollision2D = setBoxCollisionSize2D(b->boxCollision2D, 0.2f, 0.2f, b->scl);
        b->boxCollision2D = updateBoxCollisionPosition2D(b->pos, b->boxCollision2D);

        b->speed = bulletSpeed;
    }

    // Create Enemies
    Entity_Handle enemiesHandle[enemyPoolNumber];
    entity_create_many(Entity_Kind_Enemy, enemyPoolNumber, enemiesHandle);

    for (int i = 0; i < enemyPoolNumber; i++)
    {

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

        e->boxCollision2D = setBoxCollisionSize2D(e->boxCollision2D, 0.2f, 0.2f, e->scl);
        e->boxCollision2D = updateBoxCollisionPosition2D(e->pos, e->boxCollision2D);

        e->speed = enemySpeed;
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

        p->shootTimer -= os_delta_time();

        if (p->shootTimer < 0.f)
        {
            p->shootTimer = 0.f;
        }

        clear_back_buffer();

        // UpdatePlayer
        os_set_cursor_mode(Cursor_Mode::Hidden);
        if (os_key_down('W'))
        {
            p->pos.y += p->speed * os_delta_time();
        }
        if (os_key_down('S'))
        {
            p->pos.y -= p->speed * os_delta_time();
        }
        if (os_key_down('D'))
        {
            p->pos.x += p->speed * os_delta_time();
        }
        if (os_key_down('A'))
        {
            p->pos.x -= p->speed * os_delta_time();
        }

        p->dir = { 0.f, 0.f };

        if (os_key_down('I'))
        {
            p->dir.y = 1;
        }
        if (os_key_down('K'))
        {
            p->dir.y = -1;
        }
        if (os_key_down('L'))
        {
            p->dir.x = 1;
        }
        if (os_key_down('J'))
        {
            p->dir.x = -1;
        }

        if (p->dir.lenght() > 0.001f && p->shootTimer <= 0)
        {
            p->dir = p->dir.normalized();

            p->lastDir = p->dir;

            ShootBullet(p, bulletHandle, bulletPoolNumber);
            p->shootTimer = p->shootCooldown;
        }

        draw_sprite(p->tex, curr_frame, p->tint, Mat4::transform(p->pos, p->rot, p->scl));

        p->boxCollision2D = updateBoxCollisionPosition2D(p->pos, p->boxCollision2D);

        // UpdateBullets
        for (int i = 0; i < bulletPoolNumber; i++)
        {
            Bullet* b = EntityGet(Bullet, bulletHandle[i]);

            b = UpdateBullet(b);

            if (b->enabled)
            {
                draw_sprite(
                    b->tex,
                    curr_frame,
                    b->tint,
                    Mat4::transform(
                        b->pos,
                        b->rot,
                        b->scl
                    )
                );
            }
        }

        // UpdateEnemies
        for (int i = 0; i < enemyPoolNumber; i++)
        {
            Enemy* e = EntityGet(Enemy, enemiesHandle[i]);

            e = updateEnemy(e);

            if (e->enabled)
            {
                draw_sprite(e->tex, e->curr_frame, e->tint, Mat4::transform(e->pos, e->rot, e->scl));
            }
        }

        CheckBulletEnemyCollision(
            bulletHandle,
            bulletPoolNumber,
            enemiesHandle,
            enemyPoolNumber
        );

        //draw_sprite(&monk_run_texture, curr_frame, Color.White, Mat4::transform(F32.Zero, F32.Zero, Vec3(F32.One) * 3.0f));
        os_swap_buffers();
    }

    entity_storage_done();
    texture_done(&monk_run_texture);
    draw_done();
    app_done();
 }