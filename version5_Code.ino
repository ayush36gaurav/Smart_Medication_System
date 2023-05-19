#include <DS3231.h>        // Importing RTC3231 Library
#include <Wire.h>          // i2C Conection Library
#include <LiquidCrystal.h> //Libraries
#include <EEPROM.h>

LiquidCrystal lcd(2, 3, 4, 5, 6, 7); // Arduino pins to lcd

#define bt_up A0
#define bt_down A1
#define bt_left A2
#define bt_right A3
#define bt_edit A4
#define bt_plus A5
#define bt_minus A6

// Green LED glow for medicine intake time alert
// Red LED glow for warning of less medicine
// Blue LED glow for medicine missed
// # define B1_Green 2
// # define B1_Red 3
// # define B1_Blue 4
// # define B2_Green 5
// # define B2_Red 6
// # define B2_Blue 7
// # define B3_Green 42
// # define B3_Red 44
// # define B3_Blue 46
// # define B4_Green 48
// # define B4_Red 50
// # define B4_Blue 52
#define buzzer 16
#define pintrig 14
#define pinecho 15
// # define high 13

int Green_LED[4] = {3, 6, 13, 9};                                   // Setting the corresponding 
int Red_LED[4] = {2, 5, 12, 8};                                     // pins for each LED light
int Blue_LED[4] = {4, 7, 11, 10};                                   // in each compartment

DS3231 rtc(SDA, SCL);
Time t;

int hh, mm, ss;                                                     // for storing the current time

int Morning_Alarm[4] = {22, 42, 0, 1};                              // For storing the 
int Afternoon_Alarm[4] = {22, 42, 15, 1};                           // alarm time for
int Evening_Alarm[4] = {22, 42, 30, 1};                             // 4 different instances
int Night_Alarm[4] = {22, 42, 45, 1};                               // throughout the day

// For accessing each of the four events of the day
int *event_array[4] = {Morning_Alarm, Afternoon_Alarm, Evening_Alarm, Night_Alarm};

int Morning[4] = {1, 1, 0, 1};                                      // Used to determine
int Afternoon[4] = {1, 0, 0, 1};                                    // if a particular medicine
int Evening[4] = {0, 1, 1, 0};                                      // is to be taken at a
int Night[4] = {1, 1, 1, 1};
int *mode_array[4] = {Morning, Afternoon, Evening, Night};

int count_Medicine[4] = {3, 1, 4, 6};

bool flags[4] = {1, 1, 1, 1};                                       // flags -> If true, medicine has not been taken
bool currFlag[4] = {0, 0, 0, 0};

int setAlarm = 0, currMode = 0, cursor = 1, currLine = 0;
int line_length[4] = {3, 4, 5, 4};

int stop = 0;

// Setting necessary constants
int Medicine_threshold = 5;     
int Distance_threshold = 12;
char characters[4] = {'M', 'A', 'E', 'N'};

void setup()
{
    pinMode(pintrig, OUTPUT);
    pinMode(pinecho, INPUT);
    Serial.begin(9600);
    rtc.begin();

    pinMode(bt_left, INPUT_PULLUP);
    pinMode(bt_right, INPUT_PULLUP);
    pinMode(bt_plus, INPUT_PULLUP);
    pinMode(bt_minus, INPUT_PULLUP);
    pinMode(bt_up, INPUT_PULLUP);
    pinMode(bt_down, INPUT_PULLUP);
    pinMode(bt_edit, INPUT_PULLUP);

    // Initialize the LED and buzzer pins as output pins
    for (int i = 2; i < 10; i++)
    {
        pinMode(i, OUTPUT);
    }

    pinMode(buzzer, OUTPUT);
    pinMode(10, INPUT);

    for (int i = 42; i < 53; i = i + 2)
    {
        pinMode(i, OUTPUT);
    }

    lcd.begin(16, 4); // Configura lcd numero columnas y filas
    lcd.setCursor(0, 1);
    lcd.print("Smart Medication");
    lcd.setCursor(0, 2);
    lcd.print("     System     ");
    lcd.setCursor(0, 3);
    lcd.print("   Starting...  ");
    delay(2000);
    lcd.clear();

    stop = EEPROM.read(50);
    if (stop == 0)
    {
    }
    else
    {
        WriteEeprom();
    }

    EEPROM.write(50, 0);

    rtc.setTime(22, 41, 50);
}

