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

#include "calculators.h"
#include "pagetype.h"
#include "buffcalculatorpage.h"

#include "ui_buffcalculatorpage.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

using namespace Calculators;

class SpinBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    SpinBoxDelegate(QObject* parent = nullptr)
        : QStyledItemDelegate(parent)
    {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override
    {
        Q_UNUSED(index);
        Q_UNUSED(option);

        QSpinBox* editor = new QSpinBox(parent);
        editor->setFrame(false);
        editor->setMinimum(0);
        editor->setMaximum(6);
        return editor;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {
        int value = index.model()->data(index, Qt::EditRole).toInt();

        auto spinBox = static_cast<QSpinBox*>(editor);
        spinBox->setValue(value);
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override
    {
        auto spinBox = static_cast<QSpinBox*>(editor);
        spinBox->interpretText();
        int value = spinBox->value();
        model->setData(index, value, Qt::EditRole);
    }

    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const override
    {
        Q_UNUSED(index);
        editor->setGeometry(option.rect);
    }
};


struct BuffCalculatorPage::Private
{
    Private(BuffCalculatorPage& page) : parent(page) {}

    BuffCalculatorPage& parent;

    Ui::BuffCalculatorPage ui;

//    QStandardItemModel characters;
    QStandardItemModel buffs;
//    QStandardItemModel result;
};

BuffCalculatorPage::BuffCalculatorPage(QWidget* parent)
    : QWidget(parent)
    , d(new Private(*this))
{
    d->ui.setupUi(this);

    d->buffs.setColumnCount(2);

    d->ui.availableBuffsView->horizontalHeader()->setVisible(true);
    d->ui.availableBuffsView->horizontalHeader()->setStretchLastSection(true);
    d->ui.availableBuffsView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    d->ui.availableBuffsView->verticalHeader()->setVisible(false);

    const auto buffs = buffList();
#if 1
    d->ui.availableBuffsView->setItemDelegateForColumn(1, new SpinBoxDelegate(this));
    for (const auto& buff : buffs) {
        QList<QStandardItem*> row;
        row << new QStandardItem(buff.name);
        row << new QStandardItem();
        d->buffs.appendRow(row);
    }
    d->ui.availableBuffsView->setModel(&d->buffs);
#else
    d->ui.availableBuffsView->setRowCount(buffs.size());
    d->ui.availableBuffsView->setColumnCount(2);
    int row = 0;
    for (const auto& buff : buffs) {
        d->ui.availableBuffsView->setItem(row, 0, new QTableWidgetItem(buff.name));
        auto spinbox = new QSpinBox;
        d->ui.availableBuffsView->setCellWidget(row, 1, spinbox);
        spinbox->setRange(0, 10);

        ++row;
    }
#endif
    d->ui.availableBuffsView->resizeColumnsToContents();

//  for (int row = 0; row < 4; ++row) {
//      for (int column = 0; column < 4; ++column) {
//          QStandardItem *item = new QStandardItem(QString("row %0, column %1").arg(row).arg(column));
//          model.setItem(row, column, item);
//      }
//  }

//    qDebug() << "model" << d->ui.charactersView->model();
//    qDebug() << "model" << d->ui.availableBuffsView->model();

//    qDebug() << "model" << d->ui.resultView->model();
}

BuffCalculatorPage::~BuffCalculatorPage()
{
    delete d;
    d = nullptr;
}

#include "buffcalculatorpage.moc"
