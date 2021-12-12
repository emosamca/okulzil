// Visual Micro is in vMicro>General>Tutorial Mode
// 
/*
    Name:       Hv.Asb.MYO için zil projesi
    Created:	08.03.2020 22:14:59
    Author:     Alb.M.Emrah HARMAN & Tðm.Mert GÖBEL
*/

//#define debug
#define NOKIALCD
//#define testspeech
//#define saattest
//#include <DHT.h>   // DHT 21 için kütüphane
#include <Eeprom24C32_64.h>   // saat üzerinde bulunan 24C32 EEPROM kütüphanesi
#include <DFRobotDFPlayerMini.h>  // MP3 player için gerekli kütüphaneler
#include <Wire.h>
#include <SoftwareSerial.h>
#if defined (NOKIALCD)
#include <LCD5110_Basic.h> // Nokia 5110 84x84 grafik LCD için gerekli kütüphane
#else
#include "ssd1306.h"
#endif
#include <DS3231.h>   // RTC için gerekli kütüphane
#include <EEPROM.h>

#if defined (NOKIALCD)
// 5110 LCD için hazýrlýklar
// 7- SCLK 6- DIN 5- D/C 3- RST 4- CS 
LCD5110 display(7, 6, 5, 3, 4);
extern uint8_t SmallFont[];
#endif

// MP3 player için hazýrlýklar
SoftwareSerial mp3SoftwareSerial(9, 10); // (RX, TX);
DFRobotDFPlayerMini mp3DFPlayer;
#define busy_pin    11
#define playing     LOW
#define not_playing HIGH
int filecount = 0;
bool isplaymusic = false;
long playtiming = 0;
long songstarttime;
int calansarki;
int startdizin = 5;
int calinacakdizin;
int calinacakmuzik;
int ogrencimuzikklasor = 50;
int ogrencimuzikcount;
int alarmvolume = 24;
int muzikvolume = 16;
int hareketlimuzikstart = 672;
int marsstart = 31;
int marsstop = 36;
int marscalinmagunu = 0;
int teneffussayisi = 0;
int marscalacakteneffus = 0;
bool isplaymars = false;
int zilsayisi = 0;

// RTC ön hazýrlýklarý
DS3231 rtcClock;
RTCDateTime dt;
#define int_pin     2
int now_dayofweek;
int now_hour;
int now_minute;
int setalarm_hour;
int setalarm_minute;
int setalarm_playtime;
int setalarmsongnumber;
int setderssaati;
bool isAlarm = false;
bool alarm_find = false;
float temperature;
float humidity;


char Time[] = "  :  :  ";
char Date[] = "  /  /20  ";
char Temp[] = "000.00";
char Alarm[] = "( )  :  ->   ";
String Gun;

// EEPROM için hazýrlýklar
#define EEPROM_ADDRESS 0x57
#define max_length  2048    // byte
static Eeprom24C32_64 eeprom2(EEPROM_ADDRESS);
word holidayaddress = 1000;

// 1 pazartesi, 2 salý, ... 7 pazar
int alarmdayofweek;
// saat
int alarmhour;
// dakika
int alarmminute;
// alarm kaç saniye çalacak
int playtime;
// hangi þarký çalacak. bütün þarkýlar 01 klasöründe olacak
int alarmsongnumber;
// hangi ders basliyor?
int derssaati;
// alarm set edildiðinde kaç dakika var çalmasýna?
long nextalarmminute;
// eepromdan byte okuyoruz
byte eeprom_readed;
// hangi müzik türü aktif
int musictype;
// Röle için hazýrlýklar
#define role_pin    8
#define release     HIGH
#define pull        LOW

// DHT için hazýrlýklar
//#define DHTPIN      8
//#define DHTTYPE     DHT22
//DHT dht(DHTPIN, DHTTYPE);

// buton pinleri
#define ding_dong_button    A0
#define load_button         A1
#define alarm_stop_button   A2
#define open_button			A3

