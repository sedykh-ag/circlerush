#include "Engine.h"
#include <stdlib.h>
#include <memory.h>

#include <vector>
#include <queue>
#include "vec3.h"
#include "geometry.h"
#include "utils.h"

//
//  You are free to modify this file
//

//  is_key_pressed(int button_vk_code) - check if a key is pressed,
//                                       use keycodes (VK_SPACE, VK_RIGHT, VK_LEFT, VK_UP, VK_DOWN, 'A', 'B')
//
//  get_cursor_x(), get_cursor_y() - get mouse cursor position
//  is_mouse_button_pressed(int button) - check if mouse button is pressed (0 - left button, 1 - right button)
//  clear_buffer() - set all pixels in buffer to 'black'
//  is_window_active() - returns true if window is active
//  schedule_quit_game() - quit game after act()

// game variables

const float spawn_wait = 5.0f;
const int spawn_limit = 3;
const double rect_size = 0.24;
const double ball_size = 0.12;
double angular_velocity = 3.0;
const double balls_to_rects_ratio = 0.3;
const double projectiles_velocity = 0.7;
const float gameover_wait = 5.0f;


bool space_pressed = false;
float time_since_spawn = 0.0f;
float time_since_gameover = 0.0f;
bool gameover = false;

// Coordinate system
auto origin = point3(0, 0, 0);
auto aspect_ratio = double(SCREEN_WIDTH) / double(SCREEN_HEIGHT);
double h = 4.0;
double v = 3.0;
auto horizontal = point3(h, 0.0, 0.0);
auto vertical = point3(0.0, v, 0.0);
auto ll_corner = origin - horizontal/2 - vertical/2;

// Geometry
Sphere sphere(point3(0, 0, 0), 1.0, 0x889CB5); // background sphere
std::vector<Sphere> main_balls;
std::vector<Rectangle> rects;
std::vector<Sphere> balls;
std::vector<Explosion> explosions;

// initialize game data in this function
void initialize()
{
  main_balls.clear();
  rects.clear();
  balls.clear();
  explosions.clear();

  main_balls.push_back(Sphere(point3(1.0, 0.0, 0.0), 0.14, 0xFFFFFF));
  main_balls.push_back(Sphere(point3(-1.0, 0.0, 0.0), 0.14, 0xFFFFFF));

  time_since_spawn = 0.0;
  time_since_gameover = 0.0;
  gameover = false;
}

