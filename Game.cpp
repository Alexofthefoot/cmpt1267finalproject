#include <stdio.h>
#include "Game.h"
#include "SDL/SDL_image.h"



const int playerWidth = 128;
const int playerHeight = 128;

enum PlayerFacing
{
	Player_Facing_Up = 0,		//0
	Player_Facing_Left = 1,		//1
	Player_Facing_Down = 2,		//2
	Player_Facing_Right = 3,	//3
};

enum PlayerRunCycle
{
	cycle = 0
};

Game::Game()
{
	//mSurface = nullptr;
	mWindow = nullptr;
	mRenderer = nullptr;
	mTexture = nullptr;
	mTicksCount = 0;
	mIsRunning = true;
	mPlayerFacing = Player_Facing_Right; 
	mPlayerLives = 3;
	mCycle = 0;
	mHayBales = 0;
	mMaxHayBales = 3;
	mMusicplaying = 1;
	mChomp = nullptr;
	mBackground = nullptr;
}

bool Game::Initialize()
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return false;
	}
	// Initialize IMG
	if (IMG_Init(IMG_INIT_PNG) == 0)
	{
		SDL_Log("Unable to initialize SDL_image: %s", SDL_GetError());
		return false;
	}

	// Create an SDL Window
	mWindow = SDL_CreateWindow(
		"CMPT 1267", // Window title
		100,	// Top left x-coordinate of window
		100,	// Top left y-coordinate of window
		1024,	// Width of window
		768,	// Height of window
		0		// Flags (0 for no flags set)
	);

	if (!mWindow)
	{
		SDL_Log("Failed to create window: %s", SDL_GetError());
		return false;
	}

	//// Create SDL renderer
	mRenderer = SDL_CreateRenderer(
		mWindow, // Window to create renderer for
		-1,		 // Usually -1
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
	);

	if (!mRenderer)
	{
		SDL_Log("Failed to create renderer: %s", SDL_GetError());
		return false;
	}

	// Set up the music and sound effects
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	mBackground = Mix_LoadMUS("Assets/jauntytune.wav");
	mChomp = Mix_LoadWAV("assets/chewing.wav");
	Mix_PlayMusic(mBackground, -1);


	// Display the welcome screen & wait for further action
	Welcome();

	// if there is any texture loaded, destroy it. Otherwise set it to the sprite sheet
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
	}
	mTexture = LoadFromFile("Assets/spritesheet2.png");
	if (mTexture == NULL)
	{
		SDL_Log("Unable to load png file: %s", SDL_GetError());
		return false;
	}

	//Set players info
	mPlayerPos.x = 1024.0f / 2.0f; 
	mPlayerPos.y = 768.0f / 2.0f;
	mPlayerDir.x = 0.0f;
	mPlayerDir.y = 0.0;
	return true;
}

void Game::Welcome()
{
	srand(time(0));
	//Display welcome screen
	mTexture = IMG_LoadTexture(mRenderer, "Assets/WelcomeScreen.png");
	SDL_RenderCopy(mRenderer, mTexture, NULL, NULL);
	SDL_RenderPresent(mRenderer);

	//Wait until enter is pressed
	SDL_Event event;
	while (true)
	{
		if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
		{
			mIsRunning = false;
			return;
		}
		else if (event.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.sym == SDLK_RETURN)
			{
				mTexture = nullptr;
				return;
			}
			if (event.key.keysym.sym == SDLK_p)
			{
				Togglemusic();
			}
		}
			
		
	}
}

void Game::Togglemusic()
{
	if (Mix_PlayingMusic() == 0)
	{
		Mix_PlayMusic(mBackground, -1);
	}
	else
	{
		if (Mix_PausedMusic() == 1)
		{
			Mix_ResumeMusic();
		}
		else
		{
			Mix_PauseMusic();
		}
	}
}

void Game::RunLoop()
{
	while (mIsRunning)
	{
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::IncrementRunCycle()
{
	mCycleCount++;
	if (mCycleCount > 12)
	{
		mCycleCount = mCycleCount % 12;
	}
	mCycle = mCycleCount / 6;
}

void Game::ProcessInput()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			// If we get an SDL_QUIT event, end loop
		case SDL_QUIT:
			mIsRunning = false;
			break;
		}
	}

	// Get state of keyboard
	const Uint8* state = SDL_GetKeyboardState(NULL);
	// If escape is pressed, also end loop
	if (state[SDL_SCANCODE_ESCAPE])
	{
		mIsRunning = false;
	}

	mPlayerDir.x = 0.0f;
	mPlayerDir.y = 0.0f;

	// Update player direction and movement based on W/A/S/D
	if (state[SDL_SCANCODE_W])
	{
		mPlayerDir.y -= 4.0f;
		mPlayerFacing = Player_Facing_Up;
	}
	if (state[SDL_SCANCODE_S])
	{
		mPlayerDir.y += 4.0f;
		mPlayerFacing = Player_Facing_Down;
	}
	if (state[SDL_SCANCODE_A])
	{
		mPlayerDir.x -= 4.0f;
		mPlayerFacing = Player_Facing_Left;
	}
	if (state[SDL_SCANCODE_D])
	{
		mPlayerDir.x += 4.0f;
		mPlayerFacing = Player_Facing_Right;
	}
	// Pause/play music
	if (state[SDL_SCANCODE_P])
	{
		Togglemusic();
	}
	IncrementRunCycle();
}

