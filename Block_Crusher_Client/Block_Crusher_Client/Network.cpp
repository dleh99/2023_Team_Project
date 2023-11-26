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

// 게임 시작 변수
bool m_gameStart = false;

int NetworkInit()
{
	int ret;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		cout << "WSA START ERROR" << endl;
		return 1;
	}

	g_socket = socket(AF_INET, SOCK_STREAM, 0);				// AF_INET = IPv4 주소체계, SOCK_STREAM = TCP 소켓, 0 = 시스템이 알아서 프로토콜 지정
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

		// 패킷의 종류에 따라 어떻게 처리할 것인가?
		// ex) bool start를 true로 바꿔서 게임이 작동되게 한다 

		switch (type) {
		case SC_LOGIN: {	// 처음 로그인 했을 때 받는 패킷. 아이디를 서버는 클라에게 아이디를 부여한다
			SC_LOGININFO_PACKET* packet = reinterpret_cast<SC_LOGININFO_PACKET*>(ptr);
			//cout << "서버에서 클라로 위치를 보냄" << endl;
			// int id = packet->id
			id = packet->id;
			start_x = packet->x;
			start_y = packet->y;
			start_z = packet->z;
			break;
		}
		case SC_START: {	// 게임 시작 조건이 달성되면(6명) 게임을 시작함. 초기 지형 위치 보냄
			SC_START_PACKET* packet = reinterpret_cast<SC_START_PACKET*>(ptr);
			m_gameStart = true;
			//cout << "시작 패킷 받음" << endl;
			break;
		}
		case SC_MOVE_PLAYER: {
			SC_MOVE_PACKET* packet = reinterpret_cast<SC_MOVE_PACKET*>(ptr);
			//cout << packet->id << "의 위치를 받아왔습니다." << packet->x << ", " << packet->y << ", " << packet->z << endl;
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
	cout << "소켓 close" << endl;
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

// 소켓 함수 오류 출력
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

// 소켓 함수 오류 출력
void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[오류] %s\n", (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}