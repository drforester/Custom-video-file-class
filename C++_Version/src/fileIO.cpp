/*
	Slurps a file, meaning that it reads all frames and loads into volatile
	memory. Only use this method for a file containing a few hundred frames.
	For long sequences, read in frames one-by-one, incrementing the file-
	pointer until the end of file is encountered.
*/
#include <stdio.h>
#include <fstream>
#include <string>
#include <vector>

#include "opencv2/opencv.hpp"
#include "../include/fileIO.hpp"


// constructor
fileIO::fileIO (std::string fileName) {
    infilename = fileName;
}

// destructor
fileIO::~fileIO () {}

void fileIO::readFileHeader(bool verbose) {
    
    // get file size in bytes
    std::ifstream in(infilename, std::ios::binary | std::ios::ate);
    uint64_t FILESIZE = in.tellg();
    if (verbose) {
        std::cout << "filesize: " << FILESIZE << " bytes" << std::endl;
        std::cout << "file header size: " << FILE_HEADER_SIZE << " bytes" << endl;
    }
    
    // read the file header & save to a buffer
    std::cout <<  "Reading file header..." << std::endl;
    char buffer0[FILE_HEADER_SIZE];
    std::ifstream datafile( infilename, std::ios::binary );
    datafile.read (buffer0, FILE_HEADER_SIZE);

    // go through the buffer, split on endl, read height, width, etc
    std::stringstream ss(buffer0);
    std::string to;
    while(std::getline(ss, to, '\n'))
    {
        if (to.find("CAMERA_BIT_DEPTH") != std::string::npos){
            size_t found = to.find("="); // the string position of the "="
            if (found!=std::string::npos){
                BIT_DEPTH = std::atoi(to.substr(found+1).c_str());
                if (verbose)
                    std::cout << "...bit depth: " << BIT_DEPTH << std::endl;
            }
        }

        if (to.find("FILE_NUMERICAL_FORMAT") != std::string::npos){
            size_t found = to.find("="); // the string position of the "="
            if (found!=std::string::npos){
                DTYPE = to.substr(found+1);
                if (verbose)
                    std::cout << "...dtype: " << DTYPE << std::endl;
            }
        }

        if (to.find("FRAME_HEADER_SIZE") != std::string::npos){
            size_t found = to.find("="); // the string position of the "="
            if (found!=std::string::npos){
                FRAME_HEADER_SIZE = std::atoi(to.substr(found+1).c_str());
                if (verbose)
                    std::cout << "...frame header size: " << FRAME_HEADER_SIZE << std::endl;
            }
        }

        if (to.find("META_HEADER_SIZE") != std::string::npos){
            size_t found = to.find("="); // the string position of the "="
            if (found!=std::string::npos){
                META_HEADER_SIZE = std::atoi(to.substr(found+1).c_str());
                if (verbose)
                    std::cout << "...meta header size: " << META_HEADER_SIZE << std::endl;
            }
        }

        if (to.find("IMAGE_HEIGHT") != std::string::npos){
            size_t found = to.find("="); // the string position of the "="
            if (found!=std::string::npos){
                ROWS = std::atoi(to.substr(found+1).c_str());
                if (verbose)
                    std::cout << "...rows: " << ROWS << std::endl;
            }
        }

        if (to.find("IMAGE_WIDTH") != std::string::npos){
            size_t found = to.find("="); // the string position of the "="
            if (found!=std::string::npos){
                COLS = std::atoi(to.substr(found+1).c_str());
                if (verbose)
                    std::cout << "...cols: " << COLS << std::endl;
            }
        }

    }
    datafile.close();
    
    
    // calculate the total number of frames in this unread file
     uint8_t DEPTH = 1; // assume all images are single-color
     nbytes = 2; // the default
     std::string floatStr ("float");
     if (DTYPE.compare(0,5,floatStr) == 0 )
         nbytes = 4;
     std::string u16Str ("uint16");
     if (DTYPE.compare(0,6,u16Str) == 0 )
         nbytes = 2;
     
     if (0)
         std::cout << "nbytes: " << nbytes << std::endl;
     
     TOTAL_FRAMES = (FILESIZE-FILE_HEADER_SIZE)/((ROWS*COLS*DEPTH*nbytes)
                     +FRAME_HEADER_SIZE+META_HEADER_SIZE);
     if (verbose)
         std::cout << "total number of frames in file: " << TOTAL_FRAMES << std::endl;
 

}


