#ifndef LILKA_RESOURCES_H
#define LILKA_RESOURCES_H

#include <Arduino.h>

namespace lilka {

/// Зображення
///
/// Містить розміри, прозорий колір та пікселі зображення (в 16-бітному форматі, 5-6-5).
/// Пікселі зберігаються в рядку зліва направо, зверху вниз.
class Bitmap {
public:
    inline Bitmap(uint32_t width, uint32_t height, int32_t transparentColor = -1)
        : width(width), height(height), transparentColor(transparentColor) {
        pixels = new uint16_t[width * height];
    }
    inline ~Bitmap() {
        delete[] pixels;
    }
    uint32_t width;
    uint32_t height;
    /// 16-бітний колір (5-6-5), який буде прозорим. За замовчуванням -1 (прозорість відсутня).
    int32_t transparentColor;
    uint16_t *pixels;
};

/// Клас для завантаження ресурсів.
class Resources {
public:
    /// Завантажити зображення в форматі BMP з файлу.
    ///
    /// \param filename Шлях до файлу.
    /// \param transparentColor 16-бітний колір (5-6-5), який буде прозорим. За замовчуванням -1 (прозорість відсутня).
    /// \return Вказівник на зображення.
    ///
    /// \warning Пам'ять для зображення виділяється динамічно. Після використання зображення, його потрібно видалити за
    /// допомогою `delete`.
    ///
    /// Приклад:
    ///
    /// \code
    /// lilka::Bitmap *bitmap = lilka::resources.loadBitmap("image.bmp", lilka::display.color565(255, 255, 0)); //
    /// Жовтий колір буде прозорим if (!bitmap) {
    ///     Serial.println("Failed to load image");
    ///     return;
    /// }
    /// // Відобразити зображення на екрані
    /// lilka::display.draw16bitRGBBitmapWithTranColor(50, 100, bitmap->pixels, bitmap->transparentColor, bitmap->width,
    /// bitmap->height);
    /// // Звільнити пам'ять
    /// delete bitmap;
    /// \endcode
    Bitmap *loadBitmap(String filename, int32_t transparentColor = -1);
    /// Прочитати вміст файлу.
    ///
    /// TODO: Update sdcard/filesystem stuff
    ///
    /// \param filename Шлях до файлу.
    /// \return Вміст файлу.
    /// \warning Не використовуйте цей метод для читання великих файлів, оскільки весь вміст файлу зберігається в
    /// пам'яті. Для великих файлів використовуйте методи `sdcard` та `filesystem`.
    int readFile(String filename, String &fileContent);
};

/// Екземпляр класу `Resources`, який можна використовувати для завантаження ресурсів.
/// Вам не потрібно інстанціювати `Resources` вручну.
extern Resources resources;

} // namespace lilka

#endif // LILKA_RESOURCES_H
