#include "Network.h"

WSADATA wsa;
SOCKET g_socket;
SOCKADDR_IN serveraddr;
char recvBuf[BUF_SIZE];

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

	return ret;
}

//void send_packet(void* packet)
//{
//	unsigned char* p = reinterpret_cast<unsigned char*>(packet);
//	size_t sent = 0;
//	send(g_socket, p, sizeof(p), 0);
//}

void send_keyboard_packet(int direction)
{
	CS_MOVE_PACKET p{};
	p.size = sizeof(CS_MOVE_PACKET);
	p.type = CS_MOVE;
	p.direction = direction;
	send(g_socket, reinterpret_cast<const char*>(&p), sizeof(p), 0);
}

DWORD WINAPI do_recv()
{
	int ret;

	while (true) {
		ZeroMemory(recvBuf, BUF_SIZE);
		ret = recv(g_socket, recvBuf, BUF_SIZE, 0);
		if (ret == SOCKET_ERROR) err_display("RECV()");
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

				// int id = packet->id
				break;
			}
			case SC_START: {	// 게임 시작 조건이 달성되면(8명) 게임을 시작함. 사람 초기 위치, 초기 지형 위치 보냄
				SC_START_PACKET* packet = reinterpret_cast<SC_START_PACKET*>(ptr);
				break;
			}
			}
			ptr += size;
		}
	}
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