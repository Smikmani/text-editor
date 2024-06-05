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
	bool changed;
	char *data;
	Line *lines;
	size_t lineCount;
	size_t count;
} Data;

static Data data = {.changed = true} ;

static Cursor cur = {0};


static int windowWidth = 800;

static int windowHeight = 600;



void addChar(char *c)
{
	if(data.data==NULL)
	{
		

		data.count += 1;

		data.data = (char *) malloc(sizeof(char) * (2)); 
	
		data.data[1] = '\0';
		data.data = strcpy(data.data, c);
		
		cur.position += 1;

		return;
	}

	int positionToAdd = cur.position;
	
		
	char *placeholder = (char *) malloc(sizeof(char) * (data.count+1));

	placeholder[data.count] = '\0';

	if(placeholder == NULL)
	{
		exit(1);
	}
	
	placeholder = strcpy(placeholder, data.data);
	
	data.data = (char *) malloc(sizeof(char) * (data.count+2)); 
	
	data.data[data.count+1] = '\0';
	
	for(int i =0; i < positionToAdd ; ++i)
	{
		data.data[i] = placeholder[i];

	}

	data.data[positionToAdd] = *c;
	
	for(int i =positionToAdd ; i < data.count ; ++i )
	{
		data.data[i+1] = placeholder[i];

	}

	cur.position += 1;
	data.count += 1;

	free(placeholder);
}

void removeChar()
{

	if(data.count==0 || cur.position == 0)
	{
		return;
	}

	int positionToRemove = cur.position - 1;
	
		
	char *placeholder = (char *) malloc(sizeof(char) * (data.count+1));

	placeholder[data.count] = '\0';

	if(placeholder == NULL)
	{
		exit(1);
	}
	
	placeholder = strcpy(placeholder, data.data);
	
	data.data = (char *) malloc(sizeof(char) * (data.count+2)); 
	
	data.data[data.count+1] = '\0';
	
	for(int i =0; i < positionToRemove ; ++i)
	{
		data.data[i] = placeholder[i];

	}

	
	for(int i = positionToRemove + 1; i < data.count ; ++i )
	{
		data.data[i] = placeholder[i+1];

	}

	cur.position -= 1;
	data.count -= 1;

	free(placeholder);
}

int getCursorLineIndex()
{
	
	for(int i = 0; i < data.lineCount; ++i)
	{
		if(data.lines[i].start <= cur.position && data.lines[i].end >= cur.position)
		{
			return i;
		}
	}

	return 0;
}

void getCharacterWidth(SDL_Renderer * renderer, TTF_Font * font, SDL_Color color, int *width, char c )
{
	char str[2] = {c ,'\0'};
	SDL_Surface * surface = TTF_RenderText_Solid(font,
 												str, 
												color);

	SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);
	int y;
	SDL_QueryTexture(texture,NULL,NULL,width,&y);

}

void moveCursorUp(void)
{
	if(data.count==0)
	{
		return;
	}
	int cursorLineIndex = getCursorLineIndex();

	Line currentLine = data.lines[cursorLineIndex];

	int cursorPositionOnLine = cur.position - currentLine.start;

	if(cursorLineIndex>0)
	{
		if((data.lines[cursorLineIndex-1].end - data.lines[cursorLineIndex-1].start) < cursorPositionOnLine)
		{
			cur.position = data.lines[cursorLineIndex-1].end ;
		}
		else
		{
			cur.position = data.lines[cursorLineIndex-1].start + cursorPositionOnLine;
		}
	}	
}

void moveCursorDown(void)
{
	if(data.count==0)
	{
		return;
	}
	int cursorLineIndex = getCursorLineIndex();

	Line currentLine = data.lines[cursorLineIndex];

	int cursorPositionOnLine = cur.position - currentLine.start;

	if(cursorLineIndex < data.lineCount-1)
	{
		if((data.lines[cursorLineIndex+1].end - data.lines[cursorLineIndex+1].start) < cursorPositionOnLine)
		{
			cur.position = data.lines[cursorLineIndex+1].end ;
		}
		else
		{
			cur.position = data.lines[cursorLineIndex+1].start + cursorPositionOnLine;
		}
	}
}
void moveCursorRight(void)
{
	
	if(cur.position < data.count)
	{
		cur.position += 1;
	}
	
}

void moveCursorLeft(void)
{
	if(cur.position > 0)
	{
		cur.position -= 1;
	}
	
}

