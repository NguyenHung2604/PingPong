#include <SDL2/SDL.h>
#include <iostream>
#include <SDL2/SDL_ttf.h>
#include <chrono>
#include <cstring>
#include <SDL2/SDL_mixer.h>
#include <time.h>
using namespace std;

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const int BALL_WIDTH = 15;
const int BALL_HEIGHT = 15;
const int PADDLE_HEIGHT = 100;
const int PADDLE_WIDTH = 10;
const float PADDLE_SPEED = 7.0f; 
const float BALL_SPEED = 8.0f;

enum Buttons
{
    Paddle1up = 0,
    Paddle1down,
    Paddle2up,
    Paddle2down,
};

enum class CollisionType
{
    None,
    Top,
    Mid,
    Bot,
    Left,
    Right
};

struct Contact
{
    CollisionType type;
    float penetration;   
};

class Vec2
{
public:
    Vec2()
        : x(0.0f), y(0.0f)
    {}

    Vec2(float x, float y)
        : x(x), y(y)
    {}

    Vec2 operator+(Vec2 const &rhs)
    {
        return Vec2(x + rhs.x, y + rhs.y); 
    }

    Vec2 &operator += (Vec2 const &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }    

    Vec2 operator *(float rhs)
    {
        return Vec2(x* rhs, y*rhs);
    }

    float x, y;
};


class Ball
{
public:
    Ball(Vec2 position, Vec2 velocity) 
        : position(position), velocity(velocity)
    {
        rect.x = static_cast <int> (position.x);
        rect.y = static_cast <int> (position.y);
        rect.w = BALL_WIDTH;
        rect.h = BALL_HEIGHT;
    }

    void Update(float dt)
    {
        position += velocity*dt;
    }

    // stactic_cast<int>: float -> int
    void Draw(SDL_Renderer *renderer)
    {
        rect.x = static_cast <int> (position.x);
        rect.y = static_cast <int> (position.y);
        SDL_RenderFillRect(renderer, &rect);
    }

    void CollidewithPaddle(Contact const & contact)
    {
        position.x += contact.penetration;
        velocity.x = -velocity.x;

        if(contact.type == CollisionType::Top)
        {
            velocity.y = -0.75f*BALL_SPEED;
        }
        else if(contact.type == CollisionType::Bot)
        {
            velocity.y = 0.75f*BALL_SPEED;
        }  
    }

    void CollidewithWall(Contact const& contact)
    {
        if((contact.type == CollisionType::Top) || (contact.type == CollisionType::Bot))
        {
            position.y +=  contact.penetration;
            velocity.y  = -velocity.y;
        }
        else if(contact.type == CollisionType::Left)
        {
            position.x = WINDOW_WIDTH / 2.0f;
            position.y = WINDOW_HEIGHT / 2.0f;
            velocity.x = -BALL_SPEED;
            velocity.y = -0.75*BALL_SPEED;
        }
        else if(contact.type == CollisionType::Right)
        {
            position.x = WINDOW_WIDTH / 2.0f;
            position.y = WINDOW_HEIGHT / 2.0f;
            velocity.x = BALL_SPEED;
            velocity.y = 0.75*BALL_SPEED;
        }
    }
    Vec2 position;
    Vec2 velocity;
    SDL_Rect rect{};    


};
class Paddle
{
public:
    Paddle(Vec2 position, Vec2 velocity)
        : position(position), velocity(velocity)
        {
            rect.x = static_cast <int>(position.x);
            rect.y = static_cast <int>(position.y);
            rect.w = PADDLE_WIDTH;
            rect.h = PADDLE_HEIGHT;
        }

        void update(float dt)
        {
            position += velocity*dt;
            if(position.y < 0)
            {   
                // topscreen
                position.y = 0;
            }
            else if(position.y > (WINDOW_HEIGHT - PADDLE_HEIGHT)){
                // downscreen
                position.y = WINDOW_HEIGHT - PADDLE_HEIGHT;
            }
        }

        void Draw(SDL_Renderer *renderer)
        {
            rect.y = static_cast <int> (position.y);
            SDL_RenderFillRect(renderer, &rect);
        }

    Vec2 position;
    Vec2 velocity;
    SDL_Rect rect{};

};

class PlayerScore
{
public:
    PlayerScore(Vec2 position, SDL_Renderer *renderer, TTF_Font *font)
        : renderer(renderer), font(font)
    {
        surface = TTF_RenderText_Solid(font, "0", {255, 255, 225, 255});
        texture = SDL_CreateTextureFromSurface(renderer, surface);

        int width, height;
        SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);
        rect.x = static_cast <int> (position.x);
        rect.y = static_cast <int> (position.y);
        rect.w = width;
        rect.h = height;
    }
    ~PlayerScore()
    {
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }

    void setScore(int score)
    {
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);

        surface = TTF_RenderText_Solid(font,to_string(score).c_str(), {255, 255, 255, 255});
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        
        int width, height;
        SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);
        rect.w = width;
        rect.h = height;
    }


    void Draw()
    {
        SDL_RenderCopy(renderer, texture, nullptr, &rect);
    }


    SDL_Renderer* renderer;
	TTF_Font* font;
	SDL_Surface* surface{};
	SDL_Texture* texture{};
	SDL_Rect rect{};
};