int dingdongsongnumber = 47;
int satirsira = 0;
// eeprom baþlangýç
void eeprom_init()
{
#if defined NOKIALCD
	writeNokia(getNextSatir(), "EEPROM.", true, LEFT, false);
#else
	writeCenter(getNextSatir(), 8, "Harici eeprom...",true);
#endif 
	eeprom2.initialize();
#if defined NOKIALCD
	writeNokia(getNextSatir(), "EEPROM.OK", true, LEFT, true);
#else
	writeCenter(satirsira, 8, "Harici eeprom...OK", true);
#endif
}
// display baþlangýç
void display_init()
{
#if defined NOKIALCD
    display.InitLCD();
	display.setFont(SmallFont);
	display.clrScr();
	writeNokia(getNextSatir(), "EKRAN.OK", true, LEFT, true);
	delay(500);
#else
	ssd1306_setFixedFont(ssd1306xled_font6x8);

	ssd1306_128x64_i2c_init();

	ssd1306_clearScreen();
	writeCenter(getNextSatir(), 8, "Ekran...OK", true);
#endif
}
// RTC baþlangýç
void clock_init()
{
#if defined NOKIALCD
	writeNokia(getNextSatir(), "RTC.", true, LEFT, false);
#else
	writeCenter(getNextSatir(), 8, "RTC init...", true);
#endif
	rtcClock.begin();
    rtcClock.armAlarm1(false);
    rtcClock.armAlarm2(false);
    rtcClock.clearAlarm1();
    rtcClock.clearAlarm2();
    rtcClock.enableOutput(false);
    delay(100);
#if defined saattest
	rtcClock.setDateTime(2020, 5,25,7, 59, 45);// __DATE__, __TIME__);     // 2020, 3, 10, 15, 55, 45);
#endif
#if defined NOKIALCD
	writeNokia(getNextSatir(), "RTC.OK", true, LEFT, true);
#else
	writeCenter(satirsira, 8, "RTC init...OK", true);
#endif
}
// MP3 player baþlangýç
void mp3player_init()
{
	int dizindosya = 0;
#if defined NOKIALCD
	writeNokia(getNextSatir(), "MP3.", true, LEFT, false);
#else
	writeCenter(getNextSatir(), 8, "MP3 init...", true);
#endif
    mp3SoftwareSerial.begin(9600);
	delay(1000);
    if (!mp3DFPlayer.begin(mp3SoftwareSerial))
    {
        while (true)
        {
            delay(0);
        }
    }
    mp3DFPlayer.volume(alarmvolume);
	delay(50);
	mp3DFPlayer.setTimeOut(2400);
	delay(500);
	zilsayisi= mp3DFPlayer.readFileCountsInFolder(1);
	filecount = mp3DFPlayer.readFileCountsInFolder(startdizin);
#if defined NOKIALCD
	writeNokia(getNextSatir(), "Diz5."+String(filecount), true, LEFT, true);
#else
	writeCenter(getNextSatir(), 8, "Dizin 5 :" + String(filecount), true);
#endif
#if defined (debug)
	Serial.print("dizinde bulunan sarki 5:");
	Serial.println(filecount);
#endif
	delay(500);
	dizindosya= mp3DFPlayer.readFileCountsInFolder(startdizin + 1);
	filecount += dizindosya;
#if defined NOKIALCD
	writeNokia(getNextSatir(), "Diz6." + String(dizindosya), true, LEFT, true);
#else
	writeCenter(getNextSatir(), 8, "Dizin 6 :" + String(dizindosya), true);
#endif
#if defined (debug)
	Serial.print("dizinde bulunan sarki 6:");
	Serial.println(filecount);
#endif
	delay(500);
	dizindosya = mp3DFPlayer.readFileCountsInFolder(startdizin + 2);
	filecount += dizindosya;
#if defined NOKIALCD
	writeNokia(getNextSatir(), "Diz7." + String(dizindosya), true, LEFT, true);
#else
	writeCenter(getNextSatir(), 8, "Dizin 7 :" + String(dizindosya), true);
#endif
#if defined (debug)
	Serial.print("dizinde bulunan sarki 7:");
	Serial.println(filecount);
#endif
	delay(500);
	dizindosya = mp3DFPlayer.readFileCountsInFolder(startdizin + 3);
	filecount += dizindosya;
#if defined NOKIALCD
	writeNokia(getNextSatir(), "Diz8." + String(dizindosya), true, LEFT, true);
#else
	writeCenter(getNextSatir(), 8, "Dizin 8 :" + String(dizindosya), true);
#endif
#if defined (debug)
	Serial.print("dizinde bulunan sarki 8:");
	Serial.println(filecount);
#endif
	delay(500);
	dizindosya = mp3DFPlayer.readFileCountsInFolder(ogrencimuzikklasor);
	ogrencimuzikcount = dizindosya;
#if defined NOKIALCD
	writeNokia(getNextSatir(), "Diz50." + String(dizindosya), true, LEFT, true);
#else
	writeCenter(getNextSatir(), 8, "Dizin 50 :" + String(dizindosya), true);
#endif
#if defined (debug)
	Serial.print("dizinde bulunan sarki 50:");
	Serial.println(filecount);
#endif
	delay(500);
	dizindosya = mp3DFPlayer.readFileCountsInFolder(ogrencimuzikklasor+1);
	ogrencimuzikcount += dizindosya;
#if defined NOKIALCD
	writeNokia(getNextSatir(), "Diz51." + String(dizindosya), true, LEFT, true);
#else
	writeCenter(getNextSatir(), 8, "Dizin 51 :" + String(dizindosya), true);
#endif
#if defined (debug)
	Serial.print("dizinde bulunan sarki 51:");
	Serial.println(filecount);
#endif
	delay(500);

#if defined (debug)
	Serial.print("dizinde bulunan sarki :");
	Serial.println(filecount);
#endif
#if defined NOKIALCD
	writeNokia(getNextSatir(), "Topl." + String(filecount), true, LEFT, true);
	writeNokia(getNextSatir(), "MP3.OK", true, LEFT, true);
#else
	writeCenter(getNextSatir(), 8, "Toplam :" + String(filecount), true);
	writeCenter(getNextSatir(), 8, "MP3 init...OK", true);
#endif
}
// DHT baþlangýç
void dht_init()
{
#if defined NOKIALCD
	writeNokia(getNextSatir(), "DHT.", true, LEFT, false);
#else
	writeCenter(getNextSatir(), 8, "DHT init...", true);
#endif
	//dht.begin();
#if defined NOKIALCD
	writeNokia(getNextSatir(), "DHT.OK", true, LEFT, true);
#else
	writeCenter(satirsira, 8, "DHT init...OK", true);
#endif
}
// portlarý ayarla
void ports_init()
{
#if defined NOKIALCD
	writeNokia(getNextSatir(), "Port.", true, LEFT, false);
#else
	writeCenter(getNextSatir(), 8, "Port init...",true);
#endif
	pinMode(role_pin, OUTPUT);
    pinMode(alarm_stop_button, INPUT_PULLUP);
    pinMode(load_button, INPUT_PULLUP);
    pinMode(ding_dong_button, INPUT_PULLUP);
	pinMode(open_button, INPUT_PULLUP);
    pinMode(int_pin, INPUT);
    pinMode(busy_pin, INPUT);
#if defined NOKIALCD
	writeNokia(getNextSatir(), "Port.OK", true, LEFT, true);
#else
	writeCenter(satirsira, 8, "Port init...OK", true);
#endif
}
// port varsayýlanlarý
void ports_default()
{
    digitalWrite(role_pin, release);
}
// alarm interrupt
void alarm_function()
{
	isAlarm = true;
}

void interrupt_init()
{
	attachInterrupt(0, alarm_function, FALLING);
}

