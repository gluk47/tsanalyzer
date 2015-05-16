#include "TimeSeries.h"
#include <algorithm>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include <cmath>

constexpr double M_2PI = 2 * M_PI;

TimeSeries::TimeSeries(QVector<float> values) : _Values (values) {
}

void TimeSeries::readFile(const QString &fileName) {
    QFile file (fileName);
    file.open(QIODevice::ReadOnly);
    QTextStream in (&file);
    _Values.clear();
    while (not in.atEnd()) {
        QString ln = in.readLine();
        bool ok;
        float f = ln.toFloat(&ok);
        if (ok)
            _Values.append(f);
    }
    dirty = true;
}

const QVector<unsigned> &TimeSeries::encoded() const {
    if (dirty)
        _Encode();
    return _Encoded;
}

TimeSeries::limits TimeSeries::getLimits() const {
    limits ans;
    ans.inf = ans.sup = _Values[0];
    for (int i = 1; i < _Values.size(); ++i) {
        if (ans.inf > _Values[i])
            ans.inf = _Values[i];
        else if (ans.sup < _Values[i])
            ans.sup = _Values [i];
    }
    return ans;
}

void TimeSeries::removeTrend() {
    const int size = _Values.size();
    float avg = std::accumulate (_Values.begin(),
                                 _Values.end(),
                                 0.f,
                                [size](float sum, float b)
                                { return sum + b / size; });
    for (auto& v : _Values)
        v -= avg;
}

TimeSeries TimeSeries::tendencySeries() const {
    QVector <float> v (size());
    v[0] = 0;
    for (int i = 1; i < size(); ++i) {
        if (_Values[i] > _Values[i - 1])
            v[i] = 1;
        else if (_Values[i] < _Values[i - 1])
            v[i] = -1;
        else
            v[i] = 0;
    }
    TimeSeries ans (std::move (v));
    return ans;
}

double abs (double _) {
    return _<0?-_:_;
}

float TimeSeries::harmonicComplexity() const {
    /// The C# source for this function has been graciously donated
    /// by nastyaloginovaa@gmail.com
    const int n = _Values.size();
    // sin (A) and cos (B) coefficients of a Fourier series
    QVector <double> A (n/2, 0.), B(n/2, 0.);

    {
        QVector <double> fiA (n, 0.), fiB(n, 0.);
        TimeSeries corrected = *this;
        corrected.removeTrend();

        for (int k = 0; k < n / 2; ++k) {
            for (int i = 0; i < n; ++i) {
                fiB[i] = corrected[i] * sin(M_2PI * k * i / (n - 1));
                fiA[i] = corrected[i] * cos(M_2PI * k * i / (n - 1));
            }
            double sumA = 0., sumB = 0.;
            for (int i = 1; i < n - 1; ++i) {
                sumB += fiB[i];
                sumA += fiA[i];
            }

            B[k] = abs(2. / (n - 1) * ((fiB[0] + fiB[n - 1]) / 2. + sumB));
            A[k] = abs(2. / (n - 1) * ((fiA[0] + fiA[n - 1]) / 2. + sumA));
        }
    }

    // amplitudes of the Fourier series
    QVector <double> R (n/2);
    for (int i = 0; i < n / 2; ++i)
        R[i] = sqrt (A[i] * A[i] + B[i] * B[i]);

    std::sort (R.begin(), R.end(), [](float a, float b){return b < a;});
    double sum = std::accumulate (R.begin(), R.end(), 0.);
    double complexity = 0.;
    for (int i = 0; i < R.size(); ++i)
        complexity += R[i] / sum * i;

    return complexity;
}

void TimeSeries::_Encode() const {
    _Encoded.clear();
    dirty = false;
    if (_Values.isEmpty())
        return;

    auto range = getLimits();
    float step = (range.sup - range.inf) / _NLevels;
    for (auto value : _Values) {
        unsigned code = (value - range.inf) / step;
        if (code == _NLevels)
            --code;
        _Encoded.append(code);
    }
}
