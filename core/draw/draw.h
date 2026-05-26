#pragma once
#include "graphics.h"

fn draw_init() -> void;
fn draw_sprite(const Texture* tex, s32 sprite, Vec4 tint, const Mat4& transform) -> void;
fn draw_sprite(Vec4 tint, const Mat4& transform) -> void;
fn draw_done() -> void;