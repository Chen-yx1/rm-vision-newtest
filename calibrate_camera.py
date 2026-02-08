#!/usr/bin/env python3
"""
RoboMaster 相机标定脚本
用于生成棋盘格标定图像或执行相机标定

使用方法：
1. 生成测试图像：python calibrate_camera.py generate
2. 执行相机标定：python calibrate_camera.py
"""

import cv2
import numpy as np
import os
import sys
import glob
import argparse
from datetime import datetime

def print_banner():
    """打印程序横幅"""
    print("=" * 60)
    print("RoboMaster 相机标定工具")
    print("=" * 60)

def create_calibration_images(output_dir="calibration_images", num_images=20):
    """
    创建虚拟棋盘格标定图像（用于测试）
    
    参数：
        output_dir: 输出目录
        num_images: 生成的图像数量
    """
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
        print(f"✅ 创建目录: {output_dir}")
    
    # 棋盘格参数
    pattern_size = (9, 6)  # 内角点数量 (列, 行)
    square_size = 25       # 方格大小（毫米）
    
    print(f"📸 生成 {num_images} 张棋盘格标定图像...")
    print(f"  棋盘格: {pattern_size[0]}x{pattern_size[1]} 内角点")
    print(f"  方格大小: {square_size}mm")
    
    for i in range(num_images):
        # 创建白色背景图像
        img = np.ones((480, 640, 3), np.uint8) * 255
        
        # 生成世界坐标系中的角点
        obj_points = np.zeros((pattern_size[0] * pattern_size[1], 3), np.float32)
        obj_points[:, :2] = np.mgrid[0:pattern_size[0], 0:pattern_size[1]].T.reshape(-1, 2)
        obj_points *= square_size
        
        # 随机旋转和平移（模拟不同角度拍摄）
        rvec = np.random.rand(3, 1) * 0.8 - 0.4  # -0.4到0.4弧度
        tvec = np.array([0, 0, 600 + np.random.rand() * 300], dtype=np.float32).reshape(3, 1)
        
        # 虚拟相机内参
        camera_matrix = np.array([[800, 0, 320], [0, 800, 240], [0, 0, 1]], dtype=np.float32)
        dist_coeffs = np.zeros((5, 1), np.float32)
        
        # 将3D点投影到2D图像平面
        img_points, _ = cv2.projectPoints(obj_points, rvec, tvec, camera_matrix, dist_coeffs)
        
        # 绘制棋盘格角点
        for point in img_points.reshape(-1, 2):
            cv2.circle(img, tuple(point.astype(int)), 4, (0, 0, 0), -1)
        
        # 绘制棋盘格线
        points = img_points.reshape(pattern_size[1], pattern_size[0], 2)
        for row in range(pattern_size[1]):
            for col in range(pattern_size[0] - 1):
                pt1 = tuple(points[row, col].astype(int))
                pt2 = tuple(points[row, col + 1].astype(int))
                cv2.line(img, pt1, pt2, (0, 0, 0), 2)
        
        for col in range(pattern_size[0]):
            for row in range(pattern_size[1] - 1):
                pt1 = tuple(points[row, col].astype(int))
                pt2 = tuple(points[row + 1, col].astype(int))
                cv2.line(img, pt1, pt2, (0, 0, 0), 2)
        
        # 添加图像编号
        cv2.putText(img, f"Chessboard {i+1:03d}", (20, 30), 
                   cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)
        
        # 保存图像
        filename = os.path.join(output_dir, f"chessboard_{i+1:03d}.jpg")
        cv2.imwrite(filename, img)
        
        # 显示进度
        if (i + 1) % 5 == 0:
            print(f"  已生成 {i+1}/{num_images} 张图像")
    
    print(f"✅ 标定图像已保存到: {output_dir}/")
    print("⚠️  注意：这些是虚拟图像，仅用于测试。")
    print("   请使用真实相机拍摄棋盘格进行实际标定。")

