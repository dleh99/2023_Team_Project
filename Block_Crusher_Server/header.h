#pragma once

#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <vector>
#include <thread>
#include <array>
#include <chrono>
#include <fstream>
#include <atomic>
#include <mutex>
#include <sqlext.h>
#include <concurrent_priority_queue.h>

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

struct XMFLOAT3 {
	float x;
	float y;
	float z;
};

struct Range_Pos {
	int x;
	int y;
	int z;
};