#include "AnalyzeWidget.h"
#include "ui_AnalyzeWidget.h"

#include "helpers.h"
#include "TimeSeries.h"

#include <assert.h>
#include <future>
#include <deque>

#include <QDebug>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QTextStream>

AnalyzeWidget::AnalyzeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AnalyzeWidget) {
    ui->setupUi(this);
    ui->status->hide();
    ui->progressBar->hide();

    ui->correlations->setRowCount(coordinates_t::nValues);
    ui->correlations->setColumnCount(coordinates_t::nValues);
    for (size_t i = 0; i < coordinates_t::nValues; ++i) {
        QString label = coordinates_t::coordinateName(i);
        label.replace(' ', '\n');
        ui->correlations->setHorizontalHeaderItem(i, new QTableWidgetItem(label));
        ui->correlations->setVerticalHeaderItem  (i, new QTableWidgetItem(label));
    }
    ui->correlations->hide();
    ui->correlations->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->correlations->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    correlations_label = new QLabel(tr("Матрица корреляций"), ui->correlations);
    correlations_label->setAlignment(Qt::AlignCenter);
    correlations_label->setAttribute(Qt::WA_TransparentForMouseEvents);
    correlations_label->setWordWrap(true);

    connect(ui->correlations->verticalHeader(), &QHeaderView::geometriesChanged,
            this, &AnalyzeWidget::resizeCorrelations);
    connect(ui->correlations->horizontalHeader(), &QHeaderView::geometriesChanged,
            this, &AnalyzeWidget::resizeCorrelations);

    checkPaths();
    correlations_label->hide();
}

AnalyzeWidget::~AnalyzeWidget() {
    delete ui;
}

void AnalyzeWidget::on_browseSetPath_clicked() {
    auto ans = QFileDialog::getExistingDirectory(this,
                                 tr("Укажите папку с выборкой рядов"),
                                 ui->setPath->text());
    if (not ans.isNull())
        ui->setPath->setText(ans);
    ui->status->hide();
}

void processSeries (const QDir& destDir, const QDir& dir, const QString& fname) {
    QFile coordFile (destDir.absoluteFilePath(fname + ".coords"));
    if (coordFile.exists())
            return;

    TimeSeries ts;
    ts.readFile(dir.absoluteFilePath(fname));
    TimeSeries tendency_ts = ts.tendencySeries();

    QFile codedfile (destDir.absoluteFilePath(fname));
    codedfile.open (QIODevice::WriteOnly | QIODevice::Truncate);
    auto coded = ts.encoded();
    for (unsigned c : coded)
        codedfile.write(reinterpret_cast <const char*> (&c), sizeof (c));
    codedfile.close();

    QFile tendencyFile (destDir.absoluteFilePath(fname + ".tendency"));
    tendencyFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    for (unsigned c : tendency_ts.values())
        tendencyFile.write(reinterpret_cast <const char*>(&c), sizeof(c));
    tendencyFile.close();

    QProcess arc;
    arc.start("../lzma e -so \"" + codedfile.fileName() + "\"");
    QProcess t_arc;
    t_arc.start("../lzma e -so \"" + tendencyFile.fileName() + "\"");

    coordFile.open (QIODevice::WriteOnly | QIODevice::Truncate);
    QTextStream coords (&coordFile);

    coords << ts.harmonicComplexity() << "\n"
           << tendency_ts.harmonicComplexity() << "\n";

    coords << ts.fractalDimensionality() << "\n"
           << tendency_ts.fractalDimensionality() << "\n";

    coords << ts.charDifDim() << "\n"
           << tendency_ts.charDifDim() << "\n";

    arc.waitForFinished();
    auto compressed = arc.readAllStandardOutput();

//        qDebug () << "compressed size:" << compressed.size();

    if (compressed.size() == 0) {
        qDebug () << "!!! Compressed size == 0, something is wrong!";
        coordFile.remove();
/*            QMessageBox::warning(this, tr ("Ошибка сжатия"),
                             tr("Похоже, указан неправильный файл в качестве компрессора. "
                                "Он говорит, что сжал все данные до размера 0 байт, так не бывает."));
        ui->progressBar->hide();
        status ("Ошибка. Укажите другой компрессор.");
        return;
*/
    } else {
        coords << 1. / (static_cast <double> (codedfile.size() / compressed.size()) - 1)
               << "\n";
        t_arc.waitForFinished();
        auto t_compressed = t_arc.readAllStandardOutput();

        coords << 1. / (static_cast <double> (tendencyFile.size()
                                              / t_compressed.size()) - 1)
               << "\n";
    }
}

void AnalyzeWidget::processAllSeries() {
    QDir dir (ui->setPath->text());
    QDir destDir(destPath());
    destDir.mkdir(destDir.absolutePath());
    status ("Обработка временных рядов...");
    ui->progressBar->show();
    ui->progressBar->setValue(0);
    QStringList lst = dir.entryList(QDir::Readable | QDir::Files);
    for (auto i = lst.begin(); i != lst.end(); ) {
        if (i->endsWith(".coeffts"))
            i = lst.erase(i);
        else
            ++i;
    }
    ui->progressBar->setMaximum(lst.size());
    QApplication::processEvents();
    for (QString fname : lst) {
        if (stop)
            break;

        processSeries (destDir, dir, fname);
        ui->progressBar->setValue(ui->progressBar->value() + 1);
        if (ui->progressBar->value() % 2)
            QApplication::processEvents();
    }

    status("Все временные ряды обработаны!");
    ui->progressBar->hide();
}