cv::Mat fileIO::readFrameMat(uint16_t ifirst , uint16_t ilast, uint16_t iframe, bool verbose) {
    
    uint32_t nbFrameElems = ROWS * COLS;
    bool isFirst;
    bool isLast;
    (iframe == ifirst) ? isFirst = true : isFirst = false;
    (iframe == ilast) ? isLast = true : isLast = false;
    
    if (isFirst) {
        if (verbose) cout << endl << "opening file " << infilename << " ..." << endl;
        DATAFILE = fopen(infilename.c_str(), "rb");
        // skip past the file header
        fseek(DATAFILE, FILE_HEADER_SIZE, SEEK_CUR);
        // skip to first frame
        for (uint16_t ii=0; ii<(iframe-1); ii++) {
            fseek(DATAFILE, FRAME_HEADER_SIZE, SEEK_CUR);
            fseek(DATAFILE, META_HEADER_SIZE, SEEK_CUR);
            fseek(DATAFILE, sizeof(uint16_t)*nbFrameElems, SEEK_CUR);
        }
    }
    
    std::vector<char> fbuf(FRAME_HEADER_SIZE);
    if(FRAME_HEADER_SIZE > 0) {
        std::fread(&fbuf[0], sizeof fbuf[0], fbuf.size(), DATAFILE);
        if (verbose) {
            cout << "frame header: ";
            for (char n : fbuf)
                cout << n;
        }
        cout << endl;
    }
    
    std::vector<char> mbuf(META_HEADER_SIZE);
    if(META_HEADER_SIZE > 0) {
        std::fread(&mbuf[0], sizeof mbuf[0], mbuf.size(), DATAFILE);
        if (verbose) {
            cout << "meta header: ";
            for (char n : mbuf)
                cout << n;
        }
        cout << endl;
    }
    
    
    // Read a frame in data file, copy to temp 1D array, memcpy to 2D array
    uint16_t datavec[nbFrameElems];
    uint16_t data2d[ROWS][COLS];
    uint16_t * frame;

    cv::Mat tempMat(ROWS, COLS, CV_16UC1);   
    frame = new uint16_t [nbFrameElems];
    fread(datavec, sizeof(uint16_t), nbFrameElems, DATAFILE);
    memcpy( frame, datavec, nbFrameElems*sizeof(uint16_t) );
    memcpy( data2d[0], frame, (ROWS*COLS*sizeof(uint16_t)) );
    memcpy( tempMat.data, data2d, ROWS*COLS*sizeof(uint16_t) );
    
    
    
    if (isLast) {
        if (verbose) cout << endl << "closing file " << infilename << " ..." << endl;
        fclose(DATAFILE);
    }
    
    return tempMat;
}



void fileIO::writeFileHeader(fileHeader GFH, bool verbose) {
    std::cout <<  "Writing file header..." << std::endl;
    DATAFILE = fopen(infilename.c_str(), "wb");
    
    std::string fileHeaderStr = "0000000620\r\n"
                           "FILE_DATA_BEGIN=0000002048\r\n"
                           "CAMERA_BIT_DEPTH=" + GFH.camera_bit_depth + "\r\n"
                           "CAMERA_FRAME_RATE=" + GFH.camera_frame_rate + "\r\n"
                           "FILE_BYTE_ORDERING=LITTLE-ENDIAN\r\n"
                           "FILE_NUMERICAL_FORMAT=" + GFH.file_numerical_format + "\r\n"
                           "FILE_PLAYER=CameraPyxisLWIR.FilePlayer\r\n"
                           "FRAME_HEADER_SIZE=" + GFH.frame_header_size + "\r\n"
                           "META_HEADER_SIZE=" + GFH.meta_header_size + "\r\n"
                           "IMAGE_ADJUST_INCREMENT=1\r\n"
                           "IMAGE_ADJUST_RESOLUTION=0\r\n"
                           "IMAGE_HEIGHT=" + GFH.image_height + "\r\n"
                           "IMAGE_MAX_PIXEL_VALUE=16383\r\n"
                           "IMAGE_MIN_PIXEL_VALUE=0\r\n"                                
                           "IMAGE_WIDTH=" + GFH.image_width + "\r\n"
                           "POLARIMETER_POL_CAL=none\r\n"
                           "POLARIMETER_SP_NUC=none\r\n"
                           "POLARIMETER_TP_NUC=none\r\n"
                           "VIEW_0=Raw, 0, 0, 0, 0\r\n"
                           "VIEW_1=RAW/Unprocessed, 0, 0, 640, 512\r\n";
                           
    
    //pad to 2048 bytes
    size_t paddingSize = FILE_HEADER_SIZE - sizeof(char)*fileHeaderStr.size();
    std::string paddingStr;
    paddingStr.insert(0, paddingSize, ' ');
    fileHeaderStr += paddingStr;
    
    cout << fileHeaderStr << endl;
    fwrite( fileHeaderStr.c_str(), sizeof(char), fileHeaderStr.size(), DATAFILE );
    

}


void fileIO::writeFrameMat(std::string fh, std::string mh, cv::Mat mat, bool isLast, bool verbose) {
    
    // write the frame header
    fwrite(fh.c_str(), sizeof(char), fh.size(), DATAFILE);
    
    // write the meta header
    fwrite(mh.c_str(), sizeof(char), mh.size(), DATAFILE);
     
    // convert the frame data to a vector, then write to the file
    std::vector<ushort> array;
    for (int i = 0; i < mat.rows; ++i) {
        array.insert(array.end(), mat.ptr<ushort>(i), mat.ptr<ushort>(i)+mat.cols);
    }
    //fwrite((char*)&array[0], sizeof(ushort), sizeof(array), DATAFILE);
    fwrite( reinterpret_cast<const char*>(&array[0]), sizeof(ushort), array.size(), DATAFILE );
    
    if (isLast) {
        if (verbose) cout << endl << "closing file " << infilename << " ..." << endl;
        fclose(DATAFILE);
    }    
    
}    
