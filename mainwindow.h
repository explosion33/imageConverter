#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "convert.h"

#include <QMainWindow>
#include <QFileDialog>
#include <QRegularExpression>
#include <QSettings>
#include <QtConcurrent/QtConcurrentRun>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Convert *converter;
    QSettings settings;
    QThread thread;


public slots:
    /**
     * @brief clearFileList clears selected files, and changes gui display
     */
    void clearFileList();

    /**
     * @brief getJpegOptions Slot to open a jpegOptions Pane
     * @param sizes 0-100 sizes per jpeg quality
     * @param isChecked default state of keepQuality checkbox
     */
    void getJpegOptions(std::vector<uint> sizes, bool isChecked);

signals:
    void requestConvert(QStringList files, QString outType);

private slots:
    /**
    * @brief selectFiles function to handle selectFiles button
    */
   void selectFiles();

   /**
    * @brief convertSelectedFiles function to handle convert button
    */
   void convertSelectedFiles();



private:
    Ui::MainWindow *ui;
    QStringList selectedFiles;

    /**
     * @brief displayFileNames displays the filenames selected by the user
     * @param files a list of filenames| D:/pth/image.png, test.png
     */
    void displayFileNames(QStringList files);

    /**
     * @brief clearFileNames clears displayed filenames
     */
    void clearFileNames();

    /**
     * @brief sanitizeFileName Removes the path from the filename
     * @param file Path to file| D:/pth/to/file.png
     * @return File name without path| file.png
     */
    QString sanitizeFileName(QString file);


};
#endif // MAINWINDOW_H
