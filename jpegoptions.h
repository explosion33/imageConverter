#ifndef JPEGOPTIONS_H
#define JPEGOPTIONS_H

#include <QDialog>

namespace Ui {
class JpegOptions;
}

class JpegOptions : public QDialog
{
    Q_OBJECT

public:
     JpegOptions(std::vector<uint> sizes, bool isChecked = true, QWidget *parent = nullptr);
    ~JpegOptions();

private:
    Ui::JpegOptions *ui;
    std::vector<uint> sizes;

    /**
     * @brief getFormatedSize Formats the current size with (### B/KB/MB/GB/TB)
     * @return Fromatted QString
     */
    QString getFormatedSize();

    /**
     * @brief done Redeclares QDialog's done function to emit data first
     * @param r Code to finish dialog box with
     */
    void done(int r);

private slots:
    /**
     * @brief sliderChanged updates the rest of the widget on slider change
     */
    void sliderChanged();
    /**
     * @brief editBoxChanged updates the rest of the widget on edit box change
     * @param text The text from the box
     */
    void editBoxChanged(const QString &text);

signals:
    /**
     * @brief userInput signal to send the final user input through a thread
     */
    void userInput(std::vector<uint>);

};

#endif // JPEGOPTIONS_H
