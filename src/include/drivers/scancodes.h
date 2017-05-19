#define KB_L_CTRL 0x80
#define KB_L_SHIFT 0x81
#define KB_R_SHIFT 0x82
#define KB_L_ALT 0x83
#define KB_CAPS 0x84
#define KB_F1 0x85
#define KB_F2 0x86
#define KB_F3 0x87
#define KB_F4 0x88
#define KB_F5 0x89
#define KB_F6 0x8A
#define KB_F7 0x8B
#define KB_F8 0x8C
#define KB_F9 0x8D
#define KB_F10 0x8E
#define KB_F11 0x91
#define KB_F12 0x92
#define KB_NUMLOCK 0x8F
#define KB_SCROLLOCK 0x90

int kb_us[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', KB_L_CTRL,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', KB_L_SHIFT, 0,
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', KB_R_SHIFT, '*', KB_L_ALT, ' ', KB_CAPS,
    KB_F1, KB_F2, KB_F3, KB_F4, KB_F5, KB_F6, KB_F7, KB_F8, KB_F9, KB_F10,
    KB_NUMLOCK, KB_SCROLLOCK, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.',
    KB_F11, KB_F12
};

int kb_us_s[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', KB_L_CTRL,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', KB_L_SHIFT, 0,
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', KB_R_SHIFT, '*', KB_L_ALT, ' ', KB_CAPS,
    KB_F1, KB_F2, KB_F3, KB_F4, KB_F5, KB_F6, KB_F7, KB_F8, KB_F9, KB_F10,
    KB_NUMLOCK, KB_SCROLLOCK, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.',
    KB_F11, KB_F12
};
