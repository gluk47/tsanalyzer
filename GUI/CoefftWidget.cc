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
