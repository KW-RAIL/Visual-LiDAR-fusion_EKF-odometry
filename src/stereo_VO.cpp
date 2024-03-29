#include <stereo_VO.h>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

cv::Mat getCalib(string dataset_path, string N_Camera){

    // Load calibration data
    string calib = dataset_path + "calib.txt";
    ifstream file(calib);

    if (!file.is_open()){
        cerr << "Unable to open file" << endl;
    }

    string line;
    Mat calibMat = Mat(3, 4, CV_64F);

    if(file.is_open()){
        while(getline(file, line)){
            stringstream ss(line);
            string value, v;
            while (ss >> value){
                if (value == N_Camera){
                    int idx = 0;
                    while (ss >> v){
                        int col = idx % 4;
                        int row = idx / 4;
                        // cout << "row : " << row << " col : " << col << " val : " << v << endl;
                        calibMat.at<double>(row, col) = stod(v); // stoi 대신 stod 사용
                        idx++;}
                }
            }
        }
        file.close();
    }

    return calibMat;
}

// stereo image를 입력 받아서 disparity를 계산하는 과정으로 stereo_rectify의 결과 이미지들이 나와야 함.
void calculate_disparity(Mat img_l, Mat img_r, Mat &disparity){

    // int blocksize = 11;

    Mat img_disparity_16s;

    // cv::Ptr<cv::StereoSGBM> stereo = cv::StereoSGBM::create(0, ndisparities, blocksize, P1, P2);

    // cout << img_l.size() << endl;
    // cout << img_r.size() << endl;
    
    stereoBM->compute(img_l, img_r, img_disparity_16s);
    // stereoSGBM->compute(img_l, img_r, img_disparity_16s);

    img_disparity_16s.convertTo(disparity, CV_8UC1);
}


int main(int argc, char* argv[]) {

    // set path of KITTI dataset
    // {$PATH}/EKF_Localization/dataset/KITTI/00/
    string dataset_path = argv[1];

    string left_dir = dataset_path + "image_0/";
    string right_dir = dataset_path + "image_1/";
    

    // Calculate num of images
    int num_images = 0;
    for (const auto& entry : fs::directory_iterator(left_dir)){
        if (entry.is_regular_file()) {
            num_images++;
        }
    }

    // Load calibration data
    Mat proj_l = getCalib(dataset_path, "P0:");
    Mat proj_r = getCalib(dataset_path, "P1:");

    double f_x = proj_l.at<double>(0,0);
    double f_y = proj_l.at<double>(1,1);
    double c_x = proj_l.at<double>(0,2);
    double c_y = proj_l.at<double>(1,2);


    // Load first image pair
    Mat first_l = imread(left_dir + "000000.png", CV_8UC1);
    Mat first_r = imread(right_dir + "000000.png", CV_8UC1);

    if (first_l.empty() || first_r.empty()) {
        cerr << "Failed to load images!" << endl;
        return -1;
    }

    
    // 필요한 작업을 생각해보면
    // 1. stereo rectification
    //  -> https://alida.tistory.com/59
    //  -> https://velog.io/@cjh1995-ros/1.-%EC%A0%80%EA%B0%80-%EC%B9%B4%EB%A9%94%EB%9D%BC%EC%97%90%EC%84%9C-%EA%B3%A0%EA%B0%80%EC%B9%B4%EB%A9%94%EB%9D%BC-%EA%B8%B0%EB%8A%A5-%EA%B5%AC%ED%98%84
    
    // 2. calculate disparity
    // 3. 3D point recover?
    // 4. feature matching
    // 5. calculate R, t



    // Stereo rectification & disparity
    Mat R1, R2, P1, P2, Q;
    Mat imgU1, imgU2, disparity;

    // calculate_rect(first_l, proj_l, proj_r, R1, R2, P1, P2, Q);
    // stereo_rectify(first_l, first_r, proj_l, proj_r, imgU1, imgU2, R1, R2, P1, P2, Q);
    calculate_disparity(first_l, first_r, disparity);
    imshow("disp", disparity);
    waitKey(0);

    // for문으로 전체 처리

    string filename_format = "%06d.png";
    Mat curr_l, curr_r;
    Mat pred_l, pred_r;

    for (int Frame = 1; Frame < num_images; Frame++){
        // pred_lr을 이전 timestamp 프레임으로 저장
        if (Frame == 1){
            pred_l = first_l.clone();
            pred_r = first_r.clone();
        }
        else{
            pred_l = curr_l.clone();
            pred_r = curr_r.clone();
        }

        char filename[100];
        sprintf(filename, filename_format.c_str(), Frame);
        string curr_filename_l = left_dir + filename;
        string curr_filename_r = right_dir + filename;

        curr_l = imread(curr_filename_l, CV_8UC1);
        curr_r = imread(curr_filename_r, CV_8UC1);

        if (curr_l.empty() || curr_r.empty()) {
            cerr << "Failed to load images!" << endl;
            return -1;
        }

        // stereo_rectify(curr_l, curr_r, calib_l, calib_r, imgU1, imgU2, R1, R2, P1, P2, Q);
        calculate_disparity(curr_l, curr_r, disparity);
        imshow("disp", disparity);
        imshow("curr_l", curr_l);
        imshow("curr_r", curr_r);
        waitKey(0);

    }




    return 0;
}
