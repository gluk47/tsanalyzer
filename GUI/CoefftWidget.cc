#include "CoefftWidget.h"
#include "ui_CoefftWidget.h"

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
