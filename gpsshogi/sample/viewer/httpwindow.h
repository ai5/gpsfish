/****************************************************************************
**
** Copyright (C) 2004-2006 Trolltech AS. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef HTTPWINDOW_H
#define HTTPWINDOW_H

#include <QDialog>

class QFile;
class QLabel;
class QLineEdit;
class QProgressDialog;
class QPushButton;
class QNetworkAccessManager;
class QNetworkReply;

class HttpWindow : public QDialog
{
    Q_OBJECT

public:
    HttpWindow(QWidget *parent = 0, bool reload = false);
    const QString& getFilename() const { return fileName; }
    const QString& getLastUrl() const { return lastUrl; }
    static void setLastUrl(QString url) { lastUrl = url; }
private slots:
    void downloadFile();
    void cancelDownload();
    void httpRequestFinished(QNetworkReply *);
    void updateDataReadProgress(qint64 bytesRead, qint64 totalBytes);
    void enableDownloadButton();

private:
    QLabel *statusLabel;
    QLabel *urlLabel;
    QLineEdit *urlLineEdit;
    QProgressDialog *progressDialog;
    QPushButton *quitButton;
    QPushButton *downloadButton;

    QNetworkAccessManager *http;
    QNetworkReply *reply;
    QFile *file;
    bool httpRequestAborted;
    QString fileName;
    static QString lastUrl;
};

#endif
