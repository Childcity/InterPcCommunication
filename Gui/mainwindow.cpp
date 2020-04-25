#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->mainStreamModeSwitcher->addItems({"TcpServer", "TcpClient"});
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    emit sigCloseEvent();
    this->hide();
    event->accept();
}

void MainWindow::on_connectionHost_textChanged(const QString &)
{
    parseUrl();
}

void MainWindow::on_connectionPort_textChanged(const QString &)
{
    parseUrl();
}

void MainWindow::on_wsServerPort_textChanged(const QString &arg)
{
    emit sigWsServerPortChanged(arg.toInt());
}

void MainWindow::parseUrl()
{
    QString urlStr = ui->connectionHost->text() + ":" + ui->connectionPort->text();
    QUrl url = QUrl("tcp://" + urlStr);

    if (! url.isValid()) {
        qWarning() << "Invalid url: " << urlStr;
        return;
    }

    emit sigConnectionInfoChanged(url.toString());
}

void MainWindow::on_mainStreamModeSwitcher_currentIndexChanged(int index)
{
    MainStream::StreamCommunicationType type;

    switch (index) {
        case StreamCommunicationType::TcpClient:
            type = StreamCommunicationType::TcpClient;
            ui->connectionHost->setEnabled(true);
            break;
        case StreamCommunicationType::TcpServer:
        default:
            type = StreamCommunicationType::TcpServer;
            ui->connectionHost->setEnabled(false);

    }

    emit sigConnectionTypeChanged(type);
}

void MainWindow::on_sendTextEdit_textChanged()
{
    QByteArray allText = ui->sendTextEdit->toPlainText().toUtf8();
    emit sigNewStreamData(allText.mid(sentOffset_).replace("\n", "\r\n"));
    sentOffset_ = allText.size();
}

void MainWindow::slotNewStreamData(const QByteArray &data)
{
    ui->receiveTextEdit->insertPlainText(QString::fromUtf8(data));
}

void MainWindow::on_pushButton_clicked()
{
    std::uint64_t bytesCount = ui->mbSpinBox->text().toUInt() * 1048576;
    testBuffer_.fill('F', bytesCount);
    testBuffer_.front() = 'S'; // This will begin speed test
    testBuffer_.back() = 'E';  // This will end speed test
    emit sigNewStreamData(testBuffer_);
}
