#include "TimeSeries.h"
#include <algorithm>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include <cmath>
#include <limits>

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

float TimeSeries::herstValue() const {
    if (size() < 2) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    double R=0, S=0;
    const int n=_Values.size();
    double EX=0, MX=0; //среднее значение временного ряда
    double W[n]; //вектор отклонений
    double Wmax=0, Wmin=0;

    int i;
    for (i=0; i<n; i++) EX+=_Values[i]; MX=EX/n;

    EX=0;
    for (i=0; i<n; i++) { EX+=_Values[i]; W[i]=EX - (i+1)*MX; }

    for (i=0; i<n; i++)
    {
         if (Wmax<W[i]) Wmax=W[i];
         if (Wmin>W[i]) Wmin=W[i];
    }
    R=Wmax-Wmin;

    for (i=0; i<n; i++) { S+= pow((_Values[i]-MX), 2); }
    S=sqrt(S/n);

    return log(R/S)/log(n);
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

double TimeSeries::charDifDim() const {
    const int n = _Values.size();
    auto coded = encoded();
    double dtV_m;
    int m;

    const QVector <double> abc = [this]{
        QVector <double> ans;
        for (unsigned k = 0; k < nLevels(); ++k)
            ans.append(k); // k-мощность алфавита, элементы алфавита - целые от 0 до k-1
        return ans;
    }();

    QVector <double> Cm(n, 0), dC(n, 0);
    QVector <double> ci (nLevels(), 0); //вектор частотной встречаемости слов
    QVector <QVector <double>> wi_m; //вектор многообразия слов длины m

    wi_m.append(abc);
    for (int j1 = 0; j1 < n; ++j1) {
        for (unsigned j = 0; j < nLevels(); ++j) {
            if (coded[j1] == abc[j]) {
                ++ci[j];
                break;
            }
        }
    }

    for (unsigned j = 0; j < nLevels(); ++j) {
        if (ci[j])
            Cm[0]+=((ci[j]/n)*(log(n/ci[j])) / log(nLevels()));
    }

    for (m = 1; m < n; ++m) {
        int j = pow (nLevels(), m); //число возможных слов длины m над алфавитом abc
        wi_m.append(QVector <double>());
        for (unsigned j1 = 0; j1 < nLevels(); ++j1) {
            int j0 = wi_m[m-1].size(); //число различных слов длины m-1
            for (int j2 = 0; j2 < j0; ++j2)
                wi_m[m].append(abc[j1] * pow(10,m) + wi_m[m-1][j2]);
        }
        ci.fill(0,j);
        for (int j1=0; j1<(n-m); ++j1) {//цикл по словам временного ряда
            dtV_m=0;
            for (int j2=0; j2<m+1; ++j2)
                dtV_m+= (coded[j1+j2] * pow(10,m-j2));
            for (int j0 = 0; j0 < j; ++j0) //пробег по всем словам длины m над алфавитом abc
                if (dtV_m == wi_m[m][j0]) {
                    ++ci[j0];
                    break;
                }
        }
        for (int j0 = 0; j0 < j; ++j0) {
            if (ci[j0])
                Cm[m]+=((ci[j0]/(n-m))*(log((n-m)/ci[j0])) / log(j));
        }
        dC[m]=Cm[m-1]-Cm[m];
        if (dC[m] < dC[m-1])
            break;
    }
    return (m / floor(log(n)/log(nLevels())));
}
