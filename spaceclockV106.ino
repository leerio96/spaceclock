//**********************************************************************
//本程序是网上开源的，做了整理规范，修改了部分内容，添加了注释，编写了教程。

//*210409:
//**********************************************************************
/*太空人表盘  时间天气
 * revised by LEERIO
 * date:2021.8.8
 *
 * 更新V102  
 * 优化太空人的卡顿问题
 * 文本显示，加上了自己的名字
 * 
 *更新V103   
 *Serial.begin(115200); //初始化串口波特率115200
 *太空人还是有些许卡顿
 *多个目标wifi待选
 *有几率一直在读条   连上又断  待解决 Exception (28):与Exception (9):
 *度娘说异常9是内存对齐错误的数据读写
 *异常28原因可能是在打印的过程中，错误的打印了字符串
 *重启信息 ets Jan  8 2013,rst cause:2, boot mode:(3,6)
 *注：会有各种奇怪的问题导致ESP8266重启，后续过程中遇到过好多次了
 *   多半是内存使用不当

 　 1. 如果你要用很大长度的数组，那么可以换用更小的数据类型。比如，int值要占用两个字节，你可以用byte（只占用一个字节）代替；
    2. esp8266有时会莫明重启，大部分情况是变量设置不当，虽然编译通过了，但变量在调用过程中出现异常，造成函数运行时变量内存溢出，写复杂代码时最好是不要一次写了很多再编译调试。
    3. 尽量少用全局变量，全局变量在整个生命周期都会存在，非常耗内存，内存不足也是esp8266不稳定因素之一。长串的变量尽量作长度控制，设定边界，如果你的项目很大，一定要规划好变量。
    4. 尽量不要在loop循环中定义变量，尽量只放函数封装，貌似这里定义了变量不会像函数中的局变量用完就回收。
 *更新V104  
 * Serial.begin(9600); //初始化串口波特率9600   改到多少波特率好像没有什么大影响，无论是9600还是115200
 * 天气滚动条显示会卡顿太空人转圈，改为5秒一滚动
 * 读条问题被解决
 * 途径：
 *      源代码中同时使用#include <ESP8266HTTPClient.h>与#include <ESP8266WiFi.h>
 *      注：<ESP8266HTTPClient.h>更为方便专门用于HTTP通讯，但<ESP8266WiFi.h>功能更加强大，支持TCP协议物联网通讯
 *      可能存在冲突？
 *      V104中修改了请求逻辑，仅使用<ESP8266HTTPClient.h>库进行HTTP访问请求，可以一次直接进入主界面了
 *更新V105  
 *如果自带wifi库连接不上，手机连接配网，更改读取页面提示   **待测试**
 *
 *更新V106
    实验Ticker多任务处理，能不能解决卡顿问题  失败 无论改滚动还是改太空人都卡顿
    优化了一下界面
    
   接线说明
   屏幕  ESP8266-12F  
   GND<----->G
   VCC<----->3V
   SCL<----->D5
   SDA<----->D7
   RES<----->D0
   DC <----->D6
   BLK<----->3V

*/

#include "main.h"

#define  VERSION   "V106"

Ticker ticker1; //多任务处理1


void connect_wifi()                               //联网
{   //存有的wifi库  名称   密码
  wifiMulti.addAP("rio", "liyue123");
  wifiMulti.addAP("413", "3.1415926");
  //wifiMulti.addAP("ssid", "password");

  //WiFi.mode(WIFI_STA);

  while (wifiMulti.run()!= WL_CONNECTED) {         //未连接上的话
    delay(1000);
    for (uint8_t n = 0;n<51||(WiFi.status() != WL_CONNECTED); n++){             //每500毫秒检测一次状态
      PowerOn_Loading(50);
      Serial.println(n);
      for (;n>50&& wifiMulti.run()!= WL_CONNECTED;){    //待改，等待时间数值
        wifi_manager();
        Serial.println("wait for manager");
          while(loadNum < 194){                           //让动画走完
              wifimanager_Loading(1);
                          }
        }
    }
  }
  
  while(loadNum < 194){                           //让动画走完
    PowerOn_Loading(1);
  }
  
  if (wifiMulti.run() == WL_CONNECTED)
  {Serial.println(WiFi.status());
  Serial.print("\nWiFi connected to: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP:   ");
  Serial.println(WiFi.localIP());                 //得到IP地址
  }
  
}

