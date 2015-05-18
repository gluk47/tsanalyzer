#ifndef TIMESERIES_H_c634c583_bf0e_4691_8238_d2a8bedffa2d
#define TIMESERIES_H_c634c583_bf0e_4691_8238_d2a8bedffa2d

#include <QVector>

class TimeSeries {
public:
    TimeSeries(QVector<float> values = QVector <float>());

    void readFile (const QString& fileName);

    const QVector <float>& values() const noexcept {
        return _Values;
    }
    float operator[] (int index) const {
        return _Values[index];
    }
    void setValues(QVector <float> v) {
        _Values = std::move (v);
        dirty = true;
    }

    unsigned nLevels () const noexcept {
        return _NLevels;
    }
    void setNLevels(unsigned n) noexcept {
        _NLevels = n;
        dirty = true;
    }

    const QVector <unsigned>& encoded() const;
    struct limits {
        float inf, sup;
    };
    limits getLimits () const;
    void removeTrend ();
    /**
     * @brief return tendency series, where each value is replaced by 0 or ±1,
     *  showing that it is equal, more or less than the previous one.
     */
    TimeSeries tendencySeries() const;

    /**
     * @brief count the harmonic (Fourier) complexity.
     *
     * The C# source for this function has been graciously donated by
     * @author nastyaloginovaa@gmail.com
     */
    float harmonicComplexity () const;

    float herstValue () const;

    float fractalDimensionality () const { return 2 - herstValue(); }
    /// Мера символьного разнообразия
    double charDifDim () const;

    int size() const { return _Values.size(); }
private:
    /// How many intervals are there in the values domain
    unsigned _NLevels = 8;
    /// The real value of the series
    QVector <float> _Values;
    /**
     * @brief The encoded version of the series
     *
     * The values domain is separated into @c nLevels half-segments of the form
     * [a_i; a_{i+1}); the last half-segment is actually a segment.
     * Then each value is replaced with the number of the corresponding half-segment.
     */
    mutable QVector <unsigned> _Encoded;

    /**
     * @brief update the @c _Encoded variable.
     *
     * This function is declared as const since it is legit
     *  to call it from other const functions.
     */
    void _Encode () const;

    /// Does the @c endoded series contain the actual data same as @c values
    mutable bool dirty = true;
};

#endif // TIMESERIES_H
