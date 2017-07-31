#include "tcpserverdialog.h"
#include "ui_tcpserverdialog.h"
#include <QMessageBox>
#include <QFileDialog>

TcpServerDialog::TcpServerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TcpServerDialog)
{
    ui->setupUi(this);
    setFixedSize(350, 180);
    tcpPort = 6666 ;
    tcpServer = new QTcpServer(this);

    //连接server有新连接响应函数
    //server有新连接->server发送消息
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(sendMessage()));

    initServer();
}

TcpServerDialog::~TcpServerDialog()
{
    delete ui;
}

void TcpServerDialog::initServer()
{
    payloadSize = 64 * 1024 ;
    TotalBytes = 0;
    bytesWritten = 0;
    bytesToWrite = 0;

    ui->serverStatusLabel->setText("请选择要传送的文件");
    ui->progressBar->reset();
    ui->serverOpenBtn->setEnabled(true);
    ui->serverSendBtn->setEnabled(false);
    tcpServer->close();     //?
}

void TcpServerDialog::refused()
{
    tcpServer->close();
    ui->serverStatusLabel->setText("对方拒绝接收");
}

void TcpServerDialog::closeEvent(QCloseEvent *)
{
    on_serverCloseBtn_clicked();
}

void TcpServerDialog::sendMessage()
{
    ui->serverSendBtn->setEnabled(false);
    clientConnection = tcpServer->nextPendingConnection();

    //连接tcp socket有新数据响应函数
    //tcp socket有新数据->server 更新对应客户端
    connect(clientConnection, SIGNAL(bytesWritten(qint64)), this, SLOT(updateClientProgress(qint64)));

    ui->serverStatusLabel->setText(QString("开始传送文件 %1").arg(theFileName));

    localFile = new QFile(fileName);
    if (!localFile->open(QFile::ReadOnly))
    {
        QMessageBox::warning(this, "应用程序", QString("无法读取文件 %1:\n %2").arg(fileName).arg(localFile->errorString()));
        return ;
    }
    TotalBytes = localFile->size();
    QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_5_8);
    time.start();

    QString currentFile = fileName.right(fileName.size() - fileName.lastIndexOf('/')-1);
    sendOut << qint64(0) << qint64(0) << currentFile;
    TotalBytes += outBlock.size();
    sendOut.device()->seek(0);
    sendOut << TotalBytes << qint64(outBlock.size() - sizeof(qint64)*2);
    bytesToWrite = TotalBytes - clientConnection->write(outBlock);
    outBlock.resize(0);
}

void TcpServerDialog::updateClientProgress(qint64 numBytes)
{
    QApplication::instance()->processEvents();
    bytesWritten += (int)numBytes ;
    if (bytesToWrite > 0)
    {
        outBlock = localFile->read(qMin(bytesToWrite, payloadSize));
        bytesToWrite -= (int)clientConnection->write(outBlock);
        outBlock.resize(0);
    }
    else
    {
        localFile->close();
    }

    ui->progressBar->setMaximum(TotalBytes);
    ui->progressBar->setValue(bytesWritten);

    float useTime = time.elapsed();
    double speed = bytesWritten / useTime;
    ui->serverStatusLabel->setText(QString("已发送：%1 MB (%2MB/s)"
                                           "\n共%3MB 已用时：%4s"
                                           "\n估计剩余时间：%5s")
                                   .arg(bytesWritten/(1024*1024))
                                   .arg(double(speed * 1000/(1024*1024)), 0 , 'f', 2)
                                   .arg(TotalBytes/(1024*1024))
                                   .arg(useTime/1000, 0, 'f', 0)
                                   .arg(TotalBytes/speed/1000 - useTime/1000,0,'f',0));

    if (bytesWritten == TotalBytes) {
        localFile->close();
        tcpServer->close();
        ui->serverStatusLabel->setText(QString("传送文件%1成功").arg(theFileName));
    }
}

void TcpServerDialog::on_serverOpenBtn_clicked()
{
    fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
    {
        theFileName = fileName.right(fileName.size() - fileName.lastIndexOf('/') - 1);
        ui->serverStatusLabel->setText(QString("要发送的文件为\"%1\"").arg(theFileName));
        ui->serverSendBtn->setEnabled(true);
        ui->serverOpenBtn->setEnabled(false);
    }
}

void TcpServerDialog::on_serverSendBtn_clicked()
{
    if (!tcpServer->listen(QHostAddress::Any, tcpPort))
    {
        qDebug() << tcpServer->errorString();
        close();
        return ;
    }
    ui->serverStatusLabel->setText("等待对方接收");
    emit sendFileName(theFileName);
}

void TcpServerDialog::on_serverCloseBtn_clicked()
{
    if (tcpServer->isListening())
    {
        tcpServer->close();
        if (localFile->isOpen())
        {
            localFile->close();
            clientConnection->abort();
        }
    }
    close();
}
