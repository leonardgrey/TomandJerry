#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <cab202_graphics.h>
#include <cab202_timers.h>

#include <time.h>

#include <stdarg.h>

#ifndef M_PI
#define M_PI        3.14159265358979323846264338327950288   /* pi             */
#endif

/* 
gcc testing.c -o testing -I../ZDK -L../ZDK -lzdk -lncurses
*/

// defines the global variables
// game state
bool game_over = false;
bool game_paused = false;
// scores
int tom_score_current = 0;
int jerry_score_current = 0;
// current level scores
int cheese_consumed = 0;
int c_score_tom = 0;

char player_current = 'J';
// entitiy counts
int cheese_current = 0;
int trap_current = 0;
int firework_current = 0;
// times
double initial_time;
double game_time;
int minutes = 0;

int level_current = 1;
int W, H;

// jerry variables
int jerry_lives = 5;
int j_x, j_y, j_init_x=10, j_init_y=10;
double j_dx=0.07, j_dy=0.07;
double jxx, jyy;
#define JERRY_IMG 'J'
// tom variables
int tom_lives = 5;
double t_x, t_y, t_dx, t_dy, t_init_x=30, t_init_y=30;
#define TOM_IMG 'T'
// array of struct, stores pixels 'x', 'y' and 'angle' location
int index_wall = 0;
struct walls 
{
    int x1;
    int y1;

    double angle; 
} global_walls[80000];

#define CHEESE_IMG 'C'
#define TRAP_IMG '#'
#define DOOR_IMG 'X'
#define FIREWORK_IMAGE 'R'

struct global_entitities
{
    int x, y;

} global_cheese[5], global_trap[5], door_loc;

struct global_fireworks
{
    double x, y;
} global_firework[50];

// returns the max value of 2 integers
int find_max(int one, int two) {
    if(one > two) { return one; } 
    else { return two; }
}
//returns the min value of 2 integers
int find_min(int one, int two) {
    if(one < two) { return one; } 
    else { return two; }
}
// checks if an element is in the 'y' array
bool value_in_array(int size, int element) {
    for (int i = 0; i < size; i++)
    {
        if(global_walls[i].y1 == element) { return true; }
        else { return false; }
    }
}
// returns the calculated y from y=mx+c
double straight_line(double x1, double y1, double x2, double y2, double x) {
    double m = (y2 - y1) / (x2 - x1);            
    double c = y1 - (x1 * m);

    double y = (m * x) + c; 
    return y; 
}
// loads the file and calculated the y,x of jerry tom and walls
void load_File(FILE *stream) {
    while(!feof(stream)) 
    {
        char command;
        double arg1, arg2, arg3, arg4;

        int items_scanned = fscanf(stream, "%c %lf %lf %lf %lf", &command, &arg1, &arg2, &arg3, &arg4);
        
        if(items_scanned == 3) 
        {
            if(command == 'J') 
            {
                j_init_x = round(arg1 * W);
                j_init_y = round(arg2 * H) + 5;
            }
            else if(command == 'T') 
            {
                t_init_x = round(arg1 * W);
                t_init_y = round(arg2 * H) + 5;
            }
        }
        else if(items_scanned == 5) 
        {          
            int x1 = round(arg1 * W) ;
            int y1 = round(arg2 * (H-5)+5);
            int x2 = round(arg3 * W) ;
            int y2 = round(arg4 * (H-5)+5);
            
            // if its a vertical wall
            if(x1 == x2) 
            {
                int max = find_max(y1, y2);
                int min = find_min(y1, y2);
                
                int ind_y = min;   
                // loop throw from the top y to the low y value
                while(ind_y <= max) 
                {
                    // adds the static x location and the y value (added each loop), angle it 90 for vertical walls
                    global_walls[index_wall].x1 = x1;
                    global_walls[index_wall].y1 = ind_y;
                    global_walls[index_wall].angle = 90.00;

                    ind_y++;
                    index_wall++;
                }
            }
            else if(y1 == y2) 
            {
                int max = find_max(x1, x2);
                int min = find_min(x1, x2);

                int ind_x = min; 

                while(ind_x <= max) 
                {
                    // adds the static y location and x value, angle 0 for vertical walls
                    global_walls[index_wall].x1 = ind_x;
                    global_walls[index_wall].y1 = y1;
                    global_walls[index_wall].angle = 0.00;

                    ind_x++;
                    index_wall++;
                }
            } 
            // angled walls
            else
            { 
                // loops through from the smallest x to the biggest x
                // i gets increased by 0.2 each loop
                for (double i = find_min(x1, x2); i < find_max(x1, x2); i+=0.2)
                {
                    // get the y_loc. use the 4 points to calc m,c then i to get y_loc
                    double y_loc = straight_line(x1, y1, x2, y2, i);
                    // if the rounded yloc isnt already added to walls
                    if(!value_in_array(index_wall, round(y_loc)))
                    {
                        // add the wall x,y to the walls array.
                        global_walls[index_wall].x1 = round(i);
                        global_walls[index_wall].y1 = round(y_loc);
                        index_wall++;
                    }
                }
            }
        }
    }
}
// adds the border x,y to the walls array
void add_border() {
    for (int i = 0; i < W-1; i++)
    {
        global_walls[index_wall].x1 = i;
        global_walls[index_wall].y1 = 4;
        global_walls[index_wall].angle = 0.00;
        index_wall++;

        global_walls[index_wall].x1 = i;
        global_walls[index_wall].y1 = H-1;
        global_walls[index_wall].angle = 0.00;
        index_wall++;
    }
    for (int i = 4; i < H-1; i++)
    {
        global_walls[index_wall].x1 = 0;
        global_walls[index_wall].y1 = i;
        global_walls[index_wall].angle = 90.00;
        index_wall++;

        global_walls[index_wall].x1 = W-1;
        global_walls[index_wall].y1 = i;
        global_walls[index_wall].angle = 90.00;
        index_wall++;
    }
}

