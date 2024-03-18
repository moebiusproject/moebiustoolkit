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
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QThreadPool>

#include <optional>

struct SearchGameDialog::Private {
    SearchGameDialog& parent;
    Ui::SearchGameDialog ui;
    QPushButton* startStop = nullptr;
    QPushButton* save = nullptr;
    QPushButton* cancel = nullptr;

    GameSearcher searcher;
    bool running = false;

    QAbstractItemModel* configuredGamesModel = nullptr;
    QList<GameSearcher::Result> configuredGames;

    Private(SearchGameDialog* dialog) : parent(*dialog) {}
    void changeStartingLocation();
    void onDialogFinished();
    void onSearchFinished();
    void updateUi();
};

SearchGameDialog::SearchGameDialog(QWidget* parentObject)
    : QDialog(parentObject)
    , d(*new Private(this))
{
    setModal(true); // Block the underneath window, to make our life easier.
    setAttribute(Qt::WA_DeleteOnClose);
    d.ui.setupUi(this);

    d.startStop = new QPushButton(tr("Start"), this);
    d.save = d.ui.buttonBox->button(QDialogButtonBox::Save);
    d.cancel = d.ui.buttonBox->button(QDialogButtonBox::Cancel);
    d.startStop->setObjectName(QLatin1String("Start"));
    d.save->setObjectName(QLatin1String("Save"));
    d.cancel->setObjectName(QLatin1String("Cancel"));

    d.ui.buttonBox->addButton(d.startStop, QDialogButtonBox::ActionRole);

    connect(d.ui.startLocation, &QLineEdit::textChanged,
            this, [this] { d.updateUi(); });
    connect(d.ui.change, &QPushButton::clicked,
            this, [this] { d.changeStartingLocation(); });

    connect(d.startStop, &QPushButton::clicked, this, [=, this] {
        d.running = !d.running;
        d.searcher.setStart(d.ui.startLocation->text());
        QThreadPool::globalInstance()->start(&d.searcher);
        d.updateUi();
        // TODO: implement stop!
        d.startStop->setEnabled(!d.running);
    });

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
    connect(&d.searcher, &GameSearcher::finished, this,
            [this]() { d.onSearchFinished(); });

    connect(this, &QDialog::finished, this,
            [this]() { d.onDialogFinished(); });
}

SearchGameDialog::~SearchGameDialog()
{
    delete &d;
}

void SearchGameDialog::setConfiguredGames(QAbstractItemModel* model)
{
    Q_ASSERT(d.configuredGames.isEmpty()); // Confirm this is only called on a new dialog.
    Q_ASSERT(model->columnCount() == 2);
    d.configuredGamesModel = model;
    for (int row = 0, last = model->rowCount(); row != last; ++row) {
        d.configuredGames.append({model->data(model->index(row, 1)).toString(),
                                  model->data(model->index(row, 0)).toString()});
    }
}

void SearchGameDialog::Private::changeStartingLocation()
{
    auto dialog = new QFileDialog(&parent, tr("Choose the starting location for the search"));
    dialog->setModal(true);
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

void SearchGameDialog::Private::onDialogFinished()
{
    if (parent.result() == QDialog::Rejected)
        return;

    auto model = ui.results->model();
    for (int readRow = 0, last = model->rowCount(); readRow != last; ++readRow) {
        const auto item = ui.results->item(readRow, 0);
        const bool checked = item->checkState() == Qt::Checked;
        const bool enabled = item->flags().testFlag(Qt::ItemIsEnabled);
        if (!checked || !enabled)
            continue;

        const QString name     = model->data(model->index(readRow, 1)).toString();
        const QString location = model->data(model->index(readRow, 2)).toString();
        const int next = configuredGamesModel->rowCount();
        configuredGamesModel->insertRow(next);
        configuredGamesModel->setData(configuredGamesModel->index(next, 0), name, Qt::DisplayRole);
        configuredGamesModel->setData(configuredGamesModel->index(next, 1), location, Qt::DisplayRole);
    }
}

void SearchGameDialog::Private::onSearchFinished()
{
    running = false;
    updateUi();

    ui.results->clearContents();

    auto findConfigured = [this](const QString& path) -> std::optional<QString> {
        auto result = std::find_if(configuredGames.begin(), configuredGames.end(),
                                  [&path](const GameSearcher::Result& entry) {
            return entry.path == path;
        });
        if (result != configuredGames.end())
            return result->name;
        else
            return std::nullopt;
    };

    auto searchResults = searcher.results();
    ui.results->setRowCount(searchResults.size());
    for (int entry = 0, last = searchResults.size(); entry != last; ++entry) {
        const auto searchResult = searchResults[entry];
        const auto configured = findConfigured(searchResult.path);
        auto check = new QTableWidgetItem;
        check->setCheckState(configured.has_value() ? Qt::Checked : Qt::Unchecked);
        auto name = new QTableWidgetItem(configured.has_value() ?
                                         configured.value() : searchResult.name);
        auto path = new QTableWidgetItem(searchResult.path);
        ui.results->setItem(entry, 0, check);
        ui.results->setItem(entry, 1, name);
        ui.results->setItem(entry, 2, path);

        auto unset = [](QTableWidgetItem* item, Qt::ItemFlags flag) {
            auto flags = item->flags();
            flags &= ~flag;
            item->setFlags(flags);
        };
        // Disable the whole row if already configured. Paths are always read-only.
        unset(path, Qt::ItemIsEditable);
        if (configured) {
            unset(path, Qt::ItemIsEnabled);
            unset(check, Qt::ItemIsEnabled);
            unset(name, Qt::ItemIsEnabled);
        }
    }
}

void SearchGameDialog::Private::updateUi()
{
    startStop->setText(running ? tr("Stop") : tr("Start"));
    save->setEnabled(!running);
    cancel->setEnabled(!running);

    QFileInfo info(ui.startLocation->text());
    if (!info.exists())
        ui.progressLabel->setText(tr("The location doesn't exist"));
    else if (!info.isDir())
        ui.progressLabel->setText(tr("The location is not a directory"));
    else
        ui.progressLabel->clear();
    startStop->setEnabled(ui.progressLabel->text().isEmpty());
}