void Game::UpdateGame()
{
	// Update tick counts (for next frame)
	mTicksCount = SDL_GetTicks();


	// Update player position based on direction
	mPlayerPos.x += mPlayerDir.x;
	mPlayerPos.y += mPlayerDir.y;

	
	// Did the player go off the screen?
	if (OffScreen())
	{
		mIsRunning = false;
	}

	UpdateEnvironment();
}

void Game::UpdateEnvironment()
{
	SpawnHayBales();
}

void Game::SpawnHayBales()
{
	if (myHaybales.size() < mMaxHayBales)
	{
		Haybale haybale;
		haybale.Initialize();
		myHaybales.push_back(haybale);
		
	}
	printf("Number of bales: %d", myHaybales.size());
	for (Haybale h : myHaybales)
	{
		printf("\n%d\n     %d\n", h.GetXPosition(), h.GetYPosition());
	}
}

bool Game::OffScreen()
{
	// offset calculated based on window offset along with centre point of image = 100 + 64
	int offset = -164;
	if (mPlayerPos.x <= offset || mPlayerPos.x >= 1024 + offset ||
		mPlayerPos.y <= offset || mPlayerPos.y >= 768 + offset)
	{
		return true;
	}
	return false;
}

void Game::GenerateOutput()
{
	// Set background draw color to green
	SDL_SetRenderDrawColor(mRenderer,0,	230, 0, 255);
	// Clear back buffer
	SDL_RenderClear(mRenderer);
	
	//First set the grassy background
	SDL_Rect srcRect{985,0,610,460};
	SDL_RenderCopy(mRenderer, mTexture, &srcRect, NULL);

	//then hay
	srcRect.x = 500;
	srcRect.y = 0;
	srcRect.w = 460;
	srcRect.h = 450;

	SDL_Rect Hay_Rect{0,0,playerWidth, playerHeight};

	for (Haybale h : myHaybales)
	{
		Hay_Rect.x = h.GetXPosition();
		Hay_Rect.y = h.GetYPosition();
		//SDL_RenderFillRect(mRenderer, &Hay_Rect);
		SDL_RenderCopy(mRenderer, mTexture, &srcRect, &Hay_Rect);
	}


	//then character
	srcRect.x = 128*mCycle;
	srcRect.y = 128*mPlayerFacing;
	srcRect.w = 128;
	srcRect.h = 128;

	SDL_Rect Player_Rect{
		mPlayerPos.x,			// Top left x
		mPlayerPos.y,			// Top left y
		playerWidth*2,			// Width
		playerHeight*2			// Height
	};
	//SDL_RenderFillRect(mRenderer, &Player_Rect);       THIS GIVES THE SQUARE AROUND THE IMAGE, NOT NEEDED!

	SDL_RenderCopy(mRenderer, mTexture, &srcRect, &Player_Rect);


	// Display remaining lives/health in top corner (for now using the same haybale image)
	SDL_Rect Healthbar_Rect{ 5,5,50,50 };
	srcRect.y = 128 * 3; //have it always facing right
	for (int i = 0; i < mPlayerLives; i++)
	{
		//SDL_RenderFillRect(mRenderer, &Healthbar_Rect);        THIS GIVES THE SQUARE AROUND THE IMAGE, NOT NEEDED in final copy!
		SDL_RenderCopy(mRenderer, mTexture, &srcRect, &Healthbar_Rect);
		Healthbar_Rect.x += 60;
	}

	// Swap front buffer and back buffer (making it all visible)
	SDL_RenderPresent(mRenderer);
}



SDL_Texture* Game::LoadFromFile(std::string filename)
{
	SDL_Surface* loadedSurface = IMG_Load(filename.c_str());
	SDL_Texture* mTexture = SDL_CreateTextureFromSurface(mRenderer, loadedSurface);

	if (loadedSurface == nullptr)
	{
		printf("Error! Cannot Load the Image!!");
		SDL_FreeSurface(loadedSurface);
		return false;
	}
	
	SDL_FreeSurface(loadedSurface);
	return mTexture;
	
}

void Game::UnloadData()
{
	//Free loaded images
	if (mTexture != NULL)
	{
		mTexture = NULL;
	}
}

void Game::Shutdown()
{
	UnloadData();
	SDL_DestroyRenderer(mRenderer);
	//SDL_FreeSurface(mSurface);
	SDL_DestroyWindow(mWindow);
	IMG_Quit();
	SDL_Quit();
}