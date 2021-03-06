#include <primitiv/config.h>

#include <cstring>

#include <primitiv/cuda16_device.h>
#include <primitiv/internal/cuda_utils.h>
#include <primitiv/device_ops/cuda16/common.h>

namespace primitiv {
namespace devices {

void CUDA16::conv2d_fw_impl(
    const Tensor &x, const Tensor &w,
    std::uint32_t padding0, std::uint32_t padding1,
    std::uint32_t stride0, std::uint32_t stride1,
    std::uint32_t dilation0, std::uint32_t dilation1,
    Tensor &y) {
  const Shape x_shape = x.shape();
  const Shape w_shape = w.shape();
  const Shape y_shape = y.shape();

  // Specifies a target device.
  CUDA_CALL(::cudaSetDevice(dev_id_));

  // Prepares descriptors.
  const cuda::CuDNNTensorDescriptor x_desc(
      w_shape.has_batch() ? 1 : x_shape.batch(),
      x_shape[2], x_shape[1], x_shape[0],
      ::CUDNN_DATA_HALF);
  const cuda::CuDNNTensorDescriptor y_desc(
      w_shape.has_batch() ? 1 : y_shape.batch(),
      y_shape[2], y_shape[1], y_shape[0],
      ::CUDNN_DATA_HALF);
  const cuda::CuDNNFilterDescriptor w_desc(
      w_shape[3], w_shape[2], w_shape[1], w_shape[0],
      ::CUDNN_DATA_HALF);
  const cuda::CuDNNConvolutionDescriptor conv_desc(
      padding1, padding0, stride1, stride0, dilation1, dilation0,
      ::CUDNN_DATA_HALF);

  // Obtains the most efficient algorithm.
  ::cudnnConvolutionFwdAlgo_t algo;
  CUDNN_CALL(::cudnnGetConvolutionForwardAlgorithm(
        state_->cudnn.get(),
        x_desc.get(), w_desc.get(), conv_desc.get(), y_desc.get(),
        CUDNN_CONVOLUTION_FWD_PREFER_FASTEST, 0, &algo));

  // Obtains workspace size/memory.
  std::size_t ws_size;
  CUDNN_CALL(::cudnnGetConvolutionForwardWorkspaceSize(
        state_->cudnn.get(),
        x_desc.get(), w_desc.get(), conv_desc.get(), y_desc.get(),
        algo, &ws_size));
  std::shared_ptr<void> ws_ptr = state_->pool.allocate(ws_size);

  // Performs forward operations.
  const std::size_t x_shift = x_shape.has_batch() * x_shape.volume();
  const std::size_t w_shift = w_shape.volume();
  const std::size_t y_shift = y_shape.volume();
  const float alpha = 1.f;
  const float beta = 0.f;
  const half *x_ptr = CDATA(half, x);
  const half *w_ptr = CDATA(half, w);
  half *y_ptr = MDATA(half, y);
  for (std::uint32_t bn = 0; bn < w_shape.batch(); ++bn) {
    CUDNN_CALL(::cudnnConvolutionForward(
          state_->cudnn.get(),
          &alpha, x_desc.get(), x_ptr, w_desc.get(), w_ptr,
          conv_desc.get(), algo, ws_ptr.get(), ws_size,
          &beta, y_desc.get(), y_ptr));
    x_ptr += x_shift;
    w_ptr += w_shift;
    y_ptr += y_shift;
  }
}

void CUDA16::conv2d_bw_impl(
    const Tensor &x, const Tensor &w, const Tensor &, const Tensor &gy,
    std::uint32_t padding0, std::uint32_t padding1,
    std::uint32_t stride0, std::uint32_t stride1,
    std::uint32_t dilation0, std::uint32_t dilation1,
    Tensor &gx, Tensor &gw) {
  const Shape x_shape = x.shape();
  const Shape w_shape = w.shape();
  const Shape y_shape = gy.shape();

  // Specifies a target device.
  CUDA_CALL(::cudaSetDevice(dev_id_));

  // Prepares descriptors.
  const cuda::CuDNNTensorDescriptor x_desc(
      w_shape.has_batch() ? 1 : x_shape.batch(),
      x_shape[2], x_shape[1], x_shape[0],
      ::CUDNN_DATA_HALF);
  const cuda::CuDNNTensorDescriptor y_desc(
      w_shape.has_batch() ? 1 : y_shape.batch(),
      y_shape[2], y_shape[1], y_shape[0],
      ::CUDNN_DATA_HALF);
  const cuda::CuDNNFilterDescriptor w_desc(
      w_shape[3], w_shape[2], w_shape[1], w_shape[0],
      ::CUDNN_DATA_HALF);
  const cuda::CuDNNConvolutionDescriptor conv_desc(
      padding1, padding0, stride1, stride0, dilation1, dilation0,
      ::CUDNN_DATA_HALF);

  // Obtains the most efficient algorithms.
  ::cudnnConvolutionBwdDataAlgo_t x_algo;
  ::cudnnConvolutionBwdFilterAlgo_t w_algo;
  CUDNN_CALL(::cudnnGetConvolutionBackwardDataAlgorithm(
        state_->cudnn.get(),
        w_desc.get(), y_desc.get(), conv_desc.get(), x_desc.get(),
        CUDNN_CONVOLUTION_BWD_DATA_PREFER_FASTEST, 0, &x_algo));
  CUDNN_CALL(::cudnnGetConvolutionBackwardFilterAlgorithm(
        state_->cudnn.get(),
        x_desc.get(), y_desc.get(), conv_desc.get(), w_desc.get(),
        CUDNN_CONVOLUTION_BWD_FILTER_PREFER_FASTEST, 0, &w_algo));

  // Obtains workspace sizes/memory.
  std::size_t x_ws_size, w_ws_size;
  CUDNN_CALL(::cudnnGetConvolutionBackwardDataWorkspaceSize(
        state_->cudnn.get(),
        w_desc.get(), y_desc.get(), conv_desc.get(), x_desc.get(),
        x_algo, &x_ws_size));
  CUDNN_CALL(::cudnnGetConvolutionBackwardFilterWorkspaceSize(
        state_->cudnn.get(),
        x_desc.get(), y_desc.get(), conv_desc.get(), w_desc.get(),
        w_algo, &w_ws_size));
  const std::size_t ws_size = std::max(x_ws_size, w_ws_size);
  std::shared_ptr<void> ws_ptr = state_->pool.allocate(ws_size);

  // Performs backward operations.
  const std::size_t x_shift = x_shape.has_batch() * x_shape.volume();
  const std::size_t w_shift = w_shape.volume();
  const std::size_t y_shift = y_shape.volume();
  const float alpha = 1.f;
  const float beta = 1.f;
  const half *x_ptr = CDATA(half, x);
  const half *w_ptr = CDATA(half, w);
  const half *gy_ptr = CDATA(half, gy);
  half *gx_ptr = MDATA(half, gx);
  half *gw_ptr = MDATA(half, gw);
  for (std::uint32_t bn = 0; bn < w_shape.batch(); ++bn) {
    CUDNN_CALL(::cudnnConvolutionBackwardData(
          state_->cudnn.get(),
          &alpha, w_desc.get(), w_ptr, y_desc.get(), gy_ptr,
          conv_desc.get(), x_algo, ws_ptr.get(), ws_size,
          &beta, x_desc.get(), gx_ptr));
    CUDNN_CALL(::cudnnConvolutionBackwardFilter(
          state_->cudnn.get(),
          &alpha, x_desc.get(), x_ptr, y_desc.get(), gy_ptr,
          conv_desc.get(), w_algo, ws_ptr.get(), ws_size,
          &beta, w_desc.get(), gw_ptr));
    x_ptr += x_shift;
    w_ptr += w_shift;
    gy_ptr += y_shift;
    gx_ptr += x_shift;
    gw_ptr += w_shift;
  }
}

}  // namespace devices
}  // namespace primitiv