void loop()
{
    // Get the current time using the RTC library
    t = rtc.getTime();
    hh = t.hour, DEC;
    mm = t.min, DEC;
    ss = t.sec, DEC;

    currMode = getEvent();

    // Display the required contents on LCD display
    lcd.setCursor(0, 0);
    lcd.print("    ");
    lcd.print((hh / 10) % 10);
    lcd.print(hh % 10);
    lcd.print(":");
    lcd.print((mm / 10) % 10);
    lcd.print(mm % 10);
    lcd.print(":");
    lcd.print((ss / 10) % 10);
    lcd.print(ss % 10);
    lcd.print("    ");

    lcd.setCursor(0, 1);
    lcd.print(Medicine_threshold);
    lcd.print(" ");
    lcd.print((count_Medicine[0] / 10) % 10);
    lcd.print(count_Medicine[0] % 10);
    lcd.print(" ");
    lcd.print((count_Medicine[1] / 10) % 10);
    lcd.print(count_Medicine[1] % 10);
    lcd.print(" ");
    lcd.print((count_Medicine[2] / 10) % 10);
    lcd.print(count_Medicine[2] % 10);
    lcd.print(" ");
    lcd.print((count_Medicine[3] / 10) % 10);
    lcd.print(count_Medicine[3] % 10);
    lcd.print(" ");
    lcd.print((Medicine_threshold / 10) % 10);
    lcd.print(Medicine_threshold % 10);
    lcd.print(" ");

    lcd.setCursor(0, 2);
    lcd.print("   ");
    lcd.print(characters[currMode]);
    lcd.print("  ");
    lcd.print(mode_array[currMode][0]);
    lcd.print(" ");
    lcd.print(mode_array[currMode][1]);
    lcd.print(" ");
    lcd.print(mode_array[currMode][2]);
    lcd.print(" ");
    lcd.print(mode_array[currMode][3]);
    lcd.print("   ");

    lcd.setCursor(0, 3);
    lcd.print("  ");
    lcd.print((event_array[currMode][0] / 10) % 10);
    lcd.print(event_array[currMode][0] % 10);
    lcd.print(":");
    lcd.print((event_array[currMode][1] / 10) % 10);
    lcd.print(event_array[currMode][1] % 10);
    lcd.print(":");
    lcd.print((event_array[currMode][2] / 10) % 10);
    lcd.print(event_array[currMode][2] % 10);
    lcd.print("   ");
    lcd.print(event_array[currMode][3]);
    lcd.print("  ");

    checkTime(Morning_Alarm[0], Morning_Alarm[1], Morning_Alarm[2], Morning_Alarm[3], Morning, Night, 0);
    //  runBuzzer(Morning_Alarm[0], Morning_Alarm[1] + 5, Morning_Alarm[2],0);
    checkTime(Afternoon_Alarm[0], Afternoon_Alarm[1], Afternoon_Alarm[2], Afternoon_Alarm[3], Afternoon, Morning, 1);
    //  runBuzzer(Afternoon_Alarm[0], Afternoon_Alarm[1], Afternoon_Alarm[2],1);
    checkTime(Evening_Alarm[0], Evening_Alarm[1], Evening_Alarm[2], Evening_Alarm[3], Evening, Afternoon, 2);
    //  runBuzzer(Evening_Alarm[0], Evening_Alarm[1] + 5, Evening_Alarm[2],2);
    checkTime(Night_Alarm[0], Night_Alarm[1], Night_Alarm[2], Night_Alarm[3], Night, Evening, 3);
    //  runBuzzer(Night_Alarm[0], Night_Alarm[1] + 5, Night_Alarm[2],3);

    // If medicine count is less than threshold in any compartment, relay this information using a RED LED
    for (int i = 0; i < 4; i++)
    {
        if (count_Medicine[i] <= Medicine_threshold)
        {
            digitalWrite(Red_LED[i], HIGH);
        }
        else
        {
            digitalWrite(Red_LED[i], LOW);
        }
    }

    ultra_sonic(0);
    ultra_sonic(1);
    ultra_sonic(2);
    ultra_sonic(3);

    setupClock();
    blinking();
}

