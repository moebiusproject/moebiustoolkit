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

#include "resourcemanager.h"

class tst_ResourceManager: public QObject
{
    Q_OBJECT

private slots:
    void test_data();
    void test();
};

void tst_ResourceManager::test_data()
{
    QTest::addColumn<QString>("filePath");

    QTest::newRow("BG1 EE")
            << QFINDTESTDATA("../../data/manager/bg1ee/chitin.key");
}

void tst_ResourceManager::test()
{
    QFETCH(QString, filePath);
    if (filePath.isEmpty())
        QSKIP("File not found, skipping test.");
    ResourceManager manager;
    manager.load(filePath);

    const QByteArray resourceData = manager.defaultResource(QLatin1String("xplevel"), TdaType);
    const QByteArray overriddenData = manager.resource(QLatin1String("xplevel"), TdaType);

    const QString directoryName = filePath.section(QLatin1Char('/'), 0, -2, QString::SectionIncludeTrailingSep);

    auto compareFiles = [&directoryName](const char* file, const QByteArray& contents) {
        QFile resource(directoryName + QLatin1String(file));
        qDebug() << resource.fileName();
        QVERIFY(resource.exists());
        QVERIFY(resource.open(QIODevice::ReadOnly));
        QCOMPARE(resource.readAll(), contents);
    };

    compareFiles("resource.2da", resourceData);
    compareFiles("overridden.2da", overriddenData);
}

QTEST_MAIN(tst_ResourceManager)

#include "tst_resourcemanager.moc"
