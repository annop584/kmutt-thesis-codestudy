/*
 * @file SURF_FlannMatcher
 * @brief SURF detector + descriptor + FLANN Matcher
 * @author A. Huaman
 */
#include <stdio.h>
#include <iostream>
#include <stdio.h>
#include <iostream>
#import <opencv2/opencv.hpp>
#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <math.h>


using namespace std;
using namespace cv;


/*
 * @function main
 * @brief Main function
 */
int main( int argc, char**  argv )
{
    vector<Mat> imgs;
//    Mat img_1 = imread( "/Users/annopkobhiran/Documents/Xcode/kmutt/testopencv/codes/idealteethstitch/img3/ref.png", IMREAD_GRAYSCALE );
//     Mat img_2 = imread( "/Users/annopkobhiran/Documents/Xcode/kmutt/testopencv/codes/idealteethstitch/img3/5.png", IMREAD_GRAYSCALE );
//    Mat img_1 = imread( "/Users/annopkobhiran/Documents/Xcode/kmutt/testopencv/codes/idealteethstitch/imgs/1.png", IMREAD_GRAYSCALE );
//    Mat img_2 = imread( "/Users/annopkobhiran/Documents/Xcode/kmutt/testopencv/codes/idealteethstitch/imgs/3.png", IMREAD_GRAYSCALE );
//    Mat img_1 = imread( "/Users/annopkobhiran/Documents/Xcode/kmutt/testopencv/codes/idealteethstitch/img4/7.png", IMREAD_GRAYSCALE );
//    Mat img_2 = imread( "/Users/annopkobhiran/Documents/Xcode/kmutt/testopencv/codes/idealteethstitch/img4/12.png", IMREAD_GRAYSCALE );

    
//    Mat img_1 = imread( "/Users/annopkobhiran/Documents/Xcode/kmutt/testopencv/codes/idealteethstitch/imgs/1.png", IMREAD_GRAYSCALE );
    //Mat img_1 = imread( "/Users/annopkobhiran/Movies/Thesis/Dataset/1low/frames/200.png", IMREAD_GRAYSCALE );
//    Mat img_1 = imread( "/Users/annopkobhiran/Documents/Xcode/kmutt/testopencv/codes/idealteethstitch/img2/rz/IMG_0515.jpg", IMREAD_GRAYSCALE );
//    Mat img_2 = imread( "/Users/annopkobhiran/Documents/Xcode/kmutt/testopencv/codes/idealteethstitch/img2/rz/IMG_0516.jpg", IMREAD_GRAYSCALE );
    Mat img_1 = imread( "/Users/annopkobhiran/Documents/Xcode/kmutt/testopencv/codes/idealteethstitch/imgtest/rz/test1.png", IMREAD_GRAYSCALE );
    Mat img_2 = imread( "/Users/annopkobhiran/Documents/Xcode/kmutt/testopencv/codes/idealteethstitch/imgtest/rz/tests.png", IMREAD_GRAYSCALE );
    
    
   
    Mat I = cv::Mat::eye(3,3,CV_64F);
    Mat result(Size(6*img_1.cols,6*img_1.rows),CV_8UC3);

    
    Ptr<ORB> detector = ORB::create();
    std::vector<KeyPoint> keypoints_1, keypoints_2;
    Mat descriptors_1, descriptors_2;
    detector->detectAndCompute( img_1, Mat(), keypoints_1, descriptors_1 );
    detector->detectAndCompute( img_2, Mat(), keypoints_2, descriptors_2 );
    //-- Step 2: Matching descriptor vectors using FLANN matcher
    BFMatcher matcher(NORM_HAMMING);
    std::vector< DMatch > matches,matches21;
    matcher.match( descriptors_1, descriptors_2, matches );
    matcher.match( descriptors_2, descriptors_1, matches21 );
    double max_dist = 0; double min_dist = 100;
    //-- Quick calculation of max and min distances between keypoints
    for( int i = 0; i < descriptors_1.rows; i++ )
    { double dist = matches[i].distance;
        if( dist < min_dist ) min_dist = dist;
        if( dist > max_dist ) max_dist = dist;
    }
    printf("-- Max dist : %f \n", max_dist );
    printf("-- Min dist : %f \n", min_dist );
    //-- Draw only "good" matches (i.e. whose distance is less than 2*min_dist,
    //-- or a small arbitary value ( 0.02 ) in the event that min_dist is very
    //-- small)
    //-- PS.- radiusMatch can also be used here.
    std::vector< DMatch > good_matches;
    //                                    for( int i = 0; i < descriptors_1.rows; i++ )
    //                                    {
    //                                        if( matches[i].distance <= 3*min_dist )
    //                                        { good_matches.push_back( matches[i]); }
    //                                    }
    for( int i = 0; i < descriptors_1.rows; i++ )
    {
        DMatch forward = matches[i];
        DMatch backward = matches21[forward.trainIdx];
        if( backward.trainIdx == forward.queryIdx )
            good_matches.push_back( forward );
    }
    
    //            for( int i = 0; i < descriptors_1.rows; i++ )
    //            {
    //                good_matches.push_back( matches[i] );
    //            }
    //-- Draw only "good" matches
    Mat img_matches;
    drawMatches( img_1, keypoints_1, img_2, keypoints_2,
                good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
                vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
    //-- Show detected matches
    imshow( "Good Matches", img_matches );
    
    waitKey();
    
    std::vector<Point2f> obj;
    std::vector<Point2f> scene;
    
    for( int i = 0; i < good_matches.size(); i++ )
    {
        //-- Get the keypoints from the good matches
        obj.push_back( keypoints_1[ good_matches[i].queryIdx ].pt );
        scene.push_back( keypoints_2[ good_matches[i].trainIdx ].pt );
    }
    
    vector<char> inliers;
    Mat HOMO = findHomography( obj, scene ,inliers,RANSAC,2);
    Mat R = estimateRigidTransform(obj, scene, 1);
    Mat H = cv::Mat(3,3,R.type());
    H.at<double>(0,0) = R.at<double>(0,0);
    H.at<double>(0,1) = R.at<double>(0,1);
    H.at<double>(0,2) = R.at<double>(0,2);
    
    H.at<double>(1,0) = R.at<double>(1,0);
    H.at<double>(1,1) = R.at<double>(1,1);
    H.at<double>(1,2) = R.at<double>(1,2);
    
    H.at<double>(2,0) = 0.0;
    H.at<double>(2,1) = 0.0;
    H.at<double>(2,2) = 1.0;
    cout << "INDEX = "<< endl << 1<< endl << endl;
    cout << "H = "<< endl << " "  << H << endl << endl;
    
    I=H*I;
    cout << "I = "<< endl << " "  << I << endl << endl;
    cout << "HOMO = "<< endl << " "  << HOMO << endl << endl;
    

   warpPerspective(img_2, result, HOMO, result.size(),cv::INTER_CUBIC + cv::WARP_INVERSE_MAP , cv::BORDER_TRANSPARENT);
    imshow( "Perspec", result );
    waitKey();
//    float f = 1, w = img_1.cols, h = img_1.rows;
    float f = 2.733067285454205e+02, w = img_1.cols, h = img_1.rows, dis1 = 1.222239726849928e+02, dis2 = 1.590436693361728e+02;
    std::vector<cv::Mat> Rs, Ts;
    cv::Mat1f K = (cv::Mat1f(3, 3) <<
                   f, 0, w/2,
                   0, f, h/2,
                   0, 0,   1);
    
    cv::decomposeHomographyMat(HOMO,
                               K,
                               Rs, Ts,
                               cv::noArray());
    std::cout << "r = " << std::endl;
    for (auto R_ : Rs) {
        cv::Mat1d rvec;
        cv::Rodrigues(R_, rvec);
//        std::cout << rvec*180/CV_PI << std::endl << std::endl;
        std::cout << R_ << std::endl << std::endl;
    }
    
    std::cout << std::endl;
    
    std::cout << "t = " << std::endl;
    for (auto t_ : Ts) {
        std::cout << t_ << std::endl << std::endl;
    }
    
    
    //**************************** start get rotation, translation, scale ****************************
    Mat smat = H.clone();
    cout << "CLONE SMAT = "<< endl << " "  << smat << endl << endl;
    cout << "CLONE SMAT 21 = "<< endl << " "  << smat.at<double>(1,0) << endl << endl;
    cout << "CLONE SMAT 11 = "<< endl << " "  << smat.at<double>(0,0) << endl << endl;
    float ax = atan2(smat.at<double>(2,1), smat.at<double>(2,2))*180/M_PI;
    float ay = atan2(-1*smat.at<double>(2,0),sqrt((smat.at<double>(2,1)*smat.at<double>(2,1))+(smat.at<double>(2,2)*smat.at<double>(2,2))))*180/M_PI;
    float az = atan2(smat.at<double>(1,0), smat.at<double>(0,0))*180/M_PI;
    cout << "ax " << ax << endl;
    cout << "ay " << ay << endl;
    cout << "az " << az << endl;
    float ss = smat.at<double>(0,1);
    float sc = smat.at<double>(0,0);
    float scale_recovered = sqrt(ss*ss + sc*sc);
    cout << "scale_recovered" << scale_recovered << endl;
    
    float tx = H.at<double>(0,2);
    float ty = H.at<double>(1,2);
    
    cout << "tx= "<< tx << endl;
    cout << "ty= "<< ty << endl;
     //**************************** end get rotation, translation, scale ****************************
    return 0;
}
/*
 * @function readme
 */



