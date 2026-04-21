import camera
import numpy as np
import cv2

# 连接相机
ip = '127.0.0.1'
ret = camera.DfConnect(ip)
if ret != 0:
    print("Connect Camera Error!")
    exit(-1)

# 必须连接相机成功后，才可获取相机分辨率
width = np.zeros(1, dtype=np.int32)
height = np.zeros(1, dtype=np.int32)
channels = np.zeros(1, dtype=np.int32)
camera.DfGetCameraResolution(width, height)
camera.DfGetCameraChannels(channels)
print("width:", width[0], "height:", height[0], "channels:", channels[0])

width    = int(width[0])
height   = int(height[0])
channels = int(channels[0])

# 获取相机标定参数
calib = camera.CalibrationParam()
ret = camera.DfGetCalibrationParam(calib)
if ret == 0:
    K  = np.array(camera.get_intrinsic(calib),  dtype=np.float32).reshape(3, 3)
    RT = np.array(camera.get_extrinsic(calib),  dtype=np.float32).reshape(4, 4)
    D  = np.array(camera.get_distortion(calib), dtype=np.float32).flatten()[:5]
    print("\nintrinsic:")
    print(K)
    print("\nextrinsic:")
    print(RT)
    print("\ndistortion (k1,k2,p1,p2,k3):")
    print(D)
else:
    print("Get Calibration Param Error!")

# 读取并设置JSON配置
cfg = camera.DfReadJson("3.json")
status, maxnum = camera.DfSetParamJson(cfg)

# 采集数据
camera.DfCaptureData(maxnum, "time")

# 分配内存保存采集结果
pointcloud_data = np.zeros((width * height * 3,), dtype=np.float32)
depth_data      = np.zeros((height, width),       dtype=np.float32)

# 获取深度图数据
camera.DfGetDepthDataFloat(depth_data)
cv2.imwrite("depth.tiff", depth_data)
print("Get Depth!")

# 获取点云数据
camera.DfGetPointcloudData(pointcloud_data)
print("Get Pointcloud!")

# 根据通道数选择不同的亮度图获取方式
if channels == 1:
    # 灰度相机
    brightness_data = np.zeros((height, width), dtype=np.uint8)
    camera.DfGetBrightnessData(brightness_data)
    cv2.imwrite("bright.bmp", brightness_data)
    print("Get Brightness!")

    # 保存灰度点云
    camera.savePointcloudToPcd(pointcloud_data, brightness_data, channels, "pointcloud.pcd")
    camera.savePointcloudToPly(pointcloud_data, brightness_data, channels, "pointcloud.ply")

elif channels == 3:
    # 彩色相机 - 使用BGR格式（点云颜色正确）
    color_data = np.zeros((height, width, 3), dtype=np.uint8)
    camera.DfGetColorBrightnessData(color_data, camera.XemaColor.Bgr)
    print("Get color Brightness!")

    # 交换R和B通道保存图像，原始BGR数据用于点云
    color_rgb = color_data[:, :, ::-1]
    cv2.imwrite("bright.bmp", color_rgb)

    # 保存彩色点云（使用原始BGR数据）
    color_for_pcd = np.ascontiguousarray(color_data).reshape(-1)
    camera.savePointcloudToPcd(pointcloud_data, color_for_pcd, channels, "color_cloud.pcd")
    camera.savePointcloudToPly(pointcloud_data, color_for_pcd, channels, "color_cloud.ply")

else:
    print(f"警告: 不支持的通道数 {channels}")

print("Capture Success!")

# 断开相机
ret = camera.DfDisconnect(ip)
if ret == 0:
    print('相机已断开')