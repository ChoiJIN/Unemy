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
	
	int number;			// ���� ������ �÷��̾� ��
	Player *players;	// ��Ʈ��ũ �÷��̾�
	Player me;			// �÷��� ����
};

Current current;