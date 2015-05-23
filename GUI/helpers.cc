#include "helpers.h"
#include <QWidget>
#include <QString>
#include <cmath>

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
    case 2: return "Фрактальная размерность";
    case 3: return "Фрактальная размерность тенденций";
    case 4: return "Символьное разнообразие: окно";
    case 5: return "Символьное разнообразие тенденций: окно";
    case 6: return "Символьное разнообразие: разность";
    case 7: return "Символьное разнообразие тенденций: разность";
    case 8: return "Колмогоровская сложность";
    case 9: return "Колмогоровская сложность тенденций";
    }
    return "???";
}

QString plural (const char* base,
                const char* one,
                const char* some,
                const char* many,
                int value,
                bool prepend_number) {
    value = abs (value);
    QString ans = base;
    if (prepend_number)
        ans = QString::number(value) + ans;
    if (10 < value and value < 20)
        return ans + many;

    char rest = value % 10;
    if (rest == 1)
        return ans + one;
    if (0 < rest and rest < 4)
        return ans + some;
    else
        return ans + many;
}
