constexpr int SERVER_PORT = 4000;
constexpr int BUF_SIZE = 1024;

constexpr int MAX_USER = 2000;

constexpr int WORLD_WIDTH = 12000;		// �� = 60cm(px), ���� = �� 200�� 60 * 200
constexpr int WORLD_LENGHT = 12000;		// �� = 60cm(px), ���� = �� 200�� 60 * 200
constexpr int WORLD_HEIGHT = 3600;		// �� = 60cm(px), ���� = �Ʒ� 30��, �� 30�� 60 * 60

string SERVER_IP = "127.0.0.1";

/*
	��Ŷ ���� ��Ģ
	��Ŷ�� �� ó���� unsigned char size�� ��Ŷ�� ����� ��Ÿ�� ������ ����� ��.
	�� ������ char type�� ��Ŷ�� Ÿ��, ��, ���� ������ ��Ŷ�� �������� ǥ���� �� -> ���̵� ���� ����

	CS -> 'Ŭ���̾�Ʈ'�� '����'����
	SC -> '����'�� 'Ŭ���̾�Ʈ'����
*/

// packet id
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;

constexpr char SC_LOGIN = 2;
constexpr char SC_START = 3;

#pragma pack(push, 1)

struct CS_LOGIN_PACKET {
	unsigned char		size;
	char				type;
};

struct CS_MOVE_PACKET {
	unsigned char		size;
	char				type;
	char				direction;		// 0 = Up, 1 = Down, 2 = Left, 3 = Right
	short				x, y;			// �̵��ϴ� ��ǥ, ���߿� �ٲ� ��
};

//===========================
struct SC_LOGININFO_PACKET
{
	unsigned char		size;
	char				type;
	int					id;
};
struct SC_START_PACKET
{
	unsigned char		size;
	char				type;
	// �ʱ� ����(��ġ, ����)
	// �ʱ� ���� ��ġ
};

#pragma pack(pop)