void wifi_manager(){//手机配网
    WiFiManager wifiManager; // 建立WiFiManager对象
    // 自动连接WiFi。以下语句的参数是连接ESP8266时的WiFi名称
    wifiManager.autoConnect("spaceclock-192-168-4-1");
    
    // 如果您希望该WiFi添加密码，可以使用以下语句：
    // wifiManager.autoConnect("AutoConnectAP", "12345678");
    // 以上语句中的12345678是连接AutoConnectAP的密码
    
    // WiFi连接成功后将通过串口监视器输出连接成功信息 
    Serial.println("wifi_manager()配网"); 
    Serial.print("ESP8266 Connected to ");
    Serial.println(WiFi.SSID());              // WiFi名称
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());           // IP
  
  }

void PowerOn_Loading(uint8_t delayTime)           //开机联网显示的进度条，输入延时时间
{
  clk.setColorDepth(8);
  clk.createSprite(200, 50);                      //创建Sprite
  clk.fillSprite(0x0000);                         //填充颜色

  clk.drawRoundRect(0,0,200,16,8,0xFFFF);         //画一个圆角矩形
  clk.fillRoundRect(3,3,loadNum,10,5,0xFFFF);     //画一个填充的圆角矩形
  clk.setTextDatum(CC_DATUM);                     //显示对齐方式
  clk.setTextColor(TFT_GREEN, 0x0000);            //文本的前景色和背景色
  clk.drawString("Connecting to WiFi",100,40,2);  //显示文本
  //drawString(const String& string, int32_t x, int32_t y, uint8_t font)
  
  clk.pushSprite(20,110);                         //Sprite中内容一次推向屏幕（x,y）
  clk.deleteSprite();                             //删除Sprite
  loadNum += 1;                                   //进度条位置变化，直到加载完成
  if(loadNum>=194){
    loadNum = 194;
  }
  delay(delayTime);
}

void wifimanager_Loading(uint8_t delayTime)           //开机联网显示的进度条，输入延时时间
{
  clk.setColorDepth(8);
  clk.createSprite(200, 50);                      //创建Sprite
  clk.fillSprite(0x0000);                         //填充颜色

  clk.drawRoundRect(0,0,200,16,8,0xFFFF);         //画一个圆角矩形
  clk.fillRoundRect(3,3,loadNum,10,5,0xFFFF);     //画一个填充的圆角矩形
  clk.setTextDatum(CC_DATUM);                     //显示对齐方式
  clk.setTextColor(TFT_GREEN, 0x0000);            //文本的前景色和背景色
  clk.drawString("Please Connect to WiFi spaceclock",100,40,2);  //显示文本
  //drawString(const String& string, int32_t x, int32_t y, uint8_t font)
  
  clk.pushSprite(20,110);                         //Sprite中内容一次推向屏幕（x,y）
  clk.deleteSprite();                             //删除Sprite
  loadNum += 1;                                   //进度条位置变化，直到加载完成
  if(loadNum>=194){
    loadNum = 194;
  }
  delay(delayTime);
}
void digitalClockDisplay()                        //时间显示
{
  clk.setColorDepth(8);

  //--------------------中间时间区显示开始--------------------
  //时分
  clk.createSprite(140, 48);                      //创建Sprite，先在Sprite内存中画点，然后将内存中的点一次推向屏幕，这样刷新比较快
  clk.fillSprite(bgColor);                        //背景色
  //clk.loadFont(FxLED_48);
  clk.setTextDatum(CC_DATUM);                     //显示对齐方式
  clk.setTextColor(TFT_BLACK, bgColor);           //文本的前景色和背景色
  clk.drawString(hourMinute(),70,24,7);           //绘制时和分
    //drawString(const String& string, int32_t x, int32_t y, uint8_t font)
    //clk.unloadFont();
  clk.pushSprite(28,40);                          //Sprite中内容一次推向屏幕
  clk.deleteSprite();                             //删除Sprite
  
  //秒
  clk.createSprite(40, 32);
  clk.fillSprite(bgColor);
  clk.loadFont(FxLED_32);                         //加载字体
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_BLACK, bgColor); 
  clk.drawString(num2str(second()),20,16);
  clk.unloadFont();                               //卸载字体
  clk.pushSprite(170,60);
  clk.deleteSprite();
  //--------------------中间时间区显示结束--------------------

  //--------------------底部时间区显示开始--------------------
  clk.loadFont(ZdyLwFont_20);                     //加载汉字字体
  
  //星期
  clk.createSprite(58, 32);
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_BLACK, bgColor);
  clk.drawString(week(),29,16);                   //周几
  clk.pushSprite(1,168);
  clk.deleteSprite();
  
  //月日
  clk.createSprite(98, 32);
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_BLACK, bgColor);  
  clk.drawString(monthDay(),49,16);               //几月几日
  clk.pushSprite(61,168);
  clk.deleteSprite();

  clk.unloadFont();                               //卸载字体
  //--------------------底部时间区显示结束--------------------
}

