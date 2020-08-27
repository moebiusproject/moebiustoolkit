/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2020 Alejandro Exojo Piqueras
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtTest>

#include "bifffile.h"
#include "resourcetype.h"

class tst_BiffFile: public QObject
{
    Q_OBJECT

private slots:
    void test_data();
    void test();
};

void tst_BiffFile::test_data()
{
    QTest::addColumn<QString>("filePath");

    QTest::newRow("BG1 EE areas")
            << QFINDTESTDATA("../../data/bif/AREAS.BIF");
    QTest::newRow("BG1 EE defaults")
            << QFINDTESTDATA("../../data/bif/DEFAULT.BIF");
    QTest::newRow("BG1 EE dialogs")
            << QFINDTESTDATA("../../data/bif/DIALOG.BIF");
    QTest::newRow("BG1 EE items")
            << QFINDTESTDATA("../../data/bif/ITEMS.BIF");
    QTest::newRow("BG1 EE spells")
            << QFINDTESTDATA("../../data/bif/spells.bif");
}

void tst_BiffFile::test()
{
    QFETCH(QString, filePath);
    if (filePath.isEmpty())
        QSKIP("File not found, skipping test.");
    qDebug() << QTest::currentDataTag() << filePath;

    QFile file(filePath);
    QVERIFY(file.open(QIODevice::ReadOnly));

    QDataStream stream(&file);
    BiffFile biff;

    QVERIFY(!biff.isValid());

    stream >> biff;

    QVERIFY(biff.isValid());

    QCOMPARE(biff.fileEntries.first().offset, 24u);

    auto onlyFileName = [](const QString& string) {
        return string.section(QLatin1Char('/'), -1);
    };

    // Given a FILENAME.BIF under test, we ship the expected contents of the
    // Nth entry in a file named contents/FILENAME.BIF/42 in the same test dir.
    const QString fileName = onlyFileName(filePath);
    const QString directoryName = filePath.section(QLatin1Char('/'), 0, -2, QString::SectionIncludeTrailingSep);
    const QString contentsName = directoryName + QLatin1String("contents/") + fileName;
    QDir directory(contentsName);
    if (!directory.exists())
        return;

    directory.setFilter(QDir::Files);
    QDirIterator iterator(directory);

    while (iterator.hasNext()) {
        const QString contentsFilePath = iterator.next();
        const QString contentsFileName = onlyFileName(contentsFilePath);

        bool ok = false;
        // Entry inside the biff
        const quint32 fileEntry = contentsFileName.toUInt(&ok);
        QVERIFY(ok);

        const BiffFile::FileEntriesIndex fileEntryIndex = biff.fileEntries.at(fileEntry);
        file.seek(fileEntryIndex.offset);
        QByteArray contentsData;
        contentsData.resize(fileEntryIndex.size);
        file.read(contentsData.data(), fileEntryIndex.size);

        QFile expected(contentsFilePath);
        expected.open(QIODevice::ReadOnly);
        QCOMPARE(contentsData, expected.readAll());
    }
}

QTEST_MAIN(tst_BiffFile)

#include "tst_bifffile.moc"