void end_game(int result) {
    clear_screen();
    if(result == 1)
    {
        char lost_msg_1[40];
        snprintf(lost_msg_1, 40, "You made it to level %d", level_current);
        char lost_msg_2[50];
        snprintf(lost_msg_2, 50, "Jerry: %d   Tom: %d", jerry_score_current, tom_score_current);
        draw_string( (W/2)-11, (H/2)-1, lost_msg_1 );
        draw_string( (W/2)-9, (H/2)+1, lost_msg_2 );
    }
    else if (result == 2)
    {
        char win_msg_1[] = "You won by completing all the levels";
        char win_msg_2[50];
        snprintf(win_msg_2, 50, "Jerry: %d   Tom: %d", jerry_score_current, tom_score_current);
        draw_string( (W/2)-18, (H/2)-1, win_msg_1 );
        draw_string( (W/2)-9, (H/2)+1, win_msg_2 );
    }
    show_screen();
    wait_char();
    game_over = true;
}
// loads the current_level(arg) file
void do_load_file(int argc, char *argv[], int level) {
    for (int i = 1; i < argc; i++) 
    {
        FILE *stream = fopen(argv[level], "r");
        if(stream != NULL) 
        {
            load_File(stream);
            fclose(stream);
        }
        else
        {
            end_game(2);
        }
            
        show_screen();
    }
}

void draw_statusbar(int score_current, int lives_current) {

    char student_number[] = "Student number: n10479732";
    draw_string(0, 0, student_number);

    char score[30];
    snprintf(score, 20, "Score: %d", score_current);
    draw_string(30, 0, score);

    char lives[20];
    snprintf(lives, 20, "Lives: %d", lives_current);
    draw_string(60, 0, lives);

    char player[15];
    snprintf(player, 20, "Player: %c", player_current);
    draw_string(90, 0, player);

    char time[30];
    snprintf(time, 20, "Time: %d:%.0lf", minutes, game_time);
    draw_string(30, 3, time);

    char cheese[30];
    snprintf(cheese, 20, "Cheese: %d", cheese_current);
    draw_string(0, 2, cheese);

    char trap[30];
    snprintf(trap, 20, "Traps: %d", trap_current);
    draw_string(30, 2, trap);

    char fireworks[30];
    snprintf(fireworks, 20, "Firework: %d", firework_current);
    draw_string(60, 2, fireworks);

    char lvl[1]; 
    snprintf(lvl, 20, "Level: %d", level_current);
    draw_string(90, 2, lvl);

    char cheese_consumedd[30];

    snprintf(cheese_consumedd, 60, "Cheese eaten: %d", cheese_consumed);
    draw_string(60, 3, cheese_consumedd);

    char paused[20];
    snprintf(paused, 10, "%s", game_paused ? "Paused" : "Unpaused");
    draw_string(90, 3,  paused);
}