String week()                                     //星期
{
  String wk[7] = {"日","一","二","三","四","五","六"};
  String s = "周" + wk[weekday()-1];
  return s;
}

String monthDay()                                 //月日
{
  String s = String(month());
  s = s + "月" + day() + "日";
  return s;
}

String hourMinute()                               //时分
{
  String s = num2str(hour());
  s = s + ":" + num2str(minute());
  return s;
}

String num2str(int digits)                        //数字转换成字符串，保持2位显示
{
  String s = "";
  if (digits < 10)
    s = s + "0";
  s = s + digits;
  return s;
}

void printDigits(int digits)                      //打印时间数据
{
  Serial.print(":");
  if (digits < 10)                                //打印两位数字
    Serial.print('0');
  Serial.print(digits);
}

time_t getNtpTime()                               //获取NTP时间
{
  IPAddress ntpServerIP;                          //NTP服务器的IP地址

  while (Udp.parsePacket() > 0) ;                 //之前的数据没有处理的话一直等待 discard any previously received packets
  WiFi.hostByName(ntpServerName, ntpServerIP);    //从网站名获取IP地址
  
  sendNTPpacket(ntpServerIP);                     //发送数据包
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();                 //接收数据
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);    //从缓冲区读取数据
      
      unsigned long secsSince1900;
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0;                                       //没获取到数据的话返回0
}

void sendNTPpacket(IPAddress &address)            //发送数据包到NTP服务器
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE);       //缓冲区清零

  packetBuffer[0] = 0b11100011;                   //LI, Version, Mode   填充缓冲区数据
  packetBuffer[1] = 0;                            //Stratum, or type of clock
  packetBuffer[2] = 6;                            //Polling Interval
  packetBuffer[3] = 0xEC;                         //Peer Clock Precision
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  Udp.beginPacket(address, 123);                  //NTP服务器端口123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);       //发送udp数据
  Udp.endPacket();                                //发送结束
}

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)//显示回调函数
{
  if (y >= tft.height()) return 0;
  tft.pushImage(x, y, w, h, bitmap);
  return 1;
}

void getCityCode()                                //发送HTTP请求并且将服务器响应通过串口输出
{
  String URL = "http://wgeo.weather.com.cn/ip/?_="+String(now());
  HTTPClient httpClient;
  //WiFiClient client; 
  
  httpClient.begin(URL);
  //httpClient.begin(client,URL);                          //配置请求地址。此处也可以不使用端口号和PATH而单纯的
  httpClient.setUserAgent("esp8266");             //用户代理版本，其实没什么用 最重要是后端服务器支持
  //httpClient.setUserAgent("Mozilla/5.0 (iPhone; CPU iPhone OS 11_0 like Mac OS X) AppleWebKit/604.1.38 (KHTML, like Gecko) Version/11.0 Mobile/15A372 Safari/604.1");//设置请求头中的User-Agent
  httpClient.addHeader("Referer", "http://www.weather.com.cn/");
 
  int httpCode = httpClient.GET();                //启动连接并发送HTTP请求
  Serial.print("Send GET request to URL: ");
  Serial.println(URL);
  
  if (httpCode == HTTP_CODE_OK) {                 //如果服务器响应OK则从服务器获取响应体信息并通过串口输出
    String str = httpClient.getString();
    int aa = str.indexOf("id=");
    if (aa > -1){                                 //应答包里找到ID了
       cityCode = str.substring(aa+4,aa+4+9);     //9位长度
       Serial.println(cityCode); 
       getCityWeater();                           //获取天气信息
       LastTime2 = millis();
    }
    else{                                         //没有找到ID
      Serial.println("获取城市代码失败");  
    }
  } 
  else{
    Serial.println("请求城市代码错误：");
    Serial.println(httpCode);
  }


  httpClient.end();                               //关闭与服务器连接
}

