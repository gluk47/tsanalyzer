#include "GenerateWidget.h"
#include "ui_GenerateWidget.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>

#include <assert.h>
#include <cmath>
#include <functional>
#include <iostream>
#include <random>

#include "helpers.h"
#include "CoefftWidget.h"

using std::cerr;
constexpr float M_2PI = M_PI * 2, M_2_E = 2 / M_E;

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
void generate_series (std::function <float (float, unsigned)> generator,
                      unsigned npoints,
                      const QString& fname) {
    assert (npoints > 1);

    const float step = 1.f / (npoints - 1);

    QFile file (fname);
    file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    QTextStream out(&file);
    for (float arg = 0.f; npoints-->0; arg += step)
        out << generator (arg, npoints) << "\n";
}

double stochastic (unsigned m, double r) {
    static double last_r = -1;
    static QMap <unsigned, double> cache;
    if (r != last_r) {
        cache.clear();
        cache.insert(0u, 1.);
    }

    if (cache.contains(m))
        return cache [m];

    auto fm = stochastic (m - 1, r);
    auto ans = 4 * r * fm * (1 - fm);
    cache.insert(m, ans);
    return ans;
}

double series_generator (const QVector <double>& k, double x, unsigned index) {
    return k[0] * sin (k[1] * M_2PI * x)
         + k[2] * x
         + k[3] * exp (k[4] * M_2_E * x)
         + k[5] * log (k[6] * M_E * (x + 1))
         + k[7] * stochastic(k[8] + index, k[9]);
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

    ADD(a); // 0
    ADD(α); // 1
    ADD(b); // 2
    ADD(c); // 3
    ADD(γ); // 4
    ADD(d); // 5
    ADD(δ); // 6
    w->setMin(1e-2);
    w->setMax(3);

    ADD(h); // 7
    ADD(m); // 8
    w->setMin(100);
    w->setMax(101);
    w->setStep(2);

    ADD(r); // 9
    w->setMin(.91);
    w->setMax(.96);
    w->setStep(.05);

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
    const unsigned npoints = ui->nValues->value();
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
    forAllCoefficients([this, errmean, errdisp, setPath, &id, npoints](const QVector <double>& k){
//                if (setSize > 3000) {
//                    std::cerr << "aborting generation, setSize exceeds 3000\n";
//                    return;
//                }
                const QString fname =
                        setPath.absoluteFilePath("ts_" + QString::number(id++));
                generate_series(
                   [k, errmean, errdisp, npoints](float step, unsigned index){
                       return series_generator(k, step, index)
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
    generate_series([](float, unsigned){return get_error(0., .8);}, npoints,
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
    for (k [index] = range.min; k [index] <= range.max; k [index] += range.step)
        iterateK(k, f, index + 1);
}

void GenerateWidget::on_browseSetPath_clicked() {
///@todo
}
