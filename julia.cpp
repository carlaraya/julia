#include <SDL2/SDL.h>
#include <stdio.h>

#include <math.h>
//MPFR C++, used for arbitrary floating-point precision
//#include <mpreal.h>

//Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 600;

//Constants for julia set generation
int pixelsPerUnit = 240;
const double bounds = 2.0;
const double bounds2 = bounds*bounds;
const int maxIter = 100;

double cRe = 0;
double cIm = 0;

int colors[maxIter+1];
const int bn = 6;
int between[][3] = { {150,0,0}, {255,255,0}, {0,255,0}, {0,255,255}, {0,127,255}, {0,0,255} };

//Starts up SDL and creates window
bool init();

//Shuts down SDL
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;
	
//The surface contained by the window
SDL_Surface* gScreenSurface = NULL;

//Convert pixel coordinates to complex number
void coord2z(int x, int y, double* a, double* b);

//Put pixel
void PutPixel32_nolock(SDL_Surface * surface, int x, int y, Uint32 color);

//Generate julia set
void generateJulia();

int main(int argc, char* args[])
{
    //Colors and stuff
    int ci = 0;
    int segment = maxIter / (bn - 1);
    for (int bi = 0; bi < (bn - 1); bi++)
    {
        for (int i = 0; i < segment; i++)
        {
            int curr[3];
            colors[ci] = 0;
            for (int chan = 0; chan < 3; chan++)
            {
                curr[chan] = (int)((((double)between[bi+1][chan])- between[bi][chan])/ segment * i)+ between[bi][chan];
                colors[ci] += curr[chan] << (8 * chan);
            }
            //printf("%d %d %d %d\n", curr[0], curr[1], curr[2], ci);
            ci++;
        }
    }
    colors[maxIter] = 0;

	//Start up SDL and create window
	if(!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
        //Main loop flag
        bool quit = false;

        //Event handler
        SDL_Event e;

        //Generate julia set
        generateJulia();

        //While application is running
        while(!quit)
        {
            //Handle events on queue
            while(SDL_PollEvent(&e)!= 0)
            {
                //User requests quit
                if(e.type == SDL_QUIT)
                {
                    quit = true;
                }
                //If mouse clicked, generate julia set
                else if (e.type == SDL_MOUSEBUTTONDOWN)
                {
                    if (e.button.button == SDL_BUTTON_LEFT)
                    {
                        coord2z(e.button.x, e.button.y, &cRe, &cIm);
                        generateJulia();
                        printf("%g %g\n", cRe, cIm);
                    }
                }
            }

            //Update the surface
            SDL_UpdateWindowSurface(gWindow);
        }
	}

	//Free resources and close SDL
	close();
	return 0;
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO)< 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Create window
		gWindow = SDL_CreateWindow("julia", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if(gWindow == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Get window surface
			gScreenSurface = SDL_GetWindowSurface(gWindow);
		}
	}

	return success;
}

void close()
{
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	SDL_Quit();
}

void PutPixel32_nolock(SDL_Surface * surface, int x, int y, Uint32 color)
{
    Uint8 * pixel = (Uint8*)surface->pixels;
    pixel += (y * surface->pitch)+ (x * sizeof(Uint32));
    *((Uint32*)pixel)= color;
}

void coord2z(int x, int y, double* a, double* b)
{
    *a = (double)(x - SCREEN_WIDTH / 2)/ pixelsPerUnit;
    *b = (double)(y - SCREEN_HEIGHT / 2)/ pixelsPerUnit;
}

void generateJulia()
{
    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            //Do this stuff for every pixel
            int escaped = 0;
            double a, b;
            double a2, b2;
            double a3, b3;
            coord2z(x, y, &a, &b);
            a2 = a;
            b2 = b;
            int i;
            for (i = 0; i < maxIter; i++)
            {
                //Math
                a3 = a2*a2 - b2*b2 + cRe;
                b3 = 2*a2*b2 + cIm;
                a2 = a3;
                b2 = b3;
                if (a2 * a2 + b2 * b2 > bounds2)
                {
                    break;
                }
            }
            
            PutPixel32_nolock(gScreenSurface, x, y, colors[i]);
        }
    }
}