int getEvent()
{
    /* Used to determine the next alarm of the day */
    if (setAlarm == 1)
    {
        return currMode;
    }

    if (hh < Morning_Alarm[0])
        return 0;
    else if (hh == Morning_Alarm[0])
    {
        if (mm < Morning_Alarm[1])
            return 0;
        else if (mm == Morning_Alarm[1] && ss < Morning_Alarm)
            return 0;
    }

    if (hh < Aafternoon_Alarm[0])
        return 0;
    else if (hh == Aafternoon_Alarm[0])
    {
        if (mm < Aafternoon_Alarm[1])
            return 0;
        else if (mm == Aafternoon_Alarm[1] && ss < Aafternoon_Alarm)
            return 0;
    }

    if (hh < Evening_Alarm[0])
        return 0;
    else if (hh == Evening_Alarm[0])
    {
        if (mm < Evening_Alarm[1])
            return 0;
        else if (mm == Evening_Alarm[1] && ss < Evening_Alarm)
            return 0;
    }

    if (hh < Night_Alarm[0])
        return 0;
    else if (hh == Night_Alarm[0])
    {
        if (mm < Night_Alarm[1])
            return 0;
        else if (mm == Night_Alarm[1] && ss < Night_Alarm)
            return 0;
    }

    return 0;
}

void blinking()
{
    /* Indicates the cursor location in the edit mode */
    if (setAlarm == 0)
        // return if edit mode is off
        return;

    if (currLine == 0)
    {
        switch (cursor)
        {
        case 1:
            lcd.setCursor(4, 0);
            lcd.print("  ");
            break;
        case 2:
            lcd.setCursor(7, 0);
            lcd.print("  ");
            break;
        case 3:
            lcd.setCursor(10, 0);
            lcd.print("  ");
            break;
        default:
            cursor = 1;
            break;
        }
    }
    else if (currLine == 1)
    {
        switch (cursor)
        {
        case 1:
            lcd.setCursor(1, 1);
            lcd.print("  ");
            break;
        case 2:
            lcd.setCursor(4, 1);
            lcd.print("  ");
            break;
        case 3:
            lcd.setCursor(7, 1);
            lcd.print("  ");
            break;
        case 4:
            lcd.setCursor(10, 1);
            lcd.print("  ");
            break;
        case 5:
            lcd.setCursor(13, 1);
            lcd.print("  ");
            break;
        default:
            cursor = 1;
            break;
        }
    }
    else if (currLine == 2)
    {
        switch (cursor)
        {
        case 1:
            lcd.setCursor(3, 2);
            lcd.print("  ");
            break;
        case 2:
            lcd.setCursor(6, 2);
            lcd.print(" ");
            break;
        case 3:
            lcd.setCursor(8, 2);
            lcd.print(" ");
            break;
        case 4:
            lcd.setCursor(10, 2);
            lcd.print(" ");
        case 5:
            lcd.setCursor(12, 2);
            lcd.print(" ");
            break;
        default:
            cursor = 1;
            break;
        }
    }
    else
    {
        switch (cursor)
        {
        case 1:
            lcd.setCursor(2, 3);
            lcd.print("  ");
            break;
        case 2:
            lcd.setCursor(5, 3);
            lcd.print("  ");
            break;
        case 3:
            lcd.setCursor(8, 3);
            lcd.print("  ");
            break;
        case 4:
            lcd.setCursor(13, 3);
            lcd.print("  ");
            break;
        default:
            cursor = 1;
            break;
        }
    }
}

int count(int arr[])
{
    int count_1 = 0;
    for (int i = 0; i < 4; i++)
    {
        if (arr[i] == 1)
        {
            count_1++;
        }
    }
    return count_1;
}

void ultra_sonic(int index)
{
    if (setAlarm == 1)
        return;
        
    float distance;
    float time_;
    float spd = 0.0346;

    digitalWrite(pintrig, LOW);
    delay(2);
    digitalWrite(pintrig, HIGH);
    delay(10);
    digitalWrite(pintrig, LOW);
    delay(2);

    time_ = pulseIn(pinecho, HIGH);
    // distance(in cm) = speed(in cm/us) * time(in us)

    distance = (spd * time_) / 2;
    if (distance > Distance_threshold)
    {
        count_Medicine[index]--;
        digitalWrite(Green_LED[index], LOW);
        digitalWrite(Blue_LED[index], LOW);

        if (currFlag[index] == 1)
        {
            flags[index] = 0;
            currFlag[index] = 0;
        }
    }
}

