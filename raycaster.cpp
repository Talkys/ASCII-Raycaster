#define _USE_MATH_DEFINES
#include <iostream>
#include <cmath>
#include <curses.h>
#include <signal.h>
#include <vector>
using namespace std;

float ray(float, float, float, float, string, int, float, float);
void raycast_in_fov(float[], float, float, float, float, string, int, float, int, float);
int is_wall(float, float, string, int);
void set_steps(float &, float &, float, float magnitude = 1);
void render_view(float[], string, int, int, float, float, int, int, float, float, float);
void player_movement(int, float, float &, float &, float, string, int);
int get_predominant_bearing(float);
int get_player_char(float);
void turn(float, float &);
void set_starting_loc(float &, float &, string, int, int);

const string _MAP[] = {
    "##############",
    "#.........#..#",
    "#.........#..#",
    "#..####..##..#",
    "#..#.........#",
    "#..#.#####...#",
    "#............#",
    "##############"
};
const int _MAP_WIDTH  = _MAP[0].length();
const int _MAP_HEIGHT = (sizeof(_MAP) / sizeof(_MAP[0]));

/*Configuracoes*/
const float _FOV = 50;
const float _VIEW_DISTANCE = 3.5;
const float _MOVEMENT_DISTANCE = 0.1;
const float _SPRINTING_DISTANCE = .2;
const float _RETINAL_DISTANCE = .5;
const float _RAY_RESOLUTION = .001;
const float _KEY_TURN_AMT = .02;
const float _MOUSE_SENSITIVITY = .01;

int main()
{
    /*Inicialização geral*/
    float player_x = 1.5;
    float player_y = 1.5;
    float player_a = 0.5;

    int term_width = 120;
    int term_height = 55;
    float* distances = new float[term_width];

    string map = "";
    for(int i = 0; i < _MAP_HEIGHT; i++)
        map += _MAP[i];

    set_starting_loc(
        player_x,
        player_y,
        map,
        _MAP_WIDTH,
        _MAP_HEIGHT
    );

    /*Inicialização ncurses*/
    initscr();
    //resize_term(term_height, term_width);
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    nonl();
    cbreak();
    nodelay(stdscr, TRUE);
    echo();
    if(has_colors())
    {
        start_color();
        init_pair(1, COLOR_BLACK, COLOR_BLACK);   // BLACK
        init_pair(2, COLOR_RED, COLOR_RED);       // RED
        init_pair(3, COLOR_GREEN, COLOR_GREEN);   // GREEN
        init_pair(4, COLOR_YELLOW, COLOR_YELLOW); // YELLOW
        init_pair(5, COLOR_BLUE, COLOR_BLUE);     // BLUE
        init_pair(6, COLOR_MAGENTA, COLOR_MAGENTA); // MAGENTA
        init_pair(7, COLOR_CYAN, COLOR_CYAN);     // CYAN
        init_pair(8, COLOR_WHITE, COLOR_WHITE);   // WHITE
    }

    /*Modificações ANSI*/
    char resize_buff[30];
    sprintf(resize_buff,
            "printf '\e[8;%d;%dt'",
            term_height, term_width); // ANSI sequence, resizes terminal
    system(resize_buff);
    system("printf '\e[?25l'");

    /*Loop do jogo*/
    int input;
    bool keep_going = true;
    float mouse_last = 0;
    char buffer[512];
    size_t max_size = sizeof(buffer);

    while(keep_going)
    {
        input = getch();
        switch(input)
        {
            case 'w':
                player_movement(0, _MOVEMENT_DISTANCE, player_x, player_y, player_a, map, _MAP_WIDTH);
                break;
            case 'd':
                player_movement(1, _MOVEMENT_DISTANCE, player_x, player_y, player_a, map, _MAP_WIDTH);
                break;
            case 's':
                player_movement(2, _MOVEMENT_DISTANCE, player_x, player_y, player_a, map, _MAP_WIDTH);
                break;
            case 'a':
                player_movement(3, _MOVEMENT_DISTANCE, player_x, player_y, player_a, map, _MAP_WIDTH);
                break;
            case KEY_BACKSPACE:
                keep_going = false;
                break;
            case KEY_LEFT:
            case 'q':
                turn(-1 * _KEY_TURN_AMT, player_a);
                break;
            case KEY_RIGHT:
            case 'e':
                turn(_KEY_TURN_AMT, player_a);
                break;
        }

        raycast_in_fov(
            distances,
            player_x,
            player_y, 
            player_a,
            _VIEW_DISTANCE,
            map,
            _MAP_WIDTH,
            _FOV,
            term_width,
            _RAY_RESOLUTION
        );

        render_view(
            distances,
            map,
            _MAP_WIDTH,
            _MAP_HEIGHT,
            player_x,
            player_y,
            term_width,
            term_height,
            _RETINAL_DISTANCE,
            player_a,
            _VIEW_DISTANCE
        );
    }

    /*Limpeza*/
    system("printf '\e[?25h'");
    delete distances;
    endwin();
    return 0;
}

void cast_shadow(int shade, int x, int y) {
    attron(COLOR_PAIR(shade));
    mvaddch(y, x, ' ');
    attroff(COLOR_PAIR(shade));
}

