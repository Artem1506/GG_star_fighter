#define ST7735_DRIVER       // Выбираем драйвер
#define TFT_WIDTH  128
#define TFT_HEIGHT 160

#define TFT_MOSI  23
#define TFT_SCLK  18
#define TFT_CS     5
#define TFT_DC    21
#define TFT_RST    4  // Или -1, если ножка не подключена

#define LOAD_GLCD     // Загружаем стандартный шрифт
#define LOAD_FONT2    // Маленький шрифт
#define LOAD_FONT4    // Средний
#define LOAD_FONT6    // Большой числовой
#define LOAD_FONT7    // Очень большой числовой
#define LOAD_FONT8    // Монширфт

#define SPI_FREQUENCY  27000000  // Увеличиваем частоту SPI до 27 MHz (можно 40000000)
