/*
 * This file is part of Moebius Toolkit.
 * Copyright (C) 2023 Alejandro Exojo Piqueras
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

#include "searchgamedialog.h"

#include "gamesearcher.h"
#include "ui_searchgamedialog.h"

#include <QDebug>
#include <QFileDialog>
#include <QStandardPaths>
#include <QThreadPool>

struct SearchGameDialog::Private {
    Ui::SearchGameDialog ui;
    SearchGameDialog& parent;
    QAbstractItemModel* configuredGamesModel = nullptr;
    GameSearcher searcher;

    Private(SearchGameDialog* dialog) : parent(*dialog) {}
    void changeStartingLocation();
};

SearchGameDialog::SearchGameDialog(QWidget* parentObject)
    : QDialog(parentObject)
    , d(*new Private(this))
{
    setModal(true); // Block the underneath window, to make our life easier.
    connect(this, &QDialog::finished, this, &QObject::deleteLater);
    d.ui.setupUi(this);

    static bool running = false; // Placeholder for the real helper object that searches.
    auto startStop = d.ui.buttonBox->addButton(tr("Start"), QDialogButtonBox::ActionRole);
    connect(startStop, &QPushButton::clicked, this, [=, this] {
        running = !running;
        d.searcher.setStart(d.ui.startLocation->text());
        QThreadPool::globalInstance()->start(&d.searcher);
        startStop->setText(running ? tr("Stop") : tr("Start"));
        d.ui.buttonBox->button(QDialogButtonBox::Save)->setEnabled(!running);
        d.ui.buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(!running);
    });

    connect(d.ui.change, &QPushButton::clicked,
            this, [this] { d.changeStartingLocation(); });

    const QStringList homeLocations = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    const QStringList appsLocations = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    qDebug() << "HomeLocation" << homeLocations;
    qDebug() << "ApplicationsLocation" << appsLocations;
#if defined(Q_OS_LINUX)
    d.ui.startLocation->setText(homeLocations.isEmpty() ? QString() : homeLocations.first());
#elif defined(Q_OS_MACOS)
    d.ui.startLocation->setText(appsLocations.isEmpty() ? QString() : appsLocations.first());
#else
    d.ui.startLocation->setText(QLatin1String("C:/"));
#endif

    connect(&d.searcher, &GameSearcher::progress, this, [this](const QString& name) {
        const int width = d.ui.progressLabel->width();
        QFontMetrics metrics(d.ui.progressLabel->fontMetrics());
        d.ui.progressLabel->setText(metrics.elidedText(name, Qt::ElideMiddle, width));
    });
    connect(&d.searcher, &GameSearcher::finished, this, [=, this]() {
        running = false;
        startStop->setText(running ? tr("Stop") : tr("Start"));
        d.ui.buttonBox->button(QDialogButtonBox::Save)->setEnabled(!running);
        d.ui.buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(!running);
    });
}

SearchGameDialog::~SearchGameDialog()
{
    delete &d;
}

void SearchGameDialog::setConfiguredGames(QAbstractItemModel* model)
{
    Q_ASSERT(model->columnCount() == 2);
    d.configuredGamesModel = model;
}

void SearchGameDialog::Private::changeStartingLocation()
{
    auto dialog = new QFileDialog(&parent, tr("Choose the starting location for the search"));
    const QString location = ui.startLocation->text();
    if (const QFileInfo info(location); info.isDir())
        dialog->setDirectory(location);
    dialog->setFileMode(QFileDialog::Directory);
    dialog->setFilter(QDir::Dirs|QDir::NoDotAndDotDot);
    dialog->show();

    QObject::connect(dialog, &QFileDialog::fileSelected,
                     &parent, [this, dialog](const QString& file) {
        ui.startLocation->setText(file);
        dialog->deleteLater();
    });
}
