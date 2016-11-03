#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include <SDL_ttf.h>

#define W 640
#define H 480
#define TILESW 15
#define TILESH 11
#define BS 40
#define BS2 (BS/2)
#define PLYR_SZ 40

enum gamestates {READY, ALIVE, GAMEOVER} gamestate = READY;

struct player {
        SDL_Rect pos;
        struct vel {
                int x;
                int y;
        } vel;
        int hearts;
        int bombs;
        int money;
        int flags;
} player[4];

struct tile {
        int solid;
        SDL_Color color;
} room[TILESH][TILESW];

int nr_players = 1;
int idle_time = 30;
int frame = 0;

SDL_Event event;
SDL_Renderer *renderer;
SDL_Surface *surf;
SDL_Texture *tex[20];
TTF_Font *font;

void setup();
void new_game();
void key_move(int down);
void update_stuff();
void draw_stuff();
void text(char *fstr, int value, int height);

//the entry point and main game loop
int main()
{
        setup();
        new_game();

        for(;;)
        {
                while(SDL_PollEvent(&event)) switch(event.type)
                {
                        case SDL_QUIT:    exit(0);
                        case SDL_KEYDOWN: key_move(1); break;
                        case SDL_KEYUP:   key_move(0); break;
                }

                update_stuff();
                draw_stuff();
                SDL_Delay(1000 / 60);
                frame++;
        }
}

//initial setup to get the window and rendering going
void setup()
{
        srand(time(NULL));

        SDL_Init(SDL_INIT_VIDEO);
        SDL_Window *win = SDL_CreateWindow("Zel",
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, W, H, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_PRESENTVSYNC);
        if(!renderer) exit(fprintf(stderr, "Could not create SDL renderer\n"));

        for(int i = 0; i < 0; i++)
        {
                char file[80];
                sprintf(file, "res/sheet-%d.bmp", i);
                surf = SDL_LoadBMP(file);
                SDL_SetColorKey(surf, 1, 0xffff00);
                tex[i] = SDL_CreateTextureFromSurface(renderer, surf);
        }

        TTF_Init();
        font = TTF_OpenFont("res/LiberationSans-Regular.ttf", 42);
}

void key_move(int down)
{
        if(event.key.repeat)
                return;

        int amt = down ? 4 : -4;

        switch(event.key.keysym.sym)
        {
                case SDLK_UP:    player[0].vel.y -= amt; break;
                case SDLK_DOWN:  player[0].vel.y += amt; break;
                case SDLK_LEFT:  player[0].vel.x -= amt; break;
                case SDLK_RIGHT: player[0].vel.x += amt; break;
        }
}

//start a new game
void new_game()
{
        gamestate = ALIVE;
        player[0].pos.x = (W - PLYR_SZ) / 2;
        player[0].pos.y = H - 100;
        player[0].pos.w = PLYR_SZ;
        player[0].pos.h = PLYR_SZ;

        for(int x = 0; x < TILESW; x++) for(int y = 0; y < TILESH; y++)
        {
                int edge_x = (x == 0 || x == TILESW-1);
                int door_x = (edge_x && y == TILESH/2);
                int edge_y = (y == 0 || y == TILESH-1);
                int door_y = (edge_y && x == TILESW/2);

                if((edge_x || edge_y) && !door_x && !door_y)
                {
                        room[y][x].solid = 1;
                        room[y][x].color = (SDL_Color){0, 30, 50, 255};
                }
        }
}

//when we hit something
void game_over()
{
        gamestate = GAMEOVER;
}

//update everything that needs to update on its own, without input
void update_stuff()
{
        int newx = player[0].pos.x;
        int newy = player[0].pos.y;

        if(player[0].vel.y)
                newx = (player[0].pos.x + BS2 - 1) / BS2 * BS2;

        if(player[0].vel.x)
                newy = (player[0].pos.y + BS2 - 1) / BS2 * BS2;

        newx += player[0].vel.x;
        newy += player[0].vel.y;

        int already_stuck = 0;
        int would_be_stuck = 0;

        int tx = player[0].pos.x/BS;
        int ty = player[0].pos.y/BS;

        if(tx >= 0 && tx < TILESW && ty >= 0 && ty < TILESH)
                already_stuck = room[ty][tx].solid;

        tx = newx/BS;
        tx = newy/BS;

        if(tx >= 0 && tx < TILESW && ty >= 0 && ty < TILESH)
                would_be_stuck = room[ty][tx].solid;

        printf("already %d wouldbe %d\n", already_stuck, would_be_stuck);

        if(!would_be_stuck || already_stuck)
        {
                player[0].pos.x = newx;
                player[0].pos.y = newy;
        }
}

//draw everything in the game on the screen
void draw_stuff()
{
        SDL_Rect dest = {0, 0, W, H};

        SDL_SetRenderDrawColor(renderer, 60, 90, 120, 255);
        SDL_RenderClear(renderer);
        SDL_RenderFillRect(renderer, &dest);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        for(int i = 0; i < nr_players; i++)
        {
                SDL_SetRenderDrawColor(renderer, 200, 100, 0, 255);
                SDL_RenderFillRect(renderer, &player[0].pos);
        }

        for(int x = 0; x < TILESW; x++) for(int y = 0; y < TILESH; y++)
        {
                SDL_SetRenderDrawColor(renderer,
                        room[y][x].color.r,
                        room[y][x].color.g,
                        room[y][x].color.b,
                        room[y][x].color.a);
                SDL_RenderFillRect(renderer, &(SDL_Rect){BS*x, BS*y, BS, BS});
        }

        text("Zel", 0, 10);

        SDL_RenderPresent(renderer);
}

void text(char *fstr, int value, int height)
{
        if(!font) return;
        int w, h;
        char msg[80];
        snprintf(msg, 80, fstr, value);
        TTF_SizeText(font, msg, &w, &h);
        SDL_Surface *msgsurf = TTF_RenderText_Blended(font, msg, (SDL_Color){255, 255, 255});
        SDL_Texture *msgtex = SDL_CreateTextureFromSurface(renderer, msgsurf);
        SDL_Rect fromrec = {0, 0, msgsurf->w, msgsurf->h};
        SDL_Rect torec = {(W - w)/2, height, msgsurf->w, msgsurf->h};
        SDL_RenderCopy(renderer, msgtex, &fromrec, &torec);
        SDL_DestroyTexture(msgtex);
        SDL_FreeSurface(msgsurf);
}
