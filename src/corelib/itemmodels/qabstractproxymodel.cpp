/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qabstractproxymodel.h"
#include "qitemselectionmodel.h"
#include <private/qabstractproxymodel_p.h>
#include <QtCore/QSize>
#include <QtCore/QStringList>
#include <QtCore/QMap>


QT_BEGIN_NAMESPACE

/*!
    \since 4.1
    \class QAbstractProxyModel
    \brief The QAbstractProxyModel class provides a base class for proxy item
    models that can do sorting, filtering or other data processing tasks.
    \ingroup model-view
    \inmodule QtCore

    This class defines the standard interface that proxy models must use to be
    able to interoperate correctly with other model/view components. It is not
    supposed to be instantiated directly.

    All standard proxy models are derived from the QAbstractProxyModel class.
    If you need to create a new proxy model class, it is usually better to
    subclass an existing class that provides the closest behavior to the one
    you want to provide.

    Proxy models that filter or sort items of data from a source model should
    be created by using or subclassing QSortFilterProxyModel.

    To subclass QAbstractProxyModel, you need to implement mapFromSource() and
    mapToSource(). The mapSelectionFromSource() and mapSelectionToSource()
    functions only need to be reimplemented if you need a behavior different
    from the default behavior.

    \note If the source model is deleted or no source model is specified, the
    proxy model operates on a empty placeholder model.

    \sa QSortFilterProxyModel, QAbstractItemModel, {Model/View Programming}
*/

/*!
    \property QAbstractProxyModel::sourceModel

    \brief the source model of this proxy model.
*/

//detects the deletion of the source model
void QAbstractProxyModelPrivate::_q_sourceModelDestroyed()
{
    invalidatePersistentIndexes();
    model = QAbstractItemModelPrivate::staticEmptyModel();
}

/*!
    Constructs a proxy model with the given \a parent.
*/

QAbstractProxyModel::QAbstractProxyModel(QObject *parent)
    :QAbstractItemModel(*new QAbstractProxyModelPrivate, parent)
{
    setSourceModel(QAbstractItemModelPrivate::staticEmptyModel());
}

/*!
    \internal
*/

QAbstractProxyModel::QAbstractProxyModel(QAbstractProxyModelPrivate &dd, QObject *parent)
    : QAbstractItemModel(dd, parent)
{
    setSourceModel(QAbstractItemModelPrivate::staticEmptyModel());
}

/*!
    Destroys the proxy model.
*/
QAbstractProxyModel::~QAbstractProxyModel()
{

}

/*!
    Sets the given \a sourceModel to be processed by the proxy model.

    Subclasses should call beginResetModel() at the beginning of the method,
    disconnect from the old model, call this method, connect to the new model,
    and call endResetModel().
*/
void QAbstractProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    Q_D(QAbstractProxyModel);
    d->model.removeBindingUnlessInWrapper();
    // Special case to handle nullptr models. Otherwise we will have unwanted
    // notifications.
    if (!sourceModel && d->model == QAbstractItemModelPrivate::staticEmptyModel())
        return;
    if (sourceModel != d->model) {
        if (d->model)
            disconnect(d->model, SIGNAL(destroyed()), this, SLOT(_q_sourceModelDestroyed()));

        if (sourceModel) {
            d->model.setValueBypassingBindings(sourceModel);
            connect(d->model, SIGNAL(destroyed()), this, SLOT(_q_sourceModelDestroyed()));
        } else {
            d->model.setValueBypassingBindings(QAbstractItemModelPrivate::staticEmptyModel());
        }
        d->model.notify();
    }
}

/*!
    Returns the model that contains the data that is available through the proxy model.
*/
QAbstractItemModel *QAbstractProxyModel::sourceModel() const
{
    Q_D(const QAbstractProxyModel);
    if (d->model == QAbstractItemModelPrivate::staticEmptyModel())
        return nullptr;
    return d->model;
}

QBindable<QAbstractItemModel *> QAbstractProxyModel::bindableSourceModel()
{
    Q_D(QAbstractProxyModel);
    return QBindable<QAbstractItemModel *>(&d->model);
}

/*!
    \reimp
 */
bool QAbstractProxyModel::submit()
{
    Q_D(QAbstractProxyModel);
    return d->model->submit();
}

/*!
    \reimp
 */
void QAbstractProxyModel::revert()
{
    Q_D(QAbstractProxyModel);
    d->model->revert();
}


/*!
  \fn QModelIndex QAbstractProxyModel::mapToSource(const QModelIndex &proxyIndex) const

  Reimplement this function to return the model index in the source model that
  corresponds to the \a proxyIndex in the proxy model.

  \sa mapFromSource()
*/

