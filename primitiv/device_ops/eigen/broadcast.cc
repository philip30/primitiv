#include <primitiv/config.h>

#include <primitiv/eigen_device.h>
#include <primitiv/device_ops/eigen/common.h>

namespace primitiv {
namespace devices {

void Eigen::broadcast_fw_impl(
    const Tensor &x, std::uint32_t dim, std::uint32_t size, Tensor &y) {
  // TODO(odashi): Optimize this functions using Eigen operations.

  const std::uint32_t repeat = x.shape().size();
  const std::uint32_t skip1 = y.shape().lower_volume(dim);
  const std::uint32_t skip2 = skip1 * size;
  float *dest = MDATA(y);
  const float *src = CDATA(x);
  for (std::uint32_t i = 0; i < repeat; ++i) {
    std::uint32_t offset = i % skip1 + (i / skip1) * skip2;
    float tmp = src[i];
    for (std::uint32_t j = 0; j < size; ++j) {
      dest[offset] = tmp;
      offset += skip1;
    }
  }
}

}  // namespace devices
}  // namespace primitiv
