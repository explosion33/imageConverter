#include "jpegoptions.h"
#include "ui_jpegoptions.h"

#include <QIntValidator>

JpegOptions::JpegOptions(std::vector<uint> sizes, bool isChecked, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JpegOptions)
{
    this->sizes = sizes;

    //setup default values for UI
    ui->setupUi(this);
    ui->qualityEdit->setValidator( new QIntValidator(0, 100, this)); //accepts only numbers from 0-100
    ui->estimatedSize->setText(getFormatedSize()); //sets default estimated size
    ui->keepQuality->setChecked(isChecked); //sets checkbox to what the user initialized

    //event updates for slider bar and line edit box
    connect(ui->qualitySlider, &QSlider::valueChanged, this, &JpegOptions::sliderChanged);
    connect(ui->qualityEdit, &QLineEdit::textChanged, this, &JpegOptions::editBoxChanged);

}

JpegOptions::~JpegOptions() {
    delete ui;
}

QString JpegOptions::getFormatedSize() {
    //divide by 1000 until value is less than 1000 upping the unit eacch time
    double size = sizes[ui->qualitySlider->value()];
    QString units[5] = {"bytes", "KB", "MB", "GB", "TB"};
    uint i;
    for (i = 0; i<5; i++) {
        if (size < 1000) break;
        size /= 1000;
    }

    return QString::number(size, 'f', 2)+ " " + units[i];
}

void JpegOptions::sliderChanged() {
    //on silder change update number value and estimated size
    ui->qualityEdit->setText(QString::number(ui->qualitySlider->value()));

    ui->estimatedSize->setText(getFormatedSize());
}

void JpegOptions::editBoxChanged(const QString &text) {
    //update size and slider with new values
    int val = text.toUInt();

    ui->qualitySlider->setValue(val);
    ui->estimatedSize->setText(getFormatedSize());

}

void JpegOptions::done(int r) {
    //if the user accepted the dialog box, read and return object values
    //otherwise return default values
    std::vector<uint> out;
    if (r == QDialog::Accepted) {
        out.push_back(ui->qualitySlider->value());
        out.push_back((ui->keepQuality->isChecked()));
    }

    else
        out = {100, 1}; //quality:100 applyForAll: true

    //send data to thread
    emit userInput(out);

    //call parents done method
    QDialog::done(r);
}


