#include "kb_scancode_tables.h"
#include <kernel/utils.h>

#define ESC 27
#define BACKSPACE 8
#define TAB 9
#define NEW_LINE 10

uint8_t kb_sc_ascii_lower[256];
uint8_t kb_sc_ascii_upper[256];
uint8_t kb_sc_ascii_2byte[256];

void kb_scancode_tables_init()
{
    memset(&kb_sc_ascii_lower, 0, 256);    
    memset(&kb_sc_ascii_upper, 0, 256);    
    memset(&kb_sc_ascii_2byte, 0, 256);

    kb_sc_ascii_lower[0x01] = ESC;
    kb_sc_ascii_lower[0x02] = '1';    
    kb_sc_ascii_lower[0x03] = '2';    
    kb_sc_ascii_lower[0x04] = '3';    
    kb_sc_ascii_lower[0x05] = '4';    
    kb_sc_ascii_lower[0x06] = '5';    
    kb_sc_ascii_lower[0x07] = '6';    
    kb_sc_ascii_lower[0x08] = '7';    
    kb_sc_ascii_lower[0x09] = '8';    
    kb_sc_ascii_lower[0x0A] = '9';
    kb_sc_ascii_lower[0x0B] = '0';
    kb_sc_ascii_lower[0x0C] = '-';
    kb_sc_ascii_lower[0x0D] = '=';
    kb_sc_ascii_lower[0x0E] = BACKSPACE;
    kb_sc_ascii_lower[0x0F] = TAB;
    kb_sc_ascii_lower[0x10] = 'q';
    kb_sc_ascii_lower[0x11] = 'w';
    kb_sc_ascii_lower[0x12] = 'e';
    kb_sc_ascii_lower[0x13] = 'r';
    kb_sc_ascii_lower[0x14] = 't';
    kb_sc_ascii_lower[0x15] = 'y';
    kb_sc_ascii_lower[0x16] = 'u';
    kb_sc_ascii_lower[0x17] = 'i';
    kb_sc_ascii_lower[0x18] = 'o';
    kb_sc_ascii_lower[0x19] = 'p';
    kb_sc_ascii_lower[0x1A] = '[';
    kb_sc_ascii_lower[0x1B] = ']';
    kb_sc_ascii_lower[0x1C] = NEW_LINE;
    kb_sc_ascii_lower[0x1E] = 'a';
    kb_sc_ascii_lower[0x1F] = 's';
    kb_sc_ascii_lower[0x20] = 'd';
    kb_sc_ascii_lower[0x21] = 'f';
    kb_sc_ascii_lower[0x22] = 'g';
    kb_sc_ascii_lower[0x23] = 'h';
    kb_sc_ascii_lower[0x24] = 'j';
    kb_sc_ascii_lower[0x25] = 'k';
    kb_sc_ascii_lower[0x26] = 'l';
    kb_sc_ascii_lower[0x27] = ';';
    kb_sc_ascii_lower[0x28] = '\'';
    kb_sc_ascii_lower[0x29] = '`';
    kb_sc_ascii_lower[0x2B] = '\\';
    kb_sc_ascii_lower[0x2C] = 'z';
    kb_sc_ascii_lower[0x2D] = 'x';
    kb_sc_ascii_lower[0x2E] = 'c';
    kb_sc_ascii_lower[0x2F] = 'v';   
    kb_sc_ascii_lower[0x30] = 'b';
    kb_sc_ascii_lower[0x31] = 'n';
    kb_sc_ascii_lower[0x32] = 'm';
    kb_sc_ascii_lower[0x33] = ',';
    kb_sc_ascii_lower[0x34] = '.';
    kb_sc_ascii_lower[0x35] = '/';
    kb_sc_ascii_lower[0x37] = '*';
    kb_sc_ascii_lower[0x39] = ' ';
    kb_sc_ascii_lower[0x47] = '7';
    kb_sc_ascii_lower[0x48] = '8';
    kb_sc_ascii_lower[0x49] = '9';
    kb_sc_ascii_lower[0x4A] = '-';
    kb_sc_ascii_lower[0x4B] = '4';
    kb_sc_ascii_lower[0x4C] = '5';
    kb_sc_ascii_lower[0x4D] = '6';
    kb_sc_ascii_lower[0x4E] = '+';
    kb_sc_ascii_lower[0x4F] = '1';
    kb_sc_ascii_lower[0x50] = '2';
    kb_sc_ascii_lower[0x51] = '3';
    kb_sc_ascii_lower[0x52] = '0';
    kb_sc_ascii_lower[0x53] = '.';   

    kb_sc_ascii_upper[0x01] = ESC;
    kb_sc_ascii_upper[0x02] = '!';    
    kb_sc_ascii_upper[0x03] = '@';    
    kb_sc_ascii_upper[0x04] = '#';    
    kb_sc_ascii_upper[0x05] = '$';    
    kb_sc_ascii_upper[0x06] = '%';    
    kb_sc_ascii_upper[0x07] = '^';    
    kb_sc_ascii_upper[0x08] = '&';    
    kb_sc_ascii_upper[0x09] = '*';    
    kb_sc_ascii_upper[0x0A] = '(';
    kb_sc_ascii_upper[0x0B] = ')';
    kb_sc_ascii_upper[0x0C] = '_';
    kb_sc_ascii_upper[0x0D] = '+';
    kb_sc_ascii_upper[0x0E] = BACKSPACE;
    kb_sc_ascii_upper[0x0F] = TAB;
    kb_sc_ascii_upper[0x10] = 'Q';
    kb_sc_ascii_upper[0x11] = 'W';
    kb_sc_ascii_upper[0x12] = 'E';
    kb_sc_ascii_upper[0x13] = 'R';
    kb_sc_ascii_upper[0x14] = 'T';
    kb_sc_ascii_upper[0x15] = 'Y';
    kb_sc_ascii_upper[0x16] = 'U';
    kb_sc_ascii_upper[0x17] = 'I';
    kb_sc_ascii_upper[0x18] = 'O';
    kb_sc_ascii_upper[0x19] = 'P';
    kb_sc_ascii_upper[0x1A] = '{';
    kb_sc_ascii_upper[0x1B] = '}';
    kb_sc_ascii_upper[0x1C] = NEW_LINE;
    kb_sc_ascii_upper[0x1E] = 'A';
    kb_sc_ascii_upper[0x1F] = 'S';
    kb_sc_ascii_upper[0x20] = 'D';
    kb_sc_ascii_upper[0x21] = 'F';
    kb_sc_ascii_upper[0x22] = 'G';
    kb_sc_ascii_upper[0x23] = 'H';
    kb_sc_ascii_upper[0x24] = 'J';
    kb_sc_ascii_upper[0x25] = 'K';
    kb_sc_ascii_upper[0x26] = 'L';
    kb_sc_ascii_upper[0x27] = ':';
    kb_sc_ascii_upper[0x28] = '\"';
    kb_sc_ascii_upper[0x29] = '~';
    kb_sc_ascii_upper[0x2B] = '|';
    kb_sc_ascii_upper[0x2C] = 'Z';
    kb_sc_ascii_upper[0x2D] = 'X';
    kb_sc_ascii_upper[0x2E] = 'C';
    kb_sc_ascii_upper[0x2F] = 'V';   
    kb_sc_ascii_upper[0x30] = 'B';
    kb_sc_ascii_upper[0x31] = 'N';
    kb_sc_ascii_upper[0x32] = 'M';
    kb_sc_ascii_upper[0x33] = '<';
    kb_sc_ascii_upper[0x34] = '>';
    kb_sc_ascii_upper[0x35] = '?';
    kb_sc_ascii_upper[0x37] = '*';
    kb_sc_ascii_upper[0x39] = ' ';
    kb_sc_ascii_upper[0x47] = '7';
    kb_sc_ascii_upper[0x48] = '8';
    kb_sc_ascii_upper[0x49] = '9';
    kb_sc_ascii_upper[0x4A] = '-';
    kb_sc_ascii_upper[0x4B] = '4';
    kb_sc_ascii_upper[0x4C] = '5';
    kb_sc_ascii_upper[0x4D] = '6';
    kb_sc_ascii_upper[0x4E] = '+';
    kb_sc_ascii_upper[0x4F] = '1';
    kb_sc_ascii_upper[0x50] = '2';
    kb_sc_ascii_upper[0x51] = '3';
    kb_sc_ascii_upper[0x52] = '0';
    kb_sc_ascii_upper[0x53] = '.';

    kb_sc_ascii_2byte[0x35] = NEW_LINE;
}
