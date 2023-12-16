#pragma once

#include <iostream>
#include "protocol.h"
#include <ws2tcpip.h> 
#include <winsock2.h> 
#include "stdafx.h"
#pragma comment(lib, "ws2_32")
#include "Scene.h"
#include "Player.h"
using namespace std;

struct Pos {
	float x;
	float y;
	float z;
};

struct Mouse {
	float cx;
	float cy;
};

int NetworkInit();
void NetCleanup();
void send_login_packet();
void send_move_packet(float x, float y, float z, float cx, float cy);
void send_bullet_add_packet(XMFLOAT3 pos, XMFLOAT3 vec, int bullet_id);
void err_quit(const char* msg);
void err_display(const char* msg);
void err_display(int errcode);
void WINAPI do_recv();

char GetMapKey();
bool GetGameState();
Pos GetStartPos();
int GetNetworkPlayerId();

Pos GetOtherPlayerPos();
int GetOtherPlayerId();
Mouse GetOtherPlayerMouse();
void SetScene(CScene* Scene);
void SetPlayers(vector<CPlayer*> players);