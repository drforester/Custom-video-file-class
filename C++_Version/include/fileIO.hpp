#ifndef FILEIO_HPP
#define FILEIO_HPP

#include <vector>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;


const uint16_t FILE_HEADER_SIZE = 2048; // size in bytes

// to instantiate the writer
struct fileHeader
{
    std::string camera_frame_rate;
    std::string camera_bit_depth;
    std::string file_numerical_format;
    std::string frame_header_size;
    std::string meta_header_size;
    std::string image_height;
    std::string image_width;
};

class fileIO
{
    public:
        std::string infilename;
        uint8_t BIT_DEPTH;
        uint16_t FRAME_HEADER_SIZE = 0;
        uint16_t META_HEADER_SIZE = 0;
        uint16_t ROWS;
        uint16_t COLS;
        uint16_t nbytes;
        uint16_t TOTAL_FRAMES;
        std::string DTYPE;
        FILE *DATAFILE = NULL;
    
        fileIO(std::string filename);
        ~fileIO();
        void readFileHeader(bool verbose);
        cv::Mat readFrameMat(uint16_t ifirst, uint16_t ilast, uint16_t iframe, bool verbose);
        
        void writeFileHeader(fileHeader, bool verbose);
        void writeFrameMat(std::string fh, std::string mh, cv::Mat Img, bool isLast, bool verbose);
};

#endif
