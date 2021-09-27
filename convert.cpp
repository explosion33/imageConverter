#include "convert.h"

#include <QDebug>


QStringList Convert::acceptedInputs =  {"png", "heif", "heic", "jpg", "jpeg"};
QStringList Convert::acceptedOutputs = {"png", "heif", "heic", "jpg", "jpeg", "ico"};

enum inFileType {
    IN_PNG = 0,
    IN_HEIF = 1,
    IN_HEIC = 2,
    IN_JPG = 3,
    IN_JPEG = 4,
};

enum outFileType {
    OUT_PNG = 0,
    OUT_HEIF = 1,
    OUT_HEIC = 2,
    OUT_JPG = 3,
    OUT_JPEG = 4,
    OUT_ICO = 5,
};

Convert::Convert() {
}

Convert::~Convert() {

}

void Convert::handleConvert(QStringList files, QString outExt) {
    progress = 0;
    lastEmittedValue = 0;
    numFiles = files.size();

    uint filenum = 0;
    for (QString file : files) {
        //create variables to hold the output path input path, and extensions
        auto t = file.toLocal8Bit();
        const char* pth = t.data();
        QString ext = getFileExtension(file);

        t = file.replace(ext, outExt).toLocal8Bit();
        const char* outPth = t.data();


        qDebug() << file << ">>" << outPth;


        //decode image to get rgb data, w, h
        std::vector<unsigned char> rgb;
        int w = 0;
        int h = 0;
        switch (Convert::acceptedInputs.indexOf(ext)) { //switch on accepted inputs list, index
            case IN_HEIF:
            case IN_HEIC: {
                int stride;
                const uint8_t* data = this->readHeifData(pth, stride, w, h);
                qDebug() << "read heif data";
                rgb = this->parseHeifData(data, w, h, stride);
                qDebug() << "parsed heif data";
                break;
            }
            case IN_PNG: {
                rgb = this->readPngData(pth,w,h);
                break;
            }
            case IN_JPEG:
            case IN_JPG: {
                rgb = this->readJpegData(pth, w, h);
                break;
            }
        }


        //encode data into a file
        switch (Convert::acceptedOutputs.indexOf(outExt)) { //switch on accepted outputs list, index
            case OUT_PNG: {
                this->writePNG(outPth, rgb, w, h);
                break;
            }
            case OUT_HEIF:
            case OUT_HEIC: {
                this->writeHeif(outPth, rgb, w, h);
                qDebug() << "wrote heif";
                break;
            }
            case OUT_JPG:
            case OUT_JPEG: {
                //open dialog box and start event loops
                //event loops are like while(true) except they can interact with signals
                if (filenum == 0) { //if its the first file
                    emit openJpegOptionsMenu(getJpegSizeEstimations(rgb,w,h,5), true);

                    QEventLoop loop;
                    connect(this, SIGNAL(stopWait()), &loop, SLOT(quit()));
                    loop.exec();
                }
                else if (!jpegSettings[1]) { //if the user selected to have them apear
                    emit openJpegOptionsMenu(getJpegSizeEstimations(rgb,w,h,5), false);

                    QEventLoop loop;
                    connect(this, SIGNAL(stopWait()), &loop, SLOT(quit())); //wait for stopWait signal before continuing
                    loop.exec();
                }

                this->writeJpeg(outPth, rgb, w, h, jpegSettings[0]);
                break;
            }
            case OUT_ICO: {
                if (w != h) squareImage(rgb,w,h);
                if (w >= 256) {
                    resizeImage(rgb, w, h , 256, 256);
                    w = 256;
                }
                else if (w >= 128) {
                    resizeImage(rgb, w, h , 128, 128);
                    w = 128;
                }
                else if (w >= 64) {
                    resizeImage(rgb, w, h , 64, 64);
                    w = 64;
                }
                else if (w >= 32) {
                    resizeImage(rgb, w, h , 32, 32);
                    w = 32;
                }
                else if (w >= 16) {
                    resizeImage(rgb, w, h , 16, 16);
                    w = 16;
                }

                this->writeIcon(outPth, rgb, w);
            }
        }

        filenum ++;

     }

    //update screen to show the process has finished
    emit finished();
    emit progressChanged(100);
}

//WRITE TO FILE

void Convert::writePNG(const char* filename, std::vector<unsigned char>& image, uint width, uint height) {
    //Encode the image

    uint err = lodepng::encode(filename, image, width, height);

    if (err != 0) {
        qDebug() << "error " << err << ": " << lodepng_error_text(err);
    }
    updateProgressBar(45);
}

