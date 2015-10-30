#include "httpwindow.h"
#if QT_VERSION >= 0x050000
#  include <QtWidgets>
#else
#  include <QtGui>
#endif
#include <QtNetwork>
#include <QNetworkAccessManager>

QString HttpWindow::lastUrl;

HttpWindow::HttpWindow(QWidget *parent, bool reload)
    : QDialog(parent)
{
    setModal(true);
    progressDialog = new QProgressDialog(this);
    progressDialog->setMinimumDuration(1000); // msec

    urlLineEdit = new QLineEdit(lastUrl);

    urlLabel = new QLabel(tr("&URL:"));
    urlLabel->setBuddy(urlLineEdit);
    statusLabel = new QLabel(tr("Please enter the URL of a csa file"));

    quitButton = new QPushButton(tr("Cancel"));
    downloadButton = new QPushButton(tr("Open"));
    downloadButton->setDefault(true);

    http = new QNetworkAccessManager(this);
    connect(http, SIGNAL(finished(QNetworkReply*)),
                  this, SLOT(httpRequestFinished(QNetworkReply*)));
    connect(urlLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(enableDownloadButton()));
    connect(downloadButton, SIGNAL(clicked()), this, SLOT(downloadFile()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(reject()));

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(urlLabel);
    topLayout->addWidget(urlLineEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(downloadButton);
    buttonLayout->addWidget(quitButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    setWindowTitle(tr("HTTP"));
    const QClipboard *clipboard = QApplication::clipboard();
    const QString text = clipboard->text();
    if (!text.isEmpty())
    {
      urlLineEdit->setText(text);
    }
    urlLineEdit->setFocus();
    if (reload)
      downloadFile();
}

void HttpWindow::downloadFile()
{
    httpRequestAborted = false;
    const QUrl url(urlLineEdit->text());
    lastUrl = url.toString();
    QFileInfo fileInfo(url.path());
    fileName = fileInfo.fileName();

    file = new QFile(fileName);
    if (!file->open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("HTTP"),
                                 tr("Unable to save the file %1: %2.")
                                 .arg(fileName).arg(file->errorString()));
        delete file;
        file = 0;
        return;
    }

    progressDialog->setWindowTitle(tr("HTTP"));
    progressDialog->setLabelText(tr("Downloading %1.").arg(fileName));
    progressDialog->setWindowModality(Qt::WindowModal);
    downloadButton->setEnabled(false);

    QNetworkRequest request(url);
    reply = http->get(request);

    connect(reply, SIGNAL(downloadProgress (qint64, qint64)),
            this, SLOT(updateDataReadProgress(qint64, qint64)));
    connect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));
}

void HttpWindow::cancelDownload()
{
    statusLabel->setText(tr("Download canceled."));
    httpRequestAborted = true;
    reply->abort();
    downloadButton->setEnabled(true);
    reject();
}

void HttpWindow::httpRequestFinished(QNetworkReply *reply)
{
    if (httpRequestAborted || 
        reply->error() != QNetworkReply::NoError) {
        if (file) {
            file->close();
            file->remove();
            delete file;
            file = 0;
        }

        QMessageBox::information(this, tr("HTTP"),
                                 tr("Download failed: %1.")
                                 .arg(reply->error()));
        reject();
    } else {
      QTextStream out(file);
      out << reply->readAll();

      file->close();
      delete file;
      file = 0;

      QString fileName = QFileInfo(QUrl(urlLineEdit->text()).path()).fileName();
      statusLabel->setText(tr("Downloaded %1 to current directory.").arg(fileName));
      downloadButton->setEnabled(true);

      accept();
    }

    reply->deleteLater();
}

void HttpWindow::updateDataReadProgress(qint64 bytesRead, qint64 totalBytes)
{
    if (httpRequestAborted)
      return;

    progressDialog->setMaximum(totalBytes);
    progressDialog->setValue(bytesRead);
}

void HttpWindow::enableDownloadButton()
{
    downloadButton->setEnabled(!urlLineEdit->text().isEmpty());
}
