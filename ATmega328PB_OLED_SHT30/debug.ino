#include <Arduino.h>
#include <SoftwareSerial.h>

#include "wifi.h"
#include "mcu_api.h"

//LED引脚
#define GreenPin  9

SoftwareSerial MySerial(12, 11); //RX, TX--调试

float temp = 15;    //测试温度数据
float humi = 35;    //测试湿度数据

void setup(void)
{  
  pinMode(GreenPin, OUTPUT);            //Green

  wifi_protocol_init();                 //Tuya-WiFi模块初始化 
  Serial.begin(9600);
  MySerial.begin(9600);                 //调试串口初始化
}

void loop(void)
{ 
  wifi_uart_service();              //Wifi串口中断处理
  
  if(MySerial.available()){
    unsigned char  value = (unsigned char)MySerial.read();
    uart_receive_input(value); 
    digitalWrite(GreenPin,1);
  }else{
    digitalWrite(GreenPin,0);
  }
 
  delay(100);
}

void Uart_PutChar(unsigned char value){
  MySerial.write(value);
}
