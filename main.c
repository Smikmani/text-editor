#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define CHAR_WIDTH 20
#define CHAR_HEIGHT 30



typedef struct Cursor
{
	int x;
	int y;
	int currentLine;
	int positionLine;
	int position;
} Cursor;

typedef struct Position{
	int x ;
	int y;
} Position;

typedef struct Line {
	int start;
	int end;
} Line;

typedef struct Data{
	char *data;
	Line *lines;
	size_t lineCount;
	size_t count;
} Data;

static Data data = {0} ;

static Cursor cur = {0};


static int windowWidth = 800;

static int windowHeight = 600;



void addChar(char *c, Data *data)
{
	for(int i = 0; i<data->lineCount; ++i)
	{
		printf("{%d,%d}",data->lines[i].start,data->lines[i].end);
	}
	printf("\n");
	if(data->data==NULL)
	{
		

		data->count += 1;

		data->data = (char *) malloc(sizeof(char) * (2)); 
	
		data->data[1] = '\0';
		data->data = strcpy(data->data, c);
		
		cur.positionLine += 1;

		cur.x += CHAR_WIDTH;

		if(c=="\n")
		{
			cur.x = 0;
			cur.y += CHAR_HEIGHT;
			cur.positionLine = 0;
			cur.currentLine += 1;	
		}

		return;
	}

	int positionToAdd = data->lines[cur.currentLine].start + cur.positionLine;
	
		
	char *placeholder = (char *) malloc(sizeof(char) * (data->count+1));

	placeholder[data->count] = '\0';

	if(placeholder == NULL)
	{
		exit(1);
	}
	
	placeholder = strcpy(placeholder, data->data);
	
	data->data = (char *) malloc(sizeof(char) * (data->count+2)); 
	
	data->data[data->count+1] = '\0';
	
	for(int i =0; i < positionToAdd ; ++i)
	{
		data->data[i] = placeholder[i];

	}

	data->data[positionToAdd] = *c;
	
	for(int i =positionToAdd ; i < data->count ; ++i )
	{
		data->data[i+1] = placeholder[i];

	}

	cur.positionLine += 1;
	data->count += 1;


	
	cur.x += CHAR_WIDTH;
	if(c=="\n")
	{
		cur.x = 0;
		cur.y += CHAR_HEIGHT;
		cur.currentLine += 1;
		cur.positionLine = 0;
	}	

	if(cur.x == (windowWidth - (windowWidth % CHAR_WIDTH) ))
	{
		cur.x = 0;
		cur.y += CHAR_HEIGHT;
		cur.currentLine += 1;
		cur.positionLine = 0;
	}

	free(placeholder);
}

void moveCursorUp(void)
{
	if(data.count==0)
	{
		return;
	}

	if(cur.currentLine>0)
	{
		cur.currentLine-=1;
	}
	else
	{
		return;
	}

	cur.y = cur.currentLine*CHAR_HEIGHT;
							

	if((data.lines[cur.currentLine].end - data.lines[cur.currentLine].start)<cur.positionLine)
	{
		cur.positionLine = data.lines[cur.currentLine].end - data.lines[cur.currentLine].start;
		cur.x = ( data.lines[cur.currentLine].end - data.lines[cur.currentLine].start ) * CHAR_WIDTH;
		return;
	}
	cur.x = cur.positionLine * CHAR_WIDTH;
	
}

void moveCursorDown(void)
{
	if(data.count==0)
	{
		return;
	}
	if(cur.currentLine < data.lineCount-1)
	{
		cur.currentLine += 1;
	}
	else
	{
		return;
	}

	cur.y = cur.currentLine*CHAR_HEIGHT;
							
	int lineLength = data.lines[cur.currentLine].end - data.lines[cur.currentLine].start;

	if(lineLength < cur.positionLine)
	{
		cur.positionLine = lineLength;
		cur.x = lineLength  * CHAR_WIDTH;
		return;
	}
	cur.x = cur.positionLine * CHAR_WIDTH;
	
}
void moveCursorRight(void)
{
	if(data.count==0)
	{
		return;
	}
	int lineLength = data.lines[cur.currentLine].end - data.lines[cur.currentLine].start;
	
	if(cur.positionLine < lineLength)
	{
		cur.positionLine += 1;
	}
	else if(cur.positionLine == lineLength)
	{

		cur.positionLine += 1;
		
		if(cur.currentLine<data.lineCount-1)
		{
			cur.currentLine += 1;
			cur.positionLine = 0;
		}
	}
	
	cur.x = cur.positionLine * CHAR_WIDTH;
	cur.y = cur.currentLine * CHAR_HEIGHT;
	
}

void moveCursorLeft(void)
{
	if(data.count==0)
	{
		return;
	}

	int lineLength = data.lines[cur.currentLine].end - data.lines[cur.currentLine].start;
	
	if(cur.positionLine > 0)
	{
		cur.positionLine -= 1;
	}
	else if(cur.positionLine == 0)
	{
		
		if(cur.currentLine>0)
		{
			cur.currentLine -= 1;
			cur.positionLine = data.lines[cur.currentLine].end;
		}
	}
	
	cur.x = cur.positionLine * CHAR_WIDTH;
	cur.y = cur.currentLine * CHAR_HEIGHT;
	
}