void Convert::writeHeif(const char *filename, std::vector<unsigned char> &image, uint w, uint h) {
    heif_context* ctx = heif_context_alloc();

    // get the default encoder
    heif_encoder* encoder;
    heif_context_get_encoder_for_format(ctx, heif_compression_HEVC, &encoder);

    // set the encoder parameters
    heif_encoder_set_lossy_quality(encoder, 100);

    // create the image
    heif_image* heif_image = nullptr;

    heif_image_create(w, h, heif_colorspace_RGB,heif_chroma_interleaved_RGB, &heif_image);

    heif_image_add_plane(heif_image, heif_channel_interleaved, w, h, 8);

    int stride;
    uint8_t* data = heif_image_get_plane(heif_image, heif_channel_interleaved, &stride);

    qDebug() << "copying data";
    //copy data
    uint count = 0;
    uint count2 = 0;
    for (uint y = 0; y < h; y++) {
        for (uint x = 0; x < w; x++) {
            data[count] = image[count2];
            data[count + 1] = image[count2 + 1];
            data[count + 2] = image[count2 + 2];
            //data[count + 3] = image[count2 + 3];

            count += 3;
            count2 += 4;

            updateProgressBar(45.0/(w*h));

        }
        qDebug() << stride << ", " << w << count;
        count += stride - (w * 3);

    }

    qDebug() << "encoding image";

    //encode the image
    heif_context_encode_image(ctx, heif_image, encoder, nullptr, nullptr);

    qDebug() << "encoded";

    heif_encoder_release(encoder);

    qDebug() << "writing to file";

    heif_error err = heif_context_write_to_file(ctx, filename);

    qDebug() << "done";

    if (err.code != 0)
        qDebug() << "error " << err.code << ": " << err.message;
}

void Convert::writeJpeg(const char *filename, std::vector<unsigned char> &image, uint w, uint h, int jpegQual = 100) {
       unsigned char *srcBuf; //!< flattened array to store rgb data
       tjhandle handle = tjInitCompress();

       const int nbands = 4;
       const int flags = 0;
       const int pixelFormat = TJPF_RGBA;
       const int jpegSubsamp = TJSAMP_411;

       unsigned char* jpegBuf = NULL;
       unsigned long  jpegSize = 0;

       srcBuf = image.data();

       updateProgressBar(10);

       tjCompress2( handle, srcBuf, w, w*nbands, h, pixelFormat, &(jpegBuf), &jpegSize, jpegSubsamp, jpegQual, flags);

       updateProgressBar(25);


       FILE *file = fopen(filename, "wb");
       if (!file) {
           qDebug() << "Could not open JPEG file: " << strerror(errno);
       }
       if (fwrite(jpegBuf, jpegSize, 1, file) < 1) {
           qDebug() << "Could not write JPEG file: " << strerror(errno);
       }
       fclose(file);

       tjDestroy(handle);
       handle = 0;

       updateProgressBar(20);
}

void Convert::writeIcon(const char *filename, std::vector<unsigned char> &image, uint w) {
        if (!(w == 16 || w == 32 || w == 64 || w == 128 || w == 256)) return;

        unsigned char cursor = 32;

        //http://www.daubnet.com/en/file-format-ico
        std::vector<unsigned char> buffer;


        buffer.insert(end(buffer), {0,0,1,0});
        buffer.insert(end(buffer), {1,0}); //number of icons
        buffer.insert(end(buffer), {cursor,cursor,0,0,1,0,32,0});

        std::vector<unsigned char> size = toBytes(image.size() + 40);
        buffer.insert(buffer.end(), size.begin(), size.end()); //size of infoHeader + AND + XOR (bytes) = size of image + 40
        buffer.insert(end(buffer), {22,0,0,0}); //offset

        //INFO HEADER
        buffer.insert(end(buffer), {40,0,0,0});


        std::vector<unsigned char> width = toBytes(w);
        buffer.insert(buffer.end(), width.begin(), width.end()); //width

        std::vector<unsigned char> height = toBytes(w*2);
        buffer.insert(buffer.end(), height.begin(), height.end()); //height - double the width


        buffer.insert(end(buffer), {1,0,32,0});
        buffer.insert(end(buffer), {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0});

        //pixel data
        uint count = image.size() - (w*4);
        for (uint y = 0; y<w; y++) {
            for (uint x = 0; x<w; x++) {
                buffer.push_back(image[count+2]);   //blue
                buffer.push_back(image[count+1]);   //green
                buffer.push_back(image[count  ]);   //red
                buffer.push_back(image[count+3]);   //alpha
                count += 4;

                updateProgressBar(40.0/(w*w));
            }
            count -= (w*4)*2;
        }

        std::ofstream outfile(filename, std::ios::out | std::ios::binary);
        outfile.write((char *)&buffer[0], buffer.size());
        outfile.close();

        updateProgressBar(10);
}