/*!
  \fn QModelIndex QAbstractProxyModel::mapFromSource(const QModelIndex &sourceIndex) const

  Reimplement this function to return the model index in the proxy model that
  corresponds to the \a sourceIndex from the source model.

  \sa mapToSource()
*/

/*!
  Returns a source selection mapped from the specified \a proxySelection.

  Reimplement this method to map proxy selections to source selections.
 */
QItemSelection QAbstractProxyModel::mapSelectionToSource(const QItemSelection &proxySelection) const
{
    QModelIndexList proxyIndexes = proxySelection.indexes();
    QItemSelection sourceSelection;
    for (int i = 0; i < proxyIndexes.size(); ++i) {
        const QModelIndex proxyIdx = mapToSource(proxyIndexes.at(i));
        if (!proxyIdx.isValid())
            continue;
        sourceSelection << QItemSelectionRange(proxyIdx);
    }
    return sourceSelection;
}

/*!
  Returns a proxy selection mapped from the specified \a sourceSelection.

  Reimplement this method to map source selections to proxy selections.
*/
QItemSelection QAbstractProxyModel::mapSelectionFromSource(const QItemSelection &sourceSelection) const
{
    QModelIndexList sourceIndexes = sourceSelection.indexes();
    QItemSelection proxySelection;
    for (int i = 0; i < sourceIndexes.size(); ++i) {
        const QModelIndex srcIdx = mapFromSource(sourceIndexes.at(i));
        if (!srcIdx.isValid())
            continue;
        proxySelection << QItemSelectionRange(srcIdx);
    }
    return proxySelection;
}

/*!
    \reimp
 */
QVariant QAbstractProxyModel::data(const QModelIndex &proxyIndex, int role) const
{
    Q_D(const QAbstractProxyModel);
    return d->model->data(mapToSource(proxyIndex), role);
}

/*!
    \reimp
 */
QVariant QAbstractProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_D(const QAbstractProxyModel);
    int sourceSection;
    if (orientation == Qt::Horizontal) {
        const QModelIndex proxyIndex = index(0, section);
        sourceSection = mapToSource(proxyIndex).column();
    } else {
        const QModelIndex proxyIndex = index(section, 0);
        sourceSection = mapToSource(proxyIndex).row();
    }
    return d->model->headerData(sourceSection, orientation, role);
}

/*!
    \reimp
 */
QMap<int, QVariant> QAbstractProxyModel::itemData(const QModelIndex &proxyIndex) const
{
    Q_D(const QAbstractProxyModel);
    return d->model->itemData(mapToSource(proxyIndex));
}

/*!
    \reimp
 */
Qt::ItemFlags QAbstractProxyModel::flags(const QModelIndex &index) const
{
    Q_D(const QAbstractProxyModel);
    return d->model->flags(mapToSource(index));
}

/*!
    \reimp
 */
bool QAbstractProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_D(QAbstractProxyModel);
    return d->model->setData(mapToSource(index), value, role);
}

/*!
    \reimp
 */
bool QAbstractProxyModel::setItemData(const QModelIndex &index, const QMap< int, QVariant >& roles)
{
    Q_D(QAbstractProxyModel);
    return d->model->setItemData(mapToSource(index), roles);
}

/*!
    \reimp
 */
bool QAbstractProxyModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    Q_D(QAbstractProxyModel);
    int sourceSection;
    if (orientation == Qt::Horizontal) {
        const QModelIndex proxyIndex = index(0, section);
        sourceSection = mapToSource(proxyIndex).column();
    } else {
        const QModelIndex proxyIndex = index(section, 0);
        sourceSection = mapToSource(proxyIndex).row();
    }
    return d->model->setHeaderData(sourceSection, orientation, value, role);
}

/*!
    \reimp
    \since 6.0
 */
bool QAbstractProxyModel::clearItemData(const QModelIndex &index)
{
    Q_D(QAbstractProxyModel);
    return d->model->clearItemData(mapToSource(index));
}

/*!
    \reimp
 */
QModelIndex QAbstractProxyModel::buddy(const QModelIndex &index) const
{
    Q_D(const QAbstractProxyModel);
    return mapFromSource(d->model->buddy(mapToSource(index)));
}

/*!
    \reimp
 */
bool QAbstractProxyModel::canFetchMore(const QModelIndex &parent) const
{
    Q_D(const QAbstractProxyModel);
    return d->model->canFetchMore(mapToSource(parent));
}

/*!
    \reimp
 */
void QAbstractProxyModel::fetchMore(const QModelIndex &parent)
{
    Q_D(QAbstractProxyModel);
    d->model->fetchMore(mapToSource(parent));
}

/*!
    \reimp
 */
