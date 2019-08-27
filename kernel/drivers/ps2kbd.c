#include "devmgr.h"
#include "ps2.h"
#include "ps2kbd.h"
#include "pic.h"
#include "isr.h"
#include "ll_io.h"

const generic_device_t* kbd;

uint8_t scancodeList[8];
uint8_t scancodeLength = 0;

static uint8_t scancodesOneByte[] = 
{
    0xFF, F9_KEY, 0xFF, F5_KEY, 
    F3_KEY, F1_KEY, F2_KEY, F12_KEY, 
    0xFF, F10_KEY, F8_KEY, F6_KEY, 
    F4_KEY, TAB_KEY, BACK_TICK_KEY, 0xFF,
    0xFF, LEFT_ALT_KEY, LEFT_SHIFT_KEY, 0xFF, 
    LEFT_CTRL_KEY, Q_KEY, ONE_KEY, 0xFF,
    0xFF, 0xFF, Z_KEY, S_KEY,
    A_KEY, W_KEY, TWO_KEY, 0xFF, 
    0xFF, C_KEY, X_KEY, D_KEY,
    E_KEY, FOUR_KEY, THREE_KEY, 0xFF,
    0xFF, SPACE_KEY, V_KEY, F_KEY,
    T_KEY, R_KEY, FIVE_KEY, 0xFF,
    0xFF, N_KEY, B_KEY, H_KEY,
    G_KEY, Y_KEY, SIX_KEY, 0xFF,
    0xFF, 0xFF, M_KEY, J_KEY,
    U_KEY, SEVEN_KEY, EIGHT_KEY, 0xFF,
    0xFF, COMMA_KEY, K_KEY, I_KEY,
    O_KEY, ZERO_KEY, NINE_KEY, 0xFF,
    0xFF, PERIOD_KEY, SLASH_KEY, L_KEY,
    SEMICOLON_KEY, P_KEY, MINUS_KEY, 0xFF,
    0xFF, 0xFF, APOSTROPHE_KEY, 0xFF,
    LEFT_BRACKET_KEY, EQUAL_KEY, 0xFF, 0xFF,
    CAPS_LOCK_KEY, RIGHT_SHIFT_KEY, ENTER_KEY, RIGHT_BRACKET_KEY,
    0xFF, BACKSLASH_KEY, 0xFF, 0xFF,
    0xFF, INT1_KEY, 0xFF, 0xFF,
    0xFF, 0xFF, BACKSPACE_KEY, 0xFF,
    0xFF, NUMPAD1_KEY, 0xFF, NUMPAD4_KEY,
    NUMPAD7_KEY, 0xFF, 0xFF, 0xFF,
    NUMPAD0_KEY, NUMPAD_PERIOD_KEY, NUMPAD2_KEY, NUMPAD5_KEY,
    NUMPAD6_KEY, NUMPAD8_KEY, ESC_KEY, NUM_LOCK_KEY,
    F11_KEY, NUMPAD_PLUS_KEY, NUMPAD3_KEY, NUMPAD_MINUS_KEY,
    NUMPAD_ASTERISK_KEY, NUMPAD9_KEY, SCROLL_LOCK_KEY, 0xFF,
    0xFF, 0xFF, 0xFF, F7_KEY
};

//Two byte scancodes (preceded by 0xE0)
static uint8_t scancodesTwoBytes[] =
{
    0xFF, RIGHT_ALT_KEY, 0xFF, 0xFF, //10
    RIGHT_CTRL_KEY, 0xFF, 0xFF, 0xFF, //14
    0xFF, 0xFF, 0xFF, 0xFF, //18
    0xFF, 0xFF, 0xFF, 0xFF, //1C
    0xFF, 0xFF, 0xFF, 0xFF, //20
    0xFF, 0xFF, 0xFF, 0xFF, //24
    0xFF, 0xFF, 0xFF, 0xFF, //28
    0xFF, 0xFF, 0xFF, 0xFF, //2c
    0xFF, 0xFF, 0xFF, 0xFF, //30
    0xFF, 0xFF, 0xFF, 0xFF, //34
    0xFF, 0xFF, 0xFF, 0xFF, //38
    0xFF, 0xFF, 0xFF, 0xFF, //3c
    0xFF, 0xFF, 0xFF, 0xFF, //40
    0xFF, 0xFF, 0xFF, 0xFF, //44
    0xFF, 0xFF, NUMPAD_SLASH_KEY, 0xFF, //48
    0xFF, 0xFF, 0xFF, 0xFF, //4c
    0xFF, 0xFF, 0xFF, 0xFF, //50
    0xFF, 0xFF, 0xFF, 0xFF, //54
    0xFF, 0xFF, NUMPAD_ENTER_KEY, 0xFF, //58
    0xFF, 0xFF, 0xFF, 0xFF, //5C
    0xFF, 0xFF, 0xFF, 0xFF, //60
    0xFF, 0xFF, 0xFF, 0xFF, //64
    0xFF, END_KEY, 0xFF, LEFT_KEY, //68
    HOME_KEY, 0xFF, 0xFF, 0xFF, //6C
    INSERT_KEY, DELETE_KEY, DOWN_KEY, 0xFF, //70
    RIGHT_KEY, UP_KEY, 0xFF, 0xFF, //74
    0xFF, 0xFF, PAGE_DOWN_KEY, 0xFF, //7A
    0xFF, PAGE_UP_KEY, 0xFF, 0xFF //7D
};

static uint8_t keyState[128] = {0};