void setupClock()
{
    /* The logic behind the edit mode and all the buttons is written here. */
    
    if (digitalRead(bt_edit) == 1)
    {
        if (setAlarm)
        {
            lcd.clear();
            setAlarm = 0;
        }
        else
        {
            setAlarm = 1;
        }
    }

    if (digitalRead(bt_right) == 1 && setAlarm == 1)
    {
        cursor = cursor + 1;
        if (cursor > line_length[currLine])
        {
            cursor = 1;
        }
    }

    if (digitalRead(bt_left) == 1 && setAlarm == 1)
    {
        cursor = cursor - 1;
        if (cursor <= 0)
        {
            cursor = line_length[currLine];
        }
    }

    if (digitalRead(bt_up) == 1 && setAlarm == 1)
    {
        currLine = currLine + 1;
        if (currLine > 3)
        {
            currLine = 0;
        }
        cursor = 1;
    }

    if (digitalRead(bt_down) == 1 && setAlarm == 1)
    {
        currLine = currLine - 1;
        if (currLine < 0)
        {
            currLine = 3;
        }
        cursor = 1;
    }

    if (digitalRead(bt_plus) == 1 && setAlarm == 1)
    {

        if (currLine == 0)
        {
            switch (cursor)
            {
            case 1:
                hh = hh + 1;
            case 2:
                mm = mm + 1;
            case 3:
                ss = ss + 1;

                if (hh > 23)
                    hh = 0;
                if (mm > 59)
                    mm = 0;
                if (ss > 59)
                    ss = 0;
            }
        }
        else if (currLine == 1)
        {
            switch (cursor)
            {
            case 1:
                count_Medicine[0] = count_Medicine[0] + 1;
                break;
            case 2:
                count_Medicine[1] = count_Medicine[1] + 1;
                break;
            case 3:
                count_Medicine[2] = count_Medicine[2] + 1;
                break;
            case 4:
                count_Medicine[3] = count_Medicine[3] + 1;
                break;
            default:
                cursor = 1;
                break;

                for (int i = 0; i < 4; i++)
                {
                    if (count_Medicine[i] > 99)
                    {
                        count_Medicine[i] = 0;
                    }
                }
            }
        }
        else if (currLine == 2)
        {
            switch (cursor)
            {
            case 1:
                currMode = currMode + 1;
                break;
            case 2:
                if (mode_array[currMode][0] == 1)
                    mode_array[currMode][0] = 0;
                else
                    mode_array[currMode][0] = 1;
                break;
            case 3:
                if (mode_array[currMode][1] == 1)
                    mode_array[currMode][1] = 0;
                else
                    mode_array[currMode][1] = 1;
                break;
            case 4:
                if (mode_array[currMode][2] == 1)
                    mode_array[currMode][2] = 0;
                else
                    mode_array[currMode][2] = 1;
                break;
            case 5:
                if (mode_array[currMode][3] == 1)
                    mode_array[currMode][3] = 0;
                else
                    mode_array[currMode][3] = 1;
            default:
                cursor = 1;
                break;
            }

            if (currMode > 3)
                currMode = 0;
        }
        else if (currLine == 3)
        {
            switch (cursor)
            {
            case 1:
                event_array[currMode][0] = event_array[currMode][0] + 1;
                break;
            case 2:
                event_array[currMode][1] = event_array[currMode][1] + 1;
                break;
            case 3:
                event_array[currMode][2] = event_array[currMode][2] + 1;
                break;
            case 4:
                if (event_array[currMode][3])
                    event_array[currMode][3] = 0;
                else
                    event_array[currMode][3] = 1;
            default:
                cursor = 1;
                break;
            }

            if (event_array[currMode][2] > 59)
                event_array[currMode][2] = 0;
            if (event_array[currMode][1] > 59)
                event_array[currMode][1] = 0;
            if (event_array[currMode][0] > 23)
                event_array[currMode][0] = 0;
        }
    }

    if (digitalRead(bt_minus) == 1 && setAlarm == 1)
    {
        if (currLine == 0)
        {
            switch (cursor)
            {
            case 1:
                hh = hh - 1;
            case 2:
                mm = mm - 1;
            case 3:
                ss = ss - 1;

                if (hh < 0)
                    hh = 23;
                if (mm < 0)
                    mm = 59;
                if (ss < 0)
                    ss = 59;
            }
        }
        else if (currLine == 1)
        {
            switch (cursor)
            {
            case 1:
                count_Medicine[0] = count_Medicine[0] - 1;
                break;
            case 2:
                count_Medicine[1] = count_Medicine[1] - 1;
                break;
            case 3:
                count_Medicine[2] = count_Medicine[2] - 1;
                break;
            case 4:
                count_Medicine[3] = count_Medicine[3] - 1;
                break;
            default:
                cursor = 1;
                break;

                for (int i = 0; i < 4; i++)
                {
                    if (count_Medicine[i] < 0)
                    {
                        count_Medicine[i] = 99;
                    }
                }
            }
        }
        else if (currLine == 2)
        {
            switch (cursor)
            {
            case 1:
                currMode = currMode + 1;
                break;
            case 2:
                if (mode_array[currMode][0] == 1)
                    mode_array[currMode][0] = 0;
                else
                    mode_array[currMode][0] = 1;
                break;
            case 3:
                if (mode_array[currMode][1] == 1)
                    mode_array[currMode][1] = 0;
                else
                    mode_array[currMode][1] = 1;
                break;
            case 4:
                if (mode_array[currMode][2] == 1)
                    mode_array[currMode][2] = 0;
                else
                    mode_array[currMode][2] = 1;
                break;
            case 5:
                if (mode_array[currMode][3] == 1)
                    mode_array[currMode][3] = 0;
                else
                    mode_array[currMode][3] = 1;
            default:
                cursor = 1;
                break;
            }

            if (currMode > 3)
                currMode = 0;
        }
        else if (currLine == 3)
        {
            switch (cursor)
            {
            case 1:
                event_array[currMode][0] = event_array[currMode][0] - 1;
                break;
            case 2:
                event_array[currMode][1] = event_array[currMode][1] - 1;
                break;
            case 3:
                event_array[currMode][2] = event_array[currMode][2] - 1;
                break;
            case 4:
                if (event_array[currMode][3])
                    event_array[currMode][3] = 0;
                else
                    event_array[currMode][3] = 1;
            default:
                cursor = 1;
                break;
            }

            if (event_array[currMode][2] < 0)
                event_array[currMode][2] = 59;
            if (event_array[currMode][1] < 0)
                event_array[currMode][1] = 59;
            if (event_array[currMode][0] < 0)
                event_array[currMode][0] = 23;
        }
    }
}

