#ifndef COEFFTWIDGET_H
#define COEFFTWIDGET_H

#include <QWidget>

namespace Ui {
class CoefftWidget;
}

class CoefftWidget : public QWidget {
    Q_OBJECT

public:
    CoefftWidget(const QString& letter, QWidget *parent = 0);
    ~CoefftWidget();

    struct info {
        double min, max, step;
    };
    info getInfo ();

public slots:
    /// Set the minimal displayed value (but not the limit for it)
    void setMin(double);
    /// Set the maximal displayed value (but not the limit for it)
    void setMax(double);
    /// Set the value step
    void setStep(double);
    /// Set the limits for the value, thus limiting minimal and maximal value.
    /// These values will be corrected if needed.
    void setLimits(double minimum, double maximum);

private slots:
    void on_valueMin_valueChanged(double arg1);

    void on_valueMax_valueChanged(double arg1);

private:
    double _Minimum = -32000, _Maximum = 32000;
    Ui::CoefftWidget *ui;

    bool updatingValues = false;
};

#endif // COEFFTWIDGET_H