//Key code to ASCII
static uint8_t qwertyMapLowerCase[] = 
{
    '`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b' /* backspace */,
    0 /* tab */, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\\',
    0 /* caps lock */, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 0 /* enter */,
    0 /* left shift */, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0 /* right shift */,
    0 /* left ctrl */, 0 /* left win */, 0 /* left alt */, ' '
};

static uint8_t qwertyMapUpperCase[] = 
{
    '~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '|',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', 0,
    0, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    0, 0, 0, ' '
};


static inline uint8_t scancode1Valid(uint8_t code)
{
    if(code < sizeof(scancodesOneByte) && scancodesOneByte[code] != 0xFF)
        return 1;
    else
        return 0;
}

static inline uint8_t scancode1ValidMultibyte(uint8_t code)
{
    if(code < sizeof(scancodesTwoBytes) + 0x10 && scancodesTwoBytes[code - 0x10] != 0xFF)
        return 1;
    else
        return 0;
}

static uint8_t scancodeConvert()
{    
    if(scancodeLength == 1 && scancode1Valid(scancodeList[0]))
    {
        scancodeLength = 0;

        //Key pressed
        uint8_t key = scancodesOneByte[scancodeList[0]];
        keyState[key] = 1;
        return key;
    }
    else if(scancodeLength == 2)
    {
        if(scancodeList[0] == 0xF0 && scancode1Valid(scancodeList[1]))
        {
            scancodeLength = 0;

            //Key released
            uint8_t key = scancodesOneByte[scancodeList[1]];
            keyState[key] = 0;
            return 0xFF;
        }
        else if(scancodeList[0] == 0xE0 && scancode1ValidMultibyte(scancodeList[1]))
        {
            scancodeLength = 0;

            //Key pressed
            uint8_t key = scancodesTwoBytes[scancodeList[1] - 0x10];
            keyState[key] = 1;
            return key;
        }
    }
    else if(scancodeLength == 3)
    {
        if(scancodeList[0] == 0xE0 && scancodeList[1] == 0xF0 && scancode1ValidMultibyte(scancodeList[2]))
        {
            scancodeLength = 0;

            //Key released
            uint8_t key = scancodesTwoBytes[scancodeList[2] - 0x10];
            keyState[key] = 1;
            return 0xFF;
        }
    }
    else if(scancodeLength == 4)
    {
        //Print screen pressed
        if(scancodeList[0] == 0xE0 && scancodeList[1] == 0x12 && scancodeList[2] == 0xE0 && 
            scancodeList[3] == 0x7C)
        {
            scancodeLength = 0;

            keyState[PRINT_KEY] = 1;
            return PRINT_KEY;
        }
    }
    else if(scancodeLength == 6)
    {
        //Print screen released
        if(scancodeList[0] == 0xE0 && scancodeList[1] == 0xF0 && scancodeList[2] == 0x7C &&
            scancodeList[3] == 0xE0 && scancodeList[4] == 0xF0 && scancodeList[5] == 0x12)
        {
            scancodeLength = 0;

            keyState[PRINT_KEY] = 0;
            return 0xFF;
        }
    }
    else if(scancodeLength == 8)
    {
        //Pause pressed
        if(scancodeList[0] == 0xE1 && scancodeList[1] == 0x14 && scancodeList[2] == 0x77 &&
            scancodeList[3] == 0xE1 && scancodeList[4] == 0xF0 && scancodeList[5] == 0x14 &&
            scancodeList[6] == 0xF0 && scancodeList[7] == 0x77)
        {
            scancodeLength = 0;
            return PAUSE_KEY;
        }
    }

    return 0xFF;
}

static uint8_t convertToAscii(uint8_t keyCode)
{
    if(keyCode == INT1_KEY)
        return '<';

    if(keyCode < BACK_TICK_KEY || keyCode > SPACE_KEY)
        return 0;

    if(keyState[LEFT_SHIFT_KEY] == 1 || keyState[RIGHT_SHIFT_KEY] == 1)
    {
        return qwertyMapUpperCase[keyCode - BACK_TICK_KEY];
    }

    return qwertyMapLowerCase[keyCode - BACK_TICK_KEY];
}

uint8_t keyboard_buffer[128];
uint8_t buffer_length = 0;

static void ps2KbdHandler(const interrupt_frame_t* regs)
{
    scancodeList[scancodeLength++] = (uint32_t)inb(PS2_DATA_PORT);
    uint8_t keyCode = scancodeConvert();

    if(keyCode != 0xFF)
    {
        uint8_t c = convertToAscii(keyCode);
        if(c != 0)
        {
            if(c == '\b')
            {
                if(buffer_length > 0)
                {
                    buffer_length--;
                    puts("\b \b");
                }
            }
            else if(buffer_length < sizeof(keyboard_buffer))
            {
                keyboard_buffer[buffer_length++] = c;
                putchar(c);
            }
        }
            
    }
    picAck(regs->int_no - 32);
}

void ps2KbdInit()
{
    kbd = devmgrFind("ps2kbd0");
    if(kbd == NULL)
    {
        kbd = devmgrFind("ps2kbd1");
        if(kbd == NULL)
        {
            printf("No PS/2 keyboard attached");
            return;
        }
    }

    printf("\nFound keyboard: %s", kbd->name);

    uint8_t intLine = kbd->id == 1 ? 12 : 1;

    registerInterruptHandler(intLine + 32, ps2KbdHandler);

    ps2DeviceSendCmd(kbd->id, PS2_DEVCMD_ENABLE_SCANNING);

    ps2DeviceEnableInterrupts(kbd->id);
    picUnmask(intLine);
}