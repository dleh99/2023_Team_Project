#include "Network.h"
#include "Scene.h"

WSADATA wsa;
SOCKET g_socket;
SOCKADDR_IN serveraddr;
//char recvBuf[BUF_SIZE];

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
CCamera* GameCamera = NULL;
CMainPlayer* pGamePlayer = NULL;

long long game_frame;

// 게임 시작 변수
bool m_gameStart = false;
bool gameResult = false;
char m_mapKey;

// 게임 모드
int gameMode = 0;		// 0: Survival Mode, 1: RPG Mode
void SetGameMode(int mode) { gameMode = mode; }
int GetGameMode() { return gameMode; }

int NetworkInit()
{
	int ret;

	/*cout << "서버 IP 주소를 입력해주세요 : ";
	cin >> SERVER_IP;*/

	const char* filePath = "IP.txt";

	ifstream inputFile(filePath);

	if (!inputFile.is_open()) {
		cerr << "파일을 열 수 없습니다" << endl;
		return 1;
	}

	getline(inputFile, SERVER_IP);

	inputFile.close();

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
	//send_login_packet();
	cout << "네트워크 연결" << endl;

	return ret;
}

//void send_packet(void* packet)
//{
//	unsigned char* p = reinterpret_cast<unsigned char*>(packet);
//	size_t sent = 0;
//	send(g_socket, p, sizeof(p), 0);
//}