void renderChar(SDL_Renderer * renderer, TTF_Font * font, SDL_Color color,int posX,int posY, char c)
{
	if(posX == cur.x && posY == cur.y)
	{
		SDL_Color color = { 0, 0, 0 };
	}
	SDL_Surface * surface = TTF_RenderText_Solid(font,
 												&c, 
												color);

	SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_Rect dstrect = { posX, posY, CHAR_WIDTH, CHAR_HEIGHT };

	SDL_RenderCopy(renderer, texture, NULL, &dstrect);

}

void renderText(SDL_Renderer * renderer, TTF_Font * font, SDL_Color color, Data *data)
{


	if(data->count == 0)
	{
		return;
	}

	int posX = 0;
	int posY = 0;
	int position = 0;
	int startLine = 0;
	Line lineToAdd = {0};
	int linesAdded = 0;

	if(data->lineCount>0)
	{
		data->lineCount = 0;
		free(data->lines);
	} 
	data->lines = (Line *) malloc(sizeof(Line)* 50 );

	if(data->lines == NULL)
	{
		fprintf(stderr, "Memory allocation failed.\n");
		return;
	}

	for(int i=0; i < data->count; ++i)
	{
	
		if(data->data[i] == '\n'  )
		{
			
			posX =0;
			posY += CHAR_HEIGHT;

			data->lineCount += 1;

			lineToAdd.end = i+1;

			data->lines[linesAdded] = lineToAdd;

			linesAdded += 1;

			lineToAdd.start = i + 1;
			continue;
		}

		if(posX == (windowWidth - (windowWidth % CHAR_WIDTH)  ))
		{

			posX = 0;
			posY += CHAR_HEIGHT;

			data->lineCount += 1;

			lineToAdd.end = i-1;

			data->lines[linesAdded] = lineToAdd;

			linesAdded += 1;

			lineToAdd.start = i ;
		}

		renderChar(renderer,font,color,posX,posY,data->data[i]);

		posX += CHAR_WIDTH;

	}
	
	
	data->lineCount += 1;
	lineToAdd.end = data->count;
	data->lines[linesAdded] = lineToAdd;

}


void renderCursor(SDL_Renderer * renderer)
{
	SDL_Rect dstrect = { cur.x, cur.y , CHAR_WIDTH, CHAR_HEIGHT };

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);

	SDL_SetRenderDrawColor(renderer, 10, 255, 255, 150);

	if(SDL_RenderFillRect(renderer,&dstrect)<0)
	{
		fprintf(stderr, "ERROR: Could not initialize SDL: %s\n", SDL_GetError());
		exit(1);
	};

}


int main(int argc, char *argv[])
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "ERROR: Could not initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

	SDL_Window *window = SDL_CreateWindow(
						"text edtr",
                         0, 
						 0,
                         windowWidth, 
						 windowHeight,
                         SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);


    if (window == NULL) {
        fprintf(stderr, "ERROR: Could not create SDL window: %s\n", SDL_GetError());
        return 1;
    }

	SDL_Renderer * renderer = SDL_CreateRenderer(window,-1,0);

	if (renderer == NULL)
	{
		fprintf(stderr, "ERROR: Could not create SDL window: %s\n", SDL_GetError());
        return 1;
	}
	if (TTF_Init() < 0) {
        fprintf(stderr, "ERROR: Could not initialize TTF: %s\n", SDL_GetError());
        return 1;
    }
	

	TTF_Font *font = TTF_OpenFont("Raleway-Regular.ttf",25);
	if(font == NULL){
		fprintf(stderr, "ERROR: Could not create SDL Font: %s\n", SDL_GetError());
        return 1;
	}

	SDL_Color color = { 250, 250, 250 };
	
	
	bool quit = false;

	while(!quit)
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
		{
			
			switch(event.type)
			{
				
				case SDL_QUIT:
					quit = true;
					break;
				case SDL_KEYDOWN:
				{
					switch (event.key.keysym.sym)
					{
					case SDLK_RETURN:{
						addChar("\n",&data);

						break;
					}
					case SDLK_UP:
					{
						moveCursorUp();
						break;
					}
					case SDLK_DOWN:
					{
						moveCursorDown();
						break;
					}
					case SDLK_RIGHT:
					{
						moveCursorRight();
						break;
					}
					case SDLK_LEFT:
					{
						moveCursorLeft();
						break;
					}

					default:
						break;
					}
				}
				break;
				case SDL_TEXTINPUT:
				{
					addChar(event.text.text,&data);
				}
				break;
				default:
					break;
				
					
			}
		}
		
		SDL_SetRenderDrawColor(renderer,0,0,0,0);
		SDL_RenderClear(renderer);
		SDL_GetWindowSize(window, &windowWidth,&windowHeight);
		renderText(renderer,font,color, &data);
		renderCursor(renderer);
		SDL_RenderPresent(renderer);
		//SDL_Delay(10);
	}


	TTF_Quit();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}