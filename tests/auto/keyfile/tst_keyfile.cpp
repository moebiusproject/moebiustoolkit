#include <QtTest>

#include "keyfile.h"

class tst_KeyFile: public QObject
{
    Q_OBJECT

private slots:
    void test_data();
    void test();
};

void tst_KeyFile::test_data()
{
    QTest::addColumn<QString>("fileName");

    QTest::addColumn<quint32>("bifCount");
    QTest::addColumn<quint32>("resourceCount");
    QTest::addColumn<quint32>("bifStart");
    QTest::addColumn<quint32>("resourceStart");

    QTest::addColumn<quint32>("bifSize24");
    QTest::addColumn<quint32>("bifNameStart24");
    QTest::addColumn<quint16>("bifNameLength24");
    QTest::addColumn<quint16>("bifLocation24");

    QTest::addColumn<quint32>("bifSize42");
    QTest::addColumn<quint32>("bifNameStart42");
    QTest::addColumn<quint16>("bifNameLength42");
    QTest::addColumn<quint16>("bifLocation42");

    QTest::newRow("BG1 EE")
            << QFINDTESTDATA("../../data/key/bg1ee/chitin.key")
               // Header.
            << quint32(80)      << quint32(36993) << quint32(24) << quint32(2320)
               // BIF entries 24 and 42.
            << quint32(3012)    << quint32(1370)  << quint16(16) << quint16(1)
            << quint32(2727956) << quint32(1676)  << quint16(16) << quint16(1);
}

void tst_KeyFile::test()
{
    QFETCH(QString, fileName);
    if (fileName.isEmpty())
        QSKIP("File not found, skipping test.");
    qDebug() << QTest::currentDataTag() << fileName;

    QFile file(fileName);
    QVERIFY(file.open(QIODevice::ReadOnly));

    QDataStream stream(&file);
    KeyFile key;

    QCOMPARE(key.version, 0);
    QVERIFY(!key.isValid());

    stream >> key;

    QCOMPARE(key.version, 1);
    QVERIFY(key.isValid());

    QFETCH(quint32, bifCount);
    QFETCH(quint32, resourceCount);
    QFETCH(quint32, bifStart);
    QFETCH(quint32, resourceStart);
    QCOMPARE(key.bifCount, bifCount);
    QCOMPARE(key.resourceCount, resourceCount);
    QCOMPARE(key.bifStart, bifStart);
    QCOMPARE(key.resourceStart, resourceStart);

    QFETCH(quint32, bifSize24);
    QFETCH(quint32, bifNameStart24);
    QFETCH(quint16, bifNameLength24);
    QFETCH(quint16, bifLocation24);
    QFETCH(quint32, bifSize42);
    QFETCH(quint32, bifNameStart42);
    QFETCH(quint16, bifNameLength42);
    QFETCH(quint16, bifLocation42);

    QCOMPARE(key.bifEntries.at(24).size, bifSize24);
    QCOMPARE(key.bifEntries.at(24).nameStart, bifNameStart24);
    QCOMPARE(key.bifEntries.at(24).nameLenght, bifNameLength24);
    QCOMPARE(key.bifEntries.at(24).location, bifLocation24);

    QCOMPARE(key.bifEntries.at(42).size, bifSize42);
    QCOMPARE(key.bifEntries.at(42).nameStart, bifNameStart42);
    QCOMPARE(key.bifEntries.at(42).nameLenght, bifNameLength42);
    QCOMPARE(key.bifEntries.at(42).location, bifLocation42);
}

QTEST_MAIN(tst_KeyFile)

#include "tst_keyfile.moc"
