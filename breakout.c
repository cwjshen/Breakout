//
// breakout.c
//
// Computer Science 50
// Problem Set 4
//

// standard libraries
#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Stanford Portable Library
#include "gevents.h"
#include "gobjects.h"
#include "gwindow.h"

// height and width of game's window in pixels
#define HEIGHT 600
#define WIDTH 400

// number of rows of bricks
#define ROWS 5

// number of columns of bricks
#define COLS 10

// radius of ball in pixels
#define RADIUS 10

// lives
#define LIVES 3

// Dimensions of paddle
#define PADWIDTH 50
#define PADHEIGHT 5

// Dimensions of bricks
#define BRICKWIDTH 34
#define BRICKHEIGHT 10

// Dimensions of ball
#define BALLWIDTH 10
#define BALLHEIGHT 10

// prototypes
void initBricks(GWindow window);
GOval initBall(GWindow window);
GRect initPaddle(GWindow window);
GLabel initScoreboard(GWindow window);
void updateScoreboard(GWindow window, GLabel label, int points);
GObject detectCollision(GWindow window, GOval ball);

int main(void)
{
    // seed pseudorandom number generator
    srand48(time(NULL));

    // instantiate window
    GWindow window = newGWindow(WIDTH, HEIGHT);

    // instantiate bricks
    initBricks(window);

    // instantiate ball, centered in middle of window
    GOval ball = initBall(window);

    // instantiate paddle, centered at bottom of window
    GRect paddle = initPaddle(window);

    // instantiate scoreboard, centered in middle of window, just above ball
    GLabel label = initScoreboard(window);

    // number of bricks initially
    int bricks = COLS * ROWS;

    // number of lives initially
    int lives = LIVES;

    // number of points initially
    int points = 0;

    /** Velocity components at which the ball moves
     *  The y velocity is a constant to avoid the annoying case
     *  where the x velocity is a lot larger than the y
     *  and it zig zags for a long time before reaching bottom.
     */
    double velocityX = 3 * drand48();
    double velocityY = 3;
    
    // Waits for the user to click to begin the game.
    waitForClick();
    
    // keep playing until game over
    while (lives > 0 && bricks > 0)
    {
        // TODO
        /** Creates an event to which the program will react to
         *  In this case, it will respond to the next mouse event.
         */
        GEvent event = getNextEvent(MOUSE_EVENT);
        
        /** When the mouse moves, it will change the starting x
         *  coordinate of the paddle to the current x coordinate
         *  of the mouse subtracted by half the width of the paddle
         *  to keep the paddle centered with the mouse
         *  The y coordinate will not change.
         */
        if (event != NULL)
        {
            if (getEventType(event) == MOUSE_MOVED)
            {
                double padx = getX(event) - getWidth(paddle) / 2;
                double pady = HEIGHT - 100;
                setLocation(paddle, padx, pady);
            }
        }
        
        // Moves the ball with the given velocity in the x and y components
        move(ball, velocityX, velocityY);
        // Adds a little delay before moving again
        pause(10);
        
        // If the ball touches the right wall
        if (getX(ball) + getWidth(ball) >= WIDTH)
        {
            // Reverses the direction of velocity
            velocityX *= -1;
        }
        // If the ball touches the left wall
        else if (getX(ball) <= 0)
        {
            velocityX *= -1;
        }
        
        // If the ball touches the top wall 
        if (getY(ball) <= 0)
        {
            velocityY *= -1;
        }
        // If the ball touches the bottom wall
        if (getY(ball) + getWidth(ball) >= HEIGHT)
        { 
            /** This is to prevent the ball from taking the same path
             *  after losing a life.
             */
            velocityX = 3 * drand48();
            velocityY = 3;
            // You lost a life!!
            lives -= 1;  
            // Removes the ball from the GUI to prep for the next round
            removeGWindow(window, ball);  
            // Puts the ball back at the center
            addAt(window, ball, (WIDTH / 2) - (BALLWIDTH / 2), (HEIGHT / 2) - (BALLHEIGHT / 2));
            // Waits for the player to click before dropping the ball again.
            waitForClick();
        }
        
        GObject object = detectCollision(window, ball);
        // If there is in fact a collision
        if (object != NULL)
        {
            // If the collision object is the paddle
            /** The "&& velocityY" part prevents the bug where
             *  the ball hits the side of the paddle and repeatedly
             *  bounces up and down within the paddle.
             */            
            if (object == paddle && velocityY > 1)
            {
                velocityY *= -1;
            }
            // If the collision object is a brick
            if (strcmp(getType(object), "GRect") == 0 && object != paddle)
            {
                velocityY *= -1;
                // Remove the brick
                removeGWindow(window, object);
                // Keeps track of amount of bricks remaining.
                bricks -= 1;
                points += 1;
                updateScoreboard(window, label, points);
            }
            // If the collision object is a label, don't bounce off it.
            // I don't think this statement is actually needed, but it was provided
            // on the PSet instructions??
            if (strcmp(getType(object), "GLabel") == 0)
            {
                continue;
            }            
        }        
    }
    
    // wait for click before exiting
    waitForClick();

    // game over
    closeGWindow(window);
    return 0;
}

