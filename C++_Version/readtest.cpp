

#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "include/fileIO.hpp"


using namespace std;
using namespace cv;

int main()
{
    //std::string datafilename = "./data/2019_09_01_14_43_41.raw";
    std::string datafilename = "./data/writeTest.raw";
    uint16_t ifirst = 1;//940; // the first frame to read (begins at 1)
    uint16_t ilast  = 3;//1140;  // the last frame to read
    
    // instantiate a graveIO class
    fileIO fio(datafilename);
    
    // read the file header
    fio.readFileHeader(true);
    
    // read each frame (headers & data) and advance the file pointer
    double minVal;
    double maxVal;
    int minIdx;
    int maxIdx;
    Mat currMat;
    Mat ImgDisp;
    for (uint16_t iframe=ifirst; iframe<=ilast; iframe++) {

        cout << "iframe: " << iframe << endl;
        currMat = fio.readFrameMat(ifirst, ilast, iframe, true);
        
        minMaxIdx(currMat, &minVal, &maxVal, &minIdx, &maxIdx);
        cout << "minVal: " << minVal << "  maxVal: " << maxVal << endl;
        ImgDisp = currMat.clone();
        ImgDisp.convertTo(ImgDisp, CV_8U, (255/maxVal));
        cv::equalizeHist(ImgDisp, ImgDisp);
        
        imshow("current frame", ImgDisp);
		waitKey(0);
    }
    
}
