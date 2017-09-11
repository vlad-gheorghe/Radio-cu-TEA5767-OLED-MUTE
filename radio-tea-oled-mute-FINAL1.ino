/*********************************************************************
Radio TEA 5767, WITH SEARCH UP-DOWN AND MUTE ON-OFF
Adaptat dupa http://blog.simtronyx.de/"
*********************************************************************/

#include <SPI.h>
#include <Button.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

Button btn_forward(2, PULLUP);
Button btn_backward(3, PULLUP);
Button mute1(4, PULLUP);

#define TEA5767_mute_left_right  0x06
#define TEA5767_MUTE_FULL        0x80
#define TEA5767_ADC_LEVEL_MASK   0xF0
#define TEA5767_STEREO_MASK      0x80

unsigned long last_pressed;

int old_frequency=-1;
int frequency=10310;

byte old_stereo=0;
byte stereo=1;

byte old_mute=1;
byte mute=0;

byte old_signal_level=1;
byte signal_level=0;

void setup()   {                
 Wire.begin();
  TEA5767_set_frequency();
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
}

void loop() {
unsigned long current_millis = millis();
    if(frequency!=old_frequency){
   
    
      set_text(old_frequency>=10000?15:29,0,value_to_string(old_frequency),BLACK,2);
      set_text(frequency>=10000?15:29,0,value_to_string(frequency),WHITE,2);
       display.setTextColor(WHITE); display.setCursor(90,4); display.setTextSize(1); display.print(" MHz");
      old_frequency=frequency;
    }
    
    TEA5767_read_data();
      
    if(old_stereo!=stereo){ 
        set_text(old_stereo?0:20,16,old_stereo?"Stereo":"Mono",BLACK,1);
        set_text(stereo?0:20,16,stereo?"Stereo":"Mono",WHITE,1);
        old_stereo=stereo;
    }
    
    if(old_signal_level!=signal_level){
        set_text(old_signal_level<10?90:83,21,String((int)old_signal_level),BLACK,1);
        set_text(signal_level<10?90:83,21,String((int)signal_level),WHITE,1);
         display.setCursor(100,21);display.setTextSize(1);display.setTextColor(WHITE);
         display.print("/15");
        old_signal_level=signal_level;
        show_signal_level(signal_level);
    }
    
    if(old_mute!=mute){
        set_text(0,25,old_mute?"Mute":"Sound ON",BLACK,1);
        set_text(0,25,mute?"Mute":"Sound ON",WHITE,1);
        old_mute=mute;
    }
      
    delay(50);
    
  
 if (btn_forward.isPressed()) {
    last_pressed = current_millis;
    
    frequency=frequency-5;
    if(frequency<8750)frequency=10800;
    TEA5767_set_frequency();
  }
  if (btn_backward.isPressed()) {
    last_pressed = current_millis;
    
    frequency=frequency+5;
    if(frequency>10800)frequency=8750;
    TEA5767_set_frequency();
  }

  if (mute1.isPressed()) {
    last_pressed = current_millis;
    
    TEA5767_mute();
  }
  
  delay(50);
  
}

unsigned char frequencyH = 0;
unsigned char frequencyL = 0;

unsigned int frequencyB;

unsigned char TEA5767_buffer[5]={0x00,0x00,0xB0,0x10,0x00};

void TEA5767_write_data(byte data_size){
   
  delay(50);
  
  Wire.beginTransmission(0x60);
  
  for(byte i=0;i<data_size;i++)
    Wire.write(TEA5767_buffer[i]);
  
  Wire.endTransmission();
  
  delay(50);
}

void TEA5767_mute(){ 
  
  if(!mute){   
    mute = 1;   
    TEA5767_buffer[0] |= TEA5767_MUTE_FULL;
    TEA5767_write_data(2);

  }   
  else{
    mute = 0;   
    TEA5767_buffer[0] &= ~TEA5767_MUTE_FULL;
    TEA5767_write_data(2);

  }

}

void TEA5767_set_frequency()
{
  frequencyB = 4 * (frequency * 10000 + 225000) / 32768;
  TEA5767_buffer[0] = frequencyB >> 8;
  if(mute)TEA5767_buffer[0] |= TEA5767_MUTE_FULL;
  TEA5767_buffer[1] = frequencyB & 0XFF;
  
  TEA5767_write_data(5);
}

int TEA5767_read_data() {
  
  unsigned char buf[5];
  memset (buf, 0, 5);
  
  Wire.requestFrom (0x60, 5); 

  if (Wire.available ()) {
    for (int i = 0; i < 5; i++) {
      buf[i] = Wire.read ();
    }
        
    stereo = (buf[2] & TEA5767_STEREO_MASK)?1:0;
    signal_level = ((buf[3] & TEA5767_ADC_LEVEL_MASK) >> 4);
    
    return 1;
  } 
  else return 0;
}

void show_signal_level(int level){
  
  byte xs=62;
  byte ys=30;
  for(int i=0;i<15;i++){
    if(i%2!=0)display.drawLine(xs+i,ys,xs+i,ys-i,level>=i?WHITE:BLACK);
  }
}

void set_text(int x,int y,String text,int color,int textsize){
  
  display.setTextSize(textsize);
  display.setTextColor(color); 
  display.setCursor(x,y);     
  display.println(text);      
  display.display();         
}


String value_to_string(int value){
  
  String value_string = String(value / 100);
  value_string = value_string + '.' + ((value%100<10)?"0":"") + (value % 100);
  return value_string;
}