/**
 * Initializes window with a grid of bricks.
 */
void initBricks(GWindow window)
{
    // TODO
    // Starting coordinate of bricks
    int brickX = 5;
    int brickY = 5;
    // String of colors to be used for each row
    string colors[ROWS] = {"BLUE", "RED", "ORANGE", "GREEN", "YELLOW"};
    
    // Iterates downwards to the subsequent row
    for (int i = 0; i < ROWS; i++)
    {
        // Iterates across to the subsequent column.
        for (int j = 0; j < COLS; j++)
        {
            // Creates the brick with given dimensions and coordinates
            GRect brick = newGRect(brickX, brickY, BRICKWIDTH, BRICKHEIGHT);
            // Adds brick to the form
            add(window, brick);
            // Sets the color of the brick
            setColor(brick, colors[i]);
            // Fills the brick solid with that color
            setFilled(brick, true);
            // Determines x coordinate of next brick
            brickX += BRICKWIDTH + 5;
        }
        // Resets x coordinate upon starting new row
        brickX = 5;
        // New y coordinate for next row of bricks
        brickY += BRICKHEIGHT + 5;
    }
}

/**
 * Instantiates ball in center of window.  Returns ball.
 */
GOval initBall(GWindow window)
{
    // TODO
    /** The x coordinate is the top left corner of the ball.
     *  To center the ball, the coordinates need to be translated
     *  towards the topleft by magnitude of half the width and half
     *  the height. 
     */    
    GOval ball = newGOval((WIDTH / 2) - (BALLWIDTH / 2), (HEIGHT / 2) - (BALLHEIGHT / 2), BALLWIDTH, BALLHEIGHT);
    add(window, ball);
    setColor(ball, "BLACK");
    setFilled(ball, true);
    
    return ball;
}

/**
 * Instantiates paddle in bottom-middle of window.
 */
GRect initPaddle(GWindow window)
{
    // TODO
    /** Since the window width is 400, and we want the paddle
     *  to be centered, the starting x coordinate for the paddle
     *  would be half the width subtracted by half the paddle width.
     *  (400 / 2) - (PADWIDTH / 2)
     */
    GRect paddle = newGRect(175, HEIGHT - 100, PADWIDTH, PADHEIGHT);
    add(window, paddle);
    setColor(paddle, "BLACK");
    setFilled(paddle, true);
    
    return paddle;
}

/**
 * Instantiates, configures, and returns label for scoreboard.
 */
GLabel initScoreboard(GWindow window)
{
    // TODO
    // Creates a label for the score
    GLabel score = newGLabel("Score: ");
    double x = (getWidth(window) - getWidth(score)) / 2;
    double y = (getHeight(window) - getHeight(score)) / 2;
    setLocation(score, x, y);
    add(window, score);
    return score;

}

/**
 * Updates scoreboard's label, keeping it centered in window.
 */
void updateScoreboard(GWindow window, GLabel label, int points)
{
    // update label
    char s[12];
    sprintf(s, "Score: %i", points);
    setLabel(label, s);

    // center label in window
    double x = (getWidth(window) - getWidth(label)) / 2;
    double y = (getHeight(window) - getHeight(label)) / 2;
    setLocation(label, x, y);
    
}


/**
 * Detects whether ball has collided with some object in window
 * by checking the four corners of its bounding box (which are
 * outside the ball's GOval, and so the ball can't collide with
 * itself).  Returns object if so, else NULL.
 */
GObject detectCollision(GWindow window, GOval ball)
{
    // ball's location
    double x = getX(ball);
    double y = getY(ball);

    // for checking for collisions
    GObject object;

    // check for collision at ball's top-left corner
    object = getGObjectAt(window, x, y);
    if (object != NULL)
    {
        return object;
    }

    // check for collision at ball's top-right corner
    object = getGObjectAt(window, x + 2 * RADIUS, y);
    if (object != NULL)
    {
        return object;
    }

    // check for collision at ball's bottom-left corner
    object = getGObjectAt(window, x, y + 2 * RADIUS);
    if (object != NULL)
    {
        return object;
    }

    // check for collision at ball's bottom-right corner
    object = getGObjectAt(window, x + 2 * RADIUS, y + 2 * RADIUS);
    if (object != NULL)
    {
        return object;
    }

    // no collision
    return NULL;
}
