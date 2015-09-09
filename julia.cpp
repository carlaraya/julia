/**
 * TODO:
 * -Print values of every single variable before/after generating fractal
 * -Fix bugs that show when zooming in and in and in
 * -Add diversity to colors without reducing maxIter
 */

#include <SDL2/SDL.h>
#include <stdio.h>

#include <math.h>
//MPFR C++, used for arbitrary floating-point precision
//#include <mpreal.h>

//Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 600;

//Constants for julia set generation
long long pixelsPerUnit = 240;
const double bounds = 2.0;
const double bounds2 = bounds*bounds;
const int maxIter = 1000;

double cRe = 0;
double cIm = 0;

int colors[maxIter+1];
const int bn = 6;
//ROYGBI
int between[][3] = { {150,0,0}, {255,255,0}, {0,255,0}, {0,255,255}, {0,127,255}, {0,0,255} };

int centerX = SCREEN_WIDTH / 2;
int centerY = SCREEN_HEIGHT / 2;

int pointX = centerX;
int pointY = centerY;

double centerRe = 0;
double centerIm = 0;

/*
//Constants for minimand generation
int mstartX = SCREEN_WIDTH - 200;
int mstartY = 10;
int mcenterX = SCREEN_WIDTH - 100;
int mcenterY = 110;
int mendX = 2 * mcenterX - mstartX;
int mendY = 2 * mcenterY - mstartY;
int mbarThickness = 10;
*/

//Starts up SDL and creates window
bool init();

//Shuts down SDL
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;
	
//The surface contained by the window
SDL_Surface* gScreenSurface = NULL;

//Convert pixel coordinates to complex number
void coord2z(int x, int y, int midx, int midy, int pixelsPerUnit, double centerRe, double centerIm, double* a, double* b);

//Put pixel
void PutPixel32_nolock(SDL_Surface * surface, int x, int y, Uint32 color);

//Make palette from ROYGBI
void generatePalette();

//Generate julia set
void generateJulia();
void generateMandelbrot();
//void generateMinimand();

int main(int argc, char* args[])
{
    generatePalette();

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
        //generateMinimand();

        SDL_UpdateWindowSurface(gWindow);

        bool mandorjulia = false;

        //While application is running
        while(!quit)
        {
            //Handle events on queue
            if (SDL_PollEvent(&e))
            {
                //User requests quit
                if(e.type == SDL_QUIT)
                {
                    quit = true;
                }
                //If mouse clicked, generate julia set
                else if (e.type == SDL_MOUSEBUTTONDOWN)
                {
                    pointX = e.button.x;
                    pointY = e.button.y;
                    if (!mandorjulia && e.button.button == SDL_BUTTON_RIGHT)
                    {
                        coord2z(pointX, pointY, centerX, centerY, pixelsPerUnit, centerRe, centerIm, &cRe, &cIm);
                        generateJulia();
                        printf("%g %g\n", cRe, cIm);
                    }
                    else if (e.button.button == SDL_BUTTON_LEFT)
                    {
                        pixelsPerUnit *= 2;
                        coord2z(pointX, pointY, centerX, centerY, pixelsPerUnit, centerRe, centerIm, &centerRe, &centerIm);
                        if (mandorjulia) generateMandelbrot();
                        else generateJulia();
                    }
                }

                else if (e.type == SDL_KEYDOWN)
                {
                    switch (e.key.keysym.sym)
                    {
                        case SDLK_m:
                            mandorjulia = true;
                            generateMandelbrot();
                            break;
                        case SDLK_j:
                            mandorjulia = false;
                            generateJulia();
                            break;
                        case SDLK_a:
                            pixelsPerUnit *= 2;
                            if (mandorjulia) generateMandelbrot();
                            else generateJulia();
                            break;
                        case SDLK_s:
                            pixelsPerUnit /= 2;
                            if (mandorjulia) generateMandelbrot();
                            else generateJulia();
                            break;
                    }
                }
            }
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

void coord2z(int x, int y, int midx, int midy, int pixelsPerUnit, double centerRe, double centerIm, double* a, double* b)
{
    *a = (double)(x - midx)/ pixelsPerUnit + centerRe;
    *b = (double)(y - midy)/ pixelsPerUnit + centerIm;
}

void generatePalette()
{
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
}

void generateJulia()
{
    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            //Do this stuff for every pixel
            double a, b;
            double a2, b2;
            double a3, b3;
            coord2z(x, y, centerX, centerY, pixelsPerUnit, centerRe, centerIm, &a, &b);
            a2 = a; //a2 = 0; change this line (and the others) to summon mandy
            b2 = b; //b2 = 0; change this line (and the others) to summon mandy
            int i;
            for (i = 0; i < maxIter; i++)
            {
                //Math
                a3 = a2*a2 - b2*b2 + cRe;//a3 = a2*a2 - b2*b2 + a; change this line (and the others) to summon mandy
                b3 = 2*a2*b2 + cIm; //b3 = 2*a2*b2 + b; change this line (and the others) to summon mandy
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

void generateMandelbrot()
{
    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            //Do this stuff for every pixel
            double a, b;
            double a2, b2;
            double a3, b3;
            coord2z(x, y, centerX, centerY, pixelsPerUnit, centerRe, centerIm, &a, &b);
            a2 = 0;
            b2 = 0;
            int i;
            for (i = 0; i < maxIter; i++)
            {
                //Math
                a3 = a2*a2 - b2*b2 + a;
                b3 = 2*a2*b2 + b;
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
/*
void generateMinimand()
{
    for (int y = mstartY; y < mendY; y++)
    {
        for (int x = mstartX; x < mendX; x++)
        {
            //Do this stuff for every pixel
            double a, b;
            double a2, b2;
            double a3, b3;
            coord2z(x, y, mcenterX, mcenterY, 50, &a, &b);
            a2 = 0;
            b2 = 0;
            int i;
            for (i = 0; i < maxIter; i++)
            {
                //Math
                a3 = a2*a2 - b2*b2 + a;
                b3 = 2*a2*b2 + b;
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
    //Draw border
    for (int y = 0; y < mendY + mbarThickness; y++)
    {
        for (int x = mstartX - mbarThickness; x < mstartX; x++)
        {
            PutPixel32_nolock(gScreenSurface, x, y, 0);
            PutPixel32_nolock(gScreenSurface, x + mendX - mstartX, y, 0);
        }
    }
    for (int y = mendY; y < mendY + mbarThickness; y++)
    {
        for (int x = mstartX; x < mendX; x++)
        {
            PutPixel32_nolock(gScreenSurface, x, y, 0);
            PutPixel32_nolock(gScreenSurface, x, y - mendY, 0);
        }
    }
}
*/