void getCityWeater()                              //获取城市天气
{
  String  URL = "http://d1.weather.com.cn/weather_index/" + cityCode + ".html?_="+String(now());
  HTTPClient httpClient;
  //WiFiClient client; 
  httpClient.begin(URL); 
  
  //httpClient.begin(client,URL);                          //配置请求地址。
  httpClient.setUserAgent("esp8266");             //用户代理版本，其实没什么用 最重要是后端服务器支持
  //httpClient.setUserAgent("Mozilla/5.0 (iPhone; CPU iPhone OS 11_0 like Mac OS X) AppleWebKit/604.1.38 (KHTML, like Gecko) Version/11.0 Mobile/15A372 Safari/604.1");//设置请求头中的User-Agent
  httpClient.addHeader("Referer", "http://www.weather.com.cn/");
 
  int httpCode = httpClient.GET();                //启动连接并发送HTTP请求
  Serial.print("Send GET request to URL: ");
  Serial.println(URL);
  
  if (httpCode == HTTP_CODE_OK) {                 //如果服务器响应OK则从服务器获取响应体信息并通过串口输出
    String str = httpClient.getString();
    int indexStart = str.indexOf("weatherinfo\":");//寻找起始和结束位置
    int indexEnd = str.indexOf("};var alarmDZ");

    String jsonCityDZ = str.substring(indexStart+13,indexEnd);//复制字符串
    Serial.println(jsonCityDZ);

    indexStart = str.indexOf("dataSK =");         //寻找起始和结束位置
    indexEnd = str.indexOf(";var dataZS");
    String jsonDataSK = str.substring(indexStart+8,indexEnd);//复制字符串
    Serial.println(jsonDataSK);

    indexStart = str.indexOf("\"f\":[");          //寻找起始和结束位置
    indexEnd = str.indexOf(",{\"fa");
    String jsonFC = str.substring(indexStart+5,indexEnd);//复制字符串
    Serial.println(jsonFC);
    
    weaterData(&jsonCityDZ,&jsonDataSK,&jsonFC);  //显示天气信息
    Serial.println("获取成功");
    
  } 
  else {
    Serial.println("请求城市天气错误：");
    Serial.print(httpCode);
  }

  httpClient.end();                               //关闭与服务器连接
}

