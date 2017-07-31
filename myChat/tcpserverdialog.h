#ifndef TCPSERVERDIALOG_H
#define TCPSERVERDIALOG_H

#include <QObject>
#include <QDialog>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTime>
#include <QFile>

namespace Ui {
class TcpServerDialog;
}

class TcpServerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TcpServerDialog(QWidget *parent = 0);
    ~TcpServerDialog();

    void initServer();
    void refused();

protected:
    void    closeEvent(QCloseEvent *);
private slots:
    void sendMessage();
    void updateClientProgress(qint64 numBytes);

private slots:
    void on_serverOpenBtn_clicked();

    void on_serverSendBtn_clicked();

    void on_serverCloseBtn_clicked();

private:
    Ui::TcpServerDialog *ui;

    qint16      tcpPort;
    QTcpServer  *tcpServer;
    QString     fileName;
    QString     theFileName;
    QFile       *localFile;

    qint64      TotalBytes;
    qint64      bytesWritten;
    qint64      bytesToWrite;
    qint64      payloadSize;
    QByteArray  outBlock;

    QTcpSocket *clientConnection;

    QTime       time;

signals:
    void sendFileName(QString fileName);
};

#endif // SENDFILEDIALOG_H
