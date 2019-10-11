#include <QtTest>

#include "tlkfile.h"

class tst_TlkFile: public QObject
{
    Q_OBJECT

private slots:
    void test_data();
    void test();
};

void tst_TlkFile::test_data()
{
    QTest::addColumn<QString>("fileName");

    QTest::newRow("BG1 classic")
            << QFINDTESTDATA("../../data/tlk/bg1/dialog.tlk");
    QTest::newRow("BG1 EE")
            << QFINDTESTDATA("../../data/tlk/bg1ee/dialog.tlk");
}

void tst_TlkFile::test()
{
    QFETCH(QString, fileName);
    if (fileName.isEmpty())
        QSKIP("File not found, skipping test.");
    qDebug() << QTest::currentDataTag() << fileName;

    TlkFile file;
}

QTEST_MAIN(tst_TlkFile)

#include "tst_tlkfile.moc"
