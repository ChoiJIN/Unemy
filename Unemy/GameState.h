enum Screen {
	START, MENU, GAME, NETWORK
};

enum PlayerState {
	ALIVE, DEAD
};

struct Player {
	int x, y;
	int size;
	PlayerState state;
};

struct Current {
	Screen screen;
	
	int number;			// 나를 제외한 플레이어 수
	Player *players;	// 네트워크 플레이어
	Player me;			// 플레이 유저
};

Current current;