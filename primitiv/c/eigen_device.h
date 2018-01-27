#ifndef PRIMITIV_C_EIGEN_DEVICE_H_
#define PRIMITIV_C_EIGEN_DEVICE_H_

#include <primitiv/c/define.h>
#include <primitiv/c/device.h>

/**
 * Creates a new Device object.
 * @param device Pointer to receive a handler.
 * @return Status code.
 * @remarks The random number generator is initialized using
 *          `std::random_device`.
 */
PRIMITIV_C_API PRIMITIV_C_STATUS primitiv_devices_Eigen_new(
    primitivDevice_t **device);

/**
 * Creates a new Device object.
 * @param rng_seed The seed value of the random number generator.
 * @param device Pointer to receive a handler.
 * @return Status code.
 */
PRIMITIV_C_API PRIMITIV_C_STATUS primitiv_devices_Eigen_new_with_seed(
    uint32_t rng_seed, primitivDevice_t **device);

#endif  // PRIMITIV_C_EIGEN_DEVICE_H_
