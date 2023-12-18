#include "Network.h"
#include "Scene.h"

WSADATA wsa;
SOCKET g_socket;
SOCKADDR_IN serveraddr;
char recvBuf[BUF_SIZE];

//string SERVER_IP = "127.0.0.1";
//string SERVER_IP = "14.51.115.70";
string SERVER_IP;

float start_x, start_y, start_z;
int id;

float otherPlayer_x, otherPlayer_y, otherPlayer_z;
Pos otherPlayerPos[3];
Mouse otherPlayerMouse[3];
Animation otherPlayerAni[3];
CScene* NetScene = NULL;
vector<CMainPlayer*> Netplayers;

long long game_frame;

// ���� ���� ����
bool m_gameStart = false;
char m_mapKey;

int NetworkInit()
{
	int ret;

	/*cout << "���� IP �ּҸ� �Է����ּ��� : ";
	cin >> SERVER_IP;*/

	const char* filePath = "IP.txt";

	ifstream inputFile(filePath);

	if (!inputFile.is_open()) {
		cerr << "������ �� �� �����ϴ�" << endl;
		return 1;
	}

	getline(inputFile, SERVER_IP);

	inputFile.close();

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

void send_move_packet(float x, float y, float z, float cx, float cy, Animation animation_state)
{
	CS_MOVE_PACKET p{};
	p.size = sizeof(CS_MOVE_PACKET);
	p.type = CS_MOVE;
	p.x = x;
	p.y = y;
	p.z = z;
	p.cxDelta = cx;
	p.cyDelta = cy;
	p.animation_state = animation_state;
	send(g_socket, reinterpret_cast<const char*>(&p), sizeof(p), 0);
}

void send_bullet_add_packet(XMFLOAT3 pos, XMFLOAT3 bullet_v, int bullet_id)
{
	CS_BULLET_ADD_PACKET p{};
	p.size = sizeof(CS_BULLET_ADD_PACKET);
	p.type = CS_BULLET_ADD;
	p.s_x = pos.x;
	p.s_y = pos.y;
	p.s_z = pos.z;
	p.b_x = bullet_v.x;
	p.b_y = bullet_v.y;
	p.b_z = bullet_v.z;
	p.bullet_id = bullet_id;
	send(g_socket, reinterpret_cast<const char*>(&p), sizeof(p), 0);
}

void send_fall_packet()
{
	CS_FALL_PACKET p{};
	p.size = sizeof(CS_FALL_PACKET);
	p.type = CS_FALL;
	send(g_socket, reinterpret_cast<const char*>(&p), sizeof(p), 0);
}

void send_score_packet(int score)
{
	CS_SCORE_PACKET p{};
	p.size = sizeof(CS_SCORE_PACKET);
	p.type = CS_SCORE;
	p.score = score;
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
			cout << packet->id << endl;
			start_x = packet->x;
			start_y = packet->y;
			start_z = packet->z;
			break;
		}
		case SC_START: {	// ���� ���� ������ �޼��Ǹ�(6��) ������ ������. �ʱ� ���� ��ġ ����
			SC_START_PACKET* packet = reinterpret_cast<SC_START_PACKET*>(ptr);
			m_gameStart = true;
			m_mapKey = packet->map_key;
			//cout << "���� ��Ŷ ����" << endl;
			break;
		}
		case SC_MOVE_PLAYER: {
			SC_MOVE_PACKET* packet = reinterpret_cast<SC_MOVE_PACKET*>(ptr);
			//cout << packet->id << "�� ��ġ�� �޾ƿԽ��ϴ�." << packet->x << ", " << packet->y << ", " << packet->z << endl;
			/*otherPlayer_id = packet->id;
			otherPlayerPos.x = packet->x;
			otherPlayerPos.y = packet->y;
			otherPlayerPos.z = packet->z;
			otherPlayerMouse.cx = packet->cxDelta;
			otherPlayerMouse.cy = packet->cyDelta;*/
			int p_id = packet->id;
			if (p_id > 2 || p_id < 0) break;
			otherPlayerPos[p_id].x = packet->x;
			otherPlayerPos[p_id].y = packet->y;
			otherPlayerPos[p_id].z = packet->z;
			otherPlayerMouse[p_id].cx = packet->cxDelta;
			otherPlayerMouse[p_id].cy = packet->cyDelta;
			otherPlayerAni[p_id] = packet->animation_state;
			//cout << "[" << p_id << "] " << packet->animation_state << endl;
			//cout << "[" << packet->id << "] ù ��� ������ : " << packet->first_frame_num << ", ���� �ð� : " << packet->server_time << ", ���� ������ : " << game_frame << endl;
			break;
		}
		case SC_BULLET_ADD: {
			SC_BULLET_ADD_PACKET* packet = reinterpret_cast<SC_BULLET_ADD_PACKET*>(ptr);
			XMFLOAT3 BPos = { packet->s_x ,packet->s_y ,packet->s_z };
			XMFLOAT3 BVec = { packet->b_x ,packet->b_y ,packet->b_z };

			NetScene->AddObjects(0, BPos, BVec, packet->player_id, packet->bullet_id);
			//cout << "�Ѿ��� �޾� �Խ��ϴ�. ��ġ :" << packet->s_x << ", " << packet->s_y << ", " << packet->s_z << ", �߻� ���� : " << packet->b_x << ", " << packet->b_y << ", " << packet->b_z << endl;
			//cout << packet->player_id << "�� �Ѿ��� �޾ƿԽ��ϴ�" << endl;
			break;
		}
		case SC_COLLISION: {
			SC_COLLISION_PACKET* packet = reinterpret_cast<SC_COLLISION_PACKET*>(ptr);

			break;
		}
		case SC_BULLET_COLLISION: {
			SC_BULLET_COLLISION_PACKET* packet = reinterpret_cast<SC_BULLET_COLLISION_PACKET*>(ptr);
			//cout << "�Ѿ� ��ȣ : " << packet->bullet_id << ", ��� ��ȣ : " << packet->block_id << ", �Ѿ� ���� : " << packet->player_id << endl;;
			NetScene->DisableObject(packet->bullet_id, packet->block_id, packet->player_id);
	
			if (id == packet->player_id) {
				int UpdatedSocre = Netplayers[id]->GetPlayerScore() + 100;
				Netplayers[id]->SetPlayerScore(UpdatedSocre);
			}
			break;
		}
		case SC_HIT: {
			SC_HIT_PACKET* packet = reinterpret_cast<SC_HIT_PACKET*>(ptr);
			// �¾��� �� ó��
			//cout << packet->bullet_id << ", " << packet->player_id << endl;
			NetScene->DisableBullet(packet->bullet_id, packet->player_id);

			if (id == packet->enemy_id) {
				int UpdatedHP = Netplayers[id]->GetPlayerHP() - 10;
				Netplayers[id]->SetPlayerHP(UpdatedHP);
			} 
			break;
		}
		case SC_DEATH: {
			SC_DEATH_PACKET* packet = reinterpret_cast<SC_DEATH_PACKET*>(ptr);
			cout << "�÷��̾� [" << packet->death_id << "]�� ����Ͽ����ϴ�." << endl;
			NetScene->DisableBullet(packet->bullet_id, packet->player_id);
			if (id == packet->death_id) {
				int UpdatedHP = Netplayers[id]->GetPlayerHP() - 10;
				Netplayers[id]->SetPlayerHP(UpdatedHP);
			}
			Netplayers[packet->death_id]->SetIsActive(false);
			Netplayers[packet->death_id]->SetDeath(true);
			break;
		}
		case SC_RESPAWN: {
			SC_RESPAWN_PACKET* packet = reinterpret_cast<SC_RESPAWN_PACKET*>(ptr);
			//cout << "�÷��̾� [" << packet->player_id << "] ��Ȱ." << endl;
			Netplayers[packet->player_id]->SetIsActive(true);
			Netplayers[packet->player_id]->SetDeath(false);
			Netplayers[packet->player_id]->SetPosition(XMFLOAT3(packet->respawn_x, packet->respawn_y, packet->respawn_z));
			Netplayers[packet->player_id]->SetPlayerHP(100);
			break;
		}
		case SC_FALL: {
			SC_FALL_PACKET* packet = reinterpret_cast<SC_FALL_PACKET*>(ptr);
			Netplayers[packet->fall_id]->SetDeath(true);
			break;
		}
		case SC_RESULT: {
			SC_RESULT_PACKET* packet = reinterpret_cast<SC_RESULT_PACKET*>(ptr);
			if (true == packet->result) cout << "�̰��" << endl;
			else cout << "����" << endl;
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

char GetMapKey()
{
	return m_mapKey;
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

int GetNetworkPlayerId()
{
	int playerId = id;

	return playerId;
}

Animation GetOtherAni(int id)
{
	return otherPlayerAni[id];
}

Pos GetOtherPlayerPos(int id)
{
	return otherPlayerPos[id];
}

Mouse GetOtherPlayerMouse(int id)
{
	return otherPlayerMouse[id];
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

void SetScene(CScene* Scene) {
	NetScene = Scene;
}

void SetPlayers(vector<CMainPlayer*> players)
{
	Netplayers = players;
}

void SetFrame(long long input)
{
	game_frame = input;
}