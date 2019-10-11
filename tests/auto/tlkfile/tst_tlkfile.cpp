#include <QtTest>

#include "tlkfile.h"

class tst_TlkFile: public QObject
{
    Q_OBJECT

private slots:
    void test();
};

void tst_TlkFile::test()
{
    TlkFile file;
}

QTEST_MAIN(tst_TlkFile)

#include "tst_tlkfile.moc"
