#include "CoefftWidget.h"
#include "ui_CoefftWidget.h"
#include "helpers.h"

CoefftWidget::CoefftWidget(const QString& letter, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CoefftWidget) {
    ui->setupUi(this);
    ui->labelLetter->setText(letter);
}

CoefftWidget::~CoefftWidget() {
    delete ui;
}

CoefftWidget::info CoefftWidget::getInfo() {
    return info {
        ui->valueMin->value(),
        ui->valueMax->value(),
        ui->valuesStep->value()
    };
}

void CoefftWidget::setMin(double v) {
    ui->valueMin->setValue(v);
}

void CoefftWidget::setMax(double v) {
    ui->valueMax->setValue(v);
}

void CoefftWidget::setStep(double v) {
    ui->valuesStep->setValue(v);
}

void CoefftWidget::setLimits(double minimum, double maximum) {
    _Minimum = minimum;
    _Maximum = maximum;

    ui->valueMin->setMinimum(minimum);
    ui->valueMin->setMaximum(maximum);

    ui->valueMax->setMinimum(minimum);
    ui->valueMax->setMaximum(maximum);
}

void CoefftWidget::on_valueMin_valueChanged(double vmin) {
    if (ui->valueMax->value() < vmin)
        ui->valueMax->setValue(vmin);
}

void CoefftWidget::on_valueMax_valueChanged(double vmax) {
    if (ui->valueMin->value() > vmax)
        ui->valueMin->setValue(vmax);
}
