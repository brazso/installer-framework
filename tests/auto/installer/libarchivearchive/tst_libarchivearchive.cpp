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

#include <libarchivearchive.h>
#include <fileutils.h>

#include <QDir>
#include <QObject>
#include <QTemporaryFile>
#include <QTest>

using namespace QInstaller;

class tst_libarchivearchive : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        m_file.path = "valid";
        m_file.compressedSize = 0; // unused
        m_file.uncompressedSize = 5242880;
        m_file.isDirectory = false;
        m_file.archiveIndex = QPoint(0, 0);
        m_file.utcTime = QDateTime(QDate::fromJulianDay(2456413), QTime(10, 50, 42), Qt::UTC);
    }

    void testIsSupportedArchive_data()
    {
        archiveFilenamesTestData();
    }

    void testIsSupportedArchive()
    {
        QFETCH(QString, filename);

        LibArchiveArchive archive(filename);
        QVERIFY(archive.open(QIODevice::ReadOnly));
        QCOMPARE(archive.isSupported(), true);
    }

    void testListArchive_data()
    {
        archiveFilenamesTestData();
    }

    void testListArchive()
    {
        QFETCH(QString, filename);

        LibArchiveArchive archive(filename);
        QVERIFY(archive.open(QIODevice::ReadOnly));

        QVector<ArchiveEntry> files = archive.list();
        QCOMPARE(files.count(), 1);
        QVERIFY(entriesMatch(files.first(), m_file));
    }

    void testCreateArchive_data()
    {
        archiveSuffixesTestData();
    }

    void testCreateArchive()
    {
        QFETCH(QString, suffix);

        const QString path1 = tempSourceFile("Source File 1.");
        const QString path2 = tempSourceFile("Source File 2.");

        const QString filename = generateTemporaryFileName() + suffix;
        LibArchiveArchive target(filename);
        QVERIFY(target.open(QIODevice::ReadWrite));
        QVERIFY(target.create(QStringList() << path1 << path2));
        QCOMPARE(target.list().count(), 2);
        target.close();
        QVERIFY(QFile(filename).remove());
    }

    void testCreateArchiveWithSpaces_data()
    {
        archiveSuffixesTestData();
    }

    void testCreateArchiveWithSpaces()
    {
        QFETCH(QString, suffix);

        const QString path1 = tempSourceFile(
            "Source File 1.",
            QDir::tempPath() + "/temp file with spaces.XXXXXX"
        );
        const QString path2 = tempSourceFile(
            "Source File 2.",
            QDir::tempPath() + "/temp file with spaces.XXXXXX"
        );

        const QString filename = QDir::tempPath() + "/target file with spaces" + suffix;
        LibArchiveArchive target(filename);
        target.setFilename(filename);
        QVERIFY(target.open(QIODevice::ReadWrite));
        QVERIFY(target.create(QStringList() << path1 << path2));
        QCOMPARE(target.list().count(), 2);
        target.close();
        QVERIFY(QFile(filename).remove());
    }

    void testExtractArchive_data()
    {
        archiveFilenamesTestData();
    }

    void testExtractArchive()
    {
        QFETCH(QString, filename);

        LibArchiveArchive source(filename);
        QVERIFY(source.open(QIODevice::ReadOnly));

        QVERIFY(source.extract(QDir::tempPath()));
        QCOMPARE(QFile::exists(QDir::tempPath() + QString("/valid")), true);
        QVERIFY(QFile(QDir::tempPath() + QString("/valid")).remove());
    }

private:
    void archiveFilenamesTestData()
    {
        QTest::addColumn<QString>("filename");
        QTest::newRow("ZIP archive") << ":///data/valid.zip";
        QTest::newRow("gzip compressed tar archive") << ":///data/valid.tar.gz";
        QTest::newRow("bzip2 compressed tar archive") << ":///data/valid.tar.bz2";
        QTest::newRow("xz compressed tar archive") << ":///data/valid.tar.xz";
    }

    void archiveSuffixesTestData()
    {
        QTest::addColumn<QString>("suffix");
        QTest::newRow("ZIP archive") << ".zip";
        QTest::newRow("gzip compressed tar archive") << ".tar.gz";
        QTest::newRow("bzip2 compressed tar archive") << ".tar.bz2";
        QTest::newRow("xz compressed tar archive") << ".tar.xz";
    }

    bool entriesMatch(const ArchiveEntry &lhs, const ArchiveEntry &rhs)
    {
        return lhs.path == rhs.path
            && lhs.utcTime == rhs.utcTime
            && lhs.isDirectory == rhs.isDirectory
            && lhs.compressedSize == rhs.compressedSize
            && lhs.uncompressedSize == rhs.uncompressedSize;
    }

    QString tempSourceFile(const QByteArray &data, const QString &templateName = QString())
    {
        QTemporaryFile source;
        if (!templateName.isEmpty()) {
            source.setFileTemplate(templateName);
        }
        source.open();
        source.write(data);
        source.setAutoRemove(false);
        return source.fileName();
    }

private:
    ArchiveEntry m_file;
};

QTEST_MAIN(tst_libarchivearchive)

#include "tst_libarchivearchive.moc"