void draw_map() {
    
    const int ch = '*';
    int current_score;
    int current_lives;

    if(player_current == 'J') 
    { 
        current_score = jerry_score_current;  
        current_lives = jerry_lives;

    }
    else if(player_current == 'T') 
    { 
        current_score=tom_score_current; 
        current_lives = tom_lives;
    }

    draw_statusbar(current_score, current_lives);
    // loops through the walls array and draws a * at x,y
    for (int i = 0; i < index_wall; i++)
    {
        int x = global_walls[i].x1;
        int y = global_walls[i].y1;

        draw_char(x, y, ch);
    }
}
// checks if x,y is at a wall location
bool wall_location(int x, int y) {
    if(x == 1 || x == W-1 || y <= 4 || y == H-1)
    {
        return true;
    }
    // loops through the walls array
    for (int i = 0; i < index_wall; i++)
    {
        int wall_x = global_walls[i].x1;
        int wall_y = global_walls[i].y1;
        double wall_angle = global_walls[i].angle;    

        if (x == wall_x && y == wall_y)
        {
            return true;
        }       
    }    
}

// checks if x,y is at an entity location ie cheese
bool entity_location(int x, int y)
{
    for (int i = 0; i < 5; i++)
    {
        if( (x == global_cheese[i].x && y == global_cheese[i].y) || x == global_trap[i].x && y == global_trap[i].y)
        {
            return true;
        }
    }
    if(x == door_loc.x && y == door_loc.y)
    {
        return true;
    }
}
// setups tom with the initial x,y and an initial 'speed'
void tom_setup() {

    t_x = t_init_x;
    t_y = t_init_y;
    
    t_dx = 0.09;
    t_dy = 0.09;
}
// setups jerry with initial x,y
void jerry_setup() {
    if (j_init_x == 0)
    {
        j_init_x = 1;
        j_init_y = 5;
    }
    j_x = j_init_x;
    j_y = j_init_y;
    jxx = j_x;
    jyy = j_y;

}
// checks if a player can move a certain direction ie left right
bool player_collide_check_wall(int direction, int player_x, int player_y) {
    // 1:left, 2:right, 3:up, 4:down
    for (int i = 0; i < index_wall; i++)
    {
        int wall_x = global_walls[i].x1;
        int wall_y = global_walls[i].y1;
        double wall_angle = global_walls[i].angle;

        if(direction == 1 && (player_y == wall_y && player_x == wall_x+1))
        {
            return true;
        }
        if(direction == 2 && (player_y == wall_y && player_x == wall_x-1))
        {
            return true;
        }
        if(direction == 3 && (player_x == wall_x && player_y == wall_y+1))
        {
            return true;
        }
        if(direction == 4 && (player_x == wall_x && player_y == wall_y-1))
        {
            return true;

        }
    }
}
// if jerry collides with any entities
int jerry_collide_check_entities() {

    if((round(j_x) == round(t_x) && round(j_y) == round(t_y)))
    {
        clear_screen();
        jerry_setup();
        tom_setup();
        if(player_current == 'J')
        {
            jerry_lives--;
        }
        else if (player_current == 'T')
        {
            tom_score_current += 5;
            c_score_tom += 5;
            j_x, jxx = j_init_x;
        }
        
    }
    for (int i = 0; i < 5; i++)
    {
        int xc = global_cheese[i].x;
        int yc = global_cheese[i].y;

        int xt = global_trap[i].x;
        int yt = global_trap[i].y;

        if(round(j_x) == xc && round(j_y) == yc)
        {
            global_cheese[i].x = 0;
            global_cheese[i].y = 0;
            jerry_score_current++;
            cheese_consumed++;
            cheese_current--;
        }
        if(round(j_x) == xt && round(j_y) == yt)
        {
            global_trap[i].x = 0;
            global_trap[i].y = 0;
            jerry_lives--;
            trap_current--;
            c_score_tom++;
            tom_score_current++;
        }
    }
}

int go_for_cheese() {
    for (int i = 0; i <= 5; i++)
    {
        int index = rand() % 5+1;

        if(global_cheese[index].x != 0)
        {
            return index;
            i=6;
        }
    }
}

void jerry_auto_move(int key) {

    if (key < 0) 
    {
        for(int i = 0; i < index_wall; i++)
        {
            int wall_x = global_walls[i].x1;
            int wall_y = global_walls[i].y1;
            double wall_angle = global_walls[i].angle;
            int jx = round(jxx + j_dx);
            int jy = round(jyy + j_dy);

            if(jx == wall_x && jy == wall_y) 
            {
                if(wall_angle == 90.00) 
                {
                    j_dx = -j_dx;
                }
                else if(wall_angle == 00.00)
                {
                    j_dy = -j_dy;

                }
                
                else
                {
                    j_dx = (double)rand() / RAND_MAX * 0.4 - 0.3;
                    j_dy = (double)rand() / RAND_MAX * 0.4 - 0.3;
                }
            }     
        }
        jxx+=j_dx;
        jyy+=j_dy;
        j_x = jxx;
        j_y = jyy;
        jerry_collide_check_entities();
    }

}

