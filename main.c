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
#define CHANGE_TIME 2000


typedef struct Cursor
{
	int position;
} Cursor;

typedef struct Line 
{
	int start;
	int end;
} Line;

typedef enum
{
	ADDITION,
	DELETE
} ChangeType;

typedef struct Change
{
	ChangeType type;
	int start;
	int end;
	char *text;	
	size_t count;
} Change;

typedef struct Data
{
	bool changed;
	char *data;
	Line *lines;
	Change *changes;
	size_t count;
	size_t lineCount;
	size_t changeCount;
	Uint32 lastTimeChangeAdded;
	size_t currentChange;
} Data;

static Data data = {.changed = true, .count = 0} ;

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
	int positionToRemove = cur.position - 1;
		
	char *placeholder = (char *) malloc(sizeof(char) * (data.count+1));

	placeholder[data.count] = '\0';

	if(placeholder == NULL)
	{
		exit(1);
	}
	
	placeholder = strcpy(placeholder, data.data);
	
	data.data = (char *) malloc(sizeof(char) * (data.count)); 
	
	data.data[data.count-1] = '\0';
	
	for(int i =0; i < positionToRemove ; ++i)
	{
		data.data[i] = placeholder[i];

	}

	
	for(int i = positionToRemove + 1 ; i < data.count ; ++i )
	{
		data.data[i-1] = placeholder[i];

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

void initializeChanges(Uint32 time, ChangeType type)
{
	if(data.changes != NULL)
	{
		free(data.changes);
	}
	Change change = {.count = 1, .start = cur.position-1, .end = cur.position-1, .type =  ADDITION};
	change.text = (char *) malloc(sizeof(char) * (2)); 
	change.text[0] = data.data[cur.position-1];
	change.text[1] = '\0';

	data.changes = (Change *) malloc(sizeof(Change)*50);
	data.changeCount = 1;
	data.lastTimeChangeAdded = time;
	data.changes[0] = change;
	data.currentChange = 1;
	

}

void updateChanges(Uint32 time, ChangeType type)
{
	if(data.currentChange==0)
	{
		initializeChanges(time,type);
		return;
	}


	int indexOfCurrentChange = data.currentChange - 1;
	size_t sizeOfCurrentChange = data.changes[indexOfCurrentChange].count;


	if(data.changes[indexOfCurrentChange].type != type)
	{
		Change change = {.count = 1, .start = cur.position - 1, .type = type ,.end = cur.position - 1};
		change.text = (char *) malloc(sizeof(char) * (2)); 
		change.text[0] = data.data[cur.position-1];
		change.text[1] = '\0';
		data.changes[data.changeCount] = change;
		data.lastTimeChangeAdded = time;
		data.currentChange += 1;
		data.changeCount = data.currentChange;

	}
	else
	{
		if((time - data.lastTimeChangeAdded) / CHANGE_TIME == 0)
		{
			
			char *placeholder = (char *) malloc(sizeof(char) * (sizeOfCurrentChange+1));
			placeholder[sizeOfCurrentChange] = '\0';

			placeholder = strcpy(placeholder, data.changes[indexOfCurrentChange].text);
			data.changes[indexOfCurrentChange].text = (char *) malloc(sizeof(char) * (sizeOfCurrentChange+2));

			data.changes[indexOfCurrentChange].text[sizeOfCurrentChange + 1] = '\0';
			data.changes[indexOfCurrentChange].text = strcpy(data.changes[indexOfCurrentChange].text,placeholder);
			data.changes[indexOfCurrentChange].text[sizeOfCurrentChange] = data.data[cur.position-1];
			data.changes[indexOfCurrentChange].count += 1;
			data.changes[indexOfCurrentChange].end = cur.position-1;
			
		}
		else
		{
			data.lastTimeChangeAdded = time;
			Change change = {.count = 1, .start = cur.position - 1, .type = type ,.end = cur.position - 1};
			change.text = (char *) malloc(sizeof(char) * (2)); 
			change.text[0] = data.data[cur.position-1];
			change.text[1] = '\0';
			data.changes[indexOfCurrentChange+1] = change;
			data.currentChange += 1;
			data.changeCount = data.currentChange;
		}
	}
	
}

void reverseChange(void)
{
	if(data.changeCount == 0 || data.currentChange == 0)
	{
		return;
	}
	int currentChangeIndex = data.currentChange - 1;

	if(data.changes[currentChangeIndex].type == ADDITION)
	{
		
		int charsToBeRemoved = data.changes[currentChangeIndex].count;
		char * placeholder = (char *) malloc(sizeof(char) * (data.count + 1));
		placeholder[data.count] = '\0';
		placeholder = strcpy(placeholder,data.data);
		data.data = (char *) malloc(sizeof(char) * (data.count - charsToBeRemoved + 1));
		data.data[data.count - charsToBeRemoved] = '\0';

		for(int i = 0; i < data.changes[currentChangeIndex].start; ++i)

		{
			data.data[i] = placeholder[i];
		}

		for(int i = data.changes[currentChangeIndex].end + 1; i < data.count; ++i)
		{
			data.data[i-charsToBeRemoved] = placeholder[i];
		}
		cur.position = data.changes[currentChangeIndex].start;
		data.count -= charsToBeRemoved;
		printf("%s",data.data);
		free(placeholder);
	}
	else if(data.changes[currentChangeIndex].type == DELETE)
	{
		int charsToBeAdded = data.changes[currentChangeIndex].count;
		char * placeholder = (char *) malloc(sizeof(char) * (data.count + 1));
		placeholder[data.count] = '\0';
		placeholder = strcpy(placeholder,data.data);
		data.data = (char *) malloc(sizeof(char) * (data.count + charsToBeAdded + 1));
		data.data[data.count + charsToBeAdded] = '\0';
		for(int i = 0; i < data.changes[currentChangeIndex].end; ++i)
		{
			data.data[i] = placeholder[i];
		}

		
		for(int i = 0, j = data.changes[currentChangeIndex].end; i < data.changes[currentChangeIndex].count; ++i , ++j)
		{
			data.data[j] = data.changes[currentChangeIndex].text[data.changes[currentChangeIndex].count- i - 1];
		}

		for(int i = data.changes[currentChangeIndex].start + 1; i < data.count + charsToBeAdded; ++i )
		{
			data.data[i] = placeholder[i-charsToBeAdded];
		}
		cur.position = data.changes[currentChangeIndex].start+1;
		data.count += charsToBeAdded;
		free(placeholder);

	}

	data.currentChange -= 1;
}

void reimplementChange(void)
{
	if(data.currentChange == data.changeCount)
	{
		return;
	}
	int currentChangeIndex = data.currentChange;

	if(data.changes[currentChangeIndex].type == ADDITION)
	{
		printf("a\n");
		int charsToBeAdded = data.changes[currentChangeIndex].count;
		char * placeholder = (char *) malloc(sizeof(char) * (data.count + 1));
		placeholder[data.count] = '\0';
		placeholder = strcpy(placeholder,data.data);
		data.data = (char *) malloc(sizeof(char) * (data.count + charsToBeAdded + 1));
		data.data[data.count + charsToBeAdded] = '\0';

		for(int i = 0; i < data.changes[currentChangeIndex].start; ++i)
		{
			data.data[i] = placeholder[i];
		}

		
		for(int i = 0, j = data.changes[currentChangeIndex].start; i < data.changes[currentChangeIndex].count; ++i , ++j)
		{
			data.data[j] = data.changes[currentChangeIndex].text[i];
		}

		for(int i = data.changes[currentChangeIndex].end + 1; i < data.count + charsToBeAdded; ++i )
		{
			data.data[i] = placeholder[i-charsToBeAdded];
		}
		cur.position = data.changes[currentChangeIndex].end+1;
		data.count += charsToBeAdded;
		free(placeholder);

	}
	else if(data.changes[currentChangeIndex].type == DELETE)
	{
		int charsToBeRemoved = data.changes[currentChangeIndex].count;
		char * placeholder = (char *) malloc(sizeof(char) * (data.count + 1));
		placeholder[data.count] = '\0';
		placeholder = strcpy(placeholder,data.data);
		data.data = (char *) malloc(sizeof(char) * (data.count - charsToBeRemoved + 1));
		data.data[data.count - charsToBeRemoved] = '\0';

		for(int i = 0; i < data.changes[currentChangeIndex].end; ++i)

		{
			data.data[i] = placeholder[i];
		}

		for(int i = data.changes[currentChangeIndex].start + 1; i < data.count; ++i)
		{
			data.data[i-charsToBeRemoved] = placeholder[i];
		}
		cur.position = data.changes[currentChangeIndex].end;
		data.count -= charsToBeRemoved;
		free(placeholder);
	}

	data.currentChange += 1;
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
	// for(int i = 0; i < data.lineCount; ++i)
	// {
	// 	printf("{%d,%d}",data.lines[i].start,data.lines[i].end);
	// }
	// printf("\n");
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
	bool isCtrlPressed = false;
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
				case SDL_WINDOWEVENT:
					if(event.window.event == SDL_WINDOWEVENT_RESIZED)
					{
						data.changed = true;
					}
					break;
				case SDL_KEYDOWN:
				{
					switch (event.key.keysym.sym)
					{
						case SDLK_BACKSPACE:
						{
							if(data.count!=0 || cur.position != 0)
							{
								data.changed = true;
								updateChanges(SDL_GetTicks(),DELETE);
								removeChar();
							}
							break;
						}
						case SDLK_RETURN:
						{	
							data.changed = true;
							addChar("\n");
							updateChanges(SDL_GetTicks(),ADDITION);
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
						case SDLK_LCTRL:
						{
							isCtrlPressed = true;
							break;
						}
						case SDLK_RCTRL:
						{
							isCtrlPressed = true;
							break;
						}
						default:
							break;
					}
				}
				break;
				case SDL_KEYUP:
				{
					switch (event.key.keysym.sym)
					{
						case SDLK_LCTRL:
						{
							isCtrlPressed = false;
							break;
						}
						case SDLK_RCTRL:
						{
							isCtrlPressed = false;
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
					updateChanges(SDL_GetTicks(),ADDITION);
				}
				break;
				default:
					break;
				
					
			}
		}
		if(isCtrlPressed)
		{
			const Uint8* keys =  SDL_GetKeyboardState(NULL);
			if(keys[SDL_SCANCODE_Z])
			{
				data.changed = true;
				reverseChange();
			}
			else if(keys[SDL_SCANCODE_Y])
			{
				data.changed = true;
				reimplementChange();
			}
			
			SDL_Delay(550);
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
	}

	if(data.data!=NULL)
	{
		free(data.data);
		free(data.changes);
	}
	if(data.lineCount!=0)
	{
		free(data.lines);
	}
	TTF_Quit();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}