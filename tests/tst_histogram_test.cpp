#include <QtTest>
#include <QCoreApplication>

#include <vector>
#include <random>
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
    bool ok = true;
    for (size_t i = 0; i < 5; i++) {
        size_t n_data = m_rng() % 1000000 + 10;
        size_t n_bins = m_rng() % 1000 + 10;
        std::vector<platform::Datapoint> data(n_data);
        std::vector<qint64> bins_scalar(n_bins);
        std::vector<qint64> bins_avx(n_bins);


        for (auto i = data.begin(); i != data.end(); i++) {
            (*i).key = m_rng() % 100000000;
            (*i).value = m_rng() % 1000000 + 90000000;
        }

        QBENCHMARK {
            platform::make_histogram(data.data(), data.size(), m_rng() % 100000000, 1LL<<62, bins_scalar.data(), bins_scalar.size(), false);
        }
        QBENCHMARK {
            platform::make_histogram(data.data(), data.size(), m_rng() % 100000000, 1LL<<62, bins_avx.data(), bins_avx.size(), true);
        }


        for (size_t i = 0; i < bins_scalar.size(); i++) {
            if (bins_scalar[i] != bins_avx[i]) {
                ok = false;
            }
        }
    }

 //   QVERIFY(ok);


}

void histogram_test::test_case2()
{
    QBENCHMARK {
    m_rng.seed(m_benchSeed);
    for (size_t i = 0; i < 10000; i++) {
        size_t n_data = m_rng() % 1000000 + 10;
        size_t n_bins = m_rng() % 1000 + 10;
        std::vector<platform::Datapoint> data(n_data);
        std::vector<qint64> bins_scalar(n_bins);

        for (auto i = data.begin(); i != data.end(); i++) {
            (*i).key = m_rng() % 100000000;
            (*i).value = m_rng() % 1000000 + 90000000;
        }

            platform::make_histogram(data.data(), data.size(), m_rng() % 100000000, 1LL<<62, bins_scalar.data(), bins_scalar.size(), false);
        }
    }
}

void histogram_test::test_case3()
{
    QBENCHMARK {
    m_rng.seed(m_benchSeed);
    for (size_t i = 0; i < 10000; i++) {
        size_t n_data = m_rng() % 1000000 + 10;
        size_t n_bins = m_rng() % 1000 + 10;
        std::vector<platform::Datapoint> data(n_data);
        std::vector<qint64> bins_avx(n_bins);

        for (auto i = data.begin(); i != data.end(); i++) {
            (*i).key = m_rng() % 100000000;
            (*i).value = m_rng() % 1000000 + 90000000;
        }

            platform::make_histogram(data.data(), data.size(), m_rng() % 100000000, 1LL<<62, bins_avx.data(), bins_avx.size(), true);
        }
    }
}
QTEST_MAIN(histogram_test)

#include "tst_histogram_test.moc"
