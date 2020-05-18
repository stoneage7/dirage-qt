#include <QtTest>
#include <QCoreApplication>

#include <vector>
#include <random>
#include <algorithm>
#include "platform.h"

class histogram_test : public QObject
{
    Q_OBJECT
    std::mt19937_64 m_rng;
    unsigned m_benchSeed;

public:
    histogram_test();
    ~histogram_test();

private slots:
    void test_case1();
    void test_case2();
    void test_case3();

};

histogram_test::histogram_test()
{
    m_rng.seed(122);
    m_benchSeed = 9423;
}

histogram_test::~histogram_test()
{
}

void histogram_test::test_case1()
{
    for (size_t i = 0; i < 100; i++) {
        int n_data = m_rng() % 100 + 10;
        int n_bins = m_rng() % 10 + 10;
        QVector<histogram::Datapoint> data(n_data);
        QVector<histogram::Datapoint::ValueType> bins_scalar(n_bins);
        QVector<histogram::Datapoint::ValueType> bins_avx(n_bins);

        for (auto i = data.begin(); i != data.end(); i++) {
            (*i).key = m_rng() % 100000000;
            (*i).value = m_rng() % 1000000 + 90000000;
        }
        std::sort(data.begin(), data.end(),
                  [] (const histogram::Datapoint &a, const histogram::Datapoint &b) {
            return a.key < b.key;
        });
        qint64 max = std::max_element(data.begin(), data.end(),
                                      [] (const histogram::Datapoint &a, const histogram::Datapoint &b) {
            return a.key < b.key;
        })->key;
        qint64 min = std::min<qint64>(max-1000, m_rng() % 100000000);

        histogram::ScalarImpl().make(data.cbegin(), data.cend(),
                                     min, max, bins_scalar.begin(), bins_scalar.end());
        histogram::AVX2Impl().make(data.cbegin(), data.cend(),
                                     min, max, bins_avx.begin(), bins_avx.end());
        QVERIFY(bins_scalar == bins_avx);
    }


}

void histogram_test::test_case2()
{
    m_rng.seed(m_benchSeed);
    for (size_t i = 0; i < 10; i++) {
        int n_data = m_rng() % 1000000 + 10;
        int n_bins = m_rng() % 50 + 10;
        QVector<histogram::Datapoint> data(n_data);
        QVector<histogram::Datapoint::ValueType> bins(n_bins);

        for (auto i = data.begin(); i != data.end(); i++) {
            (*i).key = m_rng() % 100000000;
            (*i).value = m_rng() % 1000000 + 90000000;
        }
        std::sort(data.begin(), data.end(),
                  [] (const histogram::Datapoint &a, const histogram::Datapoint &b) {
            return a.key < b.key;
        });
        qint64 max = std::max_element(data.begin(), data.end(),
                                      [] (const histogram::Datapoint &a, const histogram::Datapoint &b) {
            return a.key < b.key;
        })->key;
        qint64 min = std::min<qint64>(max-1000, m_rng() % 100000000);

        histogram::ScalarImpl impl;

        QBENCHMARK {
            impl.make(data.cbegin(), data.cend(), min, max, bins.begin(), bins.end());
        }
    }
}

void histogram_test::test_case3()
{
    m_rng.seed(m_benchSeed);
    for (size_t i = 0; i < 10; i++) {
        int n_data = m_rng() % 1000000 + 10;
        int n_bins = m_rng() % 50 + 10;
        QVector<histogram::Datapoint> data(n_data);
        QVector<histogram::Datapoint::ValueType> bins(n_bins);

        for (auto i = data.begin(); i != data.end(); i++) {
            (*i).key = m_rng() % 100000000;
            (*i).value = m_rng() % 1000000 + 90000000;
        }
        std::sort(data.begin(), data.end(),
                  [] (const histogram::Datapoint &a, const histogram::Datapoint &b) {
            return a.key < b.key;
        });
        qint64 max = std::max_element(data.begin(), data.end(),
                                      [] (const histogram::Datapoint &a, const histogram::Datapoint &b) {
            return a.key < b.key;
        })->key;
        qint64 min = std::min<qint64>(max-1000, m_rng() % 100000000);

        histogram::AVX2Impl impl;

        QBENCHMARK {
            impl.make(data.cbegin(), data.cend(), min, max, bins.begin(), bins.end());
        }
    }
}
QTEST_MAIN(histogram_test)

#include "tst_histogram_test.moc"
