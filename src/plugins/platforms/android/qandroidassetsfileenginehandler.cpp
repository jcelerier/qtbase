/****************************************************************************
**
** Copyright (C) 2012 BogDan Vatra <bogdan@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
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

#include "qandroidassetsfileenginehandler.h"
#include "androidjnimain.h"
#include <optional>

#include <QCoreApplication>
#include <QVector>
#include <QtCore/private/qjni_p.h>

QT_BEGIN_NAMESPACE

static const QLatin1String assetsPrefix("assets:");
const static int prefixSize = 7;

static inline QString cleanedAssetPath(QString file)
{
    if (file.startsWith(assetsPrefix))
        file.remove(0, prefixSize);
    file.replace(QLatin1String("//"), QLatin1String("/"));
    if (file.startsWith(QLatin1Char('/')))
        file.remove(0, 1);
    if (file.endsWith(QLatin1Char('/')))
        file.chop(1);
    return file;
}

static inline QString prefixedPath(QString path)
{
    path = assetsPrefix + QLatin1Char('/') + path;
    path.replace(QLatin1String("//"), QLatin1String("/"));
    return path;
}

struct AssetItem {
    enum class Type {
        File,
        Folder
    };

    AssetItem (const QString &rawName)
        : name(rawName)
    {
        if (name.endsWith(QLatin1Char('/'))) {
            type = Type::Folder;
            name.chop(1);
        }
    }
    Type type = Type::File;
    QString name;
};

using AssetItemList = QVector<AssetItem>;

class FolderIterator : public AssetItemList
{
public:
    static QSharedPointer<FolderIterator> fromCache(const QString &path)
    {
        QMutexLocker lock(&m_assetsCacheMutex);
        QSharedPointer<FolderIterator> *folder = m_assetsCache.object(path);
        if (!folder) {
            folder = new QSharedPointer<FolderIterator>{new FolderIterator{path}};
            if (!m_assetsCache.insert(path, folder)) {
                QSharedPointer<FolderIterator> res = *folder;
                delete folder;
                return res;
            }
        }
        return *folder;
    }

    FolderIterator(const QString &path)
        : m_path(path)
    {
        QJNIObjectPrivate files = QJNIObjectPrivate::callStaticObjectMethod(QtAndroid::applicationClass(),
                                                                            "listAssetContent",
                                                                            "(Landroid/content/res/AssetManager;Ljava/lang/String;)[Ljava/lang/String;",
                                                                            QtAndroid::assets(), QJNIObjectPrivate::fromString(path).object());
        if (files.isValid()) {
            QJNIEnvironmentPrivate env;
            jobjectArray jFiles = static_cast<jobjectArray>(files.object());
            const jint nFiles = env->GetArrayLength(jFiles);
            for (int i = 0; i < nFiles; ++i)
                push_back({QJNIObjectPrivate(env->GetObjectArrayElement(jFiles, i)).toString()});
        }
        m_path = assetsPrefix + QLatin1Char('/') + m_path + QLatin1Char('/');
        m_path.replace(QLatin1String("//"), QLatin1String("/"));
    }

    QString currentFileName() const
    {
        if (m_index < 0 || m_index >= size())
            return {};
        return at(m_index).name;
    }
    QString currentFilePath() const
    {
        if (m_index < 0 || m_index >= size())
            return {};
        return m_path + at(m_index).name;
    }

    bool hasNext() const
    {
        return !empty() && m_index + 1 < size();
    }

    std::optional<std::pair<QString, AssetItem>> next()
    {
        if (!hasNext())
            return {};
        ++m_index;
        return std::pair<QString, AssetItem>(currentFileName(), at(m_index));
    }

private:
    int m_index = -1;
    QString m_path;
    static QCache<QString, QSharedPointer<FolderIterator>> m_assetsCache;
    static QMutex m_assetsCacheMutex;
};

QCache<QString, QSharedPointer<FolderIterator>> FolderIterator::m_assetsCache(std::max(50, qEnvironmentVariableIntValue("QT_ANDROID_MAX_ASSETS_CACHE_SIZE")));
QMutex FolderIterator::m_assetsCacheMutex;

class AndroidAbstractFileEngineIterator: public QAbstractFileEngineIterator
{
public:
    AndroidAbstractFileEngineIterator(QDir::Filters filters,
                                      const QStringList &nameFilters,
                                      const QString &path)
        : QAbstractFileEngineIterator(filters, nameFilters)
    {
        m_stack.push_back(FolderIterator::fromCache(cleanedAssetPath(path)));
        if (m_stack.last()->empty())
            m_stack.pop_back();
    }

    QFileInfo currentFileInfo() const override
    {
        return QFileInfo(currentFilePath());
    }

    QString currentFileName() const override
    {
        if (!m_currentIterator)
            return {};
        return m_currentIterator->currentFileName();
    }

    virtual QString currentFilePath() const
    {
        if (!m_currentIterator)
            return {};
        return m_currentIterator->currentFilePath();
    }

    bool hasNext() const override
    {
        if (m_stack.empty())
            return false;
        if (!m_stack.last()->hasNext()) {
            m_stack.pop_back();
            return hasNext();
        }
        return true;
    }

    QString next() override
    {
        if (m_stack.empty()) {
            m_currentIterator.reset();
            return {};
        }
        m_currentIterator = m_stack.last();
        auto res = m_currentIterator->next();
        if (!res)
            return {};
        if (res->second.type == AssetItem::Type::Folder) {
            m_stack.push_back(FolderIterator::fromCache(cleanedAssetPath(currentFilePath())));
            if (m_stack.last()->empty())
                m_stack.pop_back();
        }
        return res->first;
    }

private:
    mutable QSharedPointer<FolderIterator> m_currentIterator;
    mutable QVector<QSharedPointer<FolderIterator>> m_stack;
};

class AndroidAbstractFileEngine: public QAbstractFileEngine
{
public:
    explicit AndroidAbstractFileEngine(AAssetManager *assetManager, const QString &fileName)
        : m_assetManager(assetManager)
    {
        setFileName(fileName);
    }

    ~AndroidAbstractFileEngine()
    {
        close();
    }

    bool open(QIODevice::OpenMode openMode) override
    {
        if (m_isFolder || (openMode & QIODevice::WriteOnly))
            return false;
        close();
        m_assetFile = AAssetManager_open(m_assetManager, m_fileName.toUtf8(), AASSET_MODE_BUFFER);
        return m_assetFile;
    }

    bool close() override
    {
        if (m_assetFile) {
            AAsset_close(m_assetFile);
            m_assetFile = 0;
            return true;
        }
        return false;
    }

    qint64 size() const override
    {
        if (m_assetFile)
            return AAsset_getLength(m_assetFile);
        return -1;
    }

    qint64 pos() const override
    {
        if (m_assetFile)
            return AAsset_seek(m_assetFile, 0, SEEK_CUR);
        return -1;
    }

    bool seek(qint64 pos) override
    {
        if (m_assetFile)
            return pos == AAsset_seek(m_assetFile, pos, SEEK_SET);
        return false;
    }

    qint64 read(char *data, qint64 maxlen) override
    {
        if (m_assetFile)
            return AAsset_read(m_assetFile, data, maxlen);
        return -1;
    }

    bool isSequential() const override
    {
        return false;
    }

    bool caseSensitive() const override
    {
        return true;
    }

    bool isRelativePath() const override
    {
        return false;
    }

    FileFlags fileFlags(FileFlags type = FileInfoAll) const override
    {
        FileFlags flags(ReadOwnerPerm|ReadUserPerm|ReadGroupPerm|ReadOtherPerm|ExistsFlag);
        if (m_assetFile)
            flags |= FileType;
        else if (m_isFolder)
            flags |= DirectoryType;

        return type & flags;
    }

    QString fileName(FileName file = DefaultName) const override
    {
        int pos;
        switch (file) {
        case DefaultName:
        case AbsoluteName:
        case CanonicalName:
                return prefixedPath(m_fileName);
        case BaseName:
            if ((pos = m_fileName.lastIndexOf(QChar(QLatin1Char('/')))) != -1)
                return prefixedPath(m_fileName.mid(pos));
            else
                return prefixedPath(m_fileName);
        case PathName:
        case AbsolutePathName:
        case CanonicalPathName:
            if ((pos = m_fileName.lastIndexOf(QChar(QLatin1Char('/')))) != -1)
                return prefixedPath(m_fileName.left(pos));
            else
                return prefixedPath(m_fileName);
        default:
            return QString();
        }
    }

    void setFileName(const QString &file) override
    {
        close();
        m_fileName = cleanedAssetPath(file);
        m_isFolder = !open(QIODevice::ReadOnly) && !FolderIterator::fromCache(m_fileName)->empty();
    }

    Iterator *beginEntryList(QDir::Filters filters, const QStringList &filterNames) override
    {
        if (m_isFolder)
            return new AndroidAbstractFileEngineIterator(filters, filterNames, m_fileName);
        return nullptr;
    }

private:
    AAsset *m_assetFile = nullptr;
    AAssetManager *m_assetManager;
    QString m_fileName;
    bool m_isFolder;
};


AndroidAssetsFileEngineHandler::AndroidAssetsFileEngineHandler()
{
    m_assetManager = QtAndroid::assetManager();
}

QAbstractFileEngine * AndroidAssetsFileEngineHandler::create(const QString &fileName) const
{
    if (fileName.isEmpty())
        return nullptr;

    if (!fileName.startsWith(assetsPrefix))
        return nullptr;

    QString path = fileName.mid(prefixSize);
    path.replace(QLatin1String("//"), QLatin1String("/"));
    if (path.startsWith(QLatin1Char('/')))
        path.remove(0, 1);
    if (path.endsWith(QLatin1Char('/')))
        path.chop(1);
    return new AndroidAbstractFileEngine(m_assetManager, path);
}

QT_END_NAMESPACE
