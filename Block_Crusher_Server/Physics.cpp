#include "Physics.h"

XMFLOAT3 Physics::PickPos()
{
    XMFLOAT3 pos;

    srand(static_cast<unsigned int>(time(nullptr)));
    int random = rand() % 9;

    switch (random)
    {
    case 0: {
        pos = { -100.f, 250.f, -80.f };
        break;
    }
    case 1: {
        pos = { -100.f, 250.f, -260.f };
        break;
    }
    case 2: {
        pos = { -100.f, 250.f, -440.f };
        break;
    }
    case 3: {
        pos = { -280.f, 250.f, -80.f };
        break;
    }
    case 4: {
        pos = { -280.f, 250.f, -260.f };
        break;
    }
    case 5: {
        pos = { -280.f, 250.f, -440.f };
        break;
    }
    case 6: {
        pos = { -460.f, 250.f, -80.f };
        break;
    }
    case 7: {
        pos = { -460.f, 250.f, -260.f };
        break;
    }
    case 8: {
        pos = { -460.f, 250.f, -440.f };
        break;
    }
    }

    return pos;
}