void weaterData(String *cityDZ,String *dataSK,String *dataFC)//天气信息写到屏幕上
{
  //const size_t capacity = JSON_OBJECT_SIZE(数据量) + 30  大小，对应 unsigned long
  DynamicJsonDocument doc(512); //DynamicJsonDocument doc(capacity)，                                      
  deserializeJson(doc, *dataSK);  //解析
  JsonObject sk = doc.as<JsonObject>();

  clk.setColorDepth(8);
  clk.loadFont(ZdyLwFont_20);                     //加载汉字字体
  
  //温度显示
  clk.createSprite(54, 32);                       //创建Sprite,
  //createSprite(WIDTH, HEIGHT);
  clk.fillSprite(bgColor);                        //填充颜色
  clk.setTextDatum(CC_DATUM);                     //显示对齐方式
  clk.setTextColor(TFT_BLACK, bgColor);           //文本的前景色和背景色 
  clk.drawString(sk["temp"].as<String>()+"℃",27,16);//显示文本  ，解析json的数据信息
  clk.pushSprite(185,168);                        //Sprite中内容一次推向屏幕
  clk.deleteSprite();                             //删除Sprite

  //城市名称显示
  clk.createSprite(88, 32); 
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_BLACK, bgColor); 
  clk.drawString(sk["cityname"].as<String>(),44,16);
  clk.pushSprite(151,1);
  clk.deleteSprite();
  
  //PM2.5空气指数显示
  uint16_t pm25BgColor = tft.color565(156,202,127);//优
  String aqiTxt = "优";
  int pm25V = sk["aqi"];
  if (pm25V > 200){
    pm25BgColor = tft.color565(136,11,32);        //重度，显示颜色和空气质量程度
    aqiTxt = "重度";
  }
  else if (pm25V > 150){
    pm25BgColor = tft.color565(186,55,121);       //中度
    aqiTxt = "中度";
  }
  else if (pm25V > 100){
    pm25BgColor = tft.color565(242,159,57);       //轻
    aqiTxt = "轻度";
  }
  else if (pm25V > 50){
    pm25BgColor = tft.color565(247,219,100);      //良
    aqiTxt = "良";
  }
  clk.createSprite(50, 24); 
  clk.fillSprite(bgColor);
  clk.fillRoundRect(0,0,50,24,4,pm25BgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(0xFFFF); 
  clk.drawString(aqiTxt,25,13);
  clk.pushSprite(5,130);
  clk.deleteSprite();

  //湿度显示
  clk.createSprite(56, 24); 
  clk.fillSprite(bgColor);
  clk.setTextDatum(CC_DATUM);
  clk.setTextColor(TFT_BLACK, bgColor); 
  clk.drawString(sk["SD"].as<String>(),28,13);
  //clk.drawString("100%",28,13);
  clk.pushSprite(180,130);
  clk.deleteSprite();

  scrollText[0] = "实时天气 " + sk["weather"].as<String>();//滚动显示的数据缓冲区
  scrollText[1] = "空气质量 " + aqiTxt;
  scrollText[2] = "风向 " + sk["WD"].as<String>()+sk["WS"].as<String>();
  
  //左上角滚动字幕
  deserializeJson(doc, *cityDZ);
  JsonObject dz = doc.as<JsonObject>();
  //Serial.println(sk["ws"].as<String>());
  //String aa = "今日天气:" + dz["weather"].as<String>() + "，温度:最低" + dz["tempn"].as<String>() + "，最高" + dz["temp"].as<String>() + " 空气质量:" + aqiTxt + "，风向:" + dz["wd"].as<String>() + dz["ws"].as<String>();
  //Serial.println(aa);
  scrollText[3] = "今日" + dz["weather"].as<String>();
  
  deserializeJson(doc, *dataFC);
  JsonObject fc = doc.as<JsonObject>();
  scrollText[4] = "最低温度"+fc["fd"].as<String>()+"℃";
  scrollText[5] = "最高温度"+fc["fc"].as<String>()+"℃";
  
  clk.unloadFont();                               //卸载字体
}

void scrollBanner()                               //天气滚动条显示
{
  now1 = millis();
  if(now1 - LastTime1 > 5000){                    //5秒切换一次显示内容
    if(scrollText[Dis_Count]){                    //如果滚动显示缓冲区有数据
      clkb.setColorDepth(8);
      clkb.loadFont(ZdyLwFont_20);                //加载汉字字体
      
      for(int pos = 24; pos>0; pos--){            //24点，每次移动一个点，从下往上移
        Dis_Scroll(pos);
      }
      
      //clkb.deleteSprite();                      //删除Sprite，这个我移动到Dis_Scroll函数里了
      clkb.unloadFont();                          //卸载字体
  
      if (Dis_Count >= 5){                        //总共显示五条信息
        Dis_Count = 0;                            //回第一个
      }
      else{
        Dis_Count += 1;                           //准备切换到下一个  
      }
      //Serial.println(Dis_Count);
    }
    LastTime1 = now1;
  }
}
void txtshow()                        //文本显示
{
  txt.setColorDepth(8); //颜色深度
  //clk.loadFont(ZdyLwFont_20);                     //加载汉字字体
  txt.createSprite(50, 30);                      //创建Sprite，先在Sprite内存中画点，然后将内存中的点一次推向屏幕，这样刷新比较快
  //  clk.createSprite(width,height);     
  txt.fillSprite(bgColor);                        //背景色
  //clk.loadFont(FxLED_48);
  txt.setTextDatum(CC_DATUM);                     //显示对齐方式
  txt.setTextColor(TFT_BLACK, bgColor);           //文本的前景色和背景色
  txt.drawString("by RIO",30,18,2);                  //绘制文本
  //drawString(const String& string, int32_t x, int32_t y, uint8_t font)
  //clk.unloadFont();
  txt.pushSprite(0,100);                          //Sprite中内容一次推向屏幕
  txt.deleteSprite();                             //删除Sprite
  Serial.println("txt has shown");
  
}
void Dis_Scroll(int pos){                         //字体滚动
  clkb.createSprite(148, 24);                     //创建Sprite，先在Sprite内存中画点，然后将内存中的点一次推向屏幕，这样刷新比较快
  clkb.fillSprite(bgColor);                       //背景色
  clkb.setTextWrap(false);
  clkb.setTextDatum(CC_DATUM);                    //显示对齐方式
  clkb.setTextColor(TFT_BLACK, bgColor);          //文本的前景色和背景色
  clkb.drawString(scrollText[Dis_Count],74,pos+12);//打显示内容
  clkb.pushSprite(2,4);                           //Sprite中内容一次推向屏幕
  clkb.deleteSprite();                            //删除Sprite
}

