#ifndef COEFFTWIDGET_H
#define COEFFTWIDGET_H

#include <QWidget>

namespace Ui {
class CoefftWidget;
}

class CoefftWidget : public QWidget
{
    Q_OBJECT

public:
    CoefftWidget(const QString& letter, QWidget *parent = 0);
    ~CoefftWidget();

    struct info {
        double min, max, step;
    };
    info getInfo ();

private:
    Ui::CoefftWidget *ui;
};

#endif // COEFFTWIDGET_H
