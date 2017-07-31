#include "widget.h"
#include "ui_widget.h"
#include "tcpclientdialog.h"
#include "tcpserverdialog.h"
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QHostInfo>
#include <QtNetwork/QNetworkInterface>
#include <QMessageBox>
#include <QScrollBar>
#include <QDateTime>
#include <QProcess>
#include <QDataStream>
#include <QFileDialog>
#include <QColorDialog>

const QFont Widget::HINT_MESSAGE_FONT = QFont("Times New Roman", 10);
const QFont Widget::MESSAGE_FONT = QFont("Times New Roman", 12);

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle("NChat");

    udpSocket = new QUdpSocket(this);
    port = 45454;
    udpSocket->bind(port, QUdpSocket::ShareAddress|QUdpSocket::ReuseAddressHint);

    //连接updsocket在准备状态下的响应
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));

    sendMessage(NewParticipant);

    server = new TcpServerDialog(this);

    //连接server及本客户端
    //服务端准备发送文件-》本地获取文件名
    connect(server, SIGNAL(sendFileName(QString)), this, SLOT(getFileName(QString)));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::newParticipant(QString username, QString localHostName, QString ipAddr)
{
    bool isEmpty = ui->userTableWidget->findItems(localHostName, Qt::MatchExactly).isEmpty();
    if (isEmpty)
    {
        QTableWidgetItem *user = new QTableWidgetItem(username);
        QTableWidgetItem *host = new QTableWidgetItem(localHostName);
        QTableWidgetItem *ip= new QTableWidgetItem(ipAddr);

        ui->userTableWidget->insertRow(0);
        ui->userTableWidget->setItem(0, 0, user);
        ui->userTableWidget->setItem(0, 1, host);
        ui->userTableWidget->setItem(0, 2, ip);

        ui->messageBrowser->setTextColor(Qt::gray);
        ui->messageBrowser->setCurrentFont(HINT_MESSAGE_FONT);
        ui->messageBrowser->append(QString(" %1 在线").arg(username));
        ui->userNumLabel->setText(QString("在线人数: %1 ").arg(ui->userTableWidget->rowCount()));
        sendMessage(NewParticipant);
    }
}

void Widget::participantLeft(QString username, QString localhostName, QString time)
{
    int rowNum = ui->userTableWidget->findItems(localhostName, Qt::MatchExactly).first()->row();
ui->userTableWidget->removeRow(rowNum);
    ui->messageBrowser->setTextColor(Qt::gray);
    ui->messageBrowser->setCurrentFont(HINT_MESSAGE_FONT);
    ui->messageBrowser->append(QString(" %1 于 %2 离开！").arg(username).arg(time));
    ui->userNumLabel->setText(QString("在线人数: %1").arg(ui->userTableWidget->rowCount()));
}

void Widget::sendMessage(Widget::MessageType type, QString serverAddr)
{
     QByteArray     data ;
     QDataStream    out(&data, QIODevice::WriteOnly);
     QString        username = getUsername();
     QString        localHostName = QHostInfo::localHostName();
     QString        address = getIp();
     QString        clientAddr;
     int            row ;
     out << type << username << localHostName ;

     switch (type) {
     case Message:
         if (ui->messageTextEdit->toPlainText().isEmpty())
         {
             QMessageBox::warning(nullptr, QString("warning"), QString("发送内容不能为空"), QMessageBox::Ok);
             return ;
         }
         out << address << getMessage();
         ui->messageBrowser->verticalScrollBar()->setValue(ui->messageBrowser->verticalScrollBar()->maximum());
         break;
     case NewParticipant:
         out << address ;
         break;
     case ParticipanLeft:
         break;
     case FileName:
         row = ui->userTableWidget->currentRow();
         clientAddr = ui->userTableWidget->item(row, 2)->text();
         out << address << clientAddr << fileName;
         hasPendingFile(username, address, localHostName, fileName);
         break;
     case Refuse:
         out << serverAddr ;
         break;
     default:
         break;
     }
     udpSocket->writeDatagram(data, data.length(), QHostAddress::Broadcast, port);
}

void Widget::hasPendingFile(QString username, QString serverAddr, QString clientAddr, QString filename)
{
    QString ipaddr = getIp();
    if (ipaddr == clientAddr)
    {
        int btn = QMessageBox::information(this, "接收文件", QString("来自%1(%2)的文件：%3,是否接收？")
                                           .arg(username).arg(serverAddr).arg(filename),QMessageBox::Yes, QMessageBox::No);

        if (btn == QMessageBox::Yes)
        {
            QString name = QFileDialog::getSaveFileName(0, "保存文件", fileName);
            if (!name.isEmpty())
            {
                TcpClientDialog *client = new TcpClientDialog(this);
                client->setFileName(name);
                client->setHostAddress(QHostAddress(serverAddr));
                client->show();
            }
            else
            {
                sendMessage(Refuse, serverAddr);
            }
        }
    }
}

void Widget::saveFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, "无法保存", QString("无法保存改文件%1\n%2").arg(filename).arg(file.errorString()));
        return ;
    }
    QTextStream out (&file);
    out << ui->messageBrowser->toPlainText();
}

QString Widget::getIp()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach (QHostAddress address, list) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol)
            return address.toString();
    }
    return 0;
}