void renderChar(SDL_Renderer * renderer, TTF_Font * font, SDL_Color color,int *posX,int posY, char c)
{
	char str[2] = {c ,'\0'};
	SDL_Surface * surface = TTF_RenderText_Solid(font,
 												str, 
												color);

	SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);
	int w,h;
	SDL_QueryTexture(texture,NULL,NULL,&w,&h);
	SDL_Rect dstrect = { *posX, posY, w, h };
	SDL_RenderCopy(renderer, texture, NULL, &dstrect);
	*posX += w;


}

void renderText(SDL_Renderer * renderer, TTF_Font * font, SDL_Color color)
{


	if(data.count == 0)
	{
		return;
	}

	int posX = 0;
	int posY = 0;
	int startLine = 0;
	Line lineToAdd = {0};
	int linesAdded = 0;

	if(data.lineCount>0)
	{
		data.lineCount = 0;
		free(data.lines);
	}

	data.lines = (Line *) malloc(sizeof(Line)* 50 );

	if(data.lines == NULL)
	{
		fprintf(stderr, "Memory allocation failed.\n");
		return;
	}

	for(int i=0; i < data.count; ++i)
	{
	
		if(data.data[i] == '\n'  )
		{
			
			posX =0;
			posY += CHAR_HEIGHT;

			data.lineCount += 1;

			lineToAdd.end = i;

			data.lines[linesAdded] = lineToAdd;

			linesAdded += 1;

			lineToAdd.start = i+1 ;
			continue;
		}

		if(posX >= (windowWidth - (windowWidth % CHAR_WIDTH)-30  ))
		{

			posX = 0;
			posY += CHAR_HEIGHT;

			data.lineCount += 1;

			lineToAdd.end = i-1;

			data.lines[linesAdded] = lineToAdd;

			linesAdded += 1;

			lineToAdd.start = i ;
		}

		renderChar(renderer,font,color,&posX,posY,data.data[i]);

		//posX += CHAR_WIDTH;

	}
	
	
	data.lineCount += 1;
	lineToAdd.end = data.count;
	data.lines[linesAdded] = lineToAdd;
	for(int i = 0; i < data.lineCount; ++i)
	{
		printf("{%d,%d}",data.lines[i].start,data.lines[i].end);
	}
	printf("\n");
}


void renderCursor(SDL_Renderer * renderer, TTF_Font * font, SDL_Color color)
{		
	int currentLineIndex = getCursorLineIndex();
	int posX = 0;
	int charWidth = CHAR_WIDTH;
	if(data.count>0){
		
		
		for(int i = data.lines[currentLineIndex].start ; i < cur.position; ++i)
		{
			if(data.data[i]=='\n') continue;
			getCharacterWidth(renderer,font,color,&charWidth,data.data[i]);
			posX += charWidth;
		}
		
		if(cur.position==data.lines[currentLineIndex].end)
		{
			charWidth = CHAR_WIDTH;
		}
		else
		{
			getCharacterWidth(renderer,font,color,&charWidth,data.data[cur.position]);
		}
	}
	
	
	SDL_Rect dstrect = { posX, currentLineIndex*CHAR_HEIGHT, charWidth, CHAR_HEIGHT };

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
					case SDLK_BACKSPACE:
					{
						data.changed = true;
						removeChar();
						break;
					}
					case SDLK_RETURN:
					{	
						data.changed = true;
						addChar("\n");
						break;
					}
					case SDLK_UP:
					{
						data.changed = true;
						moveCursorUp();
						break;
					}
					case SDLK_DOWN:
					{
						data.changed = true;
						moveCursorDown();
						break;
					}
					case SDLK_RIGHT:
					{
						data.changed = true;
						moveCursorRight();
						break;
					}
					case SDLK_LEFT:
					{
						data.changed = true;
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
					data.changed = true;
					addChar(event.text.text);
				}
				break;
				default:
					break;
				
					
			}
		}
		if(data.changed)
		{
			SDL_SetRenderDrawColor(renderer,0,0,0,0);
			SDL_RenderClear(renderer);
			SDL_GetWindowSize(window, &windowWidth,&windowHeight);
			renderText(renderer,font,color);
			renderCursor(renderer,font,color);
			SDL_RenderPresent(renderer);
		}
		data.changed = false;
		//SDL_Delay(10);
	}


	TTF_Quit();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}