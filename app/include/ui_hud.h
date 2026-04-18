#ifndef UI_HUD_H
#define UI_HUD_H

void draw_crosshair(int w, int h);
void draw_bottom_hud(int w, int h, double elapsed_time_sec);
void draw_victory_overlay(int w, int h, double final_time_sec);

#endif