void jerry_draw() {
    
    if(player_current == 'J')
    {
        draw_char(round(j_x), round(j_y), JERRY_IMG);
    }
    else if(player_current == 'T')
    {
        draw_char(jxx, jyy, JERRY_IMG);
    }
 
}

void tom_draw() {
    draw_char(round(t_x), round(t_y), TOM_IMG);
}
// checks and handles if tom collides with fireworks
void tom_rocket_collide_check() {
    for (int i = 0; i < 50; i++)
    {
        // get the fireworks x,y
        int x = round(global_firework[i].x);
        int y = round(global_firework[i].y);
        if (round(t_x) == x && round(t_y) == y)
        {
            if(player_current == 'J')
            {
                firework_current--;
                t_x = t_init_x;
                t_y = t_init_y;
                jerry_score_current ++;
                global_firework[i].x = 0;
            }
            if(player_current == 'T')
            {
                firework_current--;
                t_x = t_init_x;
                t_y = t_init_y;
                tom_lives--;
                global_firework[i].x = 0;

            }
        }
    }
}
// checks and handles if tom collides with walls
// random directions
bool tom_bounced = false;
void tom_collide_check() {
    srand(time(NULL));
    double random_value;

    for(int i = 0; i < index_wall; i++)
    {
        int wall_x = global_walls[i].x1;
        int wall_y = global_walls[i].y1;
        double wall_angle = global_walls[i].angle;
        int tx = round(t_x + t_dx);
        int ty = round(t_y + t_dy);

        if(tx == wall_x && ty == wall_y) 
        {
            if(wall_angle == 90.00) 
            {
                random_value = (double)rand() / RAND_MAX * 0.4 - 0.3;
                t_dy = random_value; 
                t_dx = -t_dx;
                tom_bounced = true;
            }
            else if(wall_angle == 00.00)
            {
                random_value = (double)rand() / RAND_MAX * 0.4 - 0.3;
                t_dx = random_value;
                t_dy = -t_dy;
                tom_bounced = true;
            }
            else
            {
                t_dx = (double)rand() / RAND_MAX * 0.4 - 0.3;
                t_dy = (double)rand() / RAND_MAX * 0.4 - 0.3;
                tom_bounced = true;
            } 
        }     
    }
}

double last_search = 0.0;
double t_speed = 0.06;
void tom_move() {
    tom_collide_check();

    // if tom hasnt bounced move towards jerry
    if(!tom_bounced)
    {
        if (round(t_x) > j_x)
        {
            t_x-=t_speed;
        }
        else if(round(t_x) < j_x)
        {
            t_x+=t_speed;
        }
        if(round(t_y) > j_y)
        {
            t_y-=t_speed;
        }
        else if(round(t_y) < j_y)
        {
            t_y+=t_speed;
        }
    }
    else
    {            
        t_x += t_dx;
        t_y += t_dy;
    }

    // change bounced to false every 5 seconds
    // tom seeks every 5 seconds basically
    int st = floor(game_time);
    if(fmod(st, 5) == 0 && st != last_search)
    {
        tom_bounced = false;
    }
    tom_collide_check();

}
// updates toms movement
void tom_update(int key) {
    if (key < 0) 
    {
        tom_move();
    }
}
// updates either tom or jerrys location with WASD
void update_player(int ch, char character) {
    int player_x;
    int player_y;

    if(character == 'J')
    {
        player_x = round(j_x);
        player_y = round(j_y);
    }
    else if(character == 'T')
    {
        player_x = round(t_x);
        player_y = round(t_y);
    }  
  
    if (ch == 'a' && player_x > 1 && !player_collide_check_wall(1, player_x, player_y)) 
    {
        if(character == 'J') {
            j_x--;
        }
        else if (character == 'T') {
            t_x--;
        }
        
    }
    else if (ch == 'd' && player_x < W - 2 && !player_collide_check_wall(2, player_x, player_y)) 
    {
         if(character == 'J') {
            j_x++;
        }
        else if (character == 'T') {
            t_x++;
            
        }
    }
    else if (ch == 'w' && player_y > 5 && !player_collide_check_wall(3, player_x, player_y)) 
    {
        if(character == 'J') {
            j_y--;
        }
        else if(character == 'T') {
            t_y--;
        }
    }
    else if (ch == 's' && player_y < H - 2 && !player_collide_check_wall(4, player_x, player_y)) 
    {
        if(character == 'J') {
            j_y++;
        }
        else if(character == 'T') {
            t_y++;
        }
    }
}
// draws all the cheese
void cheese_draw() {
    for (int i = 0; i < 5; i++)
    {
        int cx = global_cheese[i].x;
        int cy = global_cheese[i].y;
        // if that location is "NULL"
        if(cx != 0) 
        {
            draw_char(cx, cy, CHEESE_IMG);
        }
    } 
}
// generates a new cheese location
void cheese_generate() {
    if(cheese_current < 5)
    {
        srand(time(NULL));
        int x = rand() % (W-1)+1;
        int y = rand() % (H-1)+4;

        for (int i = 0; i <= 5; i++)
        {
            if(!wall_location(x, y))
            {
            // find next free spot in the array
                if(global_cheese[i].x == 0) 
                {
                    if(player_current == 'J')
                    {
                        global_cheese[i].x = x;
                        global_cheese[i].y = y;    
                    }
                    else if(player_current == 'T')
                    {
                        global_cheese[i].x = round(t_x);
                        global_cheese[i].y = round(t_y);  
                    }
                    cheese_current++;
                    break;
                }
            }
        }
    }
}

