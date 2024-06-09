#pragma once

#include <iostream>
#include "..\..\Block_Crusher_Server\protocol.h"
#include <ws2tcpip.h> 
#include <winsock2.h> 
#include "stdafx.h"
#include "Scene.h"
#include "Player.h"
#include <fstream>
#pragma comment(lib, "ws2_32")

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
void send_login_packet(wstring i_id, wstring i_password);
void send_crush_match_packet();
void send_rpg_match_packet();
void send_move_packet(float x, float y, float z, float cx, float cy, Animation dwDirection);
void send_bullet_add_packet(XMFLOAT3 pos, XMFLOAT3 vec, int bullet_id);
void send_fall_packet();
void send_score_packet(int score);
void send_upgrade_packet(UPGRADE_OPTION in_op);
void err_quit(const char* msg);
void err_display(const char* msg);
void err_display(int errcode);
void ProcessPacket(char* ptr);
void WINAPI do_recv();

char GetMapKey();
bool GetGameState();
bool GetGameResult();
Pos GetStartPos();
int GetNetworkPlayerId();

Animation GetOtherAni(int id);
Pos GetOtherPlayerPos(int id);
Mouse GetOtherPlayerMouse(int id);
void SetScene(CScene* Scene);
void SetPlayers(vector<CMainPlayer*> players);
void SetFrame(long long input);
void SetCamera(CCamera* pCamera);
void SetGamePlayer(CMainPlayer* &pPlayer);