def calibrate_camera(image_dir="calibration_images", pattern_size=(9,6), square_size=0.025):
    """
    执行相机标定
    
    参数：
        image_dir: 标定图像目录
        pattern_size: 棋盘格内角点数量 (列, 行)
        square_size: 棋盘格实际大小（米）
    """
    if not os.path.exists(image_dir):
        print(f"❌ 错误：目录 '{image_dir}' 不存在！")
        print("请先放置标定图像或运行: python calibrate_camera.py generate")
        return False
    
    # 查找所有图像文件
    image_files = sorted(glob.glob(os.path.join(image_dir, "*.jpg")) + 
                        glob.glob(os.path.join(image_dir, "*.png")) +
                        glob.glob(os.path.join(image_dir, "*.jpeg")) +
                        glob.glob(os.path.join(image_dir, "*.bmp")))
    
    if len(image_files) == 0:
        print(f"❌ 错误：在 '{image_dir}' 中未找到图像文件！")
        print("支持的格式: .jpg, .png, .jpeg, .bmp")
        return False
    
    print(f"📁 找到 {len(image_files)} 张标定图像")
    
    # 准备标定数据
    obj_points = []  # 世界坐标系中的3D点
    img_points = []  # 图像坐标系中的2D点
    
    # 生成世界坐标系中的棋盘格角点
    objp = np.zeros((pattern_size[0] * pattern_size[1], 3), np.float32)
    objp[:, :2] = np.mgrid[0:pattern_size[0], 0:pattern_size[1]].T.reshape(-1, 2)
    objp *= square_size
    
    print("🔍 开始检测棋盘格角点...")
    
    success_count = 0
    for i, filepath in enumerate(image_files):
        filename = os.path.basename(filepath)
        
        # 读取图像
        img = cv2.imread(filepath)
        if img is None:
            print(f"  [{i+1:03d}] ❌ 无法读取: {filename}")
            continue
        
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        
        # 查找棋盘格角点
        ret, corners = cv2.findChessboardCorners(gray, pattern_size, 
                                                 cv2.CALIB_CB_ADAPTIVE_THRESH + 
                                                 cv2.CALIB_CB_FAST_CHECK + 
                                                 cv2.CALIB_CB_NORMALIZE_IMAGE)
        
        if ret:
            # 亚像素精确化
            criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)
            corners_refined = cv2.cornerSubPix(gray, corners, (11, 11), (-1, -1), criteria)
            
            obj_points.append(objp)
            img_points.append(corners_refined)
            
            # 显示检测结果
            img_display = img.copy()
            cv2.drawChessboardCorners(img_display, pattern_size, corners_refined, ret)
            
            # 添加状态信息
            cv2.putText(img_display, f"OK: {filename}", (10, 30), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)
            
            success_count += 1
            print(f"  [{i+1:03d}] ✅ 成功: {filename}")
        else:
            print(f"  [{i+1:03d}] ❌ 失败: {filename} - 未找到棋盘格角点")
    
    print(f"\n📊 角点检测结果: {success_count}/{len(image_files)} 张图像成功")
    
    if success_count < 10:
        print(f"❌ 错误：至少需要10张成功图像，当前只有 {success_count} 张")
        print("请确保:")
        print("  1. 棋盘格完整出现在图像中")
        print("  2. 棋盘格方向多样（不同角度和距离）")
        print("  3. 图像清晰不模糊")
        return False
    
    # 获取图像尺寸
    img_sample = cv2.imread(image_files[0])
    h, w = img_sample.shape[:2]
    
    print("🎯 开始相机标定...")
    
    # 执行标定
    ret, camera_matrix, dist_coeffs, rvecs, tvecs = cv2.calibrateCamera(
        obj_points, img_points, (w, h), None, None)
    
    # 计算重投影误差
    mean_error = 0
    for i in range(len(obj_points)):
        imgpoints2, _ = cv2.projectPoints(obj_points[i], rvecs[i], tvecs[i], 
                                         camera_matrix, dist_coeffs)
        error = cv2.norm(img_points[i], imgpoints2, cv2.NORM_L2) / len(imgpoints2)
        mean_error += error
    
    mean_error /= len(obj_points)
    
    print("\n" + "=" * 60)
    print("🎉 相机标定完成！")
    print("=" * 60)
    
    print(f"\n📈 标定质量指标:")
    print(f"  重投影误差: {mean_error:.6f}")
    print(f"  (误差 < 0.5 表示标定质量良好)")
    
    print(f"\n📷 相机内参矩阵 (K):")
    print(f"  fx = {camera_matrix[0,0]:.2f}  (焦距x)")
    print(f"  fy = {camera_matrix[1,1]:.2f}  (焦距y)")
    print(f"  cx = {camera_matrix[0,2]:.2f}  (主点x)")
    print(f"  cy = {camera_matrix[1,2]:.2f}  (主点y)")
    print(f"\n  [[{camera_matrix[0,0]:.2f}, {camera_matrix[0,1]:.2f}, {camera_matrix[0,2]:.2f}]")
    print(f"   [{camera_matrix[1,0]:.2f}, {camera_matrix[1,1]:.2f}, {camera_matrix[1,2]:.2f}]")
    print(f"   [{camera_matrix[2,0]:.2f}, {camera_matrix[2,1]:.2f}, {camera_matrix[2,2]:.2f}]]")
    
    print(f"\n🔧 畸变系数 (D):")
    print(f"  k1 = {dist_coeffs[0,0]:.6f}")
    print(f"  k2 = {dist_coeffs[0,1]:.6f}")
    print(f"  p1 = {dist_coeffs[0,2]:.6f}")
    print(f"  p2 = {dist_coeffs[0,3]:.6f}")
    print(f"  k3 = {dist_coeffs[0,4]:.6f}" if len(dist_coeffs[0]) > 4 else "  k3 = 0.0")
    
    # 保存标定结果
    save_file = "camera_calibration.yml"
    fs = cv2.FileStorage(save_file, cv2.FileStorage.WRITE)
    
    fs.write("calibration_date", datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
    fs.write("image_width", w)
    fs.write("image_height", h)
    fs.write("camera_matrix", camera_matrix)
    fs.write("distortion_coefficients", dist_coeffs)
    fs.write("reprojection_error", mean_error)
    fs.write("successful_images", success_count)
    fs.write("total_images", len(image_files))
    
    fs.release()
    
    print(f"\n💾 标定结果已保存到: {save_file}")
    
    # 显示去畸变效果
    print("\n🖼️  显示去畸变效果对比...")
    
    # 获取最优新相机矩阵
    new_camera_matrix, roi = cv2.getOptimalNewCameraMatrix(
        camera_matrix, dist_coeffs, (w, h), 1, (w, h))
    
    # 加载第一张成功标定的图像
    for filepath in image_files:
        img = cv2.imread(filepath)
        if img is not None:
            # 去畸变
            dst = cv2.undistort(img, camera_matrix, dist_coeffs, None, new_camera_matrix)
            
            # 裁剪ROI区域
            x, y, w2, h2 = roi
            dst_cropped = dst[y:y+h2, x:x+w2]
            
            # 并排显示
            comparison = np.hstack([img, dst, dst_cropped])
            
            # 添加标签
            h_comparison, w_comparison = comparison.shape[:2]
            cv2.putText(comparison, "原始图像", (10, 30), 
                       cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
            cv2.putText(comparison, "去畸变图像", (w + 10, 30), 
                       cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
            cv2.putText(comparison, "裁剪后图像", (2*w + 10, 30), 
                       cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
            
            cv2.imshow("标定效果对比 (按任意键继续)", comparison)
            cv2.waitKey(2000)
            break
    
    cv2.destroyAllWindows()
    
    print("\n✅ 相机标定流程完成！")
    print("现在可以运行主程序并加载标定参数了。")
    print("使用方法：")
    print("  1. 运行主程序: ./bin/rm_vision_newtest")
    print("  2. 按 'l' 键加载标定参数")
    print("  3. 按 '3' 键显示3D坐标")
    
    return True

def main():
    """主函数"""
    parser = argparse.ArgumentParser(description='RoboMaster 相机标定工具')
    parser.add_argument('command', nargs='?', default='calibrate',
                       help='命令: generate (生成测试图像) 或 calibrate (执行标定)')
    parser.add_argument('--dir', default='calibration_images',
                       help='标定图像目录 (默认: calibration_images)')
    parser.add_argument('--num', type=int, default=20,
                       help='生成测试图像的数量 (默认: 20)')
    parser.add_argument('--pattern', default='9x6',
                       help='棋盘格内角点数量 (格式: 列x行, 默认: 9x6)')
    parser.add_argument('--size', type=float, default=0.025,
                       help='棋盘格方格实际大小 (米, 默认: 0.025)')
    
    args = parser.parse_args()
    
    print_banner()
    
    # 解析棋盘格参数
    try:
        pattern_cols, pattern_rows = map(int, args.pattern.split('x'))
        pattern_size = (pattern_cols, pattern_rows)
    except:
        print(f"❌ 错误：棋盘格参数格式无效 '{args.pattern}'")
        print("正确格式: 列x行 (例如: 9x6)")
        return
    
    if args.command.lower() == 'generate':
        print("🛠️  生成标定测试图像...")
        create_calibration_images(args.dir, args.num)
    elif args.command.lower() == 'calibrate':
        print("🎯 执行相机标定...")
        calibrate_camera(args.dir, pattern_size, args.size)
    else:
        print(f"❌ 错误：未知命令 '{args.command}'")
        print("可用命令:")
        print("  generate - 生成测试标定图像")
        print("  calibrate - 执行相机标定")
        print("\n示例:")
        print("  python calibrate_camera.py generate")
        print("  python calibrate_camera.py calibrate")
        print("  python calibrate_camera.py calibrate --dir my_calib_images --pattern 8x6")

if __name__ == "__main__":
    main()