void trap_draw() {
    for (int i = 0; i < 5; i++)
    {
        int cx = global_trap[i].x;
        int cy = global_trap[i].y;
        if(cx != 0) 
        {
            draw_char(cx, cy, TRAP_IMG);
        }
    } 
}
// generates a trap location
void trap_generate() {

    if(trap_current != 6)
    {
        for (int i = 0; i <= 5; i++)
        {
            if(!entity_location(round(t_x), round(t_y)))
            {      
                // find next free spot in the array
                if(global_trap[i].x == 0) 
                {
            
                    global_trap[i].x = round(t_x);
                    global_trap[i].y = round(t_y);  
                    
                    trap_current++;
                    break;
                }  
            }
        }
    }
}

void door_draw() {
    if(door_loc.x != 0)
    {
        draw_char(door_loc.x, door_loc.y, DOOR_IMG);   
    }
}
// if there is a door or not
bool door_active = false;
void door_generate() {
    srand(time(NULL));
    // random location in the bounds of the terminal size
    int x = rand() % (W-1)+1;
    int y = rand() % (H-1)+4;
    // if the random x,y isnt a wall location or entity
    if(!wall_location(x, y) && !entity_location(x, y))
    {
        if(!door_active) 
        {
            door_loc.x = x;
            door_loc.y = y;
            door_active = true;
        }
    }
}

void firework_generate() {
    if(firework_current != 50) 
    {
        for (int i = 0; i <= 50; i++)
        {
            // checks if the array location is "empty"
            if(global_firework[i].x == 0)
            {                
                global_firework[i].x = round(j_x);
                global_firework[i].y = round(j_y);
                firework_current++;
                break;
            }
        }       
    }
}

double firework_speed = 0.05;
// draws the fireworks and moves them
void firework_draw() {
    for (int i = 0; i <= 50; i++)
    {
        int x = round(global_firework[i].x);
        int y = round(global_firework[i].y);
        // draw if it hasnt collided yet
        if(x != 0 && !wall_location(x, y))
        {
            draw_char(x, y, FIREWORK_IMAGE);
            // moves the angle based on toms location
            if(x > round(t_x))
            {
                global_firework[i].x -= firework_speed;
            }
            else if(x < round(t_x))
            {
                global_firework[i].x += firework_speed;
            }
            if(y > round(t_y))
            {
                global_firework[i].y -= firework_speed;
            }
            else if(y < round(t_y))
            {
                global_firework[i].y += firework_speed;
            }
        }

    }
    // check if tom has collided
    tom_rocket_collide_check();
}

void draw_all() {
    clear_screen();
    draw_map();

    cheese_draw();
    trap_draw();
    door_draw();
    firework_draw();

    jerry_draw();
    tom_draw();

    show_screen();
}

void setup_game(int argc, char *argv[]) {
    W = screen_width();
    H = screen_height();
    // loads the file based on the current level
    do_load_file(argc, argv, level_current);
    jerry_setup();
    tom_setup();

    add_border();

    initial_time = get_current_time();
}

