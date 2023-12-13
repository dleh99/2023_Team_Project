#include "Physics.h"

XMFLOAT3 Physics::PickPos()
{
    XMFLOAT3 pos;

    int random = rand() % 6;

    switch (random)
    {
    case 0: {
        pos = { -60.f, 50.f, -60.f };
        break;
    }
    case 1: {
        pos = { -60.f, 50.f, 20.f };
        break;
    }
    case 2: {
        pos = { 20.f, 50.f, -60.f };
        break;
    }
    case 3: {
        pos = { 20.f, 50.f, 20.f };
        break;
    }
    case 4: {
        pos = { 0.f, 50.f, 0.f };
        break;
    }
    case 5: {
        pos = { -20.f, 50.f, 0.f };
        break;
    }
    }

    return pos;
}