void AnalyzeWidget::FindCorrelations() {
    status ("Расчёт корреляций: загрузка данных");
    QApplication::processEvents();

    QDir processed (destPath());
    ui->progressBar->setValue(0);
    QStringList lst = processed.entryList(QStringList() << "*.coords",
                                          QDir::Readable | QDir::Files);
    ui->progressBar->setMaximum(lst.size());
    QApplication::processEvents();

    QVector <coordinates_t> src_matrix (lst.size());

    int progress = 0;
    for (QString fname : lst) {
        QFile file (processed.filePath(fname));
        file.open(QIODevice::ReadOnly);
        QTextStream coordinates (&file);
        for (size_t j = 0; j < coordinates_t::nValues; ++j)
            coordinates >> src_matrix [progress].values[j];
        ui->progressBar->setValue(++progress);
        QApplication::processEvents();
    }

    status ("Расчёт корреляций");
    ui->progressBar->setValue(progress = 0);
    ui->progressBar->setMaximum(coordinates_t::nValues * (coordinates_t::nValues - 1) / 2);
    ui->correlations->setVisible(true);
    correlations_label->show();
    QApplication::processEvents();

    QVector <double> E (coordinates_t::nValues, 0.), //< empirical expectation of v[i]
                     D (coordinates_t::nValues, 0.); //< sum of squares of diffs of v[i]

    for (size_t i = 0; i < coordinates_t::nValues; ++i) {
        E [i] = std::accumulate (src_matrix.begin(), src_matrix.end(), 0.,
                                 [i](double s, const coordinates_t& v)
                                 { return s + v.values[i]; }) / src_matrix.size();

        D [i] = std::accumulate (src_matrix.begin(), src_matrix.end(), 0.,
                                [i, E](double s, const coordinates_t& v)
                                { return s + std::pow (v.values[i] - E[i], 2); });
    }

    for (size_t i = 0; i < coordinates_t::nValues; ++i) {
        if (D[i] == 0) {
            qDebug () << "Дисперия координаты"
                      << coordinates_t::coordinateName(i)
                      << "равна нулю! Исправьте выборку.";
        } else if (std::isnan(D[i])) {
            qDebug () << "Дисперия координаты"
                      << coordinates_t::coordinateName(i)
                      << "— nan! Исправьте программу или выборку.";
        }
//        QTableWidgetItem* item = ui->correlations->item(i, i);
//        if (item == nullptr) {
//            item = new QTableWidgetItem ("1");
//            ui->correlations->setItem(i, i, item);
//        }
        for (size_t j = i; j < coordinates_t::nValues; ++j) {
            double quantifier = std::accumulate (src_matrix.begin(), src_matrix.end(), 0.,
                                                [i, j, E](double s, const coordinates_t& v)
                                                { return s + (v.values[i] - E[i]) * (v.values[j] - E[j]); });
            double denominator = sqrt(D[i] * D[j]);
            // denominator maybe near 0. which will produce infinity as a result.
            // That is not considered a error.

            QString answer = QString::number(quantifier / denominator);
            auto item = ui->correlations->item(i, j);
            if (item == nullptr) {
                ui->correlations->setItem(i, j, new QTableWidgetItem (answer));
                ui->correlations->setItem(j, i, new QTableWidgetItem (answer));
            } else {
                item->setText(answer);
                ui->correlations->item(j, i)->setText(answer);
            }

            ui->progressBar->setValue(++progress);
            QApplication::processEvents();
        }
    }
}

void AnalyzeWidget::status(const QString &message) {
    ui->status->show();
    ui->status->setText(message);
}

void AnalyzeWidget::on_go_clicked() {
    ui->go->setEnabled(false);
    ui->correlations->hide();
    correlations_label->hide();

    processAllSeries();
    FindCorrelations();

    ui->status->hide();
    ui->progressBar->hide();
    ui->go->setEnabled(true);
}

void AnalyzeWidget::resizeCorrelations() {
    correlations_label->setGeometry(0, 0,
                                    ui->correlations->verticalHeader()->width(),
                                    ui->correlations->horizontalHeader()->height());
}

void AnalyzeWidget::checkPaths() {
    QFile lzma (ui->lzmaPath->text());
    lzma.open(QIODevice::ReadOnly);
    QDir setDir (ui->setPath->text());

    bool lzma_ok = lzma.isReadable(),
         set_ok = setDir.isReadable();

    setErrorBackground(ui->lzmaPath, not lzma_ok,
                       tr("Путь к архиватору"));
    setErrorBackground(ui->setPath, not set_ok,
                       tr("Папка с выборкой временных рядов"));
    ui->go->setEnabled(lzma_ok and set_ok);
}

QString AnalyzeWidget::destPath() {
    return QDir(ui->setPath->text()).filePath("processed");
}

void AnalyzeWidget::on_setPath_textChanged(const QString &arg1) {
    QDir f(arg1);
    setErrorBackground(ui->setPath, not f.isReadable(),
                       tr("Папка с выборкой временных рядов"));
    ui->go->setEnabled(f.isReadable());
}

void AnalyzeWidget::on_browseLzmaPath_clicked() {
    auto ans = QFileDialog::getOpenFileName(this,
                                 tr("Укажите архиватор lzma"),
                                 ui->lzmaPath->text());
    if (not ans.isNull())
        ui->lzmaPath->setText(ans);
    ui->status->hide();
}

void AnalyzeWidget::on_lzmaPath_textChanged(const QString &arg1) {
    QDir f(arg1);
    setErrorBackground(ui->lzmaPath, not f.isReadable(),
                       tr("Путь к архиватору"));
    ui->go->setEnabled(f.isReadable());
}

void AnalyzeWidget::on_AnalyzeWidget_destroyed() {
    stop = true;
}
