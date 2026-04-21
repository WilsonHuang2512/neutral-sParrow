#include <iostream> 
#include <iomanip>
#include <string.h>
#include "open_cam3d.h"

void saveBmp(const char* filename, unsigned char* bgr_data, int width, int height)
{
    int row_stride = width * 3;
    int file_size = 54 + row_stride * height;

    unsigned char header[54] = { 0 };
    // BMP signature
    header[0] = 'B'; header[1] = 'M';
    // File size
    header[2] = file_size & 0xFF;
    header[3] = (file_size >> 8) & 0xFF;
    header[4] = (file_size >> 16) & 0xFF;
    header[5] = (file_size >> 24) & 0xFF;
    // Pixel data offset
    header[10] = 54;
    // DIB header size
    header[14] = 40;
    // Width
    header[18] = width & 0xFF;
    header[19] = (width >> 8) & 0xFF;
    header[20] = (width >> 16) & 0xFF;
    header[21] = (width >> 24) & 0xFF;
    // Height (negative = top-down)
    int neg_height = -height;
    header[22] = neg_height & 0xFF;
    header[23] = (neg_height >> 8) & 0xFF;
    header[24] = (neg_height >> 16) & 0xFF;
    header[25] = (neg_height >> 24) & 0xFF;
    // Color planes
    header[26] = 1;
    // Bits per pixel
    header[28] = 24;

    FILE* f = nullptr;
    if (fopen_s(&f, filename, "wb") != 0 || !f) return;
    fwrite(header, 1, 54, f);
    fwrite(bgr_data, 1, row_stride * height, f);
    fclose(f);
}

int main()
{
    int ret_code = 0;

    ret_code = DfConnect("127.0.0.1");

    int width = 0, height = 0;
    int channels = 1;

    if (0 == ret_code)
    {
        ret_code = DfGetCameraResolution(&width, &height);
        std::cout << "Width: " << width << "    Height: " << height << std::endl;

        ret_code = DfGetCameraChannels(&channels);
        std::cout << "channels: " << channels << std::endl;
    }
    else
    {
        std::cout << "Connect Camera Error!";
        return -1;
    }

    // Get calibration parameters
    CalibrationParam calib_param;
    ret_code = DfGetCalibrationParam(&calib_param);

    if (0 == ret_code)
    {
        std::cout << std::fixed << std::setprecision(6);

        std::cout << "\nintrinsic:" << std::endl;
        for (int r = 0; r < 3; r++)
        {
            for (int c = 0; c < 3; c++)
                std::cout << calib_param.intrinsic[3 * r + c] << "\t";
            std::cout << std::endl;
        }

        std::cout << "\ndistortion:" << std::endl;
        for (int c = 0; c < 5; c++)
            std::cout << calib_param.distortion[c] << "\t";
        std::cout << std::endl;

        std::cout << "\nextrinsic:" << std::endl;
        for (int r = 0; r < 4; r++)
        {
            for (int c = 0; c < 4; c++)
                std::cout << calib_param.extrinsic[4 * r + c] << "\t";
            std::cout << std::endl;
        }
    }
    else
    {
        std::cout << "Get Calibration Param Error!" << std::endl;
    }

    // Allocate memory
    float* point_cloud_data = (float*)malloc(sizeof(float) * width * height * 3);
    memset(point_cloud_data, 0, sizeof(float) * width * height * 3);

    float* height_map_data = (float*)malloc(sizeof(float) * width * height);
    memset(height_map_data, 0, sizeof(float) * width * height);

    float* depth_data = (float*)malloc(sizeof(float) * width * height);
    memset(depth_data, 0, sizeof(float) * width * height);

    char* timestamp_data = (char*)malloc(sizeof(char) * 30);
    memset(timestamp_data, 0, sizeof(char) * 30);

    unsigned char* brightness_data = (unsigned char*)malloc(sizeof(unsigned char) * width * height);
    memset(brightness_data, 0, sizeof(unsigned char) * width * height);

    unsigned char* color_brightness_data = (unsigned char*)malloc(sizeof(unsigned char) * width * height * 3);
    memset(color_brightness_data, 0, sizeof(unsigned char) * width * height * 3);

    int capture_num = 0;

    if (0 == ret_code)
    {
        char status_json[20480];
        char config_json[20480];
        int num = 0;
        DfReadJson(config_json, "3.json");
        DfSetParamJson(config_json, status_json, num);

        ret_code = DfCaptureData(num, timestamp_data);

        if (0 == ret_code)
        {
            if (1 == channels)
            {
                ret_code = DfGetBrightnessData(brightness_data);
                if (0 == ret_code)
                    std::cout << "Get Brightness!" << std::endl;

                ret_code = DfGetDepthDataFloat(depth_data);
                if (0 == ret_code)
                    std::cout << "Get Depth!" << std::endl;

                ret_code = DfGetPointcloudData(point_cloud_data);
                if (0 == ret_code)
                {
                    std::cout << "Get Pointcloud!" << std::endl;
                    savePointcloudToPcd(point_cloud_data, brightness_data, channels, "1.pcd");
                    savePointcloudToPly(point_cloud_data, brightness_data, channels, "1.ply");
                }
            }
            else if (3 == channels)
            {
                ret_code = DfGetColorBrightnessData(color_brightness_data, Color::Bgr);
                if (0 == ret_code)
                {
                    std::cout << "Get color Brightness!" << std::endl;
                    int pixel_count = width * height;
                    unsigned char* bmp_buf = (unsigned char*)malloc((size_t)pixel_count * 3);
                    if (bmp_buf)
                    {
                        for (int i = 0; i < pixel_count; i++)
                        {
                            bmp_buf[i * 3 + 0] = color_brightness_data[i * 3 + 2];
                            bmp_buf[i * 3 + 1] = color_brightness_data[i * 3 + 1];
                            bmp_buf[i * 3 + 2] = color_brightness_data[i * 3 + 0];
                        }
                        saveBmp("bright.bmp", bmp_buf, width, height);
                        std::cout << "Saved bright.bmp" << std::endl;
                        free(bmp_buf);
                    }
                }

                ret_code = DfGetDepthDataFloat(depth_data);
                if (0 == ret_code)
                    std::cout << "Get Depth!" << std::endl;

                ret_code = DfGetPointcloudData(point_cloud_data);
                if (0 == ret_code)
                {
                    std::cout << "Get Pointcloud!" << std::endl;
                    savePointcloudToPcd(point_cloud_data, color_brightness_data, channels, "point_cloud.pcd");
                    savePointcloudToPly(point_cloud_data, color_brightness_data, channels, "point_cloud.ply");
                }
            }
            capture_num++;
            std::cout << "Capture num: " << capture_num << std::endl;
        }
        else
        {
            std::cout << "Capture Data Error!" << std::endl;
        }
    }

    free(brightness_data);
    free(color_brightness_data);
    free(depth_data);
    free(point_cloud_data);
    free(height_map_data);
    free(timestamp_data);

    DfDisconnect("127.0.0.1");

    return 0;
}