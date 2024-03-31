/*
    THIS FILE IS A PART OF RDR 2 SCRIPT HOOK SDK
                http://dev-c.com
            (C) Alexander Blade 2019
*/

#include <Windows.h>

const int KEYS_SIZE = 256; // Changed to 256 to include all possible keys

struct KeyState {
    DWORD time;
    BOOL isWithAlt;
    BOOL wasDownBefore;
    BOOL isUpNow;
};

KeyState keyStates[KEYS_SIZE];

void OnKeyboardMessage(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow)
{
    if (key < KEYS_SIZE)
    {
        keyStates[key].time = GetTickCount64();
        keyStates[key].isWithAlt = isWithAlt;
        keyStates[key].wasDownBefore = wasDownBefore;
        keyStates[key].isUpNow = isUpNow;
    }
}

const int NOW_PERIOD = 100, MAX_DOWN = 5000, MAX_DOWN_LONG = 30000; // ms

bool IsKeyDown(DWORD key)
{
    if (key < 0 || key >= KEYS_SIZE)
        return false;

    return (GetTickCount64() < keyStates[key].time + MAX_DOWN) && !keyStates[key].isUpNow;
}

bool IsKeyDownLong(DWORD key)
{
    if (key < 0 || key >= KEYS_SIZE)
        return false;

    return (GetTickCount64() < keyStates[key].time + MAX_DOWN_LONG) && !keyStates[key].isUpNow;
}

void ResetKeyState(DWORD key)
{
    if (key >= 0 && key < KEYS_SIZE)
        memset(&keyStates[key], 0, sizeof(KeyState));
}


bool IsKeyJustUp(DWORD key, bool exclusive)
{
    if (key < 0 || key >= KEYS_SIZE)
        return false;

    bool isJustUp = (GetTickCount64() < keyStates[key].time + NOW_PERIOD) && keyStates[key].isUpNow;
    if (isJustUp && exclusive)
        ResetKeyState(key);

    return isJustUp;
}