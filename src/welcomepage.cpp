/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2019-2021 Alejandro Exojo Piqueras
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

#include "pagetype.h"
#include "welcomepage.h"

#include "ui_welcomepage.h"

#include <QDataWidgetMapper>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>
#include <QStandardItemModel>

struct WelcomePage::Private
{
    WelcomePage& parent;
    Ui::WelcomePage ui;

    QStandardItemModel* games = nullptr;

    Private(WelcomePage& page) : parent(page) {}

    void saveGameList();
    void loadGameList();

    void updateUi();

    void configuredGamesChanged(int index);
    void openChitinKeyDialog();
};

static const auto keyConfiguredGames = QStringLiteral("ConfiguredGames");
static const auto keyName = QStringLiteral("Name");
static const auto keyLocation = QStringLiteral("Location");

WelcomePage::WelcomePage(QWidget* parent)
    : BasePage(parent)
    , d(new Private(*this))
{
    d->ui.setupUi(this);
    d->ui.locationError->hide();

    d->games = qobject_cast<QStandardItemModel*>(d->ui.configuredGames->model());
    Q_ASSERT(d->games); // QComboBox has this model by default, but just in case.
    d->games->setColumnCount(2);
    d->loadGameList();
    d->configuredGamesChanged(d->ui.configuredGames->currentIndex());
    d->updateUi();

#ifndef QT_DEBUG // Unreleased for now.
    d->ui.buffCalculator->setEnabled(false);
#endif
#ifdef Q_OS_WASM // Force enable the modules without the UI to setup locations.
    d->ui.configuredGames->setEnabled(false);
    d->ui.name->setEnabled(false);
    d->ui.location->setEnabled(false);
    d->ui.addNew->setEnabled(false);
    d->ui.save->setEnabled(false);
    d->ui.remove->setEnabled(false);
    d->ui.remove->setEnabled(false);
    d->ui.progressionCharts->setEnabled(true);
    d->ui.dualCalculator->setEnabled(true);
#endif

    connect(d->ui.configuredGames, qOverload<int>(&QComboBox::currentIndexChanged),
            std::bind(&Private::configuredGamesChanged, d, std::placeholders::_1));
    connect(d->ui.addNew, &QPushButton::clicked, [this] {
        d->games->insertRow(d->games->rowCount());
        d->ui.configuredGames->setCurrentIndex(d->games->rowCount() - 1);
        d->updateUi();
    });
    connect(d->ui.remove, &QPushButton::clicked, [this] {
        d->games->removeRow(d->ui.configuredGames->currentIndex());
        d->saveGameList();
        d->updateUi();
    });

    connect(d->ui.location, &QLineEdit::textChanged, std::bind(&Private::updateUi, d));
    connect(d->ui.locationOpen, &QPushButton::clicked, std::bind(&Private::openChitinKeyDialog, d));

    connect(d->ui.save, &QPushButton::clicked, [this] {
        const int selected = d->ui.configuredGames->currentIndex();
        const QString newName     = d->ui.name->text();
        const QString newLocation = d->ui.location->text();
        d->games->setData(d->games->index(selected, 0), newName,     Qt::DisplayRole);
        d->games->setData(d->games->index(selected, 1), newLocation, Qt::DisplayRole);
        d->saveGameList();
    });

    connect(d->ui.backstabCalculator, &QPushButton::clicked,
            this, [this]{ emit newPageRequested(PageType::BackstabCalculator); });
    connect(d->ui.buffCalculator, &QPushButton::clicked,
            this, [this]{ emit newPageRequested(PageType::BuffCalculator); });
    connect(d->ui.damageCalculator, &QPushButton::clicked,
            this, [this]{ emit newPageRequested(PageType::DamageCalculator); });
    connect(d->ui.dualCalculator, &QPushButton::clicked,
            this, [this]{ emit newPageRequested(PageType::DualCalculator); });
    connect(d->ui.gameBrowser, &QPushButton::clicked,
            this, [this]{ emit newPageRequested(PageType::GameBrowser); });
    connect(d->ui.progressionCharts, &QPushButton::clicked,
            this, [this]{ emit newPageRequested(PageType::ProgressionCharts); });
    connect(d->ui.repeatedProbability, &QPushButton::clicked,
            this, [this]{ emit newPageRequested(PageType::RepeatedProbability); });
}

WelcomePage::~WelcomePage()
{
    delete d;
    d = nullptr;
}

QString WelcomePage::gameName() const
{
    return d->ui.name->text();
}

QString WelcomePage::gameLocation() const
{
    return d->ui.location->text();
}

void WelcomePage::Private::saveGameList()
{
    QSettings settings;
    settings.beginWriteArray(keyConfiguredGames);
    for (int index = 0, size = games->rowCount(); index < size; ++index) {
        settings.setArrayIndex(index);
        const QString name     = games->data(games->index(index, 0)).toString();
        const QString location = games->data(games->index(index, 1)).toString();
        settings.setValue(keyName,     name);
        settings.setValue(keyLocation, location);
    }
    settings.endArray();
}

void WelcomePage::Private::loadGameList()
{
    QSettings settings;
    const int configuredGames = settings.beginReadArray(keyConfiguredGames);
    for (int row = 0; row < configuredGames; ++row) {
        settings.setArrayIndex(row);
        const QString name     = settings.value(keyName).toString();
        const QString location = settings.value(keyLocation).toString();
        games->insertRow(games->rowCount());
        games->setData(games->index(row, 0), name, Qt::DisplayRole);
        games->setData(games->index(row, 1), location, Qt::DisplayRole);
    }
    settings.endArray();
}

void WelcomePage::Private::updateUi()
{
    const auto keyFile = QLatin1String("chitin.key");
    const bool enable = games->rowCount() > 0;
    const QString location = ui.location->text();

    QFileInfo info(location);
    const bool validFile = info.isFile() && !info.fileName().compare(keyFile, Qt::CaseInsensitive);

    if (!enable || location.isEmpty())
        ui.locationError->clear();
    else if (!info.exists())
        ui.locationError->setText(tr("Location does not exist"));
    else if (!validFile)
        ui.locationError->setText(tr("Not a valid location. Select a \"chitin.key\" file"));
    else
        ui.locationError->clear();

    ui.configuredGames->setEnabled(enable);
    ui.save->setEnabled(enable);
    ui.remove->setEnabled(enable);
    ui.name->setEnabled(enable);
    ui.location->setEnabled(enable);
    ui.locationOpen->setEnabled(enable);

    const bool validLocation = ui.locationError->text().isEmpty();
    ui.locationError->setVisible(!validLocation);
    ui.gameBrowser->setEnabled(validLocation && !location.isEmpty());
    ui.progressionCharts->setEnabled(validLocation && !location.isEmpty());
    ui.dualCalculator->setEnabled(validLocation && !location.isEmpty());
}

void WelcomePage::Private::configuredGamesChanged(int index)
{
    ui.name->setText(games->data(games->index(index, 0)).toString());
    ui.location->setText(games->data(games->index(index, 1)).toString());
}

void WelcomePage::Private::openChitinKeyDialog()
{
    auto dialog = new QFileDialog(&parent, tr("Choose a \"chitin.key\" file"));
    if (QFileInfo(ui.location->text()).isDir())
        dialog->setDirectory(ui.location->text());
    dialog->setNameFilter(QLatin1String("chitin.key"));
    dialog->show();

    QObject::connect(dialog, &QFileDialog::fileSelected,
                     &parent, [this, dialog](const QString& file) {
        ui.location->setText(file);
        dialog->deleteLater();
    });
}
