/**************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Installer Framework.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
**************************************************************************/

#ifndef LIBARCHIVEARCHIVE_H
#define LIBARCHIVEARCHIVE_H

#include "installer_global.h"
#include "abstractarchive.h"

#include <archive.h>
#include <archive_entry.h>

#include <QThread>

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

namespace QInstaller {

class ExtractWorker : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ExtractWorker)

public:
    enum Status {
        Success = 0,
        Failure = 1,
        Canceled = 2,
        Unfinished = 3
    };

    ExtractWorker() = default;

    Status status() const;

public Q_SLOTS:
    void extract(const QString &dirPath, const quint64 totalFiles);
    void addDataBlock(const QByteArray buffer);
    void cancel();

Q_SIGNALS:
    void dataBlockRequested();
    void dataAtEnd();
    void dataReadyForRead();
    void finished(const QString &errorString = QString());

    void currentEntryChanged(const QString &filename);
    void completedChanged(quint64 completed, quint64 total);

private:
    static ssize_t readCallback(archive *reader, void *caller, const void **buff);
    bool writeEntry(archive *reader, archive *writer, archive_entry *entry);

private:
    QByteArray m_buffer;
    Status m_status;
};

class INSTALLER_EXPORT LibArchiveArchive : public AbstractArchive
{
    Q_OBJECT
    Q_DISABLE_COPY(LibArchiveArchive)

public:
    LibArchiveArchive(const QString &filename, QObject *parent = nullptr);
    explicit LibArchiveArchive(QObject *parent = nullptr);
    ~LibArchiveArchive();

    bool open(QIODevice::OpenMode mode) Q_DECL_OVERRIDE;
    void close() Q_DECL_OVERRIDE;
    void setFilename(const QString &filename) Q_DECL_OVERRIDE;

    bool extract(const QString &dirPath) Q_DECL_OVERRIDE;
    bool extract(const QString &dirPath, const quint64 totalFiles) Q_DECL_OVERRIDE;
    bool create(const QStringList &data) Q_DECL_OVERRIDE;
    QVector<ArchiveEntry> list() Q_DECL_OVERRIDE;
    bool isSupported() Q_DECL_OVERRIDE;

    void workerExtract(const QString &dirPath, const quint64 totalFiles);
    void workerAddDataBlock(const QByteArray buffer);
    void workerSetDataAtEnd();
    void workerCancel();
    ExtractWorker::Status workerStatus() const;

Q_SIGNALS:
    void dataBlockRequested();
    void workerFinished();

    void workerAboutToExtract(const QString &dirPath, const quint64 totalFiles);
    void workerAboutToAddDataBlock(const QByteArray buffer);
    void workerAboutToSetDataAtEnd();
    void workerAboutToCancel();

public Q_SLOTS:
    void cancel() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void onWorkerFinished(const QString &errorString);

private:
    static void configureReader(archive *archive);
    void configureWriter(archive *archive);
    static void configureDiskReader(archive *archive);
    static void configureDiskWriter(archive *archive);

    void initExtractWorker();

    bool writeEntry(archive *reader, archive *writer, archive_entry *entry);

    static qint64 readData(QFile *file, char *data, qint64 maxSize);
    static ssize_t readCallback(archive *reader, void *archiveData, const void **buff);

    static QString pathWithoutNamespace(const QString &path);

    quint64 totalFiles();

private:
    friend class ExtractWorker;
    friend class LibArchiveWrapperPrivate;

    struct ArchiveData
    {
        QFile file;
        QByteArray buffer;
    };

private:
    ArchiveData *m_data;
    ExtractWorker m_worker;
    QThread m_workerThread;

    bool m_cancelScheduled;
};

struct ScopedPointerReaderDeleter
{
    static inline void cleanup(archive *p)
    {
        archive_read_free(p);
    }
};

struct ScopedPointerWriterDeleter
{
    static inline void cleanup(archive *p)
    {
        archive_write_free(p);
    }
};

struct ScopedPointerEntryDeleter
{
    static inline void cleanup(archive_entry *p)
    {
        archive_entry_free(p);
    }
};

} // namespace QInstaller

#endif // LIBARCHIVEARCHIVE_H
