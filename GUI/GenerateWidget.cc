#include "GenerateWidget.h"
#include "ui_GenerateWidget.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>

#include <cmath>
#include <functional>
#include <iostream>
#include <random>

#include "helpers.h"
#include "CoefftWidget.h"

using std::cerr;
constexpr float M_2PI = M_PI * 2, M_2_E = 2 / M_E;

QString plural (const char* base,
                const char* one,
                const char* some,
                const char* many,
                int value,
                bool prepend_number = false) {
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

QString spaceNumber (int n) {
    QString ans = QString::number(n);
    for (int i = ans.size() - 3; i > 0; i -= 3)
        ans.insert(i, ' ');
    return ans;
}

float get_error (double mean, double disperse) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::normal_distribution<> d (mean, disperse);

    return d (gen);
}

/**
 * @brief Generate a time series and store it to a file
 * @param generator a function to sample data from.
 *        Should take float parameters in range 0.f..1.f
 * @param npoints how many points to sample
 * @param fname file to store data to
 */
void generate_series (std::function <float (float)> generator,
                      unsigned npoints,
                      const QString& fname) {
    const float step = 1.f / npoints;

    QFile file (fname);
    file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    QTextStream out(&file);
    for (float arg = 0.f; npoints-->0; arg += step)
        out << generator (arg) << "\n";
}

double series_generator (const QVector <double>& k, double x) {
    return k[0] * sin (k[1] * M_2PI * x)
         + k[2] * x
         + k[3] * exp (k[4] * M_2_E * x)
         + k[5] * log (k[6] * M_E * (x + 1));
}

GenerateWidget::GenerateWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GenerateWidget) {
    ui->setupUi(this);
    ui->progressBar->hide();
    ui->labelReady->hide();
    ui->labelCount->hide();
    ui->labelSetSize->hide();
    ui->labelSeries->hide();

    CoefftWidget* w;
#define ADD(letter)\
    w = new CoefftWidget (#letter); \
    kWidgets.append(w); \
    ui->kWidgets->layout()->addWidget(w);

    ADD(a);
    ADD(α);
    ADD(b);
    ADD(c);
    ADD(γ);
    ADD(d);
    ADD(δ);
    w->setMin(1e-2);
    w->setMax(3);

#undef ADD

    connect (ui->generate, SIGNAL(clicked()), this, SLOT(generate()));
    connect (ui->countSetSize, SIGNAL(clicked()), this, SLOT(countSetSize()));
}

GenerateWidget::~GenerateWidget() {
    delete ui;
}

void GenerateWidget::forAllCoefficients (std::function<void (const QVector<double>&)>&& f) {
    QVector <double> k (kWidgets.size());
    iterateK(k, f, 0);
}

void GenerateWidget::generate() {
    unsigned id = 0;
    const unsigned npoints = 1500;
    QDir setPath (ui->setPath->text());

    countSetSize();

    ui->progressBar->setMaximum(setSize);
    ui->progressBar->setValue(0);
    ui->progressBar->show();

    scope_exit restore_ui ([this]{
        ui->progressBar->hide();
    });

    ui->generate->setEnabled(false);
    QApplication::processEvents();
    const double errmean = ui->errMean->value(),
                 errdisp = ui->errDisperse->value();

    setPath.mkdir(setPath.absolutePath());

    setSize = 0;
    forAllCoefficients([this, errmean, errdisp, setPath, &id](const QVector <double>& k){
//                if (setSize > 3000) {
//                    std::cerr << "aborting generation, setSize exceeds 3000\n";
//                    return;
//                }
                const QString fname =
                        setPath.absoluteFilePath("ts_" + QString::number(id++));
                generate_series(
                   [k, errmean, errdisp](float step){
                       return series_generator(k, step)
                            + get_error(errmean, errdisp);
                   },
                   npoints,
                   fname
                );

               QFile file (fname + ".coeffts");
               file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
               QTextStream out (&file);
               for (double v : k)
                   out << v << '\n';

               ui->progressBar->setValue(id);
               ++setSize;
               if (setSize % 10 == 0)
                   QApplication::processEvents();
    });
    generate_series([](float){return get_error(0., .8);}, npoints,
                    setPath.absoluteFilePath( "ts_const"));

    ui->labelReady->show();
    ui->labelSetSize->show();
    ui->labelCount->setText(spaceNumber(setSize));
    ui->labelCount->show();
    ui->labelSeries->setText(plural("ряд", "", "а", "ов", setSize));

    ui->generate->setEnabled(true);
}

void GenerateWidget::countSetSize() {
    setSize = 1;
    ui->countSetSize->setEnabled(false);
    for (int i = 0; i < kWidgets.size(); ++i) {
        auto range = kWidgets[i]->getInfo();
        unsigned s = (range.max - range.min) / range.step;
        s += static_cast <bool> (range.min + range.step * s < range.max);
        setSize *= s;
        QApplication::processEvents();
    }
    ui->labelCount->setText(spaceNumber(setSize));
    ui->labelCount->show();
    ui->labelSetSize->show();
    ui->labelSeries->setText(plural("ряд", "", "а", "ов", setSize));
    ui->labelSeries->show();
    ui->countSetSize->setEnabled(true);
}

template <class T>
std::ostream& operator<< (std::ostream& str, const QVector<T>& data) {
    for (auto _ : data)
        str << _ << " ";
    return str;
}

void GenerateWidget::iterateK (QVector <double>& k,
                               std::function <void (const QVector <double>&)>& f,
                               unsigned index) {
//    std::cerr << "Iterating with k = " << k << "... ";
    if (index >= static_cast <unsigned> (k.size())) {
//        std::cerr << "recursion end\n";
        f (k);
        return;
    }

    auto range = kWidgets [index]->getInfo();
    for (k [index] = range.min; k [index] < range.max; k [index] += range.step)
        iterateK(k, f, index + 1);
}