void imgAnim(){
  int x=80,y=94,dt=40;                            //瘦子版dt=10毫秒 胖子30较为合适

  TJpgDec.drawJpg(x,y,i0, sizeof(i0));            //打一张图片延时一段时间，达到动画效果
  delay(dt);
  TJpgDec.drawJpg(x,y,i1, sizeof(i1));
  delay(dt);
  TJpgDec.drawJpg(x,y,i2, sizeof(i2));
  delay(dt);
  TJpgDec.drawJpg(x,y,i3, sizeof(i3));
  delay(dt);  
  TJpgDec.drawJpg(x,y,i4, sizeof(i4));
  delay(dt);  
  TJpgDec.drawJpg(x,y,i5, sizeof(i5));
  delay(dt);  
  TJpgDec.drawJpg(x,y,i6, sizeof(i6));
  delay(dt);  
  TJpgDec.drawJpg(x,y,i7, sizeof(i7));
  delay(dt);  
  TJpgDec.drawJpg(x,y,i8, sizeof(i8));
  delay(dt);  
  TJpgDec.drawJpg(x,y,i9, sizeof(i9));
  delay(dt);  
}


void setup()
{
  //Serial.begin(115200);                           //初始化串口
  Serial.begin(9600);
  Serial.println();                               //打印回车换行

  tft.init();                                     //TFT初始化
  tft.setRotation(0);                             //旋转角度0-3
  tft.fillScreen(0x0000);                         //清屏
  tft.setTextColor(TFT_BLACK, bgColor);           //设置字体颜色
  

  connect_wifi();                                 //联网处理

  Serial.println("Starting UDP");                 //连接时间服务器
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);

  TJpgDec.setJpgScale(1);                         //设置放大倍数
  TJpgDec.setSwapBytes(true);                     //交换字节
  TJpgDec.setCallback(tft_output);                //回调函数,必不可少

  TJpgDec.drawJpg(0,0,watchtop, sizeof(watchtop));//显示顶部图标 240*20
  TJpgDec.drawJpg(0,220,watchbottom, sizeof(watchbottom));//显示底部图标 240*20

  //绘制一个窗口
  tft.setViewport(0, 20, 240, 200);               //中间的显示区域大小
  tft.fillScreen(0x0000);                         //清屏
  tft.fillRoundRect(0,0,240,200,5,bgColor);       //实心圆角矩形
  //tft.resetViewport();

  //绘制线框
  tft.drawFastHLine(0,34,240,TFT_BLACK);          //这些坐标都是窗体内部坐标，tft.setViewport创造的
  tft.drawFastVLine(150,0,34,TFT_BLACK);
  tft.drawFastHLine(0,166,240,TFT_BLACK);
  tft.drawFastVLine(60,166,34,TFT_BLACK);
  tft.drawFastVLine(160,166,34,TFT_BLACK);
  txtshow();

  getCityCode();                                  //通过IP地址获取城市代码
  delay(100);
  
  TJpgDec.drawJpg(161,171,temperature, sizeof(temperature));//温度图标
  TJpgDec.drawJpg(159,130,humidity, sizeof(humidity));  //湿度图标
    
  //ticker1.attach(30, scrollBanner);                             //多任务

}

void loop()
{
  if (timeStatus() != timeNotSet){                //已经获取到数据的话
    if (now() != prevDisplay){                    //如果本次数据和上次不一样的话，刷新
      prevDisplay = now();
      digitalClockDisplay();
    }
  }

  if(millis() - LastTime2 > 600000){              //10分钟更新一次天气
    LastTime2 = millis();
    //getCityWeater();
  }

  scrollBanner();                                 //天气数据滚动显示
  imgAnim();                                      //太空人显示
}
