#ifndef TCPCLIENTDIALOG_H
#define TCPCLIENTDIALOG_H

#include <QDialog>
#include <QTcpSocket>
#include <QFile>
#include <QTime>
#include <QHostAddress>

namespace Ui {
class TcpClientDialog;
}

class TcpClientDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TcpClientDialog(QWidget *parent = 0);
    ~TcpClientDialog();

    void setHostAddress(QHostAddress address);
    void setFileName(QString fileName);

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void on_tcpClientCancelBtn_clicked();

    void on_tcpClientCloseBtn_clicked();

    void newConnect();
    void readMessage();
    void displayError(QAbstractSocket::SocketError socketError);

private:
    Ui::TcpClientDialog *ui;

    QTcpSocket *tcpClient;
    quint16 blockSize;
    QHostAddress hostAddress;
    qint16 tcpPort;

    qint64 TotalBytes;
    qint64 bytesReceived;
    qint64 bytesToReceive;
    qint64 fileNameSize;
    QString fileName;
    QFile *localFile;
    QByteArray inBlock;

    QTime time;

};

#endif // TCPCLIENTDIALOG_H
