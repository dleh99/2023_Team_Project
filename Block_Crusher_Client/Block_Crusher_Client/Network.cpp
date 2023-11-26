#include "Network.h"

WSADATA wsa;
SOCKET g_socket;
SOCKADDR_IN serveraddr;
char recvBuf[BUF_SIZE];

string SERVER_IP = "127.0.0.1";

float start_x, start_y, start_z;
int id;

float otherPlayer_x, otherPlayer_y, otherPlayer_z;
Pos otherPlayerPos;
Mouse otherPlayerMouse;
int otherPlayer_id = -1;

// ���� ���� ����
bool m_gameStart = false;

int NetworkInit()
{
	int ret;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		cout << "WSA START ERROR" << endl;
		return 1;
	}

	g_socket = socket(AF_INET, SOCK_STREAM, 0);				// AF_INET = IPv4 �ּ�ü��, SOCK_STREAM = TCP ����, 0 = �ý����� �˾Ƽ� �������� ����
	if (g_socket == INVALID_SOCKET) err_quit("socket()");

	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_IP.data(), &serveraddr.sin_addr);

	ret = connect(g_socket, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

	unsigned long noblock = -1;
	ioctlsocket(g_socket, FIONBIO, &noblock);

	send_login_packet();

	return ret;
}

//void send_packet(void* packet)
//{
//	unsigned char* p = reinterpret_cast<unsigned char*>(packet);
//	size_t sent = 0;
//	send(g_socket, p, sizeof(p), 0);
//}

void send_login_packet()
{
	CS_LOGIN_PACKET p{};
	p.size = sizeof(CS_LOGIN_PACKET);
	p.type = CS_LOGIN;
	send(g_socket, reinterpret_cast<const char*>(&p), sizeof(p), 0);
}

void send_move_packet(float x, float y, float z, float cx, float cy)
{
	CS_MOVE_PACKET p{};
	p.size = sizeof(CS_MOVE_PACKET);
	p.type = CS_MOVE;
	p.x = x;
	p.y = y;
	p.z = z;
	p.cxDelta = cx;
	p.cyDelta = cy;
	send(g_socket, reinterpret_cast<const char*>(&p), sizeof(p), 0);
}

void WINAPI do_recv()
{
	int ret;

	ZeroMemory(recvBuf, BUF_SIZE);
	ret = recv(g_socket, recvBuf, BUF_SIZE, 0);
	//if (ret == SOCKET_ERROR) err_display("RECV()");
	char* ptr = recvBuf;

	while (ptr != NULL) {
		unsigned char size = *ptr;
		if (size <= 0) {
			break;
		}
		char type = *(ptr + 1);

		// ��Ŷ�� ������ ���� ��� ó���� ���ΰ�?
		// ex) bool start�� true�� �ٲ㼭 ������ �۵��ǰ� �Ѵ� 

		switch (type) {
		case SC_LOGIN: {	// ó�� �α��� ���� �� �޴� ��Ŷ. ���̵� ������ Ŭ�󿡰� ���̵� �ο��Ѵ�
			SC_LOGININFO_PACKET* packet = reinterpret_cast<SC_LOGININFO_PACKET*>(ptr);
			//cout << "�������� Ŭ��� ��ġ�� ����" << endl;
			// int id = packet->id
			id = packet->id;
			start_x = packet->x;
			start_y = packet->y;
			start_z = packet->z;
			break;
		}
		case SC_START: {	// ���� ���� ������ �޼��Ǹ�(6��) ������ ������. �ʱ� ���� ��ġ ����
			SC_START_PACKET* packet = reinterpret_cast<SC_START_PACKET*>(ptr);
			m_gameStart = true;
			//cout << "���� ��Ŷ ����" << endl;
			break;
		}
		case SC_MOVE_PLAYER: {
			SC_MOVE_PACKET* packet = reinterpret_cast<SC_MOVE_PACKET*>(ptr);
			//cout << packet->id << "�� ��ġ�� �޾ƿԽ��ϴ�." << packet->x << ", " << packet->y << ", " << packet->z << endl;
			otherPlayer_id = packet->id;
			otherPlayerPos.x = packet->x;
			otherPlayerPos.y = packet->y;
			otherPlayerPos.z = packet->z;
			otherPlayerMouse.cx = packet->cxDelta;
			otherPlayerMouse.cy = packet->cyDelta;

			break;
		}
		}
		ptr += size;
	}

}

bool GetGameState()
{
	return m_gameStart;
}

Pos GetStartPos()
{
	Pos p = { start_x, start_y, start_z - 20 };

	switch (id)
	{
	case 0:
		p = { start_x, start_y, start_z };
		break;
	case 1:
		p = { start_x, start_y, start_z + 50 };
		break;
	case 2:
		p = { start_x, start_y, start_z + 70 };
		break;
	}

	return p;
}

int GetPlayerId()
{
	int playerId = id;

	return playerId;
}

Pos GetOtherPlayerPos()
{
	return otherPlayerPos;
}

int GetOtherPlayerId()
{
	return otherPlayer_id;
}

Mouse GetOtherPlayerMouse()
{
	return otherPlayerMouse;
}

void NetCleanup()
{
	cout << "���� close" << endl;
	closesocket(g_socket);
	WSACleanup();
}

void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	MessageBoxA(NULL, (const char*)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// ���� �Լ� ���� ���
void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[%s] %s\n", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// ���� �Լ� ���� ���
void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[����] %s\n", (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}