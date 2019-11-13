/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2019 Alejandro Exojo Piqueras
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
#include <QMetaType>

#include "tdafile.h"

struct Cell {
    QVariant row;
    QVariant column;
    QString content;
};
Q_DECLARE_METATYPE(Cell);

class tst_TdaFile: public QObject
{
    Q_OBJECT

private slots:
    void test_data();
    void test();
};

void tst_TdaFile::test_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QString>("defaultValue");
    QTest::addColumn<QStringList>("headers");

    QTest::addColumn<QStringList>("rowNames");
    QTest::addColumn<QVector<Cell>>("cells");

    QStringList zeroToForty;
    for (int i = 0; i <= 40; ++i)
        zeroToForty << QString::number(i);
    QStringList oneToFifty;
    for (int i = 1; i <= 50; ++i)
        oneToFifty << QString::number(i);
    const QStringList oneToFiftyOne = (QStringList() << oneToFifty << QString("51"));

    QTest::newRow("BGXEE backstab table")
            << QFINDTESTDATA("../../data/2da/bgxee/backstab.2da")
            << QString("0") << zeroToForty
            << (QStringList() << "THIEF" << "SHADOWDANCER" << "ASSASIN"
                << "BOUNTY_HUNTER" << "STALKER")
            << (QVector<Cell>()
                // Thief at level 1 = x2 multiplier, etc. Luckily level starts at 0.
                << Cell{"THIEF", 1, "2"}
                << Cell{"SHADOWDANCER", 1, "1"}
                << Cell{"STALKER", 9, "3"}
                << Cell{"BOUNTY_HUNTER", 10, "4"}
                );

    QTest::newRow("Tweaks Anthology shapeshifting change")
            << QFINDTESTDATA("../../data/2da/cdtweaks/anisum04.2da")
            << QString("0") << QStringList(QLatin1String("RESREF"))
            << QStringList(QString("1"))
            << (QVector<Cell>()
                << Cell{0, 0, "wwbearwe"}
                << Cell{"1", 0, "wwbearwe"}
                << Cell{0, "RESREF", "wwbearwe"}
                << Cell{"1", "RESREF", "wwbearwe"}
                );

    QTest::newRow("Tweaks Anthology THAC0 table")
            << QFINDTESTDATA("../../data/2da/cdtweaks/thac0.2da")
            << QString("0") << oneToFifty
            << (QStringList()
                << QLatin1String("MAGE") << QLatin1String("FIGHTER")
                << QLatin1String("CLERIC") << QLatin1String("THIEF")
                << QLatin1String("BARD") << QLatin1String("PALADIN")
                << QLatin1String("DRUID") << QLatin1String("RANGER")
                << QLatin1String("FIGHTER_MAGE") << QLatin1String("FIGHTER_CLERIC")
                << QLatin1String("FIGHTER_THIEF") << QLatin1String("FIGHTER_MAGE_THIEF")
                << QLatin1String("MAGE_THIEF") << QLatin1String("CLERIC_MAGE")
                << QLatin1String("CLERIC_THIEF") << QLatin1String("FIGHTER_DRUID")
                << QLatin1String("FIGHTER_MAGE_CLERIC") << QLatin1String("CLERIC_RANGER")
                << QLatin1String("MONK") << QLatin1String("SORCERER")
                << QLatin1String("SHAMAN"))
            << (QVector<Cell>()
                << Cell{"THIEF", 5, "18"}
                << Cell{"THIEF", "5", "18"}
                << Cell{"PALADIN", 15, "5"}
                << Cell{"PALADIN", "15", "6"}
                << Cell{"MONK", "15", "6"}
                << Cell{18, "15", "6"}
                );

    QTest::newRow("Tweaks Anthology XP levels")
            << QFINDTESTDATA("../../data/2da/cdtweaks/xplevel.2da")
            << QString("-1") << oneToFiftyOne
            << (QStringList()  << QLatin1String("MAGE") << QLatin1String("FIGHTER")
                << QLatin1String("PALADIN") << QLatin1String("RANGER")
                << QLatin1String("CLERIC") << QLatin1String("DRUID")
                << QLatin1String("THIEF") << QLatin1String("BARD")
                << QLatin1String("SORCERER") << QLatin1String("MONK")
                << QLatin1String("SHAMAN"))
            << (QVector<Cell>()
                << Cell{"RANGER", "15", "2100000"}
                << Cell{"DRUID", "5", "12500"}
                << Cell{"CLERIC", "8", "110000"}
                << Cell{3, "15", "2100000"}
                << Cell{5, "5", "12500"}
                << Cell{4, "8", "110000"}
                );
}

void tst_TdaFile::test()
{
    QFETCH(QString, fileName);
    if (fileName.isEmpty())
        QSKIP("File not found, skipping test.");

    QFile file(fileName);
    QVERIFY(file.open(QIODevice::ReadOnly));

    const TdaFile tda = TdaFile::from(file);

    QFETCH(QString, defaultValue);
    QFETCH(QStringList, headers);

    QCOMPARE(tda.defaultValue, defaultValue);
    QCOMPARE(tda.headers, headers);
    QCOMPARE(tda.columnCount(), headers.size());

    QFETCH(QStringList, rowNames);
    QCOMPARE(tda.rowNames(), rowNames);

    QFETCH(QVector<Cell>, cells);
    for (const Cell& cell : cells) {
        qDebug() << cell.row << cell.column << cell.content;
        if (cell.row.userType() == QMetaType::Int) {
            if (cell.column.userType() == QMetaType::Int)
                QCOMPARE(tda.cell(cell.row.toInt(), cell.column.toInt()), cell.content);
            else if (cell.column.userType() == QMetaType::QString)
                QCOMPARE(tda.cell(cell.row.toInt(), cell.column.toString()), cell.content);
            else
                qCritical() << "Unexpected type for the column:" << cell.column;
        }
        else if (cell.row.userType() == QMetaType::QString) {
            if (cell.column.userType() == QMetaType::Int)
                QCOMPARE(tda.cell(cell.row.toString(), cell.column.toInt()), cell.content);
            else if (cell.column.userType() == QMetaType::QString)
                QCOMPARE(tda.cell(cell.row.toString(), cell.column.toString()), cell.content);
            else
                qCritical() << "Unexpected type for the row:" << cell.row;
        }
        else
            qCritical() << "Unexpected type for the row:" << cell.row;
    }
}

QTEST_MAIN(tst_TdaFile)

#include "tst_tdafile.moc"
