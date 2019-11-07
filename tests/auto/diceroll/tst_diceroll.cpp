#include <QtTest>

#include "diceroll.h"

class tst_DiceRoll: public QObject
{
    Q_OBJECT

private slots:
    void debugOperator();
    void luckified();
    void test_data();
    void test();
};

void tst_DiceRoll::debugOperator()
{
    auto dice = DiceRoll().number(1).sides(4).bonus(0).luck(1);
    QTest::ignoreMessage(QtDebugMsg, "1d4+0@1");
    qDebug() << dice;
}

void tst_DiceRoll::luckified()
{
    auto dice = DiceRoll().sides(10).luck(2);
    QCOMPARE(dice.luckified(1), 3);
    QCOMPARE(dice.luckified(2), 4);
    QCOMPARE(dice.luckified(3), 5);
    QCOMPARE(dice.luckified(8), 10);
    QCOMPARE(dice.luckified(9), 10);
    QCOMPARE(dice.luckified(10), 10);

    dice.luck(-2);
    QCOMPARE(dice.luckified(1), 1);
    QCOMPARE(dice.luckified(2), 1);
    QCOMPARE(dice.luckified(3), 1);
    QCOMPARE(dice.luckified(4), 2);
    QCOMPARE(dice.luckified(8), 6);
    QCOMPARE(dice.luckified(9), 7);
    QCOMPARE(dice.luckified(10), 8);
}

void tst_DiceRoll::test_data()
{
    QTest::addColumn<int>("number");
    QTest::addColumn<int>("sides");
    QTest::addColumn<int>("bonus");
    QTest::addColumn<int>("luck");

    QTest::addColumn<int>("maximum");
    QTest::addColumn<int>("minimum");
    QTest::addColumn<double>("average");
    QTest::addColumn<double>("sigma");

    QTest::newRow("1d4")           << 1  << 4  << 0 << 0
                                   << 4  << 1  << 2.5 << qSqrt(1.25);
    QTest::newRow("2d4")           << 2  << 4  << 0 << 0
                                   << 8  << 2  << 5.0 << qSqrt(1.25*2);
    QTest::newRow("2d4+1")         << 2  << 4  << 1 << 0
                                   << 9  << 3  << 6.0 << qSqrt(1.25*2);
    QTest::newRow("1d8+3")         << 1  << 8  << 3 << 0
                                   << 11 << 4  << 7.5 << qSqrt(5.25);
    QTest::newRow("2d4+1@+1")      << 2  << 4  << 1 << 1
                                   << 9  << 5  << 7.5 << qSqrt(1.375);
    QTest::newRow("2d4+1@-1")      << 2  << 4  << 1 << -1
                                   << 7  << 3  << 4.5 << qSqrt(1.375);
    QTest::newRow("1d3+0@+2")      << 1  << 3  << 0 << 2
                                   << 3  << 3  << 3.0 << 0.0;
    QTest::newRow("1d3+0@-2")      << 1  << 3  << 0 << -2
                                   << 1  << 1  << 1.0 << 0.0;

    // +2 luck (or +3 with other sources of luck) inspired on Alora.
    QTest::newRow("1d8+0@+2")      << 1  << 8  << 0 << 2
                                   << 8  << 3  << 6.125 << qSqrt(3.359375);
    QTest::newRow("1d10+0@+2")     << 1  << 10 << 0 << 2
                                   << 10 << 3  << 7.2 << qSqrt(6.16);
    QTest::newRow("1d10+0@+3")     << 1  << 10 << 0 << 3
                                   << 10 << 4  << 7.9 << qSqrt(4.69);
    QTest::newRow("1d8+2@+2")      << 1  << 8  << 2 << 2
                                   << 10 << 5  << 8.125 << qSqrt(3.359375);
    QTest::newRow("1d10+1@+2")     << 1  << 10 << 1 << 2
                                   << 11 << 4  << 8.2 << qSqrt(6.16);
}

void tst_DiceRoll::test()
{
    QFETCH(int, number);
    QFETCH(int, sides);
    QFETCH(int, bonus);
    QFETCH(int, luck);

    QFETCH(int,    maximum);
    QFETCH(int,    minimum);
    QFETCH(double, average);
    QFETCH(double, sigma);

    auto dice = DiceRoll().number(number).sides(sides).bonus(bonus).luck(luck);
    QCOMPARE(dice.maximum(), maximum);
    QCOMPARE(dice.minimum(), minimum);
    QCOMPARE(dice.average(), average);
    QCOMPARE(dice.sigma(), sigma);
}

QTEST_MAIN(tst_DiceRoll)

#include "tst_diceroll.moc"
