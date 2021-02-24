#include <Arduino.h>
#include <U8g2lib.h>
#include <ClosedCube_SHT31D.h>
#include <SoftwareSerial.h>

#include "wifi.h"
#include "mcu_api.h"

#define SHT30_addr 0x44 // SHT30地址
#define shift      8    // OLED位置偏移量

//LED引脚
#define OrangePin 7
#define RedPin    8
#define GreenPin  9
#define BluePin   10

//类实例化
U8G2_SSD1306_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0,U8X8_PIN_NONE,19,18); //A5->19(SCK),A4->18(SDA),两页缓存
ClosedCube_SHT31D sht30;
SoftwareSerial MySerial(12, 11); //RX, TX--调试

float temp = 11,humi = 20;    //温度，湿度

//函数声明
void SHT30ReadData(SHT31D result); 
void LEDs(SHT31D result);
void Uart_PutChar(unsigned char value);
void Uart_Receive();

void setup(void){

  sht30.begin(SHT30_addr);              //SHT30初始化
  u8g2.begin();                         //u8g2初始化  
  u8g2.enableUTF8Print();               //编码设置
  u8g2.setFont(u8g2_font_u8glib_4_tr);  //设置字体
  
  //LED初始化
  pinMode(BluePin,  OUTPUT);            //Blue
  pinMode(GreenPin, OUTPUT);            //Green
  pinMode(RedPin,   OUTPUT);            //Red
  pinMode(OrangePin,OUTPUT);            //Orange
  
  wifi_protocol_init();                 //Tuya-WiFi模块初始化

//  mcu_set_wifi_mode(SMART_CONFIG);
  
  Serial.begin(19200);                  //tuya串口初始化 （由于8MHz晶振，实际波特率9600）
  MySerial.begin(19200);                //调试串口初始化 （由于8MHz晶振，实际波特率9600）
}

void loop(void)
{ 
  u8g2.firstPage();        // 标志图像循环的开始
  do{
    u8g2.setCursor(24,14);  u8g2.print("温湿度监测");
    u8g2.setCursor(8,32);   u8g2.print("温度：");
    u8g2.setCursor(96,32);  u8g2.print("℃");
    u8g2.setCursor(8,48);   u8g2.print("湿度：");
    u8g2.setCursor(96,48);  u8g2.print("％");

    SHT30ReadData(sht30.readTempAndHumidity(SHT3XD_REPEATABILITY_HIGH, SHT3XD_MODE_POLLING, 25));   //数据获取
  }while(u8g2.nextPage());  // 标志图像循环的结束
  
  wifi_uart_service();      //Wifi串口中断处理
  Uart_Receive();

  delay(100);
}

void SHT30ReadData(SHT31D result) {
    if (result.error == SHT3XD_NO_ERROR) {
      temp = result.t;      //温度数据
      humi = result.rh;     //湿度数据 
      u8g2.setCursor(48+shift/2,32);  u8g2.print(result.t);
      u8g2.setCursor(48+shift/2,48);  u8g2.print(result.rh);
      LEDs(result);
    } else {
      u8g2.setCursor(48+shift/2,32);  u8g2.print("//");
      u8g2.setCursor(48+shift/2,48);  u8g2.print("//");
      u8g2.setCursor(0+shift,64);     u8g2.print("Error->");
      u8g2.setCursor(56+shift,64);    u8g2.print(result.error);
    }
}

//LED指示灯控制
void LEDs(SHT31D result){
    //湿度状态指示
    if(result.rh>=75.0){
      digitalWrite(BluePin,1);  digitalWrite(GreenPin,1);
      u8g2.setCursor(80,64);   u8g2.print("WET!!!");
    }else if(result.rh>=60.0){
      digitalWrite(BluePin,1);  digitalWrite(GreenPin,0);
      u8g2.setCursor(80,64);   u8g2.print(F("Wet!"));
    }else if (result.rh<60.0){
      digitalWrite(BluePin,0);  digitalWrite(GreenPin,1);
    }
    
    //温度状态指示
    if(result.t>=32.0){
      digitalWrite(RedPin,1); digitalWrite(OrangePin,1);
      u8g2.setCursor(0,64);   u8g2.print("HOT!!!");
    }else if(result.t>=27.0){
      digitalWrite(RedPin,1); digitalWrite(OrangePin,0);
      u8g2.setCursor(0,64);   u8g2.print("Hot!");
    }else if(result.t<27.0){
      digitalWrite(RedPin,0); digitalWrite(OrangePin,1);
    }

    if(result.rh<60.0 && result.t<27.0){
      u8g2.setCursor(48,64);  u8g2.print("Nice!");
    }
}

// 串口发送
void Uart_PutChar(unsigned char value){
  Serial.write(value);
}

// 串口接收
void Uart_Receive(){
  if(Serial.available()){
      unsigned char  value = (unsigned char)Serial.read();
      uart_receive_input(value);
//      MySerial.write(value); 
  }
}
