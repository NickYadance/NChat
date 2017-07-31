#include "tcpclientdialog.h"
#include "ui_tcpclientdialog.h"
#include <QMessageBox>

TcpClientDialog::TcpClientDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TcpClientDialog)
{
    ui->setupUi(this);

    setFixedSize(350, 180);
    TotalBytes = 0;
    bytesReceived = 0;
    fileNameSize = 0;
    tcpClient = new QTcpSocket(this);
    tcpPort = 6666;

    //连接tcp可读状态下响应函数
    //tcp可读->client读消息
    connect(tcpClient, SIGNAL(readyRead()), this, SLOT(readMessage()));

    //连接tcp出错响应函数
    //tcp->出错->client显示错误
    connect(tcpClient, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
}

TcpClientDialog::~TcpClientDialog()
{
    delete ui;
}

void TcpClientDialog::setHostAddress(QHostAddress address)
{
    hostAddress = address ;
    newConnect();
}

void TcpClientDialog::setFileName(QString fileName)
{
    localFile = new QFile(fileName);
}

void TcpClientDialog::closeEvent(QCloseEvent *)
{
    on_tcpClientCloseBtn_clicked();
}

void TcpClientDialog::on_tcpClientCancelBtn_clicked()
{
    tcpClient->abort();
    if (localFile->isOpen())
    {
        localFile->close();
    }
}

void TcpClientDialog::on_tcpClientCloseBtn_clicked()
{
    tcpClient->abort();
    if (localFile->isOpen())
    {
        localFile->close();
    }
    close();
}

void TcpClientDialog::newConnect()
{
    blockSize= 0;
    tcpClient->abort();
    tcpClient->connectToHost(hostAddress, tcpPort);
    time.start();
}

void TcpClientDialog::readMessage()
{
    QDataStream in(tcpClient);
    in.setVersion(QDataStream::Qt_5_8);
    float useTime = time.elapsed();

    if (bytesReceived <= sizeof(qint64)*2)
    {
        if ( (tcpClient->bytesAvailable() >= sizeof(qint64)*2) && (fileNameSize == 0))
        {
            in >> TotalBytes >> fileNameSize;
            bytesReceived += sizeof(qint64)*2;
        }
        if ( (tcpClient->bytesAvailable() >= fileNameSize) && (fileNameSize != 0))
        {
            in >> fileName ;
            bytesReceived += fileNameSize ;
            if (!localFile->open(QFile::WriteOnly))
            {
                QMessageBox::warning(this, "应用程序", QString("无法读取文件%1:\n%2").arg(fileName).arg(localFile->errorString()));
                return ;
            }
        }
        else
        {
            return ;
        }
    }

    if (bytesReceived < TotalBytes)
    {
        bytesReceived += tcpClient->bytesAvailable();
        inBlock = tcpClient->readAll();
        localFile->write(inBlock);
        inBlock.resize(0);
    }

    ui->progressBar->setMaximum(TotalBytes);
    ui->progressBar->setValue(bytesReceived);

    double speed = bytesReceived / useTime;
    ui->tcpClientStatusLabel->setText(QString("已接收%1MB (%2MB/s) "
                                         "\n共%3MB 已用时ʱ:%4s\n估计剩余时间%5s")
                                      .arg(bytesReceived / (1024*1024))
                                      .arg(speed*1000/(1024*1024),0,'f',2)
                                      .arg(TotalBytes / (1024 * 1024))
                                      .arg(useTime/1000,0,'f',0)
                                      .arg(TotalBytes/speed/1000 - useTime/1000,0,'f',0));

    if (bytesReceived == TotalBytes) {
        localFile->close();
        tcpClient->close();
        ui->tcpClientStatusLabel->setText(QString("接收文件%1完毕").arg(fileName));
    }
}

void TcpClientDialog::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:

        break;
    default:
        qDebug() << tcpClient->errorString();
        break;
    }
}
