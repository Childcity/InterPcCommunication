#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../MainStream/streamcommunicationtype.hpp"

#include <QCloseEvent>
#include <QMainWindow>
#include <QUrl>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    using StreamCommunicationType = MainStream::StreamCommunicationType;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void sigCloseEvent();
    void sigConnectionInfoChanged(const QString &);
    void sigConnectionTypeChanged(const StreamCommunicationType);
    void sigWsServerPortChanged(int port);
    void sigNewStreamData(const QByteArray &);

public slots:
    void slotNewStreamData(const QByteArray &);

protected:
    void closeEvent (QCloseEvent *event) override;


private slots:
    void on_mainStreamModeSwitcher_currentIndexChanged(int index);
    void on_sendTextEdit_textChanged();
    void on_connectionHost_textChanged(const QString &arg);
    void on_connectionPort_textChanged(const QString &arg);
    void on_wsServerPort_textChanged(const QString &arg);
    void on_pushButton_clicked();

private:
    void parseUrl();

private:
    Ui::MainWindow *ui;
    int sentOffset_ = 0;
    QByteArray testBuffer_;
};
#endif // MAINWINDOW_H