void read_rtc()
{
	dt = rtcClock.getDateTime();
	delay(10);
	//temperature = rtcClock.readTemperature();
}
void display_rtc_at_lcd()
{
	Time[7] = dt.second % 10 + '0';
	Time[6] = dt.second / 10 + '0';
	Time[4] = dt.minute % 10 + '0';
	Time[3] = dt.minute / 10 + '0';
	Time[1] = dt.hour % 10 + '0';
	Time[0] = dt.hour / 10 + '0';

	Date[9] = (dt.year - 2000) % 10 + '0';
	Date[8] = (dt.year - 2000) / 10 + '0';
	Date[4] = dt.month % 10 + '0';
	Date[3] = dt.month / 10 + '0';
	Date[1] = dt.day % 10 + '0';
	Date[0] = dt.day / 10 + '0';
#if defined NOKIALCD
	writeNokia(1, Time, false, CENTER, false);
	writeNokia(2, Date, false, CENTER, false);
	//display.print(Time, CENTER, 0);
	//display.print(Date, CENTER, 8);
	//display.clrRow(2);
#else
	//display.fillRect(0, 0, 127, 48, SSD1306_BLACK);
	writeCenter(1, 16, Time, false);
	//delay(3000);
	writeCenter(2, 16, Date, false);
	//delay(3000);
#endif
	switch (dt.dayOfWeek)
	{
	case 1:  Gun = "PAZARTESI"; break;
	case 2:  Gun = "SALI"; break;
	case 3:  Gun = "CARSAMBA";  break;
	case 4:  Gun = "PERSEMBE";  break;
	case 5:  Gun = "CUMA"; break;
	case 6:  Gun = "CUMARTESI"; break;
	default: Gun = "PAZAR";
	}
#if defined NOKIALCD
	writeNokia(3, Gun, false, CENTER, true);
	//display.print(Gun, CENTER, 16);
#else
	writeCenter(5, 8, Gun, true);
	//delay(3000);
#endif
	temperature = 0;// dht.readTemperature();
	humidity = 0;// dht.readHumidity();
	String degree = String(temperature, 1);
	String humid = String(humidity, 0);
	degree = degree + "$C " + humid + "%"; //derece yerine $ iþareti kullanýldý. 
#if defined NOKIALCD
	writeNokia(4, degree, false, CENTER, true);
	//display.print(degree, CENTER, 24);
#else
	writeCenter(6, 8, degree, false);
#endif
}
#if defined NOKIALCD
void writeNokia(int row, String metin, bool inverse, int format, bool Isclear)
{
	if (Isclear)
		display.clrRow(row - 1);
	if (inverse)
		display.invertText(true);
	display.print(metin, format, (row - 1) * 8);
	if (inverse)
		display.invertText(false);
}
#else
void writeCenter(int row, int textsize, String metin, bool inverse)
{
	int startbas = 0;
	int charlen = 0;
	if (textsize == 16)
		charlen = 8;
	else if (textsize == 8)
		charlen = 6;
	startbas = (128 - (metin.length() * charlen)) / 2;
	ssd1306_negativeMode();
	if (textsize == 16)
		ssd1306_setFixedFont(ssd1306xled_font8x16);
	else if (textsize == 8)
		ssd1306_setFixedFont(ssd1306xled_font6x8);
	if (inverse)
		ssd1306_negativeMode();
	else
		ssd1306_positiveMode();        
	int leng = metin.length()+1 ;
	char emo[255];
	metin.toCharArray(emo, leng);
	ssd1306_printFixed(startbas, (row - 1)*textsize,emo, STYLE_NORMAL);
	ssd1306_positiveMode();        // Draw white text
}
#endif
void write_next_alarm(int gun, int saat, int dakika,int songnumber)
{
	//"( )  :  ->   "
	Alarm[1] = gun % 10 + '0';
	Alarm[3] = saat / 10 + '0';
	Alarm[4] = saat % 10 + '0';
	Alarm[6] = dakika / 10 + '0';
	Alarm[7] = dakika % 10 + '0';
	Alarm[10] = songnumber / 100 + '0';
	if (songnumber > 100)
		songnumber = songnumber - 100;
	Alarm[11] = songnumber / 10 + '0';
	Alarm[12] = songnumber % 10 + '0';
#if defined NOKIALCD
	writeNokia(6, Alarm, false, LEFT, false);
	String mstr = String(setderssaati);
	writeNokia(5, mstr, false, CENTER, true);
	//display.print(Alarm, LEFT, 40);
#else
	writeCenter(8, 8, Alarm, true);
#endif
#if defined (debug)
	Serial.println(Alarm);
#endif
}
void random_init()
{
#if defined NOKIALCD
	writeNokia(getNextSatir(), "Random.", false, LEFT, false);
#else
	writeCenter(getNextSatir(), 8, "Random init...", true);
#endif
	read_rtc();
	delay(50);
	long okunan = (long)dt.hour * 60 * 60 + (long)dt.minute * 60 + (long)dt.second;
#if defined (debug)
	Serial.print("Okunan:");
	Serial.println(okunan);
#endif
	randomSeed(okunan);
#if defined NOKIALCD
	writeNokia(getNextSatir(), "Random."+String(okunan), false, LEFT, false);
	writeNokia(getNextSatir(), "Random.OK", false, LEFT, false);
#else
	writeCenter(getNextSatir(), 8, "Random degeri.." + String(okunan), true);
	writeCenter(getNextSatir(), 8, "Random init...OK", true);
#endif
}
// The setup() function runs once each time the micro-controller starts
void setup()
{
	pinMode(role_pin, OUTPUT);
	digitalWrite(role_pin, release);
	delay(5000);
	Serial.begin(9600);
	display_init();	
	ports_init(); 
	ports_default();
	eeprom_init();  
	clock_init();  
	mp3player_init();  
	dht_init(); 
	random_init(); 

	interrupt_init();
	delay(50);
	isAlarm = false;
#if defined NOKIALCD
	writeNokia(1, "Hersey OK", true, LEFT, true);
	kalinanyervesayiyaz();
	delay(3000);
	display.clrScr();
#else
	writeCenter(1, 16, "Hersey OK...", true);
	kalinanyervesayiyaz();
	delay(3000);
	ssd1306_clearScreen();
#endif
	find_next_alarm();
}
// Add the main program code into the continuous loop() function
void loop()
{
	read_rtc();
	display_rtc_at_lcd();
	delay(10);
	check_alarm();
	check_load_button();
	check_music();
	check_mars();
	check_alarm_stop_button();
	check_dingdong_button();
	check_open_button();
	//while (1)
	//{
	//	check_music_test();
	//}
}
void check_open_button()
{
	if (digitalRead(open_button) == LOW)
	{
		display.clrScr();
#if defined NOKIALCD
		writeNokia(1, "ANONS", false, CENTER, true);
		writeNokia(2, "ACILDI", false, CENTER, true);
#endif
		mp3DFPlayer.stop();
		digitalWrite(role_pin, pull);
		delay(2000);
		//speech_date_time();
		long startmilis = millis();
		while (digitalRead(open_button)==HIGH)
		{
			check_dingdong_button();
			if (millis() - startmilis > (120 * 1000))
				break;
		}
		display.clrScr();
		digitalWrite(role_pin, release);
		write_next_alarm(alarmdayofweek, setalarm_hour, setalarm_minute, setalarmsongnumber);
		delay(500);

	}
}
void check_dingdong_button()
{
	if (digitalRead(ding_dong_button) == LOW)
	{
#if defined NOKIALCD
		writeNokia(5,"DING-DONG",false,CENTER,true);
#endif
		mp3DFPlayer.playFolder(4, dingdongsongnumber);
		delay(3500);
#if defined NOKIALCD
		writeNokia(5, " ", false, CENTER, true);
#endif

	}
}
void check_alarm_stop_button()
{
	if (digitalRead(alarm_stop_button) == LOW)
	{
#if defined NOKIALCD
		writeNokia(5, "ALARM STOP", false, CENTER, true);
#endif
		isplaymusic = false;
		isplaymars = false;
		mp3DFPlayer.stop();
		delay(50);
		digitalWrite(role_pin, release);
		mp3DFPlayer.volume(alarmvolume);
		delay(50);
//#if defined NOKIALCD
//		//display.print("            ", CENTER, 32);
//#else
//		writeCenter(7, 8, "         ", false);
//#endif
		delay(1000);
		mp3DFPlayer.reset();
		delay(1000);
#if defined NOKIALCD
		writeNokia(5, " ", false, CENTER, true);
#endif
	}
}
void check_music()
{
	if (isplaymusic)
	{
		if (digitalRead(busy_pin) == not_playing)
		{
			delay(1000);
			if(musictype==101)
			{
				int sayac = 1;
				do
				{
					read_rtc();
					if ((dt.hour >= 11 && dt.hour < 17) || (dt.hour>=7 && dt.hour<10))
					{
						calansarki = random(hareketlimuzikstart, filecount + 1);
						if (calansarki > filecount)
							calansarki = filecount;
						else if (calansarki == (hareketlimuzikstart-1))
							calansarki = hareketlimuzikstart;
					}
					else
					{
						calansarki = random(filecount) + 1;
						if (calansarki > filecount)
							calansarki = filecount;
						else if (calansarki == 0)
							calansarki = 1;
					}
					if (sayac > 10)
						break;
					sayac++;
				} while (hasMusicinEEPROM(calansarki));
	#if defined debug
				Serial.print("Sarki:");
				Serial.println(calansarki);
	#endif
				calinacakdizin = ((calansarki - 1) / 255) + startdizin;
				calinacakmuzik = calansarki -((calinacakdizin-startdizin) * 255);
			}
			else if (musictype == 103)
			{
				calansarki = random(ogrencimuzikcount) + 1;
				if (calansarki > ogrencimuzikcount)
					calansarki = ogrencimuzikcount;
				else if (calansarki == 0)
					calansarki = 1;
				calinacakdizin = ((calansarki - 1) / 255) + ogrencimuzikklasor;
				calinacakmuzik = calansarki - ((calinacakdizin-ogrencimuzikklasor) * 255);
			}
			mp3DFPlayer.playFolder(calinacakdizin, calinacakmuzik);
			delay(2000);
#if defined NOKIALCD
			writeNokia(5, "Muzik:" + String(calinacakdizin) + "(" + String(calinacakmuzik) + ")", true, LEFT, true);
			/*display.invertText(true);
			display.print("                ", CENTER, 32);
			display.print("Muzik", LEFT, 32);
			display.printNumI(calansarki, RIGHT, 32);
			display.invertText(false);*/
#else
			String muzik = String("Muzik   : ");
			String calan = String(calansarki);
			muzik = muzik + calan;
			writeCenter(7, 8, muzik, true);
#endif
		}
		if ((millis() - songstarttime) > playtiming)
		{
			isplaymusic = false;
			mp3DFPlayer.stop();
			delay(50);
			digitalWrite(role_pin, release);
			mp3DFPlayer.volume(alarmvolume);
			delay(50);
#if defined NOKIALCD
			//display.print("            ", CENTER, 32);
			writeNokia(5, "  ", false, CENTER, true);
#else
			writeCenter(7, 8, "                ", false);
#endif
			delay(1000);
			mp3DFPlayer.reset();
			delay(1000);
		}
	}
}
void check_mars()
{
	if (isplaymars)
	{
		if (digitalRead(busy_pin) == not_playing)
		{
			delay(1000);
			isplaymars = false;
			mp3DFPlayer.stop();
			delay(50);
			digitalWrite(role_pin, release);
			mp3DFPlayer.volume(alarmvolume);
			delay(50);
#if defined NOKIALCD
			//display.print("            ", CENTER, 32);
			writeNokia(5, "  ", false, CENTER, true);
#else
			writeCenter(7, 8, "                ", false);
#endif
			delay(1000);
			mp3DFPlayer.reset();
			delay(1000);
		}
	}
}
void check_load_button()
{
	if (!digitalRead(load_button))
	{
		delay(150);
		if (!digitalRead(load_button))
		{
			load_buton_pressed();
#if defined NOKIALCD
			display.clrScr();
#else
			ssd1306_clearScreen();
#endif
			find_next_alarm();
		}
	}
}
void load_buton_pressed()
{
	int sayac = 1;
	//while (Serial.available() >= 0)
	{
		Serial.end();// .read();
		Serial.begin(9600);
	}
#if defined NOKIALCD
	display.clrScr();
	writeNokia(1, "YUKLEMEYE", false, CENTER, false);
	writeNokia(2, "HAZIR", false, CENTER, false);
	//display.print("YUKLEMEYE", CENTER, 0);
	//display.print("HAZIR", CENTER, 8);
#else
	ssd1306_clearScreen();
	writeCenter(1, 16, "YUKLEMEYE", false);
	writeCenter(2, 16, "HAZIR", false);
#endif
	delay(1000);
	while (Serial.available() <= 0)
	{
		delay(1);
		if (!digitalRead(load_button))
			return;
	}
	// gelen ilk veriler saat bilgisi
	int _senkron = Serial.parseInt();
	int _yil = Serial.parseInt();
	int _ay = Serial.parseInt();
	int _gun = Serial.parseInt();
	int _saat = Serial.parseInt();
	int _dakika = Serial.parseInt();
	int _saniye = Serial.parseInt();
	String m = String(_senkron);
#if defined NOKIALCD
	writeNokia(4, m, true, CENTER, false);
#else
	writeCenter(6, 8, m, true);
#endif
	if (_senkron == 156 || _senkron == 162)
	{
		rtcClock.setDateTime(_yil, _ay, _gun, _saat, _dakika, _saniye);
#if defined NOKIALCD
		writeNokia(3, "Saat yuklendi", false, CENTER, false);
		//display.print("Saat", CENTER, 24);
		//display.print("Yuklendi", CENTER, 32);
#else
		writeCenter(6, 8, "Saat yuklendi.",true);
#endif
	}
	// senkronizasyon için gönderiliyor
	//if(Serial.read()=='\n')
	//	Serial.println(5555);
	//long beklemebaslangici = millis();
	//while (Serial.available() <= 0)
	//{
	//	delay(1);
	//	if ((millis() - beklemebaslangici) > 30000)
	//		return;
	//}
	if (_senkron == 156 || _senkron == 157)
	{
		// zil saatleri yuklenmeye baþlýyor
		word tempaddress = 0;
		int _dayofweek;
		int _alarmplaytime;
		int _alarmsongnumber;
		int _derssaati;
		while (Serial.available() > 0)
		{
#if defined NOKIALCD
			String say = String(sayac++);
			writeNokia(5, say, true, CENTER, true);
			//display.clrRow(2);
			//display.printNumI(sayac++, CENTER, 24);
#else
			String say = String(sayac++);
			writeCenter(7, 8, say, true);
#endif
			_dayofweek = Serial.parseInt();
			if (_dayofweek == 255)
			{
				eeprom2.writeByte(tempaddress, 255);
				delay(10);
#if defined NOKIALCD
				writeNokia(6, "Zil Yuklendi", true, CENTER, false);
				//display.print("Yuklendi.", CENTER, 32);
#else
				writeCenter(8, 8, "Zil Yuklendi.", true);
#endif
				delay(2000);
				return;
			}
			_saat = Serial.parseInt();
			_dakika = Serial.parseInt();
			_alarmplaytime = Serial.parseInt();
			_alarmsongnumber = Serial.parseInt();
			_derssaati = Serial.parseInt();
			eeprom2.writeByte(tempaddress, _dayofweek);
			delay(10);
			eeprom2.writeByte(tempaddress + 1, _saat);
			delay(10);
			eeprom2.writeByte(tempaddress + 2, _dakika);
			delay(10);
			eeprom2.writeByte(tempaddress + 3, _alarmplaytime);
			delay(10);
			eeprom2.writeByte(tempaddress + 4, _alarmsongnumber);
			delay(10);
			eeprom2.writeByte(tempaddress + 5, _derssaati);
			delay(10);
			tempaddress += 6;
			//Serial.println(5555);
			//delay(50);

		}
	}
	else if (_senkron == 162 || _senkron == 163)
	{
		// tatil günleri yuklenmeye baþlýyor
		word tempaddress = holidayaddress;
		int _ay;
		int _gun;
		sayac = 1;
		while (Serial.available() > 0)
		{
#if defined NOKIALCD
			String say = String(sayac++);
			writeNokia(5, say, true, CENTER, true);
			//display.clrRow(2);
			//display.printNumI(sayac++, CENTER, 24);
#else
			String say = String(sayac++);
			writeCenter(7, 8, say, true);
#endif
			_ay = Serial.parseInt();
			if (_ay == 255)
			{
				eeprom2.writeByte(tempaddress, 255);
				delay(10);
#if defined NOKIALCD
				writeNokia(6, "Tat.Yuklendi", true, CENTER, false);
				//display.print("Yuklendi.", CENTER, 32);
#else
				writeCenter(8, 8, "Tatil Yuklendi.", true);
#endif
				delay(2000);
				return;
			}
			_gun = Serial.parseInt();
			eeprom2.writeByte(tempaddress, _ay);
			delay(10);
			eeprom2.writeByte(tempaddress + 1, _gun);
			delay(10);
			tempaddress += 2;
			//Serial.println(5555);
			//delay(50);

		}
	}
#if defined NOKIALCD
	display.clrScr();
	writeNokia(6, "YUKLENEMEDI", true, CENTER, false);
	//display.print("YUKLENEMEDI", CENTER, 32);
#else
	ssd1306_clearScreen();
	writeCenter(8, 8, "YUKLENEMEDI", true);
#endif
	delay(6000);

#if defined NOKIALCD
	display.clrScr();
#else
	ssd1306_clearScreen();
#endif
	find_next_alarm();
}
void check_alarm()
{
	//digitalWrite(13, HIGH);
	if (isAlarm)
	{
		delay(50);
		read_rtc();
		if(dt.hour==setalarm_hour && dt.minute==setalarm_minute )
		{
			if (!isHoliday())
			{
				if (setalarmsongnumber < 100)  // 100 ve üzeri ise farklý bir durum var.
				{								// 101 de MP3 klasöründen müzik çalar. 
												// 102 de gün ve sýcaklýk bilgisini okur.
#if defined NOKIALCD
					writeNokia(5, "Alarm", true, CENTER, false);
					//display.invertText(true);	
					//display.print("Alarm", CENTER, 32);
					//display.invertText(false);
#else
					writeCenter(7, 8, "Alarm", true);
#endif
					rtcClock.clearAlarm1();
					mp3DFPlayer.stop();
					delay(100);
					mp3DFPlayer.volume(alarmvolume);
					delay(100);
					isAlarm = false;
					digitalWrite(role_pin, pull);
					delay(1500);
					mp3DFPlayer.playFolder(1, setalarmsongnumber);
					long zaman = millis();
					long zamankontrol = ((long)setalarm_playtime) * 1000;
					while ((millis() - zaman) < zamankontrol)
					{
						read_rtc();
						display_rtc_at_lcd();
						delay(50);
					}
					mp3DFPlayer.stop();
					delay(2000);
					if (setderssaati != 0)
					{
						if (setderssaati < 11)
						{
							// kaçýncý ders?
							mp3DFPlayer.playFolder(4, 61 + setderssaati);
						}
						else if (setderssaati == 11)
						{
							// iyi tatiller
							mp3DFPlayer.playFolder(4, 61);
						}
						else if (setderssaati > 11 && setderssaati < 14)
						{
							// 12 ve 13 ise 1.ve 2.etüt saatleri
							mp3DFPlayer.playFolder(4, 72 + setderssaati - 12);
						}
						else if (setderssaati == 14)
						{
							// etüt bitti
							mp3DFPlayer.playFolder(4, 74);
						}
						delay(4000);
					}

					digitalWrite(role_pin, release);

#if defined NOKIALCD
					//display.print("            ", CENTER, 32);
					writeNokia(5, " ", false, CENTER, true);
#else
					writeCenter(7, 8, "                ", false);
#endif
					find_next_alarm();
					isAlarm = false;
					isplaymusic = false;
				}
				else if (setalarmsongnumber == 101 || setalarmsongnumber==103) // yaþasýn müzik çalacaz
				{
#if defined debug
					Serial.println("Muzik caliyor.");
					Serial.print("Dosya sayisi:");
					Serial.println(filecount);
#endif
					rtcClock.clearAlarm1();
					delay(100);
					isAlarm = false;
					digitalWrite(role_pin, pull);
					delay(1500);
					isplaymusic = true;
					playtiming = (long)setalarm_playtime * 60 * 1000;
					songstarttime = millis();
					mp3DFPlayer.volume(muzikvolume);
					delay(50);
					rtcClock.clearAlarm1();
					isAlarm = false;
					musictype = setalarmsongnumber;
					find_next_alarm();
				}
				else if (setalarmsongnumber == 102) // gün bilgilerini söyleyelim
				{
#if defined debug
					Serial.println("Gun bilgileri.");
#endif
					rtcClock.clearAlarm1();
					delay(100);
#if defined NOKIALCD
					writeNokia(5, "Gun Bilgisi", true, CENTER, false);
					//display.invertText(true);	// 102 de gün ve sýcaklýk bilgisini okur.
					//display.print("Gun Bilgisi", CENTER, 32);
					//display.invertText(false);
#else
					writeCenter(7, 8, "Gun bilgisi", true);
#endif
					isAlarm = false;
					digitalWrite(role_pin, pull);
					delay(1500);
					mp3DFPlayer.volume(alarmvolume);
					delay(50);
					rtcClock.clearAlarm1();
					isAlarm = false;
					speech_date_time();
					delay(1000);
					digitalWrite(role_pin, release);
#if defined NOKIALCD
					writeNokia(5, " ", false, CENTER, true);
#else
					writeCenter(7, 8, "                ", false);
#endif
					find_next_alarm();
					isAlarm = false;
					isplaymusic = false;
				}
				else if (setalarmsongnumber == 105)  // random marþ çalacaz
				{
#if defined NOKIALCD
					writeNokia(5, "Mars", true, CENTER, false);
					//display.invertText(true);	
					//display.print("Alarm", CENTER, 32);
					//display.invertText(false);
#else
					writeCenter(7, 8, "Mars", true);
#endif
					rtcClock.clearAlarm1();
					mp3DFPlayer.stop();
					delay(100);
					mp3DFPlayer.volume(muzikvolume);
					read_rtc();
					delay(100);
					isAlarm = false;
					// tamam güzelde marþ çalacazmý bakalým.
					if (marscalinmagunu != dt.day) // gün farklý demekki bugün çalmamýz gerekiyor
					{
						teneffussayisi = 0;
						marscalacakteneffus = random(1, 5);
						marscalinmagunu = dt.day;
						String marsbilgi = String(marscalacakteneffus);
						//marsbilgi = marsbilgi + ".tenef.";
#if defined NOKIALCD
						writeNokia(5, marsbilgi, true, CENTER, false);
#else
						writeCenter(7, 8, marsbilgi, true);
#endif
						delay(5000);
					}
					teneffussayisi += 1;
					if (marscalacakteneffus == teneffussayisi)
					{
						isplaymars = true;
						digitalWrite(role_pin, pull);
						delay(1500);
						mp3DFPlayer.volume(muzikvolume);
						delay(50);
						rtcClock.clearAlarm1();
						isAlarm = false;
						musictype = setalarmsongnumber;
						calinacakmuzik = random(marsstart, zilsayisi + 1);
						String marsbilgi2 = String(calinacakmuzik);
						//marsbilgi = marsbilgi + ".mars";
#if defined NOKIALCD
						writeNokia(5, marsbilgi2, true, CENTER, false);
#else
						writeCenter(7, 8, marsbilgi, true);
#endif						
						mp3DFPlayer.playFolder(1, calinacakmuzik);
						delay(3000);

					}
					find_next_alarm();
				}
			}
			else
			{
				rtcClock.clearAlarm1();
				rtcClock.clearAlarm2();
				writeNokia(5, "Tatil", true, CENTER, true);
				delay(1000);
				writeNokia(5, " ", false, CENTER, true);
				rtcClock.clearAlarm1();
				rtcClock.clearAlarm2();
				isAlarm = false;
				find_next_alarm();
				isAlarm = false;
				isplaymusic = false;
			}
		}
		else
		{
			rtcClock.clearAlarm1();
			isAlarm = false;
			delay(1000);
			isAlarm = false;
			find_next_alarm();
		}
	}
	//digitalWrite(13, LOW);
}
void find_next_alarm()
{
    dt = rtcClock.getDateTime();
	now_dayofweek =  dt.dayOfWeek;
	now_hour = dt.hour;
	now_minute = dt.minute;
	nextalarmminute = 7 * 24 * 60 + 1;
    alarm_find = false;
	int tempalarmdayofweek;
    for (word sayac = 0;sayac < max_length;sayac = sayac + 6)
    {
        eeprom_readed = eeprom2.readByte(sayac);
        tempalarmdayofweek = eeprom_readed;
        if (tempalarmdayofweek == 255)
            break;
	    eeprom_readed = eeprom2.readByte(sayac + 1);
        alarmhour = eeprom_readed;
        eeprom_readed = eeprom2.readByte(sayac + 2);
        alarmminute = eeprom_readed;
        eeprom_readed = eeprom2.readByte(sayac + 3);
        playtime = eeprom_readed;
		eeprom_readed = eeprom2.readByte(sayac + 4);
		alarmsongnumber = eeprom_readed;
		eeprom_readed = eeprom2.readByte(sayac + 5);
		derssaati = eeprom_readed;
		long tempnextalarmminute = (tempalarmdayofweek - now_dayofweek) * 24 * 60 + (alarmhour - now_hour) * 60 + (alarmminute - now_minute);
		if (tempnextalarmminute>0 && tempnextalarmminute < nextalarmminute)
		{
			alarm_find = true;
			alarmdayofweek = tempalarmdayofweek;
			setalarm_hour = alarmhour;
			setalarm_minute = alarmminute;
			setalarm_playtime = playtime;
			setalarmsongnumber = alarmsongnumber;
			setderssaati = derssaati;
			nextalarmminute = tempnextalarmminute;
		}    
    }
	if (!alarm_find)
	{
		nextalarmminute = 7 * 24 * 60 + 1;
		for (word sayac = 0; sayac < max_length; sayac = sayac + 6)
		{
			eeprom_readed = eeprom2.readByte(sayac);
			tempalarmdayofweek = eeprom_readed;
			if (tempalarmdayofweek == 255)
				break;
			eeprom_readed = eeprom2.readByte(sayac + 1);
			alarmhour = eeprom_readed;
			eeprom_readed = eeprom2.readByte(sayac + 2);
			alarmminute = eeprom_readed;
			eeprom_readed = eeprom2.readByte(sayac + 3);
			playtime = eeprom_readed;
			eeprom_readed = eeprom2.readByte(sayac + 4);
			alarmsongnumber = eeprom_readed;
			eeprom_readed = eeprom2.readByte(sayac + 5);
			derssaati = eeprom_readed;
			long tempnextalarmminute = (tempalarmdayofweek - now_dayofweek) * 24 * 60 + (alarmhour - now_hour) * 60 + (alarmminute - now_minute);
			if (tempnextalarmminute < nextalarmminute)
			{
				alarm_find = true;
				alarmdayofweek = tempalarmdayofweek;
				setalarm_hour = alarmhour;
				setalarm_minute = alarmminute;
				setalarm_playtime = playtime;
				setalarmsongnumber = alarmsongnumber;
				setderssaati = derssaati;
				nextalarmminute = tempnextalarmminute;
			}
		}
	}
    if (alarm_find)
    {
        rtcClock.clearAlarm1();
        rtcClock.setAlarm1(alarmdayofweek, setalarm_hour, setalarm_minute, 0, DS3231_MATCH_DY_H_M_S);
		write_next_alarm(alarmdayofweek, setalarm_hour, setalarm_minute, setalarmsongnumber);
    }
}
int mysign = 3;
void kalinanyervesayiyaz()
{
	int signokunan;
	int kalinanyer;
	EEPROM.get(0, signokunan);
	EEPROM.get(2, kalinanyer);
#if defined NOKIALCD
	if (signokunan != mysign)
		writeNokia(2, "EEPROM bos...", true, LEFT, true);
	else
	{
		writeNokia(2, "EEPROM DOLU..", true, LEFT, true);
		writeNokia(3, "KALINAN : " + String(kalinanyer / 2), true, LEFT, true);
	}

#else
	if (signokunan != mysign)
		writeCenter(2, 16, "EEPROM bos...", true);
	else
	{
		writeCenter(2, 16, "EEPROM DOLU..", true);
		writeCenter(3, 16, "KALINAN : " + String(kalinanyer/2), true);
	}
#endif

}