QString Widget::getUsername()
{
//    QStringList envVariables ;
//    envVariables << "USERNAME.*" << "USER.*" << "USERDOMAIN.*" << "HOSTNAME.*" << "DOMAINNAME.*";
//    qDebug() << qgetenv("USER");
//    qDebug() << qgetenv("USERNAME");
//    QStringList env = QProcess::systemEnvironment();
//    foreach (QString string, envVariables) {
//        int index = env.indexOf(QRegExp(string));
//        if (index != -1)
//        {
//            QStringList stringList = env.at(index).split('=');
//            if (stringList.size() == 2)
//            {
//                return stringList.at(1);
//                break ;
//            }
//        }
//    }
//    return "unknown";
    return qgetenv("USERNAME");
}

QString Widget::getMessage()
{
    QString msg = ui->messageTextEdit->toHtml();
    ui->messageTextEdit->clear();
    ui->messageTextEdit->setFocus();
    return msg ;
}

void Widget::processPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(),datagram.size());
        QDataStream in(&datagram, QIODevice::ReadOnly);

        int messageType;
        in >> messageType;

        QString username, localHostName, ipAddr, message ;
        QString timeString = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        QString clientAddr , fileName;
        QString serverAddr ;
        QString ipaddr ;

        switch (messageType) {
        case Message:
            in >> username >> localHostName >> ipAddr >> message ;
            ui->messageBrowser->setTextColor(Qt::blue);
            ui->messageBrowser->setCurrentFont(MESSAGE_FONT);
            ui->messageBrowser->append("[" + username + "]" + timeString);
            ui->messageBrowser->append(message);
            break;

        case NewParticipant:
            in >> username >> localHostName >> ipAddr ;
            newParticipant(username, localHostName, ipAddr);
            break;

        case ParticipanLeft:
            in >> username >> localHostName ;
            participantLeft(username, localHostName, timeString);
            break;

        case FileName:
            in >> username >> localHostName >> ipAddr;
            in >> clientAddr >> fileName;
            hasPendingFile(username, ipAddr, clientAddr, fileName);
            break;
        case Refuse:
            in >> username >> localHostName;
            in >> serverAddr;
            ipaddr = getIp();
            if (ipaddr == serverAddr)
            {
                server->refused();
            }
            break;
        default:
            break;
        }
    }
}

void Widget::on_sendButton_clicked()
{
    sendMessage(Message);
}

void Widget::getFileName(QString filename)
{
    fileName = filename ;
    sendMessage(FileName);
}

void Widget::currentFormatChanged(const QTextCharFormat &format)
{
    ui->fontComboBox->setCurrentFont(format.font());
if (format.fontPointSize()
         < 9)
{
    ui->sizeComboBox->setCurrentIndex(3);
}
else
{
    ui->sizeComboBox->setCurrentIndex(ui->sizeComboBox->findText(QString::number(format.fontPointSize())));
}
ui->boldToolBtn->setChecked(format.font().bold());
ui->italicToolBtn->setChecked(format.font().italic());
ui->underlineToolBtn->setChecked(format.font().underline());
color = format.foreground().color();
}

void Widget::on_sendToolBtn_clicked()
{
    if (ui->userTableWidget->selectedItems().isEmpty())
    {
        QMessageBox::warning(0, "选择用户", "请先从用户列表中选择要传送的用户");
        return ;
    }
    server->show();
    server->initServer();
}

void Widget::on_fontComboBox_currentFontChanged(const QFont &f)
{
    ui->messageTextEdit->setFont(f);
    ui->messageTextEdit->setFocus();
}

void Widget::on_sizeComboBox_currentIndexChanged(int index)
{
}

void Widget::on_sizeComboBox_currentIndexChanged(const QString &arg1)
{
    ui->messageTextEdit->setFontPointSize(arg1.toDouble());
    ui->messageTextEdit->setFocus();
}

void Widget::on_boldToolBtn_clicked()
{
}

void Widget::on_boldToolBtn_clicked(bool checked)
{
    if (checked)
    {
        ui->messageTextEdit->setFontWeight(QFont::Bold);
    }
    else
    {
        ui->messageTextEdit->setFontWeight(QFont::Normal);
    }
    ui->messageTextEdit->setFocus();

}

void Widget::on_italicToolBtn_clicked(bool checked)
{
    ui->messageTextEdit->setFontItalic(checked);
    ui->messageTextEdit->setFocus();
}

void Widget::on_underlineToolBtn_clicked(bool checked)
{
    ui->messageTextEdit->setFontUnderline(checked);
    ui->messageTextEdit->setFocus();
}

void Widget::on_colorToolBtn_clicked()
{
    color = QColorDialog::getColor(color, this);
    if (color.isValid())
    {
        ui->messageTextEdit->setTextColor(color);
        ui->messageTextEdit->setFocus();
    }
}

void Widget::on_saveToolBtn_clicked()
{
    if (ui->messageBrowser->document()->isEmpty())
    {
        QMessageBox::warning(this, "无记录保存", "聊天记录为空！", QMessageBox::Ok);
        return ;
    }
    QString filename = QFileDialog::getSaveFileName(this, "保存聊天记录", "聊天记录", "文本(*.txt);;ALL File(*.*)");
    if (!filename.isEmpty())    saveFile(filename);
}
