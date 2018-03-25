#include <IRremote.h>
#include <Wire.h> /*i2c interface*/
#include <LiquidCrystal.h>
LiquidCrystal lcd(7,6,13,12,11,10);
#include <SimpleTimer.h>
SimpleTimer timer;
#include "DHT.h"            
 
const int DHTPIN = 2;       //Đọc dữ liệu từ DHT11 ở chân 2 trên mạch Arduino
const int DHTTYPE = DHT11;  //Khai báo loại cảm biến, có 2 loại là DHT11 và DHT22
 
DHT dht(DHTPIN, DHTTYPE);

enum {Main_Screen,Timer_Mode, Change_Hour, Change_Minute, Change_Second, Change_Day, Change_Date, Change_Month, Change_Year, Fix_Time};

int IRpin = 8;
IRrecv irrecv(IRpin);
decode_results results;

#define BAT_QUAT 5322
#define TAT_QUAT 17050
#define BAT_CB 28810
#define TAT_CB 24922
#define MENU 19375


#define  EXIT 23882 
#define  UP  4282
#define DOWN 16010 
#define BACK 27770 
#define SELECT 18335
#define NO_SELECT 111

int IR ;

#define Alert 4
#define Fan 9
#define MQ2 A0
int gio, phut, giay, thu, ngay, thang, nam; //set timer
int _second, _minute, _hour, _day, _date, _month, _year;// read time
int hour1_Set, Min1_Set; //set timer motor
int End_hour1_Set , End_Min1_Set ;
bool Fan_Status = false; // in order to set timer
#define ON_TIME 0// in order to set timer 
#define OFF_TIME 1// in order to ACTIVE MOTOR OFF
#define virtual_minute 100
#define virtual_hour 30

int btn_check;
int menu_count = 0;
int _menu;
static int change = 0;
float t;//temp
byte degree[8] = {
  B11110,
  B10010,
  B10010,
  B11110,
  B00000,
  B00000,
  B00000,
  B00000
};  // Create degree Celcius character

void setup() {
  // put your setup code here, to run once:

      lcd.begin(16, 2);
 irrecv.enableIRIn(); // bắt đầu nhận tín hiệu
  Wire.begin();
  dht.begin();         // Khởi động cảm biến
  lcd.createChar(1, degree); // custom character
 timer.setInterval(500L, _update);// update data sensor after each 0,5s
 pinMode(Alert , OUTPUT);
 pinMode(Fan,OUTPUT);
 digitalWrite(Alert,LOW);
 digitalWrite(Fan,HIGH);// off fan triac muc cao
   display_lcd();
   _update();
}

void loop() {
  // put your main code here, to run repeatedly:
  timer.run();

  if(irrecv.decode(&results))
  { 
     IR = results.value;
       check_button();
      
  irrecv.resume();   // nhận tiếp tục 

  }
    timer1();
    warning();
     t = dht.readTemperature(); //Đọc nhiệt độ
}


int btn_read()
{
  if (IR == UP) 
    return 4282;
  else if (IR == DOWN)
   return 16010 ;
  else if (IR == BACK)
    return 27770;
  else if (IR == EXIT)
    return 23882;
  else if (IR == SELECT)
    return 18335;
    else if(IR == BAT_QUAT)
    {
      digitalWrite(Fan,LOW);
      return BAT_QUAT;
    }
    else if(IR == TAT_QUAT)
    {
       digitalWrite(Fan,HIGH);
    return TAT_QUAT;
    }
    else if(IR == BAT_CB)
    {
     change =change +1;
    return BAT_CB;
    }
    else if(IR == TAT_CB)
    {
   change=change-1;
    return TAT_CB;
    }
    else if(IR == MENU)
    {
      menu_count=menu_count+1;
    return MENU;
    }
    else
      return NO_SELECT;
  }