void send_login_packet(wstring i_id, wstring i_password, int i_room)
{
	CS_LOGIN_PACKET p{};
	p.size = sizeof(CS_LOGIN_PACKET);
	p.type = CS_LOGIN;
	memcpy(p.id, i_id.c_str(), sizeof(p.id));
	memcpy(p.password, i_password.c_str(), sizeof(p.password));
	p.room_num = i_room;
	//wcout << L"아이디 : " << i_id << L", 비밀번호 : " << i_password << L", 방 번호 : " << i_room << endl;
	wcout << i_id << ", " << i_password << ", " << i_room << endl;
	cout << "데이터 크기 : " << p.size << endl;
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

void ProcessPacket(char* ptr)
{
	switch (ptr[1]) {
	case SC_LOGIN_FAIL: {
		SC_LOGIN_FAIL_PACKET* packet = reinterpret_cast<SC_LOGIN_FAIL_PACKET*>(ptr);
		cout << "실패 패킷이 옴" << endl;
		if (packet->login_state == LS_OUTOFROOM) {
			cout << "룸 번호는 0이상, 332 이하여야 합니다" << endl;
		}
		else if (packet->login_state == LS_FULLROOM) {
			cout << "선택하신 룸에서 게임이 진행중입니다" << endl;
		}
		else if (packet->login_state == LS_LOGIN_FAIL) {
			cout << "입력하신 ID 또는 비밀번호가 틀렸습니다" << endl;
		}
		else if (packet->login_state == LS_ALREADY_INGAME) {
			cout << "입력하신 ID의 계정이 이미 게임 실행 중입니다" << endl;
		}
		NetScene->m_TitleError = packet->login_state;

		break;
	}
	case SC_LOGIN_SUCCESS: {
		SC_LOGIN_SUCCESS_PACKET* packet = reinterpret_cast<SC_LOGIN_SUCCESS_PACKET*>(ptr);
		if (packet->login_state == LS_SIGNUP) {
			cout << "새로 입력하신 ID. 자동으로 회완가입 되어 게임에 진입합니다" << endl;
		}
		else if (packet->login_state == LS_LOGIN_SUCCESS) {
			cout << "로그인 성공" << endl;
		}
		NetScene->m_TitleError = packet->login_state;
		break;
	}
	case SC_LOGIN: {	// 처음 로그인 했을 때 받는 패킷. 아이디를 서버는 클라에게 아이디를 부여한다
		SC_LOGININFO_PACKET* packet = reinterpret_cast<SC_LOGININFO_PACKET*>(ptr);
		cout << "Login 패킷" << endl;
		// int id = packet->id
		id = packet->id;
		cout << packet->id << endl;
		start_x = packet->x;
		start_y = packet->y;
		start_z = packet->z;
		break;
	}
	case SC_START: {	// 게임 시작 조건이 달성되면(6명) 게임을 시작함. 초기 지형 위치 보냄
		SC_START_PACKET* packet = reinterpret_cast<SC_START_PACKET*>(ptr);
		m_gameStart = true;
		m_mapKey = packet->map_key;
		NetScene->AddBlocksByMapData(0, m_mapKey,false);
		id = packet->player_id;

		XMFLOAT3 pos = { packet->start_x ,packet->start_y ,packet->start_z };
		Netplayers[id]->SetPosition(pos);

		NetScene->m_SceneState = 1;
		cout << "시작 패킷 받음" << endl;
		break;
	}
	case SC_MOVE_PLAYER: {
		SC_MOVE_PACKET* packet = reinterpret_cast<SC_MOVE_PACKET*>(ptr);
		//cout << packet->id << "의 위치를 받아왔습니다." << packet->x << ", " << packet->y << ", " << packet->z << endl;
		/*otherPlayer_id = packet->id;
		otherPlayerPos.x = packet->x;
		otherPlayerPos.y = packet->y;
		otherPlayerPos.z = packet->z;
		otherPlayerMouse.cx = packet->cxDelta;
		otherPlayerMouse.cy = packet->cyDelta;*/
		int p_id = packet->id;
		if (p_id > 2 || p_id < 0) {
			cout << "이상한 id가 들어왔습니다 : " << p_id << ", x : " << packet->x << ", y : " << packet->y << ", z : " << packet->z << endl;
			break;
		}
		otherPlayerPos[p_id].x = packet->x;
		otherPlayerPos[p_id].y = packet->y;
		otherPlayerPos[p_id].z = packet->z;
		otherPlayerMouse[p_id].cx = packet->cxDelta;
		otherPlayerMouse[p_id].cy = packet->cyDelta;
		otherPlayerAni[p_id] = packet->animation_state;
		//cout << "[" << p_id << "] " << packet->animation_state << endl;
		//cout << "[" << packet->id << "] 첫 명령 프레임 : " << packet->first_frame_num << ", 서버 시간 : " << packet->server_time << ", 현재 프레임 : " << game_frame << endl;
		break;
	}
	case SC_BULLET_ADD: {
		SC_BULLET_ADD_PACKET* packet = reinterpret_cast<SC_BULLET_ADD_PACKET*>(ptr);
		XMFLOAT3 BPos = { packet->s_x ,packet->s_y ,packet->s_z };
		XMFLOAT3 BVec = { packet->b_x ,packet->b_y ,packet->b_z };

		NetScene->AddObjects(0, BPos, BVec, packet->player_id, packet->bullet_id);
		//cout << "총알을 받아 왔습니다. 위치 :" << packet->s_x << ", " << packet->s_y << ", " << packet->s_z << ", 발사 벡터 : " << packet->b_x << ", " << packet->b_y << ", " << packet->b_z << endl;
		cout << packet->player_id << "의 " << packet->bullet_id << "번 총알을 받아왔습니다" << endl;
		break;
	}
	case SC_COLLISION: {
		SC_COLLISION_PACKET* packet = reinterpret_cast<SC_COLLISION_PACKET*>(ptr);

		break;
	}
	case SC_BULLET_COLLISION: {
		SC_BULLET_COLLISION_PACKET* packet = reinterpret_cast<SC_BULLET_COLLISION_PACKET*>(ptr);
		//cout << "총알 번호 : " << packet->bullet_id << ", 블록 번호 : " << packet->block_id << ", 총알 주인 : " << packet->player_id << endl;;
		NetScene->DisableObject(packet->bullet_id, packet->block_id, packet->player_id);

		if (id == packet->player_id) {
			if (gameMode == 1)
				Netplayers[id]->IncreasePlayerBlockMoney();

			int UpdatedSocre = Netplayers[id]->GetPlayerScore() + 100;
			Netplayers[id]->SetPlayerScore(UpdatedSocre);
		}
		break;
	}
	case SC_HIT: {
		SC_HIT_PACKET* packet = reinterpret_cast<SC_HIT_PACKET*>(ptr);
		// 맞았을 때 처리
		//cout << packet->bullet_id << ", " << packet->player_id << endl;
		NetScene->DisableBullet(packet->bullet_id, packet->player_id);

		cout << packet->player_id << "가 " << packet->enemy_id << "를 " << packet->bullet_id << "번 총알로 맞춤" << endl;
		if (id == packet->enemy_id) {
			int UpdatedHP = Netplayers[id]->GetPlayerHP() - 10;
			Netplayers[id]->SetPlayerHP(UpdatedHP);
		}
		break;
	}
	case SC_DEATH: {
		SC_DEATH_PACKET* packet = reinterpret_cast<SC_DEATH_PACKET*>(ptr);
		cout << "플레이어 [" << packet->death_id << "]가 사망하였습니다." << endl;
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
		cout << "플레이어 [" << packet->player_id << "] 부활." << endl;
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
		gameResult = packet->result;
		if (true == packet->result) cout << "이겼다" << endl;
		else cout << "졌다" << endl;
		break;
	}
	default:
		cout << "Unknown Packet type " << ptr[1] << endl;
	}
}

void WINAPI do_recv()
{
	int ret;
	char recvBuf[BUF_SIZE];

	//ZeroMemory(recvBuf, BUF_SIZE);
	ret = recv(g_socket, recvBuf, BUF_SIZE, 0);
	//if (ret == SOCKET_ERROR) err_display("RECV()");
	char* ptr = recvBuf;
	//unsigned char size = *ptr;
	//size_t num_size = size;
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	if (ret > 0)
	{
		while (0 != ret) {
			if (0 == in_packet_size) in_packet_size = ptr[0];
			if (ret + saved_packet_size >= in_packet_size) {
				memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
				ProcessPacket(packet_buffer);
				ptr += in_packet_size - saved_packet_size;
				ret -= in_packet_size - saved_packet_size;
				in_packet_size = 0;
				saved_packet_size = 0;
			}
			else {
				memcpy(packet_buffer + saved_packet_size, ptr, ret);
				saved_packet_size += ret;
				ret = 0;
			}
		}
	}
	{
		//while (ptr != NULL) {
		//	unsigned char size = *ptr;
		//	if (size <= 0) {
		//		break;
		//	}
		//	char type = *(ptr + 1);

		//	 //패킷의 종류에 따라 어떻게 처리할 것인가?
		//	 //ex) bool start를 true로 바꿔서 게임이 작동되게 한다 


		//		switch (type) {
		//		case SC_LOGIN: {	// 처음 로그인 했을 때 받는 패킷. 아이디를 서버는 클라에게 아이디를 부여한다
		//			SC_LOGININFO_PACKET* packet = reinterpret_cast<SC_LOGININFO_PACKET*>(ptr);
		//			cout << "Login 패킷" << endl;
		//			// int id = packet->id
		//			id = packet->id;
		//			cout << packet->id << endl;
		//			start_x = packet->x;
		//			start_y = packet->y;
		//			start_z = packet->z;
		//			break;
		//		}
		//		case SC_START: {	// 게임 시작 조건이 달성되면(6명) 게임을 시작함. 초기 지형 위치 보냄
		//			SC_START_PACKET* packet = reinterpret_cast<SC_START_PACKET*>(ptr);
		//			m_gameStart = true;
		//			m_mapKey = packet->map_key;
		//			cout << "시작 패킷 받음" << endl;
		//			break;
		//		}
		//		case SC_MOVE_PLAYER: {
		//			SC_MOVE_PACKET* packet = reinterpret_cast<SC_MOVE_PACKET*>(ptr);
		//			//cout << packet->id << "의 위치를 받아왔습니다." << packet->x << ", " << packet->y << ", " << packet->z << endl;
		//			/*otherPlayer_id = packet->id;
		//			otherPlayerPos.x = packet->x;
		//			otherPlayerPos.y = packet->y;
		//			otherPlayerPos.z = packet->z;
		//			otherPlayerMouse.cx = packet->cxDelta;
		//			otherPlayerMouse.cy = packet->cyDelta;*/
		//			int p_id = packet->id;
		//			if (p_id > 2 || p_id < 0) {
		//				cout << "이상한 id가 들어왔습니다 : " << p_id << ", x : " << packet->x << ", y : " << packet->y << ", z : " << packet->z << endl;
		//				break;
		//			}
		//			otherPlayerPos[p_id].x = packet->x;
		//			otherPlayerPos[p_id].y = packet->y;
		//			otherPlayerPos[p_id].z = packet->z;
		//			otherPlayerMouse[p_id].cx = packet->cxDelta;
		//			otherPlayerMouse[p_id].cy = packet->cyDelta;
		//			otherPlayerAni[p_id] = packet->animation_state;
		//			//cout << "[" << p_id << "] " << packet->animation_state << endl;
		//			//cout << "[" << packet->id << "] 첫 명령 프레임 : " << packet->first_frame_num << ", 서버 시간 : " << packet->server_time << ", 현재 프레임 : " << game_frame << endl;
		//			break;
		//		}
		//		case SC_BULLET_ADD: {
		//			SC_BULLET_ADD_PACKET* packet = reinterpret_cast<SC_BULLET_ADD_PACKET*>(ptr);
		//			XMFLOAT3 BPos = { packet->s_x ,packet->s_y ,packet->s_z };
		//			XMFLOAT3 BVec = { packet->b_x ,packet->b_y ,packet->b_z };

		//			NetScene->AddObjects(0, BPos, BVec, packet->player_id, packet->bullet_id);
		//			//cout << "총알을 받아 왔습니다. 위치 :" << packet->s_x << ", " << packet->s_y << ", " << packet->s_z << ", 발사 벡터 : " << packet->b_x << ", " << packet->b_y << ", " << packet->b_z << endl;
		//			//cout << packet->player_id << "의 총알을 받아왔습니다" << endl;
		//			break;
		//		}
		//		case SC_COLLISION: {
		//			SC_COLLISION_PACKET* packet = reinterpret_cast<SC_COLLISION_PACKET*>(ptr);

		//			break;
		//		}
		//		case SC_BULLET_COLLISION: {
		//			SC_BULLET_COLLISION_PACKET* packet = reinterpret_cast<SC_BULLET_COLLISION_PACKET*>(ptr);
		//			//cout << "총알 번호 : " << packet->bullet_id << ", 블록 번호 : " << packet->block_id << ", 총알 주인 : " << packet->player_id << endl;;
		//			NetScene->DisableObject(packet->bullet_id, packet->block_id, packet->player_id);

		//			if (id == packet->player_id) {
		//				int UpdatedSocre = Netplayers[id]->GetPlayerScore() + 100;
		//				Netplayers[id]->SetPlayerScore(UpdatedSocre);
		//			}
		//			break;
		//		}
		//		case SC_HIT: {
		//			SC_HIT_PACKET* packet = reinterpret_cast<SC_HIT_PACKET*>(ptr);
		//			// 맞았을 때 처리
		//			//cout << packet->bullet_id << ", " << packet->player_id << endl;
		//			NetScene->DisableBullet(packet->bullet_id, packet->player_id);

		//			if (id == packet->enemy_id) {
		//				int UpdatedHP = Netplayers[id]->GetPlayerHP() - 10;
		//				Netplayers[id]->SetPlayerHP(UpdatedHP);
		//			} 
		//			break;
		//		}
		//		case SC_DEATH: {
		//			SC_DEATH_PACKET* packet = reinterpret_cast<SC_DEATH_PACKET*>(ptr);
		//			cout << "플레이어 [" << packet->death_id << "]가 사망하였습니다." << endl;
		//			NetScene->DisableBullet(packet->bullet_id, packet->player_id);
		//			if (id == packet->death_id) {
		//				int UpdatedHP = Netplayers[id]->GetPlayerHP() - 10;
		//				Netplayers[id]->SetPlayerHP(UpdatedHP);
		//			}
		//			Netplayers[packet->death_id]->SetIsActive(false);
		//			Netplayers[packet->death_id]->SetDeath(true);
		//			break;
		//		}
		//		case SC_RESPAWN: {
		//			SC_RESPAWN_PACKET* packet = reinterpret_cast<SC_RESPAWN_PACKET*>(ptr);
		//			cout << "플레이어 [" << packet->player_id << "] 부활." << endl;
		//			Netplayers[packet->player_id]->SetIsActive(true);
		//			Netplayers[packet->player_id]->SetDeath(false);
		//			Netplayers[packet->player_id]->SetPosition(XMFLOAT3(packet->respawn_x, packet->respawn_y, packet->respawn_z));
		//			Netplayers[packet->player_id]->SetPlayerHP(100);
		//			break;
		//		}
		//		case SC_FALL: {
		//			SC_FALL_PACKET* packet = reinterpret_cast<SC_FALL_PACKET*>(ptr);
		//			Netplayers[packet->fall_id]->SetDeath(true);
		//			break;
		//		}
		//		case SC_RESULT: {
		//			SC_RESULT_PACKET* packet = reinterpret_cast<SC_RESULT_PACKET*>(ptr);
		//			gameResult = packet->result;
		//			if (true == packet->result) cout << "이겼다" << endl;
		//			else cout << "졌다" << endl;
		//			break;
		//		}
		//		}
		//		ptr += size;

		//}
	}
}

bool GetGameState()
{
	return m_gameStart;
}

bool GetGameResult() {
	return gameResult;
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
	cout << "소켓 close" << endl;
	closesocket(g_socket);
	WSACleanup();
}

void err_quit(const char* msg)
{
	LPVOID lpMsgBuf = NULL;
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
	LPVOID lpMsgBuf = NULL;
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
	LPVOID lpMsgBuf = NULL;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[오류] %s\n", (char*)lpMsgBuf);
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

void SetCamera(CCamera* pCamera)
{
	GameCamera = pCamera;
}

void SetGamePlayer(CMainPlayer* &pPlayer)
{
	pGamePlayer = pPlayer;
}