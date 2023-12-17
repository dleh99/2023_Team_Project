#pragma once

#include <iostream>
#include "protocol.h"
#include <ws2tcpip.h> 
#include <winsock2.h> 
#include "stdafx.h"
#pragma comment(lib, "ws2_32")
#include "Scene.h"
#include "Player.h"
#include <fstream>
#include "protocol.h"

using namespace std;

struct Pos {
	float x;
	float y;
	float z;
};

struct Mouse {
	float cx = 0;
	float cy = 0;
};

int NetworkInit();
void NetCleanup();
void send_login_packet();
void send_move_packet(float x, float y, float z, float cx, float cy, Animation dwDirection);
void send_bullet_add_packet(XMFLOAT3 pos, XMFLOAT3 vec, int bullet_id);
void send_fall_packet();
void err_quit(const char* msg);
void err_display(const char* msg);
void err_display(int errcode);
void WINAPI do_recv();

char GetMapKey();
bool GetGameState();
Pos GetStartPos();
int GetNetworkPlayerId();

Animation GetOtherAni(int id);
Pos GetOtherPlayerPos(int id);
Mouse GetOtherPlayerMouse(int id);
void SetScene(CScene* Scene);
void SetPlayers(vector<CMainPlayer*> players);
void SetFrame(long long input);