int menu()
{
 if (IR == UP)
  {
    lcd.clear();
    menu_count++;
    if (menu_count > 9) menu_count = 0;

  
  }
  else if (IR == DOWN)
  {
    lcd.clear();
    menu_count--;
    if (menu_count <= 0) menu_count = 9;
  
  }
  else if (IR == EXIT)
  {
    lcd.clear();
    menu_count = 0;
 
  }
  return menu_count;
  }


int dec2bcd(byte num)
{
  return ((num / 10) * 16 + (num % 10));
}
int bcd2dec(byte num)
{
  return ((num / 16) * 10 + (num % 16));
}
//-----------------
void settime(byte _hour_, byte _minute_, byte _second_, byte _day_, byte _date_, byte _month_, byte _year_)
{
  Wire.beginTransmission(0x68);// 0x68 :  ds3231 Slave'address
  Wire.write(0);// set pointer at second register
  Wire.write(dec2bcd(_second_));
  Wire.write(dec2bcd(_minute_));
  Wire.write(dec2bcd(_hour_));
  Wire.write(dec2bcd(_day_));
  Wire.write(dec2bcd(_date_));
  Wire.write(dec2bcd(_month_));
  Wire.write(dec2bcd(_year_));
  Wire.endTransmission();
}
//----------------------
void readTime()
{
  Wire.beginTransmission(0x68);
  Wire.write(0);// set pointer at second register
  Wire.endTransmission();
  Wire.requestFrom(0x68, 7);
  _second = bcd2dec(Wire.read() & 0x7f); //remove bits not used
  _minute = bcd2dec(Wire.read() & 0x7f);
  _hour = bcd2dec(Wire.read() & 0x3f); // 24h mode
  _day = bcd2dec(Wire.read() & 0x07);
  _date = bcd2dec(Wire.read() & 0x3f);
  _month = bcd2dec(Wire.read() & 0x1f);
  _year = bcd2dec(Wire.read());
  _year = _year + 2000;
}

void display_lcd()
{
  if ((_minute == 0) && (_second == 0)) //after 1 hour - clear cld
  {
    lcd.clear();
  }

  lcd.setCursor(0, 0);
  switch (_day)
  {
    case 1: lcd.print("Sun"); break;
    case 2: lcd.print("Mon"); break;
    case 3: lcd.print("Tue"); break;
    case 4: lcd.print("Wed"); break;
    case 5: lcd.print("Thur"); break;
    case 6: lcd.print("Fri"); break;
    case 7: lcd.print("Sat"); break;
  }

  lcd.setCursor(6, 0);
  lcd.print(_date);
  lcd.print("/");
  lcd.print(_month);
  lcd.print("/");
  lcd.print(_year);
  lcd.setCursor(0, 1);
  lcd.print(_hour);
  lcd.print(":");
  lcd.print(_minute);
//   lcd.print(":");
//  lcd.print(_second);

  //-----dht
  lcd.setCursor(9, 1);
  lcd.print(t);
  lcd.setCursor(13, 1);
  lcd.write(1);
 /* lcd.setCursor(13, 1);
  lcd.print(h);
  lcd.print("%");
*/

}

void _update()
{
    readTime();
    // float h = dht.readHumidity();    //Đọc độ ẩm
  
  }

void fan_status1()
{

  int x = digitalRead(Fan);
  if (x == 0)
  {
    lcd.setCursor(0, 0);
    lcd.print("Fan is being ON ");
  }
  else if (x == 1)
  {
    lcd.setCursor(0, 0);
    lcd.print("Fan is being OFF");
  }
}