Contact CheckPaddleCollision(Ball const &Ball, Paddle const & paddle)
{
    float Ball_left = Ball.position.x;
    float Ball_Right = Ball.position.x + BALL_WIDTH;
    float Ball_Top = Ball.position.y;
    float Ball_Bot = Ball.position.y + BALL_HEIGHT;

    float paddleLeft = paddle.position.x;
    float paddleRight = paddle.position.x + PADDLE_WIDTH;
    float paddleTop = paddle.position.y;
    float paddleBot = paddle.position.y + PADDLE_HEIGHT;

    Contact contact{};

    if(Ball_left >= paddleRight)
    {
        return contact;
    }

    if(Ball_Right <= paddleLeft)
    {
        return contact;
    }

    if(Ball_Top >= paddleBot)
    {
        return contact;
    }

    if(Ball_Bot <= paddleTop)
    {
        return contact;
    }

    float paddleRangeUpper = paddleBot -(2.0f *PADDLE_HEIGHT / 3.0f);
    float paddleRangeMid =  paddleBot - (PADDLE_HEIGHT/3.0f);

    if(Ball.velocity.x < 0)
    {   
        // Left Paddle
        contact.penetration = paddleRight - Ball_left; 
    }
    else if(Ball.velocity.x > 0)
    {
        contact.penetration = paddleLeft - Ball_Right;
    }

    if((Ball_Bot > paddleTop) && (Ball_Bot < paddleRangeUpper))
    {
        contact.type = CollisionType::Top;
    }
    else if((Ball_Bot > paddleRangeUpper) && (Ball_Bot < paddleRangeMid))
    {
        contact.type = CollisionType::Mid;
    }
    else{
        contact.type = CollisionType::Bot;
    }

    return contact;
}

Contact CheckWallCollision(Ball const &Ball)
{
    float ballLeft = Ball.position.x;
	float ballRight = Ball.position.x + BALL_WIDTH;
	float ballTop = Ball.position.y;
	float ballBottom = Ball.position.y + BALL_HEIGHT;

    Contact contact {};

    if(ballLeft < 0.0f)
    {
        contact.type = CollisionType::Left;
    }
    else if(ballRight > WINDOW_WIDTH)
    {
        contact.type = CollisionType::Right;
    }
    else if(ballTop < 0.0f)
    {
        contact.type = CollisionType::Top;
        contact.penetration = -ballTop;
    }
    else if(ballBottom > WINDOW_HEIGHT)
    {
        contact.type = CollisionType::Bot;
        contact.penetration =  WINDOW_HEIGHT - ballBottom;
    }
    return contact;
}