bool hasMusicinEEPROM(int songNumber)
{
	int sign = 0;
	int playedCount = 0;
	int eeadres = 0;
	int readedsong = 0;
	int startadres = 0;
	EEPROM.get(eeadres, sign);
	sign = (int)sign;
	if (sign != mysign)
	{
#if defined (debug)
		Serial.println("EEPROM bombos");
#endif
		savePlayedMusic(songNumber);
		return false;
	}
	eeadres += sizeof(int);
	EEPROM.get(eeadres, playedCount);
	playedCount = (int)playedCount;
#if defined (debug)
	Serial.print("Kaydettigim en son konum : ");
	Serial.println(playedCount);
#endif
	if (playedCount > 0)
	{
		startadres = eeadres + sizeof(int);
		for (int k = startadres; k < (playedCount + sizeof(int)); k += sizeof(int))
		{
			EEPROM.get(k, readedsong);
			readedsong = (int)readedsong;
#if defined (debug)
			Serial.print("Adres : ");
			Serial.print(k);
			Serial.print(" Deger: ");
			Serial.println(readedsong);
#endif
			if (readedsong == songNumber)
			{
#if defined (debug)
				Serial.print("Bu sarki calinmis daha once :");
				Serial.println(songNumber);
#endif
				return true;
			}
		}
		savePlayedMusic(songNumber);
		return false;
	}
}
void savePlayedMusic(int songNumber)
{
#if defined (debug)
	Serial.println("Sarki hic calinmamis. Ekleniyor");
#endif
	int playedCount = 0;
	int eeadres = 0;
	int start = sizeof(int) * 2;
	EEPROM.get(eeadres, playedCount);
	playedCount = (int)playedCount;
#if defined (debug)
	Serial.print("donusmus : ");
	Serial.println(playedCount);
#endif
	if (playedCount != mysign)
	{
#if defined (debug)
		Serial.println("EEPROM a ilk kayit olusturuluyor.");
#endif
		EEPROM.put(eeadres, mysign);
		delay(100);
		eeadres += sizeof(int);
		EEPROM.put(eeadres, start);
		delay(100);
		eeadres = start;
#if defined (debug)
		Serial.print("Yeni adres :");
		Serial.println(eeadres);
#endif
	}
	else
	{
		eeadres += sizeof(int);
		EEPROM.get(eeadres, playedCount);
		playedCount = (int)playedCount;
#if defined (debug)
		Serial.print("Bellekte ");
		Serial.print(playedCount);
		Serial.println(" adresinde kalmisim.");
#endif
		playedCount += sizeof(int);
		if (playedCount > EEPROM.length())
			playedCount = start;
#if defined (debug)
		Serial.print("Yeni kaldigim yer : ");
		Serial.println(playedCount);
#endif
		EEPROM.put(eeadres, playedCount);
		delay(100);
		eeadres = playedCount;
	}
	EEPROM.put(eeadres, songNumber);
	delay(100);
#if defined (debug)
	Serial.print("Yeni yere su sarki kaydedildi : ");
	Serial.println(songNumber);
#endif
}
void speech_date_time()
{
	int start = 40;
	int nem = 41;
	int sicaklik = 42;
	int sicaklikend = 43;
	int eksi = 45;
	int iyidersler = 44;
	int iyimesailer = 48;
	int iyiaksamlar = 49;
	int aystart = 1;
	int gunstart = 13;
	int sifir = 20;
	int on = 30;
	int yuz = 39;
	int dingdong = 47;
	int ikibinyirmi = 50;

	int playgunonlar;
	int playgunbirler;
	int playay;
	int playgunadi;
	int playdereceeksi;
	int playdereceonlar;
	int playderecebirler;
	int playnemonlar;
	int playnembirler;
	int playyil;
#if defined testspeech
	int nemdeneme = 45;
	int sicaklikdeneme = 32;
#else
	int nemdeneme = (int)humidity;
	int sicaklikdeneme =  (int)temperature;
#endif
	if (dt.day < 10)
		playgunonlar = -1;
	else
		playgunonlar = (dt.day / 10) + on - 1;
	if (dt.day % 10 == 0)
		playgunbirler = -1;
	else
		playgunbirler = (dt.day % 10) + sifir;
	
	playay = dt.month + aystart - 1;
	playgunadi = dt.dayOfWeek + gunstart - 1;
	
	if (nemdeneme < 10)
		playnemonlar = -1;
	else
		playnemonlar = (nemdeneme / 10) + on - 1;
	if (nemdeneme % 10 == 0)
		playnembirler = -1;
	else
		playnembirler = (nemdeneme % 10) + sifir;

	if (sicaklikdeneme < 0)
	{
		playdereceeksi = eksi;
		sicaklikdeneme = abs(sicaklikdeneme);
	}
	else
		playdereceeksi = -1;
	if (sicaklikdeneme < 10)
		playdereceonlar = -1;
	else
		playdereceonlar = (sicaklikdeneme / 10) + on - 1;
	if (sicaklikdeneme % 10 == 0)
		playderecebirler = -1;
	else
		playderecebirler = (sicaklikdeneme % 10) + sifir;

	playyil = ikibinyirmi + (dt.year - 2020);
	//önce ding dong yapalým
	mp3DFPlayer.playFolder(4, dingdong);
	while (digitalRead(busy_pin) == not_playing)
		delay(1);
	while (digitalRead(busy_pin) != not_playing)
		delay(1);
	delay(1000);

	// günaydýn. bugün günlerden
	mp3DFPlayer.playFolder(4, start);
	while (digitalRead(busy_pin) == not_playing)
		delay(1);
	while (digitalRead(busy_pin) != not_playing)
		delay(1);

	// (ayýn günü)
	if (playgunonlar != -1)
	{
		mp3DFPlayer.playFolder(4, playgunonlar);
		while (digitalRead(busy_pin) == not_playing)
			delay(1);
		while (digitalRead(busy_pin) != not_playing)
			delay(1);
	}
	if (playgunbirler != -1)
	{
		mp3DFPlayer.playFolder(4, playgunbirler);
		while (digitalRead(busy_pin) == not_playing)
			delay(1);
		while (digitalRead(busy_pin) != not_playing)
			delay(1);
	}
	// (ayýn ismi)
	mp3DFPlayer.playFolder(4, playay);
	while (digitalRead(busy_pin) == not_playing)
		delay(1);
	while (digitalRead(busy_pin) != not_playing)
		delay(1);

	// yýl
	mp3DFPlayer.playFolder(4, playyil);
	while (digitalRead(busy_pin) == not_playing)
		delay(1);
	while (digitalRead(busy_pin) != not_playing)
		delay(1);

	// (günün ismi)
	mp3DFPlayer.playFolder(4, playgunadi);
	while (digitalRead(busy_pin) == not_playing)
		delay(1);
	while (digitalRead(busy_pin) != not_playing)
		delay(1);
	delay(1000);
	/*
	// nem miktarý
	mp3DFPlayer.playFolder(4, nem);
	while (digitalRead(busy_pin) == not_playing)
		delay(1);
	while (digitalRead(busy_pin) != not_playing)
		delay(1);
	// 55
	if (playnemonlar != -1)
	{
		mp3DFPlayer.playFolder(4, playnemonlar);
		while (digitalRead(busy_pin) == not_playing)
			delay(1);
		while (digitalRead(busy_pin) != not_playing)
			delay(1);
	}
	if (playnembirler != -1)
	{
		mp3DFPlayer.playFolder(4, playnembirler);
		while (digitalRead(busy_pin) == not_playing)
			delay(1);
		while (digitalRead(busy_pin) != not_playing)
			delay(1);
	}
	// sýcaklýk 
	mp3DFPlayer.playFolder(4, sicaklik);
	while (digitalRead(busy_pin) == not_playing)
		delay(1);
	while (digitalRead(busy_pin) != not_playing)
		delay(1);

	// derece bilgisi oku 23
	if (playdereceeksi != -1)
	{
		mp3DFPlayer.playFolder(4, playdereceeksi);
		while (digitalRead(busy_pin) == not_playing)
			delay(1);
		while (digitalRead(busy_pin) != not_playing)
			delay(1);
	}
	if (playdereceonlar != -1)
	{
		mp3DFPlayer.playFolder(4, playdereceonlar);
		while (digitalRead(busy_pin) == not_playing)
			delay(1);
		while (digitalRead(busy_pin) != not_playing)
			delay(1);
	}
	if (playderecebirler != -1)
	{
		mp3DFPlayer.playFolder(4, playderecebirler);
		while (digitalRead(busy_pin) == not_playing)
			delay(1);
		while (digitalRead(busy_pin) != not_playing)
			delay(1);
	}
	// derecedir 
	mp3DFPlayer.playFolder(4, sicaklikend);
	while (digitalRead(busy_pin) == not_playing)
		delay(1);
	while (digitalRead(busy_pin) != not_playing)
		delay(1);
	*/
	// iyi dersler
	int temp_play;
	if (setderssaati == 0)
		temp_play = iyidersler;
	else if (setderssaati == 1)
		temp_play = iyimesailer;
	mp3DFPlayer.playFolder(4, temp_play);
	while (digitalRead(busy_pin) == not_playing)
		delay(1);
	while (digitalRead(busy_pin) != not_playing)
		delay(1);



}
//test