//READ DATA

const uint8_t* Convert::readHeifData(const char* pth, int &stride, int &w, int &h) {
    heif_context* ctx = heif_context_alloc();
    heif_context_read_from_file(ctx, pth, nullptr);

    heif_image_handle* handle;
    heif_context_get_primary_image_handle(ctx, &handle);

    heif_image* img;
    heif_decode_image(handle, &img, heif_colorspace_RGB, heif_chroma_interleaved_RGB, nullptr);


    updateProgressBar(5);

    const uint8_t* data = heif_image_get_plane_readonly(img, heif_channel_interleaved, &stride);

    h = heif_image_get_height(img, heif_channel_interleaved);
    w = heif_image_handle_get_width(handle);

    updateProgressBar(5);




    return data;
}

std::vector<unsigned char> Convert::readPngData(const char *pth, int &w, int &h) {
    std::vector<unsigned char> image;
    unsigned width, height;

    unsigned error = lodepng::decode(image, width, height, pth);
    w = width;
    h = height;

    updateProgressBar(45);


    if(error) qDebug() << "decoder error " << error << ": " << lodepng_error_text(error);

    return image;
}

std::vector<unsigned char> Convert::readJpegData(const char *pth, int &w, int &h) {
    long unsigned int jpegSize; //!< size of the compressed file
    unsigned char* compressedImage; //!< compressed image buffer
    int jpegSubsamp; //!< will contain subsamp data
    unsigned char* buffer; //!< will contain RGBA uncompressed data

    FILE *file = fopen(pth, "rb");

    // get file size
    fseek (file , 0 , SEEK_END);
    jpegSize = ftell (file);
    rewind (file);

    // allocate memory and stire it into buffer
    compressedImage = new unsigned char[jpegSize];
    fread(compressedImage,1,jpegSize,file);


    tjhandle decompressor = tjInitDecompress();

    //get w h and subsamp data
    tjDecompressHeader2(decompressor, compressedImage, jpegSize, &w, &h, &jpegSubsamp);
    buffer = new unsigned char[w*h*4];

    //decompress image into RGBA format
    tjDecompress2(decompressor, compressedImage, jpegSize, buffer, w, 0/*pitch*/, h, TJPF_RGBA, TJFLAG_FASTDCT);

    //clean up
    tjDestroy(decompressor);

    updateProgressBar(35);

    //format output
    std::vector<unsigned char> out(buffer, buffer+w*h*4);

    updateProgressBar(10);

    return out;
}


//CONVERT DATA should all be structured as RGBARGBA vector
std::vector<unsigned char> Convert::parseHeifData(const uint8_t* data, int w, int h, int stride) {
    lastEmittedValue = (int)progress;

    std::vector<unsigned char> d;
    d.resize(w * h * 4);

    uint count = 0;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            d[4 * w * y + 4 * x + 0] = data[count];
            d[4 * w * y + 4 * x + 1] = data[count + 1];
            d[4 * w * y + 4 * x + 2] = data[count + 2];
            d[4 * w * y + 4 * x + 3] = 255;

            count += 3;

            updateProgressBar(45.0/(w*h));

        }
        count += stride - (w * 3);

    }
    return d;
}

//EXTRA

QString Convert::getFileExtension(QString path) {
    qDebug() << path;
    return path.split(".")[1];
}

template <typename T>
std::vector<unsigned char> Convert::toBytes(T in) {
    uint num = (uint)in;

    uint four = 256 * 256 * 256;
    uint three = 256 * 256;
    uint two = 256;

    uint b1,b2,b3,b4 = 0;

    b4 = num/four;
    num -= b4*four;

    b3 = num/three;
    num -= b3*three;

    b2= num/two;
    num -= b2*two;

    b1 = num;

    return {(unsigned char)b1,(unsigned char)b2,(unsigned char)b3,(unsigned char)b4};
}

void Convert::updateProgressBar(double change) {
    //divides the progress added based on how many files are being processed
    //also makes sure a significant increase >1 is reached before updating the bar
    //this prevents overloading of the signal for every 0.001 increase
    progress += change/numFiles;
    if (progress - lastEmittedValue >= 1) {
        emit progressChanged(progress);
        lastEmittedValue = progress;
    }
}


