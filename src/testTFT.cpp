#include "main.h"
// #include "my_TFT_eSPI.h"
#include "TFT_eSPI.h"
extern TFT_eSPI tft;
// extern XPT2046_Touchscreen ts;

void initTFT0(void){
  DEBUG_PRINTLN();
  DEBUG_PRINTLN("TFT_eSPI Test!");

  tft.init(); // Инициализация дисплея
  tft.setRotation(0); // Устанавливаем поворот (0-3)

  tft.fillScreen(TFT_BLACK); // Заливаем экран черным

  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Белый текст на черном фоне
  tft.setCursor(10, 10);
  tft.setTextSize(3); // Устанавливаем размер текста

  // TFT_eSPI может работать с UTF-8, если шрифт содержит символы!
  // Для растровых шрифтов GFX это не сработает так просто.
  // Для Smooth Fonts (vlw) с кириллицей - должно работать.
  tft.println("Hello World!");
  tft.setTextSize(2); // Устанавливаем размер текста
  tft.println("Hello World!");
  tft.setTextSize(1); // Устанавливаем размер текста
  tft.println("Hello World!");

  // --- Для кириллицы (если у вас есть шрифт .vlw с кириллицей) ---
  // 1. Поместите .vlw файл в папку 'data' проекта
  // 2. Настройте LittleFS/LittleFS в platformio.ini и загрузите файловую систему
  // 3. Используйте tft.loadFont("MyCyrillicFontName") и tft.drawString()
  // Пример (псевдокод, требует настройки FS и шрифта):
  /*
  if (LittleFS.begin()) {
      if (tft.loadFont("CyrillicFont-24", LittleFS)) { // Имя файла шрифта
          tft.drawString("Привет, Мир!", 10, 50);
          tft.unloadFont();
      } else {
          tft.drawString("Font Error", 10, 50);
      }
      LittleFS.end();
  }
  */
}

/* void touchTest(){
    TS_Point p = ts.getPoint(); // Получаем точку касания

    // Выводим "сырые" координаты в Serial порт
    DEBUG_PRINT("Raw X = "); DEBUG_PRINT(p.x);
    DEBUG_PRINT(", Y = "); DEBUG_PRINT(p.y);
    DEBUG_PRINT(", Pressure Z = "); DEBUG_PRINTLN(p.z);

    // --- Преобразование "сырых" координат в координаты экрана ---
    // Используем функцию map(). ВАЖНО: TS_MIN/MAX нужно откалибровать!
    // Также учтите, что оси X/Y могут быть инвертированы или поменяны местами
    // в зависимости от ориентации и вашего экрана. Этот код - лишь пример!
    int screenX = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
    int screenY = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

    // Иногда оси нужно инвертировать:
    // screenX = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
    // screenY = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);

    DEBUG_PRINT("Mapped X = "); DEBUG_PRINT(screenX);
    DEBUG_PRINT(", Y = "); DEBUG_PRINTLN(screenY);

    // Рисуем точку (маленький круг) на экране в месте касания
    tft.fillCircle(screenX, screenY, 3, ILI9341_RED);
} */