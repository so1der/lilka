#include "display.h"

#include "default_splash.h"
#include "spi.h"
#include "serial.h"
#include "fmath.h"
#include "buzzer.h"

namespace lilka {

#if LILKA_VERSION == 1
Arduino_ESP32SPI displayBus(LILKA_DISPLAY_DC, LILKA_DISPLAY_CS, LILKA_SPI_SCK, LILKA_SPI_MOSI);
#else
// Arduino_ESP32SPI displayBus(
//     LILKA_DISPLAY_DC, LILKA_DISPLAY_CS, LILKA_SPI_SCK, LILKA_SPI_MOSI, LILKA_SPI_MISO, SPI1_NUM, true
// );
Arduino_HWSPI displayBus(
    LILKA_DISPLAY_DC, LILKA_DISPLAY_CS, LILKA_SPI_SCK, LILKA_SPI_MOSI, LILKA_SPI_MISO, &SPI1, true
);
#endif

Display::Display() :
    Arduino_ST7789(
        // &displayBus, LILKA_DISPLAY_RST, LILKA_DISPLAY_ROTATION, true, LILKA_DISPLAY_WIDTH, LILKA_DISPLAY_HEIGHT, 0, 20
        &displayBus, LILKA_DISPLAY_RST, LILKA_DISPLAY_ROTATION, true, LILKA_DISPLAY_WIDTH, LILKA_DISPLAY_HEIGHT, 0, 20,
        0, 20
    ),
    splash(NULL) {
}

void Display::begin() {
    serial_log("initializing display");
#ifdef LILKA_BREADBOARD
    Arduino_ST7789::begin(40000000);
#else
    Arduino_ST7789::begin(80000000);
#endif
    setFont(FONT_10x20);
    setUTF8Print(true);
#ifdef LILKA_NO_SPLASH
    if (splash != NULL) {
#else
    if (splash == NULL) {
        splash = default_splash;
    }
#endif
        uint16_t row[default_splash_width];
        for (int i = 0; i <= 4; i++) {
            startWrite();
            writeAddrWindow(0, 0, default_splash_width, default_splash_height);
            for (int y = 0; y < default_splash_height; y++) {
                for (int x = 0; x < default_splash_width; x++) {
                    uint16_t color = splash[y * default_splash_width + x];
                    uint16_t r = ((color >> 11) & 0x1F) << 3;
                    uint16_t g = ((color >> 5) & 0x3F) << 2;
                    uint16_t b = (color & 0x1F) << 3;
                    row[x] = color565(r * i / 4, g * i / 4, b * i / 4);
                }
                writePixels(row, default_splash_width);
            }
            endWrite();
        }
        // TODO: Should not be here. Треба кудись винести.
        // const Tone helloTune[] = {{NOTE_C4, 8}, {NOTE_E4, 8}, {NOTE_E5, -4}, {NOTE_C6, 8}, {NOTE_C5, 8}};
        const Tone helloTune[] = {{NOTE_C3, 8}, {NOTE_C4, 8}, {NOTE_C5, 8}, {NOTE_C7, 4}, {0, 8}, {NOTE_C6, 4}};
        buzzer.playMelody(helloTune, sizeof(helloTune) / sizeof(Tone), 160);
        delay(800);
        for (int i = 4; i >= 0; i--) {
            startWrite();
            writeAddrWindow(0, 0, default_splash_width, default_splash_height);
            for (int y = 0; y < default_splash_height; y++) {
                for (int x = 0; x < default_splash_width; x++) {
                    uint16_t color = splash[y * default_splash_width + x];
                    uint16_t r = ((color >> 11) & 0x1F) << 3;
                    uint16_t g = ((color >> 5) & 0x3F) << 2;
                    uint16_t b = (color & 0x1F) << 3;
                    row[x] = color565(r * i / 4, g * i / 4, b * i / 4);
                }
                writePixels(row, default_splash_width);
            }
            endWrite();
        }
#ifdef LILKA_NO_SPLASH
    }
#else
#endif
    serial_log("display ok");
}

void Display::setSplash(const uint16_t* splash) {
    this->splash = splash;
}

void Display::drawImage(Image* image, int16_t x, int16_t y) {
    if (image->transparentColor == -1) {
        draw16bitRGBBitmap(x, y, image->pixels, image->width, image->height);
    } else {
        draw16bitRGBBitmapWithTranColor(x, y, image->pixels, image->transparentColor, image->width, image->height);
    }
}

// Чомусь в Arduino_GFX немає варіанту цього методу для const uint16_t[] - є лише для uint16_t.
void Display::draw16bitRGBBitmapWithTranColor(
    int16_t x, int16_t y, const uint16_t bitmap[], uint16_t transparent_color, int16_t w, int16_t h
) {
    // Цей cast безпечний, оскільки Arduino_GFX.draw16bitRGBBitmapWithTranColor не змінює bitmap.
    Arduino_ST7789::draw16bitRGBBitmapWithTranColor(x, y, const_cast<uint16_t*>(bitmap), transparent_color, w, h);
}

void Display::renderCanvas(Canvas* canvas) {
    draw16bitRGBBitmap(0, 0, canvas->getFramebuffer(), canvas->width(), canvas->height());
}

Canvas::Canvas() : Arduino_Canvas(LILKA_DISPLAY_WIDTH, LILKA_DISPLAY_HEIGHT, NULL) {
    setFont(u8g2_font_10x20_t_cyrillic);
    setUTF8Print(true);
    begin();
}

Canvas::Canvas(uint16_t width, uint16_t height) : Arduino_Canvas(width, height, NULL) {
    setFont(u8g2_font_10x20_t_cyrillic);
    setUTF8Print(true);
    begin();
}

Canvas::Canvas(uint16_t x, uint16_t y, uint16_t width, uint16_t height) :
    Arduino_Canvas(width, height, NULL, x, y, 0) { // TODO: Rotation
    setFont(u8g2_font_10x20_t_cyrillic);
    setUTF8Print(true);
    begin();
}

void Canvas::drawImage(Image* image, int16_t x, int16_t y) {
    if (image->transparentColor == -1) {
        draw16bitRGBBitmap(x, y, image->pixels, image->width, image->height);
    } else {
        draw16bitRGBBitmapWithTranColor(x, y, image->pixels, image->transparentColor, image->width, image->height);
    }
}

void Canvas::draw16bitRGBBitmapWithTranColor(
    int16_t x, int16_t y, const uint16_t bitmap[], uint16_t transparent_color, int16_t w, int16_t h
) {
    // Цей cast безпечний, оскільки Arduino_GFX.draw16bitRGBBitmapWithTranColor не змінює bitmap.
    Arduino_Canvas::draw16bitRGBBitmapWithTranColor(x, y, const_cast<uint16_t* const>(bitmap), transparent_color, w, h);
    // Arduino_Canvas::draw16bitRGBBitmapWithTranColor(x, y, (uint16_t *)(bitmap), transparent_color, w, h);
}

void Canvas::drawCanvas(Canvas* canvas) {
    draw16bitRGBBitmap(0, 0, canvas->getFramebuffer(), canvas->width(), canvas->height());
}

int16_t Canvas::x() {
    return _output_x;
}

int16_t Canvas::y() {
    return _output_y;
}

Image::Image(uint32_t width, uint32_t height, int32_t transparentColor) :
    width(width), height(height), transparentColor(transparentColor) {
    // Allocate pixels in PSRAM
    pixels = static_cast<uint16_t*>(ps_malloc(width * height * sizeof(uint16_t)));
}

Image::~Image() {
    delete[] pixels;
}

void Image::rotate(int16_t angle, Image* dest, int32_t blankColor) {
    // Rotate the image clockwise (Y-axis points down)
    int cx = width / 2;
    int cy = height / 2;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int dx = x - cx;
            int dy = y - cy;
            int x2 = cx + dx * fCos360(angle) + dy * fSin360(angle);
            int y2 = cy - dx * fSin360(angle) + dy * fCos360(angle);
            if (x2 >= 0 && x2 < width && y2 >= 0 && y2 < height) {
                dest->pixels[x + y * width] = pixels[x2 + y2 * width];
            } else {
                dest->pixels[x + y * width] = blankColor;
            }
        }
    }
}

void Image::flipX(Image* dest) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            dest->pixels[x + y * width] = pixels[(width - 1 - x) + y * width];
        }
    }
}

void Image::flipY(Image* dest) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            dest->pixels[x + y * width] = pixels[x + (height - 1 - y) * width];
        }
    }
}

Display display;

} // namespace lilka