void render_view(
    float distances[], string map,
    int map_width, int map_height,
    float player_x, float player_y,
    int terminal_width, int terminal_height,
    float retinal_distance, float player_angle,
    float view_distance
)
{
    float sky_size, projection_height;

    for(int x = terminal_width - 1; x >= 0; x--)
    {
        float distance_at_x = distances[x];
        projection_height = terminal_height * (retinal_distance / distance_at_x);

        for(int y = 0; y < terminal_height; y++)
        {
            if(distances[x] >= 0)
            {
                sky_size = (terminal_height - projection_height) * 2.f /3.f;
                if(y <= sky_size)
                    cast_shadow(1, x, y);
                else if(y <= (sky_size + projection_height))
                {
                    if (distances[x] <= view_distance * 1.f / 5.f)

                        cast_shadow(8, x, y);
                    else if (distances[x] <= view_distance * 2.f / 5.f)
                        cast_shadow(7, x, y);
                    else if (distances[x] <= view_distance * 3.f / 5.f)
                        cast_shadow(6, x, y);
                    else if (distances[x] <= view_distance * 4.f / 5.f)
                        cast_shadow(5, x, y);
                    else if (distances[x] <= view_distance * 5.f / 6.f)
                        cast_shadow(4, x, y);
                    else if (distances[x] <= view_distance * 11.f / 12.f)
                        cast_shadow(3, x, y);
                    else
                        cast_shadow(2, x, y);
                }
                else
                {
                    cast_shadow(1, x, y);
                }
            }
            else
            {
                projection_height = terminal_height * (retinal_distance / (view_distance + 1));
                cast_shadow(1, x, y);
            }
        }
    }
    refresh();
}

void raycast_in_fov(float distances[],
                    float player_x, float player_y,
                    float player_angle, float view_distance,
                    string map, int map_width,
                    float fov, int screen_resolution,
                    float ray_resolution
)
{
    fov = fov / 360.f; // Convert fov to 0-1 float as in angle
    float x_step, y_step;
    float curr_ray_angle = player_angle - (fov / 2);
    float ray_angle_step = fov / float(screen_resolution);

    // Send out rays
    for (int i = 0; i < screen_resolution; i++)
    {
        set_steps(x_step, y_step, curr_ray_angle, ray_resolution);
        distances[i] = ray(player_x, player_y, x_step, y_step, map, map_width, view_distance, ray_resolution);
        curr_ray_angle += ray_angle_step;
    }
}

float ray(float x, float y,
          float x_step, float y_step,
          string map, int map_width,
          float view_distance, float ray_resolution)
{
    for (float i = ray_resolution; i < view_distance; i += ray_resolution)
    {
        float init_x = x;
        float init_y = y;
        x += x_step;
        y += y_step;

        if (is_wall(int(x), int(y), map, map_width) > 0)
        {
            return i;
        }
    }

    return -1.0;
}

void turn(float change, float &player_angle)
{
    player_angle += change;
    if(player_angle >= 1)
        player_angle -= 1;
    else if(player_angle < 0)
        player_angle += 1;
}

void player_movement(
    int direction, float distance,
    float &player_x, float &player_y,
    float player_angle, string map,
    int map_width
)
{
    bool valid_move = true;
    float destination_x;
    float hitbox_x;
    float destination_y;
    float hitbox_y;

    if(direction >= 1 && direction <= 3)
        set_steps(destination_x, destination_y, player_angle + (.25 * (float)direction));
    else set_steps(destination_x, destination_y, player_angle);

    hitbox_x = (destination_x * distance * 2) + player_x;
    hitbox_y = (destination_y * distance * 2) + player_y;
    destination_x = (destination_x * distance) + player_x;
    destination_y = (destination_y * distance) + player_y;

    valid_move = is_wall(destination_x, destination_y, map, map_width) == 0 && is_wall(hitbox_x, hitbox_y, map, map_width) == 0;

    if(valid_move)
    {
        player_x = destination_x;
        player_y = destination_y;
    } 
}

int is_wall(float x, float y, string map, int map_width)
{
    char maybe_wall = map[int(y) * map_width + int(x)];
    
    if(maybe_wall == '#')
        return 1;
    else if(maybe_wall == '!')
        return 2;
    else
        return 0;
}

int get_predominant_bearing(float angle)
{
    if(angle <= 1.0)
        return static_cast<int>(angle * 8);
    else
        return 7;
}

void set_steps(float &x_step, float &y_step, float angle, float magnitude)
{
    if (angle >= .875)
        angle -= 1;

    float angle_in_rads = 2 * M_PI * angle; // Convert to radians
    angle_in_rads -= M_PI_2;                // Shift angle

    x_step = cos(angle_in_rads) * magnitude;
    y_step = sin(angle_in_rads) * magnitude;
}

void set_starting_loc(float &player_x, float &player_y, string map, int map_width, int map_height)
{
    for (int x = 0; x < map_width; x++)
        for (int y = 0; y < map_height; y++)
            if (map[int(y) * map_width + int(x)] == 'S')
            {
                player_x = x + .5;
                player_y = y + .5;
            }
}