void change_day()
{
  switch (change)
  {
    case 1:
      lcd.setCursor(0, 0);
      lcd.print("Sunday"); break;
    case 2:
      lcd.setCursor(0, 0);
      lcd.print("Monday"); break;
    case 3:
      lcd.setCursor(0, 0);
      lcd.print("Tuesday"); break;
    case 4:
      lcd.setCursor(0, 0);
      lcd.print("Wednesday"); break;
    case 5:
      lcd.setCursor(0, 0);
      lcd.print("Thursday"); break;
    case 6:
      lcd.setCursor(0, 0);
      lcd.print("Friday"); break;
    case 7:
      lcd.setCursor(0, 0);
      lcd.print("Saturday"); break;
  }
}
void change_month()
{
  switch (change)
  {
    case 1:
      lcd.setCursor(0, 0);
      lcd.print("January");
      lcd.setCursor(13, 1);
      lcd.print(change);
      break;
    case 2:
      lcd.setCursor(0, 0);
      lcd.print("February");
      lcd.setCursor(13, 1);
      lcd.print(change);
      break;
    case 3:
      lcd.setCursor(0, 0);
      lcd.print("March");
      lcd.setCursor(13, 1);
      lcd.print(change);
      break;
    case 4:
      lcd.setCursor(0, 0);
      lcd.print("April");
      lcd.setCursor(13, 1);
      lcd.print(change);
      break;
    case 5:
      lcd.setCursor(0, 0);
      lcd.print("May");
      lcd.setCursor(13, 1);
      lcd.print(change);
      break;
    case 6:
      lcd.setCursor(0, 0);
      lcd.print("June");
      lcd.setCursor(13, 1);
      lcd.print(change);
      break;
    case 7:
      lcd.setCursor(0, 0);
      lcd.print("July");
      lcd.setCursor(13, 1);
      lcd.print(change);
      break;
    case 8:
      // lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("August");
      lcd.setCursor(13, 1);
      lcd.print(change);
      break;
    case 9:
      lcd.setCursor(0, 0);
      lcd.print("September");
      lcd.setCursor(13, 1);
      lcd.print(change);
      break;
    case 10:
      lcd.setCursor(0, 0);
      lcd.print("October");
      lcd.setCursor(13, 1);
      lcd.print(change);
      break;
    case 11:
      lcd.setCursor(0, 0);
      lcd.print("November");
      lcd.setCursor(13, 1);
      lcd.print(change);
      break;
    case 12:
      lcd.setCursor(0, 0);
      lcd.print("December");
      lcd.setCursor(13, 1);
      lcd.print(change);
      break;
  }
}
void timer1()
{
  if ((hour1_Set == _hour) && (Min1_Set == _minute))
  {
    digitalWrite(Fan, ON_TIME);
    hour1_Set = virtual_hour; 
    Min1_Set = virtual_minute;
  }
   if ((End_hour1_Set == _hour) && (End_Min1_Set == _minute))
  {
    digitalWrite(Fan,OFF_TIME); 
    End_hour1_Set = virtual_hour; 
    End_Min1_Set = virtual_minute;
  }
}

