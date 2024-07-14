#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <list>
#include <iostream>

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 600

struct Pipe
{
    int x;                             // x-coordinate of the pipe
    int gapY;                          // y-coordinate of the gap's center
    static const int GAP_HEIGHT = 250; // The height of the gap;
    static const int PIPE_WIDTH = 50;  // Width of the pipe;
    bool passed = false;
};

struct Heli
{
    int x, y;          // Bird's position
    int width, height; // Bird's dimensions
    float velocity;    // Bird's vertical velocity
    float gravity;     // Gravity pulling the bird downwards
    float lift;        // Upwards force when the bird flaps
    float terminal_velocity;
};

void resetGame(Heli &bird, std::list<Pipe> &pipes, int &score)
{
    bird.x = WINDOW_WIDTH / 8;
    bird.y = WINDOW_HEIGHT / 2;
    bird.velocity = 0;
    pipes.clear();
    score = 0;
}

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Helicopter Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window)
    {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        SDL_Log("Unable to create renderer: %s", SDL_GetError());
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        SDL_Log("Unable to initialize SDL_image: %s", IMG_GetError());
        return 1;
    }

    // load sprites
    SDL_Texture *birdTexture = IMG_LoadTexture(renderer, "sprites/heli.png");
    if (!birdTexture)
    {
        SDL_Log("Failed to load bird texture: %s", IMG_GetError());
        return 1;
    }

    SDL_Texture *upperPipeTexture = IMG_LoadTexture(renderer, "sprites/up_pipe.png");
    if (!upperPipeTexture)
    {
        SDL_Log("Failed to load upper_pipe texture: %s", IMG_GetError());
        return 1;
    }

    SDL_Texture *lowerPipeTexture = IMG_LoadTexture(renderer, "sprites/low_pipe.png");
    if (!lowerPipeTexture)
    {
        SDL_Log("Failed to load lower_pipe texture: %s", IMG_GetError());
        return 1;
    }

    SDL_Texture *bgTexture = IMG_LoadTexture(renderer, "sprites/background.png");
    if (!bgTexture)
    {
        SDL_Log("Failed to load background texture: %s", IMG_GetError());
        return 1;
    }

    if (TTF_Init() == -1)
    {
        SDL_Log("Unable to initialize SDL_ttf: %s", TTF_GetError());
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("font.ttf", 24); // Adjust the size (24 here) as needed
    if (!font)
    {
        SDL_Log("Unable to load font: %s", TTF_GetError());
        return 1;
    }

    bool isRunning = true;
    SDL_Event event;
    int score = 0;
    bool gameOver = false;

    // Helicopter instance (player)
    Heli helicopter;
    helicopter.x = WINDOW_WIDTH / 8; // starting position
    helicopter.y = WINDOW_HEIGHT / 2; // starting position
    helicopter.width = 75;
    helicopter.height = 75;
    helicopter.velocity = 0;
    helicopter.gravity = 0.3;
    helicopter.lift = -7;
    helicopter.terminal_velocity = 10;

    std::list<Pipe> pipes;
    Uint32 lastPipeSpawnTime = SDL_GetTicks();
    const Uint32 PIPE_SPAWN_INTERVAL = 2000;

    while (isRunning)
    {
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastPipeSpawnTime > PIPE_SPAWN_INTERVAL)
        {
            Pipe newPipe;
            newPipe.x = WINDOW_WIDTH; // Assuming WINDOW_WIDTH is the width of your game window
            newPipe.gapY = (rand() % (WINDOW_HEIGHT - Pipe::GAP_HEIGHT)) + Pipe::GAP_HEIGHT / 2;
            pipes.push_back(newPipe);
            lastPipeSpawnTime = currentTime;
        }
        // Move all pipes to the left
        for (Pipe &pipe : pipes)
        {
            pipe.x -= 5; // Adjust the speed as needed
        }

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                isRunning = false;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE))
            {
                helicopter.velocity = helicopter.lift;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
        SDL_RenderClear(renderer);

        SDL_Rect bgRect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderCopy(renderer, bgTexture, NULL, &bgRect);

        // Render score
        std::string scoreText = "Score: " + std::to_string(score);
        SDL_Color textColor = {255, 255, 255, 255}; // White text; adjust as needed

        SDL_Surface *scoreSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
        if (!scoreSurface)
        {
            SDL_Log("Unable to render text: %s", TTF_GetError());
            return 1;
        }

        SDL_Texture *scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
        if (!scoreTexture)
        {
            SDL_Log("Unable to create texture from rendered text: %s", SDL_GetError());
            return 1;
        }

        int scoreWidth = scoreSurface->w;
        int scoreHeight = scoreSurface->h;

        SDL_Rect scoreRect = {10, 10, scoreWidth, scoreHeight}; // Position the score at the top-left of the screen with a little offset
        SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);

        SDL_FreeSurface(scoreSurface);
        SDL_DestroyTexture(scoreTexture);

        // Draw helicopter
        SDL_Rect birdRect = {helicopter.x, helicopter.y, helicopter.width, helicopter.height};
        SDL_RenderCopy(renderer, birdTexture, NULL, &birdRect);

        // helicopter physics
        helicopter.velocity += helicopter.gravity;
        helicopter.y += helicopter.velocity;

        // Optional: Add a terminal velocity
        if (helicopter.velocity > helicopter.terminal_velocity)
        {
            helicopter.velocity = helicopter.terminal_velocity;
        }

        if (helicopter.y < 0)
        {
            helicopter.y = 0;
            helicopter.velocity = 0; // stop the helicopter from moving upwards
        }
        else if (helicopter.y + helicopter.height > WINDOW_HEIGHT)
        {
            helicopter.y = WINDOW_HEIGHT - helicopter.height;
            helicopter.velocity = 0; // stop the helicopter from falling out of the window
            gameOver = true;
        }

        // Drawing Pipes
        for (const Pipe &pipe : pipes)
        {
            // Upper pipe
            SDL_Rect upperDest = {pipe.x, 0, Pipe::PIPE_WIDTH, pipe.gapY - Pipe::GAP_HEIGHT / 2};
            SDL_RenderCopy(renderer, upperPipeTexture, NULL, &upperDest);

            // Lower pipe
            SDL_Rect lowerDest = {pipe.x, pipe.gapY + Pipe::GAP_HEIGHT / 2, Pipe::PIPE_WIDTH, WINDOW_HEIGHT - (pipe.gapY + Pipe::GAP_HEIGHT / 2)};
            SDL_RenderCopy(renderer, lowerPipeTexture, NULL, &lowerDest);
        }

        // Removing Off-screen Pipes
        while (!pipes.empty() && pipes.front().x + Pipe::PIPE_WIDTH < 0)
        {
            pipes.pop_front();
        }

        bool hasCollided = false;
        for (Pipe &pipe : pipes)
        {
            if (!pipe.passed && helicopter.x > pipe.x + Pipe::PIPE_WIDTH)
            {
                score++;
                pipe.passed = true;
                std::cout << "Score: " << score << std::endl;
            }

            if (helicopter.x + helicopter.width > pipe.x && helicopter.x < helicopter.x + Pipe::PIPE_WIDTH)
            {
                if (helicopter.y < pipe.gapY - Pipe::GAP_HEIGHT / 2 || helicopter.y + helicopter.height > pipe.gapY + Pipe::GAP_HEIGHT / 2)
                {
                    hasCollided = true;
                    break;
                }
            }
        }

        if (hasCollided)
        {
            std::cout << "Helicopter collided with pipe!" << std::endl;
            score = 0;
            gameOver = true;
        }

        if (gameOver)
        {
            std::cout << "Game Over! Final Score: " << score << std::endl;
            SDL_Delay(1000); // Wait for 1 second
            resetGame(helicopter, pipes, score);
            gameOver = false;
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(birdTexture);
    SDL_DestroyTexture(upperPipeTexture);
    SDL_DestroyTexture(lowerPipeTexture);
    SDL_DestroyTexture(bgTexture);
    TTF_CloseFont(font);
    TTF_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
