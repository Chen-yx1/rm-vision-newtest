#include <cmath>
#include "armor_detector/kalman_filter.hpp"

namespace rm_auto_aim {

KalmanFilter::KalmanFilter() {
    initMatrices();
}

void KalmanFilter::initMatrices() {
    // 状态维度: 4 (x, vx, y, vy)
    int state_dim = 4;
    int measure_dim = 2;
    
    state_ = Eigen::VectorXd::Zero(state_dim);
    
    // 状态转移矩阵 F
    F_ = Eigen::MatrixXd::Identity(state_dim, state_dim);
    
    // 测量矩阵 H: 只测量位置，不测量速度
    H_ = Eigen::MatrixXd::Zero(measure_dim, state_dim);
    H_(0, 0) = 1;  // 测量x
    H_(1, 2) = 1;  // 测量y
    
    // 过程噪声协方差矩阵 Q
    // 假设位置和速度都有一定的不确定性
    Q_ = Eigen::MatrixXd::Zero(state_dim, state_dim);
    double dt = 0.033;  // 假设30fps
    double sigma_q_pos = 0.1;  // 位置过程噪声
    double sigma_q_vel = 0.5;  // 速度过程噪声
    
    Q_(0, 0) = sigma_q_pos * sigma_q_pos;
    Q_(1, 1) = sigma_q_vel * sigma_q_vel;
    Q_(2, 2) = sigma_q_pos * sigma_q_pos;
    Q_(3, 3) = sigma_q_vel * sigma_q_vel;
    
    // 测量噪声协方差矩阵 R
    R_ = Eigen::MatrixXd::Zero(measure_dim, measure_dim);
    double sigma_r = 5.0;  // 测量噪声（像素）
    R_(0, 0) = sigma_r * sigma_r;
    R_(1, 1) = sigma_r * sigma_r;
    
    // 误差协方差矩阵 P
    P_ = Eigen::MatrixXd::Identity(state_dim, state_dim) * 100.0;
    
    // 单位矩阵
    I_ = Eigen::MatrixXd::Identity(state_dim, state_dim);
    
    initialized_ = false;
}

void KalmanFilter::init(const cv::Point2f& initial_pos) {
    // 初始化状态
    state_(0) = initial_pos.x;  // x
    state_(1) = 0.0;            // vx
    state_(2) = initial_pos.y;  // y
    state_(3) = 0.0;            // vy
    
    // 重置误差协方差
    P_ = Eigen::MatrixXd::Identity(4, 4) * 100.0;
    
    initialized_ = true;
}

cv::Point2f KalmanFilter::predict() {
    if (!initialized_) {
        return cv::Point2f(0, 0);
    }
    
    // 预测状态
    state_ = F_ * state_;
    
    // 预测误差协方差
    P_ = F_ * P_ * F_.transpose() + Q_;
    
    return cv::Point2f(state_(0), state_(2));
}

void KalmanFilter::update(const cv::Point2f& measurement) {
    if (!initialized_) {
        return;
    }
    
    // 转换为Eigen向量
    Eigen::VectorXd z(2);
    z << measurement.x, measurement.y;
    
    // 计算卡尔曼增益
    Eigen::MatrixXd K = P_ * H_.transpose() * (H_ * P_ * H_.transpose() + R_).inverse();
    
    // 更新状态估计
    state_ = state_ + K * (z - H_ * state_);
    
    // 更新误差协方差
    P_ = (I_ - K * H_) * P_;
}

} // namespace rm_auto_aim