std::vector<uint> Convert::getJpegSizeEstimations(std::vector<unsigned char>& image, uint w, uint h, uint numSamples = 5) {
    unsigned char *srcBuf; //!< flattened array to store rgb data
    tjhandle handle = tjInitCompress();

    const int nbands = 4;
    const int flags = 0;
    const int pixelFormat = TJPF_RGBA;
    const int jpegSubsamp = TJSAMP_411;

    srcBuf = image.data();

    std::vector<uint> points;
    uint range = 100/(numSamples-1); //the range in order to get x points

    //loop through each point, with the given quality compress the image to memory, and store its file size
    for (int i = 0; i<=100; i+=range) {
        unsigned char* jpegBuf = NULL;
        unsigned long  jpegSize = 0;

        tjCompress2( handle, srcBuf, w, w*nbands, h, pixelFormat, &(jpegBuf), &jpegSize, jpegSubsamp, i, flags);
        points.push_back(jpegSize);
    }


    //interpolate values in between major points;
    std::vector<uint> out;

    uint low = 0;
    uint high = 25;
    uint num = 0;
    uint linearInterp;
    for (uint i = 0; i<=100; i++) {
        if (i == high) {
            low = high;
            high += range;
            num ++;
        }

        linearInterp = points[num] + (double)((double)(i-low)/(double)(high-low))*((double)points[num+1] - points[num]);
        out.push_back(linearInterp);
    }


    return out;
}

void Convert::saveJpegSettings(std::vector<uint> settings) {
    //save settings to variable and emit signal to stop loop
    jpegSettings = settings;
    emit stopWait();
}

void Convert::squareImage(std::vector<unsigned char> &image, int &w, int &h) {
    if (w == h) return;
    if (w < h) {
        image.resize(w*w*4);
        h = w;
        return;
    }
    std::vector<unsigned char> square;

    uint byte = 0;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < h; x++) {
            square.push_back(image[byte]);
            square.push_back(image[byte+1]);
            square.push_back(image[byte+2]);
            square.push_back(image[byte+3]);
            byte += 4;
        }
    }

    image = square;
    w = h;

}

void Convert::resizeImage(std::vector<unsigned char> &image, uint w, uint h, uint nw, uint nh) {
    if (w == nw && h == nh) return;
    struct pixel {
            unsigned char r;
            unsigned char g;
            unsigned char b;
            unsigned char a;

            pixel(unsigned char r,unsigned char g,unsigned char b,unsigned char a) {
                this->r = r;
                this->g = g;
                this->b = b;
                this->a = a;
            }

            pixel() {
                this->r = 0;
                this->g = 0;
                this->b = 0;
                this->a = 0;
            }

        };

    //convert to matrix
    std::vector<std::vector<pixel>> matrix;
    uint byte = 0;
    for (uint y = 0; y<h; y++) {
        std::vector<pixel> row;
        for (uint x = 0; x<w; x++) {
            pixel p(image[byte], image[byte+1], image[byte+2], image[byte+3]);
            byte += 4;
            row.push_back(p);
        }
        matrix.push_back(row);
    }

    //create and fill col_interp matrix
    std::vector<std::vector<pixel>> col_interp;
    std::vector<pixel> row;
    for (uint x = 0; x<w; x++)
        row.push_back(pixel());

    for (uint y = 0; y<nh; y++)
        col_interp.push_back(row);


    //create and fill final matrix
    std::vector<std::vector<pixel>> final_interp;
    std::vector<pixel> row2;
    for (uint x = 0; x<nw; x++)
        row2.push_back(pixel());

    for (uint y = 0; y<nh; y++)
        final_interp.push_back(row2);


    //ratios
    double ratioRow = (double)w / nw;
    double ratioCol = (double)h / nh;


    //fill row ratio vals
    std::vector<uint> row_pos;
    for (uint x = 0; x<nw; x++) {
        double unrounded = (x+1.0)*ratioRow;
        int rounded = unrounded;
        if (rounded < unrounded)
            rounded += 1;
        row_pos.push_back(rounded);
    }

    //fill col ratio vals
    std::vector<uint> col_pos;
    for (uint y = 0; y<nw; y++) {
        double unrounded = (y+1.0)*ratioCol;
        int rounded = unrounded;
        if (rounded < unrounded)
            rounded += 1;
        col_pos.push_back(rounded);
    }

    //interpolate columns
    for (uint x = 0; x<w; x++){
        for (uint y=0; y<row_pos.size(); y++) {
            col_interp[y][x] = matrix[row_pos[y] -1][x];
        }
    }

    //inerpolate rows
    for (uint y = 0; y<nw; y++) {
        for (uint x = 0; x<col_pos.size(); x++) {
            final_interp[y][x] = col_interp[y][col_pos[x]-1];
        }
    }

    //flatten
    std::vector<unsigned char> newImage;
    for (auto a : final_interp) {
        for (auto b : a) {
            newImage.push_back(b.r);
            newImage.push_back(b.g);
            newImage.push_back(b.b);
            newImage.push_back(b.a);
        }
    }

    image = newImage;
}
