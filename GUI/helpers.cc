#include "helpers.h"
#include <QWidget>
#include <QString>

void setErrorBackground(QWidget *w,
                        bool is_error,
                        const QString &noerrorToolTip,
                        const QString &errorToolTip) {
    if (is_error) {
        w->setStyleSheet("background-color: rgb(255, 200, 200);");
        w->setToolTip(errorToolTip);
    } else {
        w->setStyleSheet("background-color: rgb(200, 255, 200);");
        w->setToolTip(noerrorToolTip);
    }
}


const char* coordinates_t::coordinateName(size_t index) {
    switch (index) {
    case 0: return "Гармоническая сложность";
    case 1: return "Гармоническая сложность тенденций";
    case 2: return "Колмогоровская сложность";
    case 3: return "Колмогоровская сложность тенденций";
    case 4: return "Фрактальная размерность";
    case 5: return "Фрактальная размерность тенденций";
    case 6: return "Мера символьного разнообразия";
    case 7: return "Мера символьного разнообразия тенденций";
    }
    return "???";
}
