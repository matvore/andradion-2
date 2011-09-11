#include "StdAfx.h"
#include "Keyboard.h"

using namespace std;

static char key_states[KEY_KEYCOUNT];

AutoComPtr<IDirectInput> direct_input;
AutoComPtr<IDirectInputDevice> device;

static DIOBJECTDATAFORMAT c_rgodfDIKeyboard[256] = {
  { &GUID_Key, 0, 0x8000000C, 0 }, 	{ &GUID_Key, 1, 0x8000010C, 0 },
  { &GUID_Key, 2, 0x8000020C, 0 }, 	{ &GUID_Key, 3, 0x8000030C, 0 },
  { &GUID_Key, 4, 0x8000040C, 0 }, 	{ &GUID_Key, 5, 0x8000050C, 0 },
  { &GUID_Key, 6, 0x8000060C, 0 }, 	{ &GUID_Key, 7, 0x8000070C, 0 },
  { &GUID_Key, 8, 0x8000080C, 0 }, 	{ &GUID_Key, 9, 0x8000090C, 0 },
  { &GUID_Key, 10, 0x80000A0C, 0 }, 	{ &GUID_Key, 11, 0x80000B0C, 0 },
  { &GUID_Key, 12, 0x80000C0C, 0 }, 	{ &GUID_Key, 13, 0x80000D0C, 0 },
  { &GUID_Key, 14, 0x80000E0C, 0 }, 	{ &GUID_Key, 15, 0x80000F0C, 0 },
  { &GUID_Key, 16, 0x8000100C, 0 }, 	{ &GUID_Key, 17, 0x8000110C, 0 },
  { &GUID_Key, 18, 0x8000120C, 0 }, 	{ &GUID_Key, 19, 0x8000130C, 0 },
  { &GUID_Key, 20, 0x8000140C, 0 }, 	{ &GUID_Key, 21, 0x8000150C, 0 },
  { &GUID_Key, 22, 0x8000160C, 0 }, 	{ &GUID_Key, 23, 0x8000170C, 0 },
  { &GUID_Key, 24, 0x8000180C, 0 }, 	{ &GUID_Key, 25, 0x8000190C, 0 },
  { &GUID_Key, 26, 0x80001A0C, 0 }, 	{ &GUID_Key, 27, 0x80001B0C, 0 },
  { &GUID_Key, 28, 0x80001C0C, 0 }, 	{ &GUID_Key, 29, 0x80001D0C, 0 },
  { &GUID_Key, 30, 0x80001E0C, 0 }, 	{ &GUID_Key, 31, 0x80001F0C, 0 },
  { &GUID_Key, 32, 0x8000200C, 0 }, 	{ &GUID_Key, 33, 0x8000210C, 0 },
  { &GUID_Key, 34, 0x8000220C, 0 }, 	{ &GUID_Key, 35, 0x8000230C, 0 },
  { &GUID_Key, 36, 0x8000240C, 0 }, 	{ &GUID_Key, 37, 0x8000250C, 0 },
  { &GUID_Key, 38, 0x8000260C, 0 }, 	{ &GUID_Key, 39, 0x8000270C, 0 },
  { &GUID_Key, 40, 0x8000280C, 0 }, 	{ &GUID_Key, 41, 0x8000290C, 0 },
  { &GUID_Key, 42, 0x80002A0C, 0 }, 	{ &GUID_Key, 43, 0x80002B0C, 0 },
  { &GUID_Key, 44, 0x80002C0C, 0 }, 	{ &GUID_Key, 45, 0x80002D0C, 0 },
  { &GUID_Key, 46, 0x80002E0C, 0 }, 	{ &GUID_Key, 47, 0x80002F0C, 0 },
  { &GUID_Key, 48, 0x8000300C, 0 }, 	{ &GUID_Key, 49, 0x8000310C, 0 },
  { &GUID_Key, 50, 0x8000320C, 0 }, 	{ &GUID_Key, 51, 0x8000330C, 0 },
  { &GUID_Key, 52, 0x8000340C, 0 }, 	{ &GUID_Key, 53, 0x8000350C, 0 },
  { &GUID_Key, 54, 0x8000360C, 0 }, 	{ &GUID_Key, 55, 0x8000370C, 0 },
  { &GUID_Key, 56, 0x8000380C, 0 }, 	{ &GUID_Key, 57, 0x8000390C, 0 },
  { &GUID_Key, 58, 0x80003A0C, 0 }, 	{ &GUID_Key, 59, 0x80003B0C, 0 },
  { &GUID_Key, 60, 0x80003C0C, 0 }, 	{ &GUID_Key, 61, 0x80003D0C, 0 },
  { &GUID_Key, 62, 0x80003E0C, 0 }, 	{ &GUID_Key, 63, 0x80003F0C, 0 },
  { &GUID_Key, 64, 0x8000400C, 0 }, 	{ &GUID_Key, 65, 0x8000410C, 0 },
  { &GUID_Key, 66, 0x8000420C, 0 }, 	{ &GUID_Key, 67, 0x8000430C, 0 },
  { &GUID_Key, 68, 0x8000440C, 0 }, 	{ &GUID_Key, 69, 0x8000450C, 0 },
  { &GUID_Key, 70, 0x8000460C, 0 }, 	{ &GUID_Key, 71, 0x8000470C, 0 },
  { &GUID_Key, 72, 0x8000480C, 0 }, 	{ &GUID_Key, 73, 0x8000490C, 0 },
  { &GUID_Key, 74, 0x80004A0C, 0 }, 	{ &GUID_Key, 75, 0x80004B0C, 0 },
  { &GUID_Key, 76, 0x80004C0C, 0 }, 	{ &GUID_Key, 77, 0x80004D0C, 0 },
  { &GUID_Key, 78, 0x80004E0C, 0 }, 	{ &GUID_Key, 79, 0x80004F0C, 0 },
  { &GUID_Key, 80, 0x8000500C, 0 }, 	{ &GUID_Key, 81, 0x8000510C, 0 },
  { &GUID_Key, 82, 0x8000520C, 0 }, 	{ &GUID_Key, 83, 0x8000530C, 0 },
  { &GUID_Key, 84, 0x8000540C, 0 }, 	{ &GUID_Key, 85, 0x8000550C, 0 },
  { &GUID_Key, 86, 0x8000560C, 0 }, 	{ &GUID_Key, 87, 0x8000570C, 0 },
  { &GUID_Key, 88, 0x8000580C, 0 }, 	{ &GUID_Key, 89, 0x8000590C, 0 },
  { &GUID_Key, 90, 0x80005A0C, 0 }, 	{ &GUID_Key, 91, 0x80005B0C, 0 },
  { &GUID_Key, 92, 0x80005C0C, 0 }, 	{ &GUID_Key, 93, 0x80005D0C, 0 },
  { &GUID_Key, 94, 0x80005E0C, 0 }, 	{ &GUID_Key, 95, 0x80005F0C, 0 },
  { &GUID_Key, 96, 0x8000600C, 0 }, 	{ &GUID_Key, 97, 0x8000610C, 0 },
  { &GUID_Key, 98, 0x8000620C, 0 }, 	{ &GUID_Key, 99, 0x8000630C, 0 },
  { &GUID_Key, 100, 0x8000640C, 0 }, 	{ &GUID_Key, 101, 0x8000650C, 0 },
  { &GUID_Key, 102, 0x8000660C, 0 }, 	{ &GUID_Key, 103, 0x8000670C, 0 },
  { &GUID_Key, 104, 0x8000680C, 0 }, 	{ &GUID_Key, 105, 0x8000690C, 0 },
  { &GUID_Key, 106, 0x80006A0C, 0 }, 	{ &GUID_Key, 107, 0x80006B0C, 0 },
  { &GUID_Key, 108, 0x80006C0C, 0 }, 	{ &GUID_Key, 109, 0x80006D0C, 0 },
  { &GUID_Key, 110, 0x80006E0C, 0 }, 	{ &GUID_Key, 111, 0x80006F0C, 0 },
  { &GUID_Key, 112, 0x8000700C, 0 }, 	{ &GUID_Key, 113, 0x8000710C, 0 },
  { &GUID_Key, 114, 0x8000720C, 0 }, 	{ &GUID_Key, 115, 0x8000730C, 0 },
  { &GUID_Key, 116, 0x8000740C, 0 }, 	{ &GUID_Key, 117, 0x8000750C, 0 },
  { &GUID_Key, 118, 0x8000760C, 0 }, 	{ &GUID_Key, 119, 0x8000770C, 0 },
  { &GUID_Key, 120, 0x8000780C, 0 }, 	{ &GUID_Key, 121, 0x8000790C, 0 },
  { &GUID_Key, 122, 0x80007A0C, 0 }, 	{ &GUID_Key, 123, 0x80007B0C, 0 },
  { &GUID_Key, 124, 0x80007C0C, 0 }, 	{ &GUID_Key, 125, 0x80007D0C, 0 },
  { &GUID_Key, 126, 0x80007E0C, 0 }, 	{ &GUID_Key, 127, 0x80007F0C, 0 },
  { &GUID_Key, 128, 0x8000800C, 0 }, 	{ &GUID_Key, 129, 0x8000810C, 0 },
  { &GUID_Key, 130, 0x8000820C, 0 }, 	{ &GUID_Key, 131, 0x8000830C, 0 },
  { &GUID_Key, 132, 0x8000840C, 0 }, 	{ &GUID_Key, 133, 0x8000850C, 0 },
  { &GUID_Key, 134, 0x8000860C, 0 }, 	{ &GUID_Key, 135, 0x8000870C, 0 },
  { &GUID_Key, 136, 0x8000880C, 0 }, 	{ &GUID_Key, 137, 0x8000890C, 0 },
  { &GUID_Key, 138, 0x80008A0C, 0 }, 	{ &GUID_Key, 139, 0x80008B0C, 0 },
  { &GUID_Key, 140, 0x80008C0C, 0 }, 	{ &GUID_Key, 141, 0x80008D0C, 0 },
  { &GUID_Key, 142, 0x80008E0C, 0 }, 	{ &GUID_Key, 143, 0x80008F0C, 0 },
  { &GUID_Key, 144, 0x8000900C, 0 }, 	{ &GUID_Key, 145, 0x8000910C, 0 },
  { &GUID_Key, 146, 0x8000920C, 0 }, 	{ &GUID_Key, 147, 0x8000930C, 0 },
  { &GUID_Key, 148, 0x8000940C, 0 }, 	{ &GUID_Key, 149, 0x8000950C, 0 },
  { &GUID_Key, 150, 0x8000960C, 0 }, 	{ &GUID_Key, 151, 0x8000970C, 0 },
  { &GUID_Key, 152, 0x8000980C, 0 }, 	{ &GUID_Key, 153, 0x8000990C, 0 },
  { &GUID_Key, 154, 0x80009A0C, 0 }, 	{ &GUID_Key, 155, 0x80009B0C, 0 },
  { &GUID_Key, 156, 0x80009C0C, 0 }, 	{ &GUID_Key, 157, 0x80009D0C, 0 },
  { &GUID_Key, 158, 0x80009E0C, 0 }, 	{ &GUID_Key, 159, 0x80009F0C, 0 },
  { &GUID_Key, 160, 0x8000A00C, 0 }, 	{ &GUID_Key, 161, 0x8000A10C, 0 },
  { &GUID_Key, 162, 0x8000A20C, 0 }, 	{ &GUID_Key, 163, 0x8000A30C, 0 },
  { &GUID_Key, 164, 0x8000A40C, 0 }, 	{ &GUID_Key, 165, 0x8000A50C, 0 },
  { &GUID_Key, 166, 0x8000A60C, 0 }, 	{ &GUID_Key, 167, 0x8000A70C, 0 },
  { &GUID_Key, 168, 0x8000A80C, 0 }, 	{ &GUID_Key, 169, 0x8000A90C, 0 },
  { &GUID_Key, 170, 0x8000AA0C, 0 }, 	{ &GUID_Key, 171, 0x8000AB0C, 0 },
  { &GUID_Key, 172, 0x8000AC0C, 0 }, 	{ &GUID_Key, 173, 0x8000AD0C, 0 },
  { &GUID_Key, 174, 0x8000AE0C, 0 }, 	{ &GUID_Key, 175, 0x8000AF0C, 0 },
  { &GUID_Key, 176, 0x8000B00C, 0 }, 	{ &GUID_Key, 177, 0x8000B10C, 0 },
  { &GUID_Key, 178, 0x8000B20C, 0 }, 	{ &GUID_Key, 179, 0x8000B30C, 0 },
  { &GUID_Key, 180, 0x8000B40C, 0 }, 	{ &GUID_Key, 181, 0x8000B50C, 0 },
  { &GUID_Key, 182, 0x8000B60C, 0 }, 	{ &GUID_Key, 183, 0x8000B70C, 0 },
  { &GUID_Key, 184, 0x8000B80C, 0 }, 	{ &GUID_Key, 185, 0x8000B90C, 0 },
  { &GUID_Key, 186, 0x8000BA0C, 0 }, 	{ &GUID_Key, 187, 0x8000BB0C, 0 },
  { &GUID_Key, 188, 0x8000BC0C, 0 }, 	{ &GUID_Key, 189, 0x8000BD0C, 0 },
  { &GUID_Key, 190, 0x8000BE0C, 0 }, 	{ &GUID_Key, 191, 0x8000BF0C, 0 },
  { &GUID_Key, 192, 0x8000C00C, 0 }, 	{ &GUID_Key, 193, 0x8000C10C, 0 },
  { &GUID_Key, 194, 0x8000C20C, 0 }, 	{ &GUID_Key, 195, 0x8000C30C, 0 },
  { &GUID_Key, 196, 0x8000C40C, 0 }, 	{ &GUID_Key, 197, 0x8000C50C, 0 },
  { &GUID_Key, 198, 0x8000C60C, 0 }, 	{ &GUID_Key, 199, 0x8000C70C, 0 },
  { &GUID_Key, 200, 0x8000C80C, 0 }, 	{ &GUID_Key, 201, 0x8000C90C, 0 },
  { &GUID_Key, 202, 0x8000CA0C, 0 }, 	{ &GUID_Key, 203, 0x8000CB0C, 0 },
  { &GUID_Key, 204, 0x8000CC0C, 0 }, 	{ &GUID_Key, 205, 0x8000CD0C, 0 },
  { &GUID_Key, 206, 0x8000CE0C, 0 }, 	{ &GUID_Key, 207, 0x8000CF0C, 0 },
  { &GUID_Key, 208, 0x8000D00C, 0 }, 	{ &GUID_Key, 209, 0x8000D10C, 0 },
  { &GUID_Key, 210, 0x8000D20C, 0 }, 	{ &GUID_Key, 211, 0x8000D30C, 0 },
  { &GUID_Key, 212, 0x8000D40C, 0 }, 	{ &GUID_Key, 213, 0x8000D50C, 0 },
  { &GUID_Key, 214, 0x8000D60C, 0 }, 	{ &GUID_Key, 215, 0x8000D70C, 0 },
  { &GUID_Key, 216, 0x8000D80C, 0 }, 	{ &GUID_Key, 217, 0x8000D90C, 0 },
  { &GUID_Key, 218, 0x8000DA0C, 0 }, 	{ &GUID_Key, 219, 0x8000DB0C, 0 },
  { &GUID_Key, 220, 0x8000DC0C, 0 }, 	{ &GUID_Key, 221, 0x8000DD0C, 0 },
  { &GUID_Key, 222, 0x8000DE0C, 0 }, 	{ &GUID_Key, 223, 0x8000DF0C, 0 },
  { &GUID_Key, 224, 0x8000E00C, 0 }, 	{ &GUID_Key, 225, 0x8000E10C, 0 },
  { &GUID_Key, 226, 0x8000E20C, 0 }, 	{ &GUID_Key, 227, 0x8000E30C, 0 },
  { &GUID_Key, 228, 0x8000E40C, 0 }, 	{ &GUID_Key, 229, 0x8000E50C, 0 },
  { &GUID_Key, 230, 0x8000E60C, 0 }, 	{ &GUID_Key, 231, 0x8000E70C, 0 },
  { &GUID_Key, 232, 0x8000E80C, 0 }, 	{ &GUID_Key, 233, 0x8000E90C, 0 },
  { &GUID_Key, 234, 0x8000EA0C, 0 }, 	{ &GUID_Key, 235, 0x8000EB0C, 0 },
  { &GUID_Key, 236, 0x8000EC0C, 0 }, 	{ &GUID_Key, 237, 0x8000ED0C, 0 },
  { &GUID_Key, 238, 0x8000EE0C, 0 }, 	{ &GUID_Key, 239, 0x8000EF0C, 0 },
  { &GUID_Key, 240, 0x8000F00C, 0 }, 	{ &GUID_Key, 241, 0x8000F10C, 0 },
  { &GUID_Key, 242, 0x8000F20C, 0 }, 	{ &GUID_Key, 243, 0x8000F30C, 0 },
  { &GUID_Key, 244, 0x8000F40C, 0 }, 	{ &GUID_Key, 245, 0x8000F50C, 0 },
  { &GUID_Key, 246, 0x8000F60C, 0 }, 	{ &GUID_Key, 247, 0x8000F70C, 0 },
  { &GUID_Key, 248, 0x8000F80C, 0 }, 	{ &GUID_Key, 249, 0x8000F90C, 0 },
  { &GUID_Key, 250, 0x8000FA0C, 0 }, 	{ &GUID_Key, 251, 0x8000FB0C, 0 },
  { &GUID_Key, 252, 0x8000FC0C, 0 }, 	{ &GUID_Key, 253, 0x8000FD0C, 0 },
  { &GUID_Key, 254, 0x8000FE0C, 0 }, 	{ &GUID_Key, 255, 0x8000FF0C, 0 }
};

