#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
      QMainWindow(parent),
      ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //connects fileSelectButton to its function
    //connects convertButton to its function
    connect(ui->fileSelect, &QPushButton::clicked, this, &MainWindow::selectFiles);
    connect(ui->convert   , &QPushButton::clicked, this, &MainWindow::convertSelectedFiles);

    //creates a new converter object and moves it to a seperate thread
    converter = new Convert();
    converter->moveToThread(&thread);


    //connects converter object to progress bar
    //connects converter finished event to clearing the file list
    //connects the request convert event to the converters conversion function
    connect(converter, SIGNAL(progressChanged(int)), ui->progressBar, SLOT(setValue(int)));
    connect (converter, SIGNAL(finished()), this, SLOT(clearFileList()));
    connect(this, SIGNAL(requestConvert(QStringList,QString)), converter, SLOT(handleConvert(QStringList,QString)));

    //connects converter object to gui dialog box for jpeg
    connect(converter, SIGNAL(openJpegOptionsMenu(std::vector<uint>, bool)), this, SLOT(getJpegOptions(std::vector<uint>, bool)));


    //afters connections are made, the thread is started and seperated
    thread.start();

    //set default values for screen
    ui->selected->setText("");
    ui->progressBar->setValue(0);

    //adds items to output select list
    for (int i = 0; i<Convert::acceptedOutputs.size(); i++) {
        QString val = Convert::acceptedOutputs.at(i);
        ui->outputType->addItem(val);
    }

    //create a settings object and handle default values
    QSettings settings("Ethan", "Image Converter");

    //set default vals if they dont exist
    if (settings.contains("gui/lastpath"))
        settings.setValue("gui/lastpath", "/");
}

MainWindow::~MainWindow()
{
    delete ui;
    delete converter;

    thread.quit();
    thread.wait(5);
}

void MainWindow::selectFiles() {
    //condense list of input types into a string
    QString types = "Images (";
    for (int i = 0; i<Convert::acceptedInputs.size(); i++) {
        QString val = Convert::acceptedInputs.at(i);
        types += "*." + val;
        if (i != Convert::acceptedInputs.size()-1) {
            types += " ";
        }
    }
    types += ")";

    //open file select window
    selectedFiles = QFileDialog::getOpenFileNames(
                            this,
                            "Select one or more images to open",
                            settings.value("gui/lastpath").toString(),
                            types);

    //display the chosen file names
    displayFileNames(selectedFiles);

    //update settings to the last selected folder
    if (selectedFiles.size() > 0) {
        QRegularExpression re("\\w+\\.\\w*$");
        QString f = selectedFiles[0];
        QString pth = f.replace(re, "");
        settings.setValue("gui/lastpath", pth);
        settings.sync();
    }

}

QString MainWindow::sanitizeFileName(QString file) {
    //uses regex to remove everything before the last "\"
    QRegularExpression re(".*\\/");
    file.replace(re, "");
    return file;
}

void MainWindow::displayFileNames(QStringList files) {
    //takes each selected filename, removes extra path and adds it to a list
    // only allows three files to be displayed before adding "and x more"
    QString out = "";
    int range = (files.size()<3)? files.size() : 3;
    for (int i = 0; i<range; i++) {
        out += sanitizeFileName(files.at(i));

        if (i != files.size()-1) {
            out += ", ";
        }
    }

    if (files.size() > 3) {
        out += "and " + QString::number(files.size() - 3) + " more";
    }


    ui->selected->setText(out);
}

void MainWindow::clearFileNames() {
    //removes the displayed filenames
    ui->selected->setText("");
}

void MainWindow::clearFileList() {
    //clears filenames and clears all selected files
    this->clearFileNames();
    this->selectedFiles = {};
}

void MainWindow::convertSelectedFiles() {
    //called when convert button is pressed
    //sets progress bar to 0 and requests a conversion from the converter

    ui->progressBar->setValue(0);

    QString outType = ui->outputType->itemText(ui->outputType->currentIndex());


    emit requestConvert(selectedFiles, outType);
}

void MainWindow::getJpegOptions(std::vector<uint> sizes, bool isChecked) {
    //create a new dialog box
    JpegOptions dialog(sizes, isChecked);

    //connect dialog output with converter input
    connect(&dialog, SIGNAL(userInput(std::vector<uint>)), converter, SLOT(saveJpegSettings(std::vector<uint>)));

    dialog.exec();

}

