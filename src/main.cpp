#include <Arduino.h>
#include <Keypad.h>
#include <Joystick.h>
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

#define ENABLE_PULLUPS
#define NUMROTARIES 4
#define NUMBUTTONS 24
#define NUMROWS 5
#define NUMCOLS 5

#define I2C_ADDR    0x3F // <<----- Add your address here.  Find it from I2C Scanner
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7

int n = 1;

LiquidCrystal_I2C lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);


byte buttons[NUMROWS][NUMCOLS] = {
        {0,1,2,3,4},
        {5,6,7,8,9},
        {10,11,12,13,14},
        {15,16,17,18,19},
        {20,21,22,23},
};

struct rotariesdef {
        byte pin1;
        byte pin2;
        int ccwchar;
        int cwchar;
        volatile unsigned char state;
};

rotariesdef rotaries[NUMROTARIES] {
        {0,1,24,25,0},
        {2,3,26,27,0},
        {4,5,28,29,0},
        {6,7,30,31,0},
};

#define DIR_CCW 0x10
#define DIR_CW 0x20
#define R_START 0x0

#ifdef HALF_STEP
#define R_CCW_BEGIN 0x1
#define R_CW_BEGIN 0x2
#define R_START_M 0x3
#define R_CW_BEGIN_M 0x4
#define R_CCW_BEGIN_M 0x5
const unsigned char ttable[6][4] = {
        // R_START (00)
        {R_START_M,            R_CW_BEGIN,     R_CCW_BEGIN,  R_START},
        // R_CCW_BEGIN
        {R_START_M | DIR_CCW, R_START,        R_CCW_BEGIN,  R_START},
        // R_CW_BEGIN
        {R_START_M | DIR_CW,  R_CW_BEGIN,     R_START,      R_START},
        // R_START_M (11)
        {R_START_M,            R_CCW_BEGIN_M,  R_CW_BEGIN_M, R_START},
        // R_CW_BEGIN_M
        {R_START_M,            R_START_M,      R_CW_BEGIN_M, R_START | DIR_CW},
        // R_CCW_BEGIN_M
        {R_START_M,            R_CCW_BEGIN_M,  R_START_M,    R_START | DIR_CCW},
};
#else
#define R_CW_FINAL 0x1
#define R_CW_BEGIN 0x2
#define R_CW_NEXT 0x3
#define R_CCW_BEGIN 0x4
#define R_CCW_FINAL 0x5
#define R_CCW_NEXT 0x6

const unsigned char ttable[7][4] = {
        // R_START
        {R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START},
        // R_CW_FINAL
        {R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW},
        // R_CW_BEGIN
        {R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START},
        // R_CW_NEXT
        {R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},
        // R_CCW_BEGIN
        {R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START},
        // R_CCW_FINAL
        {R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW},
        // R_CCW_NEXT
        {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},
};
#endif

byte rowPins[NUMROWS] = {21,20,19,18,15};
byte colPins[NUMCOLS] = {14,16,10,9,8};

int mode = 0;
int lastKey = 0;


Keypad buttbx = Keypad( makeKeymap(buttons), rowPins, colPins, NUMROWS, NUMCOLS);

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,
                   JOYSTICK_TYPE_JOYSTICK, 32, 0,
                   false, false, false, false, false, false,
                   false, false, false, false, false);

void drawMenu() {
        lcd.home();                    // go home
        lcd.clear();
        lcd.print("XPilot ");
        lcd.print(lastKey);
        lcd.setCursor(0,1);
        switch(mode)
        {
        default:
        case 0:
                lcd.print("[SPD] HDG ALT VS");
                break;
        case 1:
                lcd.print("SPD [HDG] ALT VS");
                break;
        case 2:
                lcd.print("SPD HDG [ALT] VS");
                break;
        case 3:
                lcd.print("SPD HDG ALT [VS]");
                break;
        }
}

void rotary_init() {
        for (int i=0; i<NUMROTARIES; i++) {
                pinMode(rotaries[i].pin1, INPUT);
                pinMode(rotaries[i].pin2, INPUT);
      #ifdef ENABLE_PULLUPS
                digitalWrite(rotaries[i].pin1, HIGH);
                digitalWrite(rotaries[i].pin2, HIGH);
      #endif
        }
}

void CheckAllButtons(void) {
        if (buttbx.getKeys())
        {
                for (int i=0; i<LIST_MAX; i++)
                {
                        if ( buttbx.key[i].stateChanged )
                        {
                                switch (buttbx.key[i].kstate) {
                                case PRESSED:
                                case HOLD:
                                        Serial.print("HOLD ");
                                        Serial.print(buttbx.key[i].kchar, DEC);
                                        if( buttbx.key[i].kchar == 0 )
                                        {
                                                switch(mode)
                                                {
                                                case 3:
                                                        Joystick.setButton(7, 1);
                                                        break;
                                                case 2:
                                                        Joystick.setButton(6, 1);
                                                        break;
                                                case 1:
                                                        Joystick.setButton(5, 1);
                                                        break;
                                                case 0:
                                                        Joystick.setButton(0, 1);
                                                        break;
                                                default:
                                                        Joystick.setButton(buttbx.key[i].kchar, 1);
                                                        break;
                                                }
                                        } else {
                                                Joystick.setButton(buttbx.key[i].kchar, 1);
                                        }
                                        break;
                                case RELEASED:
                                case IDLE:
                                        Serial.print("IDLE ");
                                        Serial.print(buttbx.key[i].kchar, DEC);
                                        if( buttbx.key[i].kchar == 0 )
                                        {
                                                switch(mode)
                                                {
                                                case 3:
                                                        Joystick.setButton(7, 0);
                                                        break;
                                                case 2:
                                                        Joystick.setButton(6, 0);
                                                        break;
                                                case 1:
                                                        Joystick.setButton(5, 0);
                                                        break;
                                                case 0:
                                                        Joystick.setButton(0, 0);
                                                        break;
                                                default:
                                                        Joystick.setButton(buttbx.key[i].kchar, 0);
                                                        break;
                                                }
                                        } else {
                                                Joystick.setButton(buttbx.key[i].kchar, 0);
                                        }
                                        break;
                                }
                                lastKey = buttbx.key[i].kchar;
                                switch( lastKey )
                                {
                                case 4:
                                        mode = 0;
                                        break;
                                case 3:
                                        mode = 1;
                                        break;
                                case 2:
                                        mode = 2;
                                        break;
                                case 1:
                                        mode = 3;
                                        break;
                                default:
                                        break;
                                }
                                drawMenu();
                        }
                }
        }
}




unsigned char rotary_process(int _i) {
        unsigned char pinstate = (digitalRead(rotaries[_i].pin2) << 1) | digitalRead(rotaries[_i].pin1);
        rotaries[_i].state = ttable[rotaries[_i].state & 0xf][pinstate];
        return (rotaries[_i].state & 0x30);
}

void CheckAllEncoders(void) {
        for (int i=0; i<NUMROTARIES; i++) {
                unsigned char result = rotary_process(i);
                if (result == DIR_CCW) {
                        Joystick.setButton(rotaries[i].ccwchar + (mode * 2), 1); delay(50); Joystick.setButton(rotaries[i].ccwchar + (mode*2), 0);
                        Serial.print("CW");
                };
                if (result == DIR_CW) {
                        Joystick.setButton(rotaries[i].cwchar + (mode * 2), 1); delay(50); Joystick.setButton(rotaries[i].cwchar + (mode * 2), 0);
                        Serial.print("CCW");
                };
        }
}

void setup() {
        lcd.begin (16,2); //  <<----- My LCD was 16x2
        lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
        lcd.setBacklight(HIGH);
        drawMenu();
        Serial.begin(9600);
        Joystick.begin();
        rotary_init();
        Serial.println("Ready for feed");
}

void loop() {
        CheckAllEncoders();
        CheckAllButtons();
//  Serial.println("Ready for feed");
}