const DIDATAFORMAT c_dfDIKeyboard
= { 24, 16, 0x2, 256, 256, c_rgodfDIKeyboard };

#define KEY(name) "\x01" name "\x02"
#define NUM(name) "\x03" name

// \x01???\x02 - the string ??? is the name of the key
// \x03        - the next character is the name of a key on the numeric pad
// \x04\x??    - up to and including key ??, the keys do not have a name
static const char *KEY_NAMES =
KEY("Esc") "1234567890-=" KEY("Backspace")
KEY("Tab") "QWERTYUIOP[]" KEY("Enter")
KEY("Left Control") "ASDFGHJKL;'~"
KEY("Left Shift") "\\ZXCVBNM,./" KEY("Right Shift")
NUM("*")
KEY("Left Alt") KEY("Space") KEY("Caps Lock")
KEY("F1") KEY("F2") KEY("F3") KEY("F4") KEY("F5")
KEY("F6") KEY("F7") KEY("F8") KEY("F9") KEY("F10")
KEY("Num Lock") KEY("Scroll Lock")
NUM("7") NUM("8") NUM("9") NUM("-")
NUM("4") NUM("5") NUM("6") NUM("+")
NUM("1") NUM("2") NUM("3") NUM("0") NUM(".")
KEY("F11") KEY("F12")
"\x04\x9b"
KEY("Numpad Enter")
KEY("Right Control")
"\x04\xb4"
NUM("/")
KEY("Print Screen/Sys Rq")
KEY("Right Alt")
"\x04\xc6"
KEY("Home")
KEY("Up")
KEY("PgUp")
"\x04\xca"
KEY("Left")
"?"
KEY("Right")
"?"
KEY("End")
KEY("Down")
KEY("PgDn")
KEY("Insert") KEY("Delete")
"\x04\xda"
KEY("Left Windows") KEY("Right Windows")
KEY("Menu")
"\x04\xff";