void QAbstractProxyModel::sort(int column, Qt::SortOrder order)
{
    Q_D(QAbstractProxyModel);
    d->model->sort(column, order);
}

/*!
    \reimp
 */
QSize QAbstractProxyModel::span(const QModelIndex &index) const
{
    Q_D(const QAbstractProxyModel);
    return d->model->span(mapToSource(index));
}

/*!
    \reimp
 */
bool QAbstractProxyModel::hasChildren(const QModelIndex &parent) const
{
    Q_D(const QAbstractProxyModel);
    return d->model->hasChildren(mapToSource(parent));
}

/*!
    \reimp
 */
QModelIndex QAbstractProxyModel::sibling(int row, int column, const QModelIndex &idx) const
{
    return index(row, column, idx.parent());
}

/*!
    \reimp
 */
QMimeData* QAbstractProxyModel::mimeData(const QModelIndexList &indexes) const
{
    Q_D(const QAbstractProxyModel);
    QModelIndexList list;
    list.reserve(indexes.count());
    for (const QModelIndex &index : indexes)
        list << mapToSource(index);
    return d->model->mimeData(list);
}

void QAbstractProxyModelPrivate::mapDropCoordinatesToSource(int row, int column, const QModelIndex &parent,
                                                            int *sourceRow, int *sourceColumn, QModelIndex *sourceParent) const
{
    Q_Q(const QAbstractProxyModel);
    *sourceRow = -1;
    *sourceColumn = -1;
    if (row == -1 && column == -1) {
        *sourceParent = q->mapToSource(parent);
    } else if (row == q->rowCount(parent)) {
        *sourceParent = q->mapToSource(parent);
        *sourceRow = model->rowCount(*sourceParent);
    } else {
        QModelIndex proxyIndex = q->index(row, column, parent);
        QModelIndex sourceIndex = q->mapToSource(proxyIndex);
        *sourceRow = sourceIndex.row();
        *sourceColumn = sourceIndex.column();
        *sourceParent = sourceIndex.parent();
    }
}

/*!
    \reimp
    \since 5.4
 */
bool QAbstractProxyModel::canDropMimeData(const QMimeData *data, Qt::DropAction action,
                                          int row, int column, const QModelIndex &parent) const
{
    Q_D(const QAbstractProxyModel);
    int sourceDestinationRow;
    int sourceDestinationColumn;
    QModelIndex sourceParent;
    d->mapDropCoordinatesToSource(row, column, parent, &sourceDestinationRow, &sourceDestinationColumn, &sourceParent);
    return d->model->canDropMimeData(data, action, sourceDestinationRow, sourceDestinationColumn, sourceParent);
}

/*!
    \reimp
    \since 5.4
 */
bool QAbstractProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                       int row, int column, const QModelIndex &parent)
{
    Q_D(QAbstractProxyModel);
    int sourceDestinationRow;
    int sourceDestinationColumn;
    QModelIndex sourceParent;
    d->mapDropCoordinatesToSource(row, column, parent, &sourceDestinationRow, &sourceDestinationColumn, &sourceParent);
    return d->model->dropMimeData(data, action, sourceDestinationRow, sourceDestinationColumn, sourceParent);
}

/*!
    \reimp
 */
QStringList QAbstractProxyModel::mimeTypes() const
{
    Q_D(const QAbstractProxyModel);
    return d->model->mimeTypes();
}

/*!
    \reimp
 */
Qt::DropActions QAbstractProxyModel::supportedDragActions() const
{
    Q_D(const QAbstractProxyModel);
    return d->model->supportedDragActions();
}

/*!
    \reimp
 */
Qt::DropActions QAbstractProxyModel::supportedDropActions() const
{
    Q_D(const QAbstractProxyModel);
    return d->model->supportedDropActions();
}

/*!
    \reimp
 */
QHash<int,QByteArray> QAbstractProxyModel::roleNames() const
{
  Q_D(const QAbstractProxyModel);
  return d->model->roleNames();
}

/*!
    Equivalent to calling createIndex on the source model.

    This method is useful if your proxy model wants to maintain the
    parent-child relationship of items in the source model.
    When reimplementing mapToSource(), you can call this method to
    create an index for row \a row and column \a col of the source model.

    A typical use would be to save the internal pointer coming from the source model
    in the proxy index when reimplementing mapFromSource() and use the same internal
    pointer as \a internalPtr to recover the original source index when
    reimplementing mapToSource().
    \since 6.2
 */
QModelIndex QAbstractProxyModel::createSourceIndex(int row, int col, void *internalPtr) const
{
    if (sourceModel())
        return sourceModel()->createIndex(row, col, internalPtr);
    return QModelIndex();
}

QT_END_NAMESPACE

#include "moc_qabstractproxymodel.cpp"
