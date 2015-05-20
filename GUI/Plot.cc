#include "Plot.h"
#include "ui_Plot.h"
#include "helpers.h"

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QTextStream>

#include "helpers.h"
#include "qcustomplot.h"

Plot::Plot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Plot) {
    ui->setupUi(this);

    connect (ui->setPath, SIGNAL(textChanged(QString)),
             this, SLOT(validatePath()));
    connect (ui->refreshPlot, SIGNAL(clicked()),
             this, SLOT(refreshPlot()));

    ui->progressBar->hide();


    for (size_t i = 0; i < coordinates_t::nValues; ++i) {
        ui->axisX->addItem(coordinates_t::coordinateName(i));
        ui->axisY->addItem(coordinates_t::coordinateName(i));
    }

    ui->plotWidget->setInteraction(QCP::iRangeDrag, true);
    ui->plotWidget->setInteraction(QCP::iRangeZoom, true);

    validatePath();
}

Plot::~Plot() {
    delete ui;
}

void Plot::validatePath() {
    bool exists = QDir(ui->setPath->text()).exists();
    setErrorBackground(ui->setPath, not exists,
                       tr("Папка с выборкой временных рядов"));
    ui->refreshPlot->setEnabled(exists);
}

void Plot::on_browseSetPath_clicked() {
    auto ans = QFileDialog::getExistingDirectory(this,
                                 tr("Укажите папку с выборкой рядов"),
                                 ui->setPath->text());
    if (not ans.isNull())
        ui->setPath->setText(ans);
}

void Plot::refreshPlot() {
    enableControls(false);
    ui->progressBar->show();
    scope_exit restore_controls ([this]{
        ui->progressBar->hide();
        enableControls(true);
    });

    QApplication::processEvents();

    QDir setDir (ui->setPath->text());
    setDir.cd("processed"); //if exists
    ui->progressBar->setValue(0);
    QStringList lst = setDir.entryList(QStringList() << "*.coords",
                                       QDir::Readable | QDir::Files);
    if (lst.isEmpty()) {
        qDebug () << "";
        return;
    }

    ui->progressBar->setMaximum(lst.size());
    QApplication::processEvents();
//    scene.clear();

    ui->plotWidget->addGraph(ui->plotWidget->yAxis, ui->plotWidget->xAxis);
    ui->plotWidget->graph(0)->setPen(QColor(50, 50, 50, 255));
    ui->plotWidget->graph(0)->setLineStyle(QCPGraph::lsNone);
    ui->plotWidget->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 4));

    ui->plotWidget->xAxis->setLabel(coordinates_t::coordinateName(ui->axisX->currentIndex()));
    ui->plotWidget->yAxis->setLabel(coordinates_t::coordinateName(ui->axisY->currentIndex()));

    QVector <double> x, y;

    x.reserve(lst.size());
    y.reserve(lst.size());

    double xmin = 0, xmax = 5, ymin = 0, ymax = 5;

    auto read_file = [setDir, this](const QString& fname, double& x, double& y){
        QFile file (setDir.filePath(fname));
        file.open(QIODevice::ReadOnly);
        QTextStream coordinates (&file);
        coordinates_t point;
        for (size_t j = 0; j < coordinates_t::nValues; ++j)
            coordinates >> point.values[j];
        x = point.values[ui->axisX->currentIndex()];
        y = point.values[ui->axisY->currentIndex()];
        return std::isfinite(x) and std::isfinite(y);
    };

    double x1, y1;
    for (const QString& fname : lst) {
        if (not read_file (fname, x1, y1))
            continue;
        xmin = xmax = x1;
        ymin = ymax = y1;
        break;
    }

    int progress = 0;
    for (QString fname : lst) {
        if (not read_file (fname, x1, y1))
            continue;
        if (std::isnan(x1) or std::isnan(y1))
            // that means that the coordinate is not applicable to the series, just ignore this point
            continue;
        x.append (x1);
        xmin = std::min (xmin, x1);
        xmax = std::max (xmax, x1);
        y.append (y1);
        ymin = std::min (ymin, y1);
        ymax = std::max (ymax, y1);
        ui->progressBar->setValue(++progress);
        if (progress % 20 == 0)
            QApplication::processEvents();
    }

    ui->plotWidget->graph(0)->setData(y, x);
    qDebug () << "Range:" << xmin << ".." << xmax << "×"
              << ymin << ".." << ymax;

    xmin *= 0.98;
    xmax *= 1.02;

    ymin *= 0.98;
    ymax *= 1.02;

    ui->plotWidget->xAxis->setRange(xmin, xmax);
    ui->plotWidget->yAxis->setRange(ymin, ymax);
    ui->plotWidget->replot();
}

void Plot::enableControls(bool _) {
    ui->refreshPlot->setEnabled(_);
    ui->axisX->setEnabled(_);
    ui->axisY->setEnabled(_);
    ui->setPath->setEnabled(_);
    ui->browseSetPath->setEnabled(_);
}

void Plot::on_savePlot_clicked() {
    auto fname = QFileDialog::getSaveFileName(this,
                                              tr("Укажите папку с выборкой рядов"),
                                              ui->setPath->text(),
                                              tr("PDF (*.pdf);;"
                                                 "Png image (*.png);;"
                                                 "Jpeg image(*.jpg)"));
    if (fname.isNull())
        return;

    QFileInfo file (fname);

    if (file.suffix().toLower() == "pdf")
        ui->plotWidget->savePdf(fname);
    else if (file.suffix().toLower() == "png")
        ui->plotWidget->savePng(fname);
    else if (file.suffix().toLower() == "jpg")
        ui->plotWidget->saveJpg(fname);
}

void Plot::on_saveAll_clicked() {
    const auto x = ui->axisX->currentIndex(), y = ui->axisY->currentIndex();
    scope_exit restore ([this, x, y]{
        ui->axisX->setCurrentIndex(x);
        ui->axisY->setCurrentIndex(y);
        refreshPlot();
    });

    QDir target (ui->setPath->text());
    target.mkdir("plots");
    target.cd("plots");

    QString cmdline;
    for (unsigned i = 0; i < coordinates_t::nValues - 1; ++i)
        for (unsigned j = i + 1; j < coordinates_t::nValues; ++j) {
            ui->axisX->setCurrentIndex(i);
            ui->axisY->setCurrentIndex(j);
            refreshPlot();
            auto fname = target.absoluteFilePath(QString("%1×%2.pdf")
                                                 .arg(coordinates_t::coordinateName(i))
                                                 .arg(coordinates_t::coordinateName(j)));
            cmdline += QString ("'%1' ").arg(fname);
            ui->plotWidget->savePdf(fname);
        }

    system (QString ("pdftk %1 output '%2'")
            .arg(cmdline)
            .arg(target.absoluteFilePath("plots.pdf"))
            .toLocal8Bit());
}