void check_button()
{
    static byte isPress = false;
  static bool isFirst = true;
 static bool smart = false;
  static bool isMenuChild = false;
  int healer ;

   btn_check = btn_read();
  if (isMenuChild == false) {
    _menu = menu(); //_menu <=> btn
  }

switch(_menu)
{
  case Main_Screen:
      display_lcd();
      break;

 /*-----------gap---------------*/
    case Timer_Mode:
      if (smart == false) {
        if (btn_check ==SELECT) {
          smart = true;
          healer = btn_check;
        }
        lcd.setCursor(0, 0);
        lcd.print("Timer Mode");
      }
      else {
        if (btn_check != NO_SELECT) {
          healer = btn_check;
        }
        switch (healer) {
          case UP:
            hour1_Set = gio;
            Min1_Set = phut;
            Fan_Status = digitalRead(Fan); //check device status
            lcd.setCursor(0, 1);
            lcd.print("Set Time Start");
            break;
          case DOWN:
        
            End_hour1_Set = gio;
            End_Min1_Set = phut;
            Fan_Status = digitalRead(Fan); //check device status
            lcd.setCursor(0, 1);
            lcd.print("Set Time End  ");
            break;
          case BACK:
            break;
          case EXIT:
            lcd.clear();
            isMenuChild = false;
            smart = false;
            break;
          case SELECT:
            fan_status1();
            isMenuChild = true;
            break;
        }//switch child case 2
      }//else case 2
      break;
    
    /*-----------gap---------------*/
      
   case Change_Hour:
  if(smart == false){ 
     if (btn_check == SELECT){
     smart = true;
     healer = btn_check;
     }
     lcd.setCursor(0,0);
     lcd.print("Change Hour");
  }
  else {
    if(btn_check !=NO_SELECT){
       healer = btn_check;
       isPress = true;
    }
    switch(healer){
      case UP:
      if(isPress){
        change++;
        lcd.clear();
        isPress= false;
      } 
      if(change > 23) {
        lcd.clear();
        change =0;
      }
      lcd.setCursor(0,0);
      lcd.print("Hour :");
      lcd.setCursor(7,0);
      lcd.print(change);
      break;
      case DOWN:
      if(isPress){
        change--;
        lcd.clear();
        isPress = false;
      }
      if(change < 0){
        lcd.clear();
        change = 23;
      }
       lcd.setCursor(0, 0);
            lcd.print("Hour :");
            lcd.setCursor(7, 0);
            lcd.print(change);
            break;
       case BACK:
       gio = change;
       lcd.setCursor(0,1);
       lcd.print("Completed");
       break;
       case EXIT:
       lcd.clear();
       isMenuChild = false;
       isFirst = true;
       smart = false;
       break;
       case SELECT:
       if(isFirst){
        lcd.clear();
        isFirst = false;
       }
              change = _hour;
            lcd.setCursor(0, 0);
            lcd.print("Hour :");
            lcd.setCursor(7, 0);
            lcd.print(change);
            isMenuChild = true;
            break;
      
    }//healer
  }//else
  break;
  /*--------------------*/
    /*------------gap---------------*/
    case Change_Minute:
      if (smart == false) {
        if (btn_check ==SELECT) {
          smart = true;
          healer = btn_check;
        }
        lcd.setCursor(0, 0);
        lcd.print("Change Minute");
      }
      else {
        if (btn_check != NO_SELECT) {
          healer = btn_check;
          isPress = true;
        }
        switch (healer) {
          case UP:
            if (isPress) {
              change++;
              lcd.clear();
              isPress = false;
            }
            if (change > 59) {
              lcd.clear();
              change = 0;
            }
            lcd.setCursor(0, 0);
            lcd.print("Minute :");
            lcd.setCursor(8, 0);
            lcd.print(change);
            break;
          case DOWN:
            if (isPress) {
              change--;
              lcd.clear();
              isPress = false;
            }
            if (change < 0) {
              lcd.clear();
              change = 59;
            }
            lcd.setCursor(0, 0);
            lcd.print("Minute :");
            lcd.setCursor(8, 0);
            lcd.print(change);
            break;
          case BACK:
            phut = change;
            lcd.setCursor(0, 1);
            lcd.print("Completed");
            break;
          case EXIT:
            lcd.clear();
            isMenuChild = false;
            isFirst = true;
            smart = false;
            break;
          case SELECT:
            if (isFirst) {
              lcd.clear();
              isFirst = false;
            }
            change = _minute;
            lcd.setCursor(0, 0);
            lcd.print("Minute :");
            lcd.setCursor(8, 0);
            lcd.print(change);
            isMenuChild = true;
            break;
        }//switch child case 3
      }//else case 3
      break;
    /*------------gap---------------*/
    case Change_Second:
      if (smart == false) {
        if (btn_check ==SELECT) {
          smart = true;
          healer = btn_check;
        }
        lcd.setCursor(0, 0);
        lcd.print("Change Second");
      }
      else {
        if (btn_check != NO_SELECT) {
          healer = btn_check;
          isPress = true;
        }
        switch (healer) {
          case UP:
            if (isPress) {
              change++;
              lcd.clear();
              isPress = false;
            }
            if (change > 59) {
              lcd.clear();
              change = 0;
            }
            lcd.setCursor(0, 0);
            lcd.print("Second :");
            lcd.setCursor(8, 0);
            lcd.print(change);
            break;
          case DOWN:
            if (isPress) {
              change--;
              lcd.clear();
              isPress = false;
            }
            if (change < 0) {
              lcd.clear();
              change = 59;
            }
            lcd.setCursor(0, 0);
            lcd.print("Second :");
            lcd.setCursor(8, 0);
            lcd.print(change);
            break;
          case BACK:
            giay = change;
            lcd.setCursor(0, 1);
            lcd.print("Completed");
            break;
          case EXIT:
            lcd.clear();
            isMenuChild = false;
            smart = false;
            isFirst = true;
            break;
          case SELECT:
            if (isFirst) {
              lcd.clear();
              isFirst = false;
            }
            change = 0;
            lcd.setCursor(0, 0);
            lcd.print("Second :");
            lcd.setCursor(8, 0);
            lcd.print(change);
            isMenuChild = true;
            break;
        }//switch child case 3
      }//else case 3
      break;
    /*------------gap---------------*/
    case Change_Day:
      if (smart == false) {
        if (btn_check ==SELECT) {
          smart = true;
          healer = btn_check;
        }
        lcd.setCursor(0, 0);
        lcd.print("Change Day");
      }
      else {
        if (btn_check != NO_SELECT) {
          healer = btn_check;
          isPress = true;
        }
        switch (healer) {
          case UP:
            if (isPress) {
              change++;
              lcd.clear();
              isPress = false;
            }
            if (change > 7) {
              lcd.clear();
              change = 1;
            }
            change_day();
            break;
          case DOWN:
            if (isPress) {
              change--;
              isPress = false;
            }
            if (change < 1) {
              lcd.clear();
              change = 7;
            }
            change_day();
            break;
          case BACK:
            thu = change;
            lcd.setCursor(0, 1);
            lcd.print("Completed");
            break;
          case EXIT:
            lcd.clear();
            isMenuChild = false;
            smart = false;
            isFirst = true;
            break;
          case SELECT:
            if (isFirst) {
              lcd.clear();
              isFirst = false;
            }
            change = _day;
            change_day();
            isMenuChild = true;
            break;
        }//switch child case 3
      }//else case 3
      break;
    /*------------gap---------------*/
    case Change_Date:
      if (smart == false) {
        if (btn_check ==SELECT ) {
          smart = true;
          healer = btn_check;
        }
        lcd.setCursor(0, 0);
        lcd.print("Change Date");
      }
      else {
        if (btn_check != NO_SELECT) {
          healer = btn_check;
          isPress = true;
        }
        switch (healer) {
          case UP:
            if (isPress) {
              change++;
              lcd.clear();
              isPress = false;
            }
            if (change > 31) {
              lcd.clear();
              change = 0;
            }
            lcd.setCursor(0, 0);
            lcd.print("Date :");
            lcd.setCursor(7, 0);
            lcd.print(change);
            break;
          case DOWN:
            if (isPress) {
              change--;
              lcd.clear();
              isPress = false;
            }
            if (change < 0) {
              lcd.clear();
              change = 31;
            }
            lcd.setCursor(0, 0);
            lcd.print("Date :");
            lcd.setCursor(7, 0);
            lcd.print(change);
            break;
          case BACK:
            ngay = change;
            lcd.setCursor(0, 1);
            lcd.print("Completed");
            break;
          case EXIT:
            lcd.clear();
            isMenuChild = false;
            smart = false;
            isFirst = true;
            break;
          case SELECT:
            if (isFirst) {
              lcd.clear();
              isFirst = false;
            }
            change = _date;
            lcd.setCursor(0, 0);
            lcd.print("Date :");
            lcd.setCursor(7, 0);
            lcd.print(change);
            isMenuChild = true;
            break;
        }//switch child case 3
      }//else case 3
      break;
    /*------------gap---------------*/
    case Change_Month:
      if (smart == false) {
        if (btn_check == SELECT) {
          smart = true;
          healer = btn_check;
        }
        lcd.setCursor(0, 0);
        lcd.print("Change Month");
      }
      else {
        if (btn_check != NO_SELECT) {
          healer = btn_check;
          isPress = true;
        }
        switch (healer) {
          case UP:
            if (isPress) {
              change++;
              lcd.clear();
              isPress = false;
            }
            if (change > 12) {
              lcd.clear();
              change = 1;
            }
            change_month();
            break;
          case DOWN:
            if (isPress) {
              change--;
              lcd.clear();
              isPress = false;
            }
            if (change < 1) {
              lcd.clear();
              change = 12;
            }
            change_month();
            break;
          case BACK:
            thang = change;
            lcd.setCursor(0, 1);
            lcd.print("Completed");
            break;
          case EXIT:
            lcd.clear();
            isMenuChild = false;
            smart = false;
            isFirst = true;
            break;
          case SELECT:
            if (isFirst) {
              lcd.clear();
              isFirst = false;
            }
            change = _month;
            change_month();
            isMenuChild = true;
            break;
        }//switch child case 3
      }//else case 3
      break;
    /*------------gap---------------*/
    case Change_Year:
      if (smart == false) {
        if (btn_check ==SELECT ) {
          smart = true;
          healer = btn_check;
        }
        lcd.setCursor(0, 0);
        lcd.print("Change Year");
      }
      else {
        if (btn_check != NO_SELECT) {
          healer = btn_check;
          isPress = true;
        }
        switch (healer) {
          case UP:
            if (isPress) {
              change++;
              lcd.clear();
              isPress = false;
            }
            lcd.setCursor(0, 0);
            lcd.print("Year :");
            lcd.setCursor(7, 0);
            lcd.print(change);
            break;
          case DOWN:
            if (isPress) {
              change--;
              lcd.clear();
              isPress = false;
            }
            if (change < 0) {
              lcd.clear();
              change = _year;
            }
            lcd.setCursor(0, 0);
            lcd.print("Year :");
            lcd.setCursor(7, 0);
            lcd.print(change);
            break;
          case BACK:
            nam = change;
            lcd.setCursor(0, 1);
            lcd.print("Completed");
            break;
          case EXIT:
            lcd.clear();
            isMenuChild = false;
            smart = false;
            isFirst = true;
            break;
          case SELECT:
            if (isFirst) {
              lcd.clear();
              isFirst = false;
            }
            change = _year;
            lcd.setCursor(0, 0);
            lcd.print("Year :");
            lcd.setCursor(7, 0);
            lcd.print(change);
            isMenuChild = true;
            break;
        }//switch child case 3
      }//else case 3
      break;
    /*------------gap---------------*/
    case Fix_Time:
      if (smart == false) {
        if (btn_check == SELECT)
        {
          smart = true;
          healer = btn_check;
        }
        lcd.setCursor(0, 0);
        lcd.print("Fix Time");
      }
      else {
        if (btn_check != NO_SELECT)
        {
          healer = btn_check;
        }
        switch (healer)
        {
          case EXIT:
            lcd.clear();
            isMenuChild = false;
            smart = false;
            break;
          case SELECT:
            //set time
            settime(gio, phut, giay, thu, ngay, thang, nam - 48);
            lcd.setCursor(0, 1);
            lcd.print("Completed");
            isMenuChild = true;
            break;
        }
      } break;
  /*---------------------*/
  }// switch  
  }//void check_btn  


  void warning()
  {
    int Alert_value = analogRead(A1);   //đọc giá trị điện áp ở chân A0 - chân cảm biến
                                //(value luôn nằm trong khoảng 0-1023)
                                
  if(Alert_value < 500)
    digitalWrite(Alert,LOW);
  if(Alert_value > 800)
        digitalWrite(Alert,HIGH);
    else digitalWrite(Alert,LOW);
    }
