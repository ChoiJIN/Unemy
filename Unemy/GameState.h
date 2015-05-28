enum ScreenState {
	START, MENU, NETWORK, GAME
};

enum GameState {

};

struct Current {
	ScreenState screen;
	GameState game;
};

Current current;