void KeyInitialize(HINSTANCE hInstance, HWND hWnd) {
  AutoComPtr<IDirectInput> new_direct_input;
  AutoComPtr<IDirectInputDevice> new_device;

  if (device
      || FAILED(CoCreateInstance
                (CLSID_DirectInput, 0, CLSCTX_INPROC_SERVER, IID_IDirectInput,
                 (void **)new_direct_input.GetPtrToPtr()))
      || FAILED(new_direct_input->Initialize(hInstance, DIRECTINPUT_VERSION))
      || FAILED(new_direct_input->CreateDevice
                (GUID_SysKeyboard, new_device.GetPtrToPtr(), 0))
      || FAILED(new_device->SetDataFormat(&c_dfDIKeyboard))
      || FAILED(new_device->SetCooperativeLevel
                (hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))) {
    memset(key_states, 0, KEY_KEYCOUNT);
  } else {
    direct_input = new_direct_input;
    device = new_device;
  }
}

void KeyRelease() {device.Reset(); direct_input.Reset();}

void KeyRefreshState() {
  if (device) {
    device->Acquire();
    device->GetDeviceState(KEY_KEYCOUNT, (void *)key_states);
  }
}

bool KeyPressed(int key) {return key_states[key] & 0x80;}

string KeyName(int key) {
  const char *data_ptr = KEY_NAMES;
  int index = 0;
  string result;

  do {
    result.clear();

    switch(*data_ptr) {
    case 1:
      do {
        if (*++data_ptr != 2 && index == key) {
          result += *++data_ptr;
        }
      } while (*data_ptr != 2);
      break;
    case 4:
      index = *++data_ptr;
      break;      
    case 3:
      data_ptr++;
      result = "Numpad ";
    default:
      result += *data_ptr;
    }

    data_ptr++;
  } while (index++ < key);

  if (index > key + 1) {
    result = "Key ";
    result += (char)('0' + key / 16);
    result += (char)('0' + key % 16);
  }
  
  return result;
}
