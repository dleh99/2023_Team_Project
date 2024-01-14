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

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

struct XMFLOAT3 {
	float x;
	float y;
	float z;
};