//void check_music_test()
//{
//	isplaymusic = true;
//	int calinacakdizin;
//	int calinacakmuzik;
//	if (isplaymusic)
//	{
//		//Serial.println("Busy pin ilk:");
//		//Serial.println(digitalRead(busy_pin));
//
//		//if (digitalRead(busy_pin) == not_playing)
//		//{
//			delay(10000);
//			int sayac = 1;
//			do
//			{
//				calansarki = random(filecount) + 1;
//				if (calansarki > filecount)
//					calansarki = filecount;
//				else if (calansarki == 0)
//					calansarki = 1;
//				if (sayac > 50)
//					break;
//				sayac++;
//			} while (hasMusicinEEPROM(calansarki));
//			Serial.print("Sarki:");
//			Serial.println(calansarki);
//			calinacakdizin = (calansarki / 255)+startdizin;
//			calinacakmuzik = calansarki % 255;
//			Serial.print("Calinacak dizin : ");
//			Serial.print(calinacakdizin);
//			Serial.print(" Calinacak sarki : ");
//			Serial.println(calinacakmuzik);
//			mp3DFPlayer.playFolder(7, calinacakmuzik);
//			delay(1000);
//#if defined NOKIALCD
//			display.invertText(true);
//			display.print("                ", CENTER, 32);
//			display.print("Muzik", LEFT, 32);
//			display.printNumI(calansarki, RIGHT, 32);
//			display.invertText(false);
//#else
//			String muzik = String("Muzik   : ");
//			String calan = String(calansarki);
//			muzik = muzik + calan;
//			writeCenter(7, 8, muzik, true);
//#endif
//			//Serial.println("Busy pin sonra:");
//			//Serial.println(digitalRead(busy_pin));
//
//		//}
//		//Serial.print(millis() - songstarttime);
//		//Serial.print("--");
//		//Serial.println(playtiming);
//		/*if ((millis() - songstarttime) > playtiming)
//		{
//			isplaymusic = false;
//			mp3DFPlayer.stop();
//			delay(50);
//			digitalWrite(role_pin, LOW);
//			mp3DFPlayer.volume(25);
//			delay(50);
//			display.print("           ", CENTER, 32);
//			delay(1000);
//			mp3DFPlayer.reset();
//			delay(1000);
//		}*/
//	}
//}

bool isHoliday()
{
	word holidayadress = holidayaddress;
	byte okunanay=0;
	byte okunangun = 0;
	for (word adres = holidayadress; adres < max_length; adres+=2)
	{
		okunanay = eeprom2.readByte(adres);
		okunangun = eeprom2.readByte(adres + 1);
		if (okunanay == 255)
			return false;
		if ((uint8_t)okunanay == dt.month && (uint8_t)okunangun == dt.day)
			return true;
	}
	return false;
}
int getNextSatir()
{
	satirsira++;
#if defined NOKIALCD
	if (satirsira == 7)
		satirsira = 1;
#else
	if (satirsira == 9)
		satirsira = 1;
#endif
	return satirsira;
}