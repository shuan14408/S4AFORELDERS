/* ============================================================
   第三組：樂齡甩手棒 PK（含 LCD1602 螢幕）
   玩法：按任一顆大按鈕開始，LCD 顯示 START! 後倒數 60 秒，
         兩人用力甩，螢幕即時顯示剩餘秒數和兩人次數；
         時間到，贏家的按鈕閃爍 + LCD 顯示誰贏。
   接線：
     LCD1602(I2C)：VCC→5V(紅排)、GND→GND、SDA→A4、SCL→A5
     甩手棒滾珠模組A/B：VCC→紅排、GND→藍排、DO→D2/D3
     玩家A按鈕：燈→D8、開關→D10 ／ 玩家B按鈕：燈→D9、開關→D11
     蜂鳴器 +→D7、-→GND
   ★需先安裝程式庫：工具→管理程式庫→搜尋「LiquidCrystal I2C」
     （Frank de Brabander 版）按安裝。
   ★LCD 沒畫面：把 0x27 改成 0x3F；有亮但沒字：轉背面對比旋鈕。
   ============================================================ */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);   // 沒畫面就改成 0x3F

const int STICK_A = 2, STICK_B = 3;
const int LED_A = 8,  LED_B = 9;     // 按鈕內建燈
const int BTN_A = 10, BTN_B = 11;    // 按鈕開關
const int BUZZER = 7;
const unsigned long GAME_MS = 60000UL;   // 比賽 60 秒（可調）

long countA = 0, countB = 0;
int lastA = HIGH, lastB = HIGH;
bool running = false;
unsigned long endTime = 0;
int lastSec = -1;

void showReady() {                    // 待機畫面
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("SWING  PK");
  lcd.setCursor(0, 1); lcd.print("PRESS BUTTON!");
}

void showCounts() {                   // 第二行：兩人次數
  lcd.setCursor(0, 1);
  lcd.print("A:");  lcd.print(countA);
  lcd.print("  B:"); lcd.print(countB);
  lcd.print("   ");
}

void finish() {                       // 時間到：判勝負
  running = false;
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("A:"); lcd.print(countA); lcd.print("  B:"); lcd.print(countB);
  lcd.setCursor(0, 0);
  int led = -1;
  if (countA > countB)      { lcd.print("A WINS!"); led = LED_A; }
  else if (countB > countA) { lcd.print("B WINS!"); led = LED_B; }
  else                      { lcd.print("TIE!"); }
  if (led >= 0) {
    for (int i = 0; i < 6; i++) {
      digitalWrite(led, HIGH); tone(BUZZER, 1047, 150); delay(180);
      digitalWrite(led, LOW);  delay(120);
    }
  } else tone(BUZZER, 440, 500);      // 平手
  delay(1500);
  showReady();
}

void setup() {
  pinMode(STICK_A, INPUT_PULLUP); pinMode(STICK_B, INPUT_PULLUP);
  pinMode(BTN_A, INPUT_PULLUP);   pinMode(BTN_B, INPUT_PULLUP);
  pinMode(LED_A, OUTPUT); pinMode(LED_B, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  lcd.init();                         // 部分版本程式庫要改成 lcd.begin();
  lcd.backlight();
  showReady();
}

void loop() {
  // 按任一顆按鈕 → 顯示 START! 並開始 60 秒
  if (!running && (digitalRead(BTN_A) == LOW || digitalRead(BTN_B) == LOW)) {
    countA = 0; countB = 0; lastSec = -1;
    lcd.clear(); lcd.setCursor(5, 0); lcd.print("START!");
    digitalWrite(LED_A, HIGH); digitalWrite(LED_B, HIGH);
    tone(BUZZER, 1047, 300); delay(800);
    digitalWrite(LED_A, LOW); digitalWrite(LED_B, LOW);
    lcd.clear(); showCounts();
    running = true;
    endTime = millis() + GAME_MS;
  }

  if (running) {
    int a = digitalRead(STICK_A), b = digitalRead(STICK_B);
    bool changed = false;
    if (a != lastA) { countA++; tone(BUZZER, 1500, 15); changed = true; } lastA = a;
    if (b != lastB) { countB++; tone(BUZZER, 1800, 15); changed = true; } lastB = b;

    long remain = (long)(endTime - millis());
    if (remain < 0) remain = 0;
    int sec = (remain + 999) / 1000;
    if (sec != lastSec) {             // 每秒更新一次倒數
      lastSec = sec;
      lcd.setCursor(0, 0);
      lcd.print("TIME: "); lcd.print(sec); lcd.print("s   ");
    }
    if (changed) showCounts();        // 次數有變才更新，避免閃爍

    if (remain == 0) finish();
  }
  delay(5);
}