void do_second_level(int argc, char *argv[]) {
    clear_screen();
    // resets everything
    for (int i = 0; i < index_wall; i++)
    {
        global_walls[i].x1 = 0;
        global_walls[i].y1 = 0;
    }
    for (int i = 0; i < 5; i++)
    {
        global_cheese[i].x = 0;
        global_cheese[i].y = 0;

        global_trap[i].x = 0;
        global_trap[i].y = 0;
    }
    for (int i = 0; i < 50; i++)
    {
        global_firework[i].x = 0;
        global_firework[i].y = 0;
    }
    door_loc.x = 0;
    door_loc.y = 0;
    cheese_consumed = 0;
    cheese_current = 0;
    c_score_tom = 0;
    trap_current = 0;
    firework_current = 0;
    door_active = false;
    
    // setup game with new level
    level_current++;
    setup_game(argc, argv);
}
// handles the input of the user
void do_input(int key, int argc, char *argv[]) {
    if (key == 'q') 
    {
        game_over = true;
        return;
    }
    if (key == 'p') 
    {
        if (game_paused) {
            game_paused = false;
        }
        else 
        {
            game_paused = true;
        }   
    }
    if(key == 'z')
    {
        if(player_current == 'J') 
        {
            player_current = 'T';
            jxx = j_x;
            jyy = j_y;
        }
        else if(player_current == 'T')
        {
            player_current = 'J';
            j_x = jxx;
            j_y = jyy;
        }
    }
    if(key == 'c' && player_current == 'T')
    {
        cheese_generate();
    }
    if(key == 'f' && player_current == 'J' && level_current >= 2)
    {
        firework_generate();
    }
    if(key == 'm' && player_current == 'T')
    {
        trap_generate();
    }
    if (key == 'l')
    {
        do_second_level(argc, argv);
    }
}
// the last time a cheese and trap was spawned
double last_cheese_event = 0.0;
double last_trap_event = 0.0;
// when the current player is Jerry do this loop
void jerry_loop(int key) {

    update_player(key, 'J');
    jerry_collide_check_entities();
    
    if(!game_paused) 
    {
        tom_update(key);
    } 

    double c_time = floor(game_time);
    // checks the if the current time is dividable by 2 and it hasnt already happended
    if(fmod(c_time, 2) == 0 && c_time != last_cheese_event)
    {
        cheese_generate();
        last_cheese_event = c_time;
    }

    double t_time = floor(game_time);
    if(fmod(t_time, 3) == 0 && t_time != last_trap_event)
    {
        trap_generate();
        last_trap_event = t_time;
    }
}

double last_firework_event = 0.0;
void tom_loop(int key) {
    update_player(key, 'T');
    tom_rocket_collide_check();
    
    double f_time = floor(game_time);
    if(fmod(f_time, 5) == 0 && f_time != last_firework_event && level_current >= 2)
    {
        firework_generate();
        last_firework_event = f_time;
    }
    if(!game_paused) 
    {
        jerry_collide_check_entities();
        jerry_auto_move(key);
    } 
}
// handles when a player collides with the door
void door_collision(int argc, char *argv[]) {
    if(door_active)
    {
        if((round(j_x) == door_loc.x && round(j_y) == door_loc.y) && player_current == 'J')
        {
            do_second_level(argc, argv);
        }
        if((round(t_x) == door_loc.x && round(t_y) == door_loc.y) && player_current == 'T')
        {
            do_second_level(argc, argv);
        }
    }
}
// main loop
void loop(int argc, char *argv[]) {
    int key = get_char();
    do_input(key, argc, argv);
    // change the loop based on the current player
    if(player_current == 'J')
    {      
        jerry_loop(key);
    } 
    else if(player_current == 'T')
    {    
        tom_loop(key);
    }
    // spawns door or ends the game based on conditions
    if(cheese_consumed == 5 || c_score_tom == 5)
    {
        door_generate();
    }
    if(jerry_lives == 0 || tom_lives == 0) 
    { 
        end_game(1);
    }
    // edit the time
    if(!game_paused)
    {
        game_time = get_current_time() - initial_time;
    }
    // check for door collisions
    door_collision(argc, argv);
}

int main(int argc, char *argv[]) {
    const int DELAY = 10;
    setup_screen();
    setup_game(argc, argv);

    while (!game_over) 
    {
        draw_all();       
        loop(argc, argv);
        timer_pause(DELAY);
    }

    return 0;
}
