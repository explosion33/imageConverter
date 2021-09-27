#ifndef CONVERT_H
#define CONVERT_H

#include <QObject>
#include <QEventLoop>
#include "jpegoptions.h"

#include <vector>
#include <fstream>
#include <string>
#include <map>

#include <stdio.h>
#include <turbojpeg.h>
#include <libheif/heif.h>
#include "lodepng/lodepng.h"

typedef unsigned int uint;

class Convert : public QObject {
    Q_OBJECT

public:
    static QStringList acceptedInputs;
    static QStringList acceptedOutputs;

    QStringList files;


    Convert();
    ~Convert();


    /**
     * @brief getFileExtension gets the file extension from a path
     * @param name the path to the file
     * @return string containing the extension| "png", "jpeg"
     */
    QString getFileExtension(QString path);




private:
    double progress;
    int lastEmittedValue;
    uint numFiles;

    std::vector<uint> jpegSettings;


    /**
     * @brief writePNG Writes interlaces rgb to a png
     * @param filename The path to save the file to
     * @param image Image data in the form of {r,g,b,...}
     * @param width Width of the image
     * @param height Height of the image
     */
    void writePNG(const char* filename, std::vector<unsigned char>& image, uint width, uint height);

    /**
     * @brief writeHeif Writes RGBA image data to heif format
     * @param filename The path to save the file to
     * @param image RGBA image data
     * @param w Width in pixels
     * @param h Height in pixels
     */
    void writeHeif(const char* filename, std::vector<unsigned char>& image, uint w, uint h);

    /**
     * @brief writeJpeg Writes RGBA data to a jpeg format
     * @param filename The path to save the file to
     * @param image RGBA image data
     * @param w Width
     * @param h Height
     * @param jpegQual Quality to save the jpeg in 0-100| 0=highest compression 100=highest quality
     */
    void writeJpeg(const char* filename, std::vector<unsigned char>& image, uint w, uint h, int jpegQual);

    /**
     * @brief parseHeifData Removes the buffer bits from heif data
     * @param data The data returned from readHeifData| {r,g,b, ... , buffer, buffer, ...}
     * @param w The width of the heif image
     * @param h The height of the heif image
     * @param the width of the image including buffer data
     * @return pure interlaced rgb| {r,g,b,...}
     */

    void writeIcon(const char* filename, std::vector<unsigned char>& image, uint w);


    std::vector<unsigned char> parseHeifData(const uint8_t* data, int w, int h, int stride);

    /**
     * @brief readHeifData Reads heif data from a file
     * @param pth The path of the heif image
     * @param Stride empty int variable to store the stride data
     * @param w Empty int variable to store the width data
     * @param h Empty int variable to store the height data
     * @return array of 24bit interlaced rgb data, with buffers
     */
    const uint8_t* readHeifData(const char* pth, int &stride, int &w, int &h);

    /**
     * @brief readPngData reads data from a png file and saves it as RGBA pixels
     * @param pth the path to the file
     * @param w Reference to variable for image width
     * @param h Regerence to variable for image Height
     * @return array of 24bit interlaced rgb data
     */
    std::vector<unsigned char> readPngData(const char* pth, int &w, int &h);

    /**
     * @brief readJpegData reads data from a jpeg file and saves it as RGBA pixels
     * @param pth the path to the file
     * @param w Reference to variable for image width
     * @param h Regerence to variable for image Height
     * @return array of 24bit interlaced rgb data
     */
    std::vector<unsigned char> readJpegData(const char *pth, int &w, int &h);

    /**
     * @brief updateProgressBar A wrapper function for the direct signal progressChanged(int), only emits a signal when the progess has increased by atleast 1
     * @param change The amount of progress changed
     */
    void updateProgressBar(double change);

    /**
     * @brief getJpegSizeEstimations compresses an image x times and interpolates between those points to estimate the size/quality
     * @param image RGBA image data
     * @param w Width of the image
     * @param h Height of the image
     * @param numSamples Number of actually readings to take
     * @return Vector with 100 sizes (in bytes) in ascending order
     */
    std::vector<uint> getJpegSizeEstimations(std::vector<unsigned char>& image, uint w, uint h, uint numSamples);

    template <typename T>
    std::vector<unsigned char> toBytes(T num);

    void squareImage(std::vector<unsigned char> &image, int &w, int &h);

    void resizeImage(std::vector<unsigned char> &image, uint w, uint h, uint nw, uint nh);


signals:
    /**
     * @brief progressChanged A signal to be used with a GUI object, signals a change in a progress bar
     * @param progress the new progress to emit
     */
    void progressChanged(int progress);

    /**
     * @brief finished A signal that is emitted when a work request has been completed
     */
    void finished();

    /**
     * @brief openJpegOptionsMenu requests a jpegOptionsMenu from the GUI thread
     * @param sizes Vector of sizes per quality
     * @param isChecked Default state of the keepQuality checkbox
     */
    void openJpegOptionsMenu(std::vector<uint> sizes, bool isChecked);

    /**
     * @brief stopWait halts a blocking loop in the thread
     */
    void stopWait();

public slots:
    /**
     * @brief handleConvert Handles the conversion of files
     * @param files A list of file paths to be converted
     * @param outType The output extension for the files
     */
    void handleConvert(QStringList files, QString outType);

    /**
     * @brief saveJpegSettings Save jpeg settings returned from the gui thread
     * @param settings The returned settings
     */
    void saveJpegSettings(std::vector<uint> settings);

};

#endif // CONVERT_H