// this function is called to update game data,
// dt - time elapsed since the previous update (in seconds)
void act(float dt)
{
  if (is_key_pressed(VK_ESCAPE))
    schedule_quit_game();
  
  if (gameover && time_since_gameover < gameover_wait)
    time_since_gameover += dt;
  else if (gameover && time_since_gameover > gameover_wait)
    initialize();

  if (!space_pressed && is_key_pressed(VK_SPACE)) {
    space_pressed = true;
    angular_velocity = -angular_velocity;
  }
  else if (space_pressed && !is_key_pressed(VK_SPACE)) {
    space_pressed = false;
  }

  // spawn loop
  bool spawn_rectangle = randrange(0.0, 1.0) > balls_to_rects_ratio;

  // spawn rectangles
  if (spawn_rectangle && rects.size() <= spawn_limit && time_since_spawn > spawn_wait) {
    point3 r_corner = point3(-h / 2, randrange(-v / 2, v / 2), 0);
    point3 l_corner = r_corner - vec3(rect_size, rect_size, 0);
    vec3 velocity = -r_corner + (vertical / 2) * randrange(-1.0, 1.0);
    velocity = velocity * projectiles_velocity;
    rects.push_back(Rectangle(l_corner, r_corner, velocity, 0x000000));
    
    time_since_spawn = 0.0;
  } else { time_since_spawn += dt; }

  // spawn balls
  if (!spawn_rectangle && balls.size() <= spawn_limit && time_since_spawn > spawn_wait) {
    point3 origin = point3(-h / 2 - ball_size, 0, 0);
    vec3 velocity = -origin + (vertical / 2) * randrange(-1.0, 1.0);
    velocity = velocity * projectiles_velocity;
    balls.push_back(Sphere(origin, ball_size, velocity, 0xFFFFFF));

    time_since_spawn = 0.0;
  } else { time_since_spawn += dt; }


  // update loop
  for (auto& main_ball : main_balls)
    main_ball.orig = rotate(main_ball.orig, angular_velocity * dt);
  for (auto& rect : rects)
    rect.update(dt);
  for (auto& ball : balls)
    ball.update(dt);
  for (auto& e : explosions)
    e.update(dt);

  // remove loop
  // new collisions
  for (auto& main_ball : main_balls) {
    // collisions with balls
    for (auto it = balls.begin(); it != balls.end(); ++it) {
      if (dist(main_ball.orig, it->orig) < main_ball.rad + it->rad) {
        explosions.push_back(Explosion(it->orig, 0xFFFFFF));
        balls.erase(it);
        break;
      }
    }
    // collisions with rects
    for (auto it = rects.begin(); it != rects.end(); ++it) {
      if (dist(main_ball.orig, it->orig) < main_ball.rad + it->len / 2) {
        explosions.push_back(Explosion(it->orig, 0x000000));
        explosions.push_back(Explosion(main_balls[0].orig, 0xFFFFFF));
        explosions.push_back(Explosion(main_balls[1].orig, 0xFFFFFF));
        rects.erase(it);

        // game over
        gameover = true;
        main_balls.clear();

        break;
      }
    }
  }
  
  // remove rectangles
  for (auto it = rects.begin(); it != rects.end(); ++it) {
    if (it->l_corner.x() > h / 2 || it->r_corner.x() < -h / 2
      || it->l_corner.y() > v / 2 || it->r_corner.y() < -v / 2) {
      rects.erase(it);
      break;
    }
  }
  
  // remove spheres
  for (auto it = balls.begin(); it != balls.end(); ++it) {
    point3& orig = it->orig;
    if (orig.x() > h/2 + ball_size
        || orig.x() < -h/2 - ball_size
        || orig.y() > v/2 + ball_size 
        || orig.y() < -v/2 - ball_size) {
      balls.erase(it);
      break;
    }
  }

  // remove explosions
  for (auto it = explosions.begin(); it != explosions.end(); ++it) {
    if (!it->alive) {
      explosions.erase(it);
      break;
    }
  }
}

// fill buffer in this function
// uint32_t buffer[SCREEN_HEIGHT][SCREEN_WIDTH] - is an array of 32-bit colors (8 bits per R, G, B)
void draw()
{
  // clear backbuffer
  memset(buffer, 0, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint32_t));

  /*
    This way of rendering is used in raytracing and is very ineffective
    Polygon rendering with rasterization would be many times faster
    Unfortunately, I don't have much time left to implement it...
  */
  for (int j = SCREEN_HEIGHT - 1; j >= 0; --j)
    for (int i = 0; i < SCREEN_WIDTH; ++i)
    {
      // background color
      buffer[j][i] = 0x92A8C3;

      auto u = double(i) / SCREEN_WIDTH;
      auto v = double(j) / SCREEN_HEIGHT;
      point3 pos = ll_corner + u * horizontal + v * vertical;

      if (sphere.in(pos))
        buffer[j][i] = sphere.color;

      for (auto& main_ball : main_balls) {
        if (main_ball.in(pos))
          buffer[j][i] = main_ball.color;
      }

      for (auto& rect : rects) {
        if (rect.in(pos))
          buffer[j][i] = rect.color;
      }

      for (auto& ball : balls) {
        if (ball.in(pos))
          buffer[j][i] = ball.color;
      }

      for (auto& p : explosions) {
        if (p.in(pos))
          buffer[j][i] = p.color;
      }
    }

}

// free game data in this function
void finalize()
{
  // everything was allocated on the stack
  // no need to free anything
}