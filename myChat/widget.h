#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTextCharFormat>

class QUdpSocket ;
class TcpServerDialog ;

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

    enum MessageType{
        Message, NewParticipant, ParticipanLeft, FileName, Refuse
    };

protected:
    void    newParticipant(QString username, QString localHostName, QString ipAddr);
    void    participantLeft(QString username, QString localhostName, QString time);
    void    sendMessage(MessageType type ,QString serverAddr = "");

    void    hasPendingFile(QString username, QString serverAddr, QString clientAddr, QString filename);

    void    saveFile(const QString &filename);
    QString     getIp();
    QString     getUsername();
    QString     getMessage();

private:
    Ui::Widget  *ui;
    QUdpSocket  *udpSocket;
    qint16      port;
    QColor      color;

    QString fileName;
    TcpServerDialog *server ;

    const   static QFont HINT_MESSAGE_FONT ;
    const   static QFont MESSAGE_FONT ;
private slots:
    void        processPendingDatagrams();
    void        on_sendButton_clicked();
    void        getFileName(QString filename);
    void        currentFormatChanged(const QTextCharFormat &format);
    void on_sendToolBtn_clicked();
    void on_fontComboBox_currentFontChanged(const QFont &f);
    void on_sizeComboBox_currentIndexChanged(int index);
    void on_sizeComboBox_currentIndexChanged(const QString &arg1);
    void on_boldToolBtn_clicked();
    void on_boldToolBtn_clicked(bool checked);
    void on_italicToolBtn_clicked(bool checked);
    void on_underlineToolBtn_clicked(bool checked);
    void on_colorToolBtn_clicked();
    void on_saveToolBtn_clicked();
};

#endif // WIDGET_H
