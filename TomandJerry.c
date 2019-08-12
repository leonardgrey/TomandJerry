#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <cab202_graphics.h>
#include <cab202_timers.h>

/* 
gcc TomandJerry.c -o TomandJerry -I../ZDK -L../ZDK -lzdk -lncurses
*/

/////// QUESTION ///////
// It says seeking Tom but then bouncing of wall tom aswell

double sx, sy;
double j_x, j_y, t_x, t_y;

char t_symbol, j_symbol, cheese, trap, door;

int t_speed;
int t_traps;

int cheese_count;

int game_score;

// DEBUG
void debug(char ch) {
    draw_char(2, 2, ch);
}

void show_coor() {
    // TOM coor
    draw_char((sx-20), 2, t_symbol);
    draw_int((sx-15), (2), t_x);
    draw_int((sx-10), (2), t_y);

    // JERRY coor
    draw_char((sx-20), 3, j_symbol);
    draw_int((sx-15), (3), j_x);
    draw_int((sx-10), (3), j_y);
}

// COLLISION MECHANICS
bool Collided(double x1, double y1, double x2, double y2) {
    return round(x1) == round(x2) && round(y1) == round(y2);
}

void Collide_check() {
    Collide_TJ();
}

void Collide_TJ() {
    // jerry with tom
    if(j_x == t_x && j_y == t_y) {
        debug(*"#");
    }
}

// TOM MECHANICS
void Tom_move() {
    draw_char(t_x, t_y, t_symbol);
}

void Tom_update() {

    //   left        right           top          bot       
    if( (t_x > 2) && (t_x < (sx-2)) && (t_y > 5) && (t_y < (sy-2)) ) {
        
    //CORRECTLY MOVES TOWARDS THE MOUSE
        // change x 
        if(t_x < j_x) {
            t_x ++;
        }
        else if(t_x > j_x) {
            t_x --;
        } 

        //check y
        if(t_y < j_y) {
            t_y ++;
        }
        else if(t_y > j_y) {
            t_y --;
        }
    }
    Tom_move();
}

void Tom_Trap() {
    draw_char(round(t_x), round(t_y), trap);
}

// JERRY MECHANICS
void Jerry_startup() {
    j_x = round((sx-1) / 2);
    j_y = round((sy -1) / 2);
}

void Jerry_move() {
    draw_char(round(j_x), round(j_y), j_symbol);
    
    if(Collided(j_x, j_y, t_x, t_y)) {
        debug(*"&");
    }
}

void Jerry_update(int key_pressed) { 

    if (key_pressed == *"a" && j_x > 2) {
        j_x --;
    }
    else if(key_pressed == *"d" && j_x < (sx-2) ) {
        j_x ++;
    }
    else if(key_pressed == *"w" && j_y > 5) {
        j_y --;
    }
    else if(key_pressed == *"s" && j_y < (sy-2)) {
        j_y ++;
    } 
    Jerry_move();
}

// SPECIAL MECHANICS
void spawn_cheese() {
    if(cheese_count < 5) {
        draw_char(10, 10, cheese);
        cheese_count++;
    }
}

// MAP MECHANICS
void draw_status() {
    
}

void mapSetup () {
    char outline = *"*";

    // left line
    draw_line(1, 4, 1, sy-1, outline);
    // top line
    draw_line(1, 4, sx-1, 4, outline);
    // right line
    draw_line(sx-1, 4, sx-1, sy-1, outline);
    // bot line
    draw_line(1, sy-1, sx-1, sy-1, outline);
    
}

int test; // will be replaced with a timer later on
void setup () {
    test = 0;
    // Insert setup logic here 
    //sx = screen_width();
    //sy = screen_height();
    sx = 60;
    sy = 20;

    j_x = 30;
    j_y = 30;

    t_x = 10;
    t_y = 10;
    t_speed = 2;
    t_traps = 0;
    
    game_score = 0;
    cheese_count = 0;

// yeah alright

    t_symbol = *"T";
    j_symbol = *"J";
    cheese = *"C";
    trap = *"H";
    door = *"D";
       
    Jerry_startup();
    Jerry_move();
    Tom_move();
    mapSetup();
}

// MAIN
void loop() {
    // Insert loop logic here.
    clear_screen();
    mapSetup();

    int input = getchar();

    // update and move Jerry
    Jerry_update(input);

    // update and move Tom
    if(test % t_speed == 0) {
        Tom_update();
        Tom_Trap(); 
        spawn_cheese();
    } 
    // Check for collisions
    //Collide_check();

    // DEbug
    show_coor();

    // 
    test++;

    // update the screen
    show_screen();
}

int main() {

    setup_screen();
    setup();
	show_screen();
    
    while(true){
        loop();
    }

    getchar();
    return 0;
}