void checkTime(int HH, int MM, int SS, int mode, int Event[], int prevEvent[], int index)
{
    if (setAlarm == 1)
        return;

    if (hh == HH && mm == MM && ss == SS && mode == 1)
    {
        currFlag[index] = 1;
        for (int i = 0; i < 4; i++)
        {
            if (index == 0)
            {
                if (!flags[3])
                {
                    digitalWrite(Blue_LED[i], LOW);
                    digitalWrite(Green_LED[i], LOW);
                }
            }
            else if (flags[index - 1])
            {
                if (prevEvent[i])
                {
                    digitalWrite(Blue_LED[i], HIGH);
                }
                digitalWrite(Green_LED[i], LOW);
            }

            digitalWrite(Green_LED[i], Event[i]);
        }

        flags[index] = true;

        if (count(Event) >= 1 && mode == 1)
        {
            digitalWrite(buzzer, HIGH);
            delay(10000);
            digitalWrite(buzzer, LOW);
        }
    }
}

void runBuzzer(int HH, int MM, int SS, int index)
{
    if (setAlarm == 1)
        return;

    if (!flags[index])
    {
        return;
    }
    if (hh == HH && mm == MM && ss == SS)
    {
        digitalWrite(buzzer, HIGH);
        delay(10000);
        digitalWrite(buzzer, LOW);
    }
}

void ReadEeprom()
{
    for (int i = 0; i < 4; i++)
    {
        Morning_Alarm[i] = EEPROM.read(i + 1);
        Afternoon_Alarm[i] = EEPROM.read(i + 5);
        Evening_Alarm[i] = EEPROM.read(i + 9);
        Night_Alarm[i] = EEPROM.read(i + 13);
        Morning[i] = EEPROM.read(i + 17);
        Afternoon[i] = EEPROM.read(i + 21);
        Evening[i] = EEPROM.read(i + 25);
        Night[i] = EEPROM.read(i + 29);
        count_Medicine[i] = EEPROM.read(i + 33);
        flags[i] = EEPROM.read(i + 37);
        currFlag[i] = EEPROM.read(i + 41);
    }

    Medicine_threshold = EEPROM.read(45);
    Distance_threshold = EEPROM.read(46);
}

void WriteEeprom()
{
    for (int i = 0; i < 4; i++)
    {
        EEPROM.write(1 + i, Morning_Alarm[i]);
        EEPROM.write(5 + i, Afternoon_Alarm[i]);
        EEPROM.write(9 + i, Evening_Alarm[i]);
        EEPROM.write(13 + i, Night_Alarm[i]);
        EEPROM.write(17 + i, Morning[i]);
        EEPROM.write(21 + i, Afternoon[i]);
        EEPROM.write(25 + i, Evening[i]);
        EEPROM.write(29 + i, Night[i]);
        EEPROM.write(33 + i, count_Medicine[i]);
        EEPROM.write(37 + i, flags[i]);
        EEPROM.write(41 + i, currFlag[i]);
    }

    EEPROM.write(45, Medicine_threshold);
    EEPROM.write(46, Distance_threshold);
    //   EEPROM.write(4, mode);
}
