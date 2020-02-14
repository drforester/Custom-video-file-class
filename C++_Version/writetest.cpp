#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "include/fileIO.hpp"


using namespace std;
using namespace cv;

int main()
{
    std::string datafilename = "./data/writeTest.raw";
    
    // instantiate a graveIO class
    fileIO fio(datafilename);
    
    // write the file header
    fileHeader FH;
    FH.camera_frame_rate = "30";
    FH.camera_bit_depth = "14";
    FH.file_numerical_format = "uint16";
    FH.frame_header_size = "26";
    FH.meta_header_size = "20";
    FH.image_height = "740";
    FH.image_width = "1280";
    fio.writeFileHeader(FH, true);
    
    // write each frame (headers & data) and advance the file pointer
    uint16_t ROWS = 740;
    uint16_t COLS = 1280;
    bool isLast = false;
    uint16_t iframe = 1;
    uint16_t nbFrames = 3;
    while (not isLast) {
        
        if (iframe == nbFrames) isLast = true;
    
        cout << "iframe: " << iframe << endl;
        
        std::string thisFrameHeader = "test frame header         ";
        std::string thisMetaHeader = "test meta header    ";
        cv::Mat thisImg(ROWS, COLS, CV_16UC1);
        
        // write the image
        for (uint16_t i=0; i<ROWS; i++) {
            for (uint16_t j=0; j<COLS; j++) {
                uint16_t thisVal = (iframe*i)%65535; // some test values
                thisImg.at<ushort>(i,j) = thisVal;
            }
        }            
        
        
        fio.writeFrameMat(thisFrameHeader, thisMetaHeader, thisImg, isLast, true);
    
        iframe += 1;
    }
    
}