int main(int agrc, char *agrv[])
{
	// Initialize SDL components 
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    SDL_Window *window = SDL_CreateWindow("PingPong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    // Initialize the font
    TTF_Font *scoreFont = TTF_OpenFont("DejaVuSansMono.ttf", 40);

    // Create the player score text fields
    PlayerScore playerOneScoreText(Vec2(WINDOW_WIDTH / 4, 20), renderer, scoreFont);
    PlayerScore playerTwoScoreText(Vec2(WINDOW_WIDTH*3/4, 20), renderer, scoreFont);

    // Initialize the sound effect
    Mix_Chunk* wallHitsound = Mix_LoadWAV("Wall.wav");
    Mix_Chunk* paddleHitsound = Mix_LoadWAV("Paddle.wav");    


    // Game logic
    {   
        // Create a ball
        Ball Ball(Vec2((WINDOW_WIDTH / 2.0f) - (BALL_WIDTH / 2.0f), (WINDOW_HEIGHT / 2.0f) - (BALL_HEIGHT/2.0f))
                                ,Vec2(BALL_SPEED, 0.0f));


        // Create the paddles
        Paddle paddle1(
            Vec2(50.0f, WINDOW_HEIGHT / 2.0f),
            Vec2(0.0f, 0.0f));
        
        Paddle paddle2(
            Vec2(WINDOW_WIDTH - 50.0f, WINDOW_HEIGHT/ 2.0), Vec2(0.0f, 0.0f));
        // Initialize the score
        int point1 = 0;
        int point2 = 0;
        // Initialize the time after Get Score
        Uint32 resetTime = 0;
        bool resetting = false;

        bool running = true;
        bool buttons[4] = {};
        // Continue looping and processing until user exits
        float dt = 0.0f;
        while(running)
        {   
            auto startTime = chrono::high_resolution_clock::now();
            SDL_Event event;
            while(SDL_PollEvent(&event))
            {
                if(event.type == SDL_QUIT)
                {
                    running = false;
                }
                else if(event.type == SDL_KEYDOWN)
                {
                    if(event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        running = false;
                    }
                    else if(event.key.keysym.sym == SDLK_w)
                    {
                        buttons[Buttons::Paddle1up] = true;
                    }
                    else if(event.key.keysym.sym == SDLK_s)
                    {
                        buttons[Buttons::Paddle1down] = true;
                    }
                    else if(event.key.keysym.sym == SDLK_UP)
                    {
                        buttons[Buttons::Paddle2up] = true;
                    }
                    else if(event.key.keysym.sym == SDLK_DOWN)
                    {
                        buttons[Buttons::Paddle2down] = true;
                    }
                }
                else if(event.type == SDL_KEYUP)
                {
                    if(event.key.keysym.sym == SDLK_w)
                    {
                        buttons[Buttons::Paddle1up] = false;
                    }
                    else if(event.key.keysym.sym == SDLK_s)
                    {
                        buttons[Buttons::Paddle1down] = false;
                    }
                    else if(event.key.keysym.sym == SDLK_UP)
                    {
                        buttons[Buttons::Paddle2up] = false;
                    }
                    else if(event.key.keysym.sym == SDLK_DOWN)
                    {
                        buttons[Buttons::Paddle2down] = false;
                    }
                }
            }
            if(buttons[Buttons::Paddle1up])
            {
                paddle1.velocity.y = -PADDLE_SPEED;
            }
            else if(buttons[Buttons::Paddle1down])
            {
                paddle1.velocity.y = PADDLE_SPEED;
            }
            else{
                paddle1.velocity.y = 0.0f;
            }

            if(buttons[Buttons::Paddle2up])
            {
                paddle2.velocity.y = -PADDLE_SPEED;
            }
            else if(buttons[Buttons::Paddle2down])
            {
                paddle2.velocity.y = PADDLE_SPEED;
            }
            else{
                paddle2.velocity.y = 0.0f;
            }
        // update paddle position
            paddle1.update(dt);
            paddle2.update(dt);
        
        // update ball position
        srand(time(0));
        if(resetting)
        {
            if(SDL_GetTicks() >= resetTime)
            {
                resetting = false;
                Ball.position = Vec2(WINDOW_WIDTH/2.0f, WINDOW_HEIGHT/2.0f);
                Ball.velocity = Vec2(rand() % 2 == 0 ? -BALL_SPEED : BALL_SPEED, 0.75f*BALL_SPEED);
            }
        }
        else{
            Ball.Update(dt);
        }
        // Check the collisions
        

        if(Contact contact = CheckPaddleCollision(Ball, paddle1); contact.type != CollisionType::None)
        {
            Ball.CollidewithPaddle(contact);
            Mix_PlayChannel(-1, paddleHitsound, 0);
        }
        else if(contact = CheckPaddleCollision(Ball, paddle2); contact.type != CollisionType::None)
        {
            Ball.CollidewithPaddle(contact);
            Mix_PlayChannel(-1, paddleHitsound, 0);
        }
        else if(contact = CheckWallCollision(Ball); contact.type != CollisionType::None)
        {
            Ball.CollidewithWall(contact);
            if(contact.type == CollisionType::Left)
            {
                ++point2;
                playerTwoScoreText.setScore(point2);
                resetting = true;
                resetTime = SDL_GetTicks() + 1000;
                
            }
            else if(contact.type == CollisionType::Right)
            {
                ++point1;
                playerOneScoreText.setScore(point1);
                resetting = true;
                resetTime = SDL_GetTicks() + 1000;
                
            }   
            else{
                Mix_PlayChannel(-1, wallHitsound, 0);
            }
            
        }

        

    
        // Clear the window to black Purple
        SDL_SetRenderDrawColor(renderer,10, 0, 30, 255);
        SDL_RenderClear(renderer);
        

        // Drawing the net
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for(int y = 0; y < WINDOW_HEIGHT; ++y)
        {
            if(y % 6)
            {
                SDL_RenderDrawPoint(renderer, WINDOW_WIDTH/2, y);
            }
        }
        
        //Draw the ball
        Ball.Draw(renderer);

        //Draw the paddles
        paddle1.Draw(renderer);
        paddle2.Draw(renderer);

        //Display the score
        playerOneScoreText.Draw();
        playerTwoScoreText.Draw();

        // Calculate frame time
        auto stopTime = chrono::high_resolution_clock::now();
        dt = chrono::duration <float, chrono::milliseconds::period>(stopTime - startTime).count();
        
        // Present a backbuffer
        SDL_RenderPresent(renderer);
        }
    }

    //Clean up
    Mix_FreeChunk(wallHitsound);
    Mix_FreeChunk(paddleHitsound);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(scoreFont);
    Mix_Quit();
    TTF_Quit();
    SDL_Quit();
    return 0;
}