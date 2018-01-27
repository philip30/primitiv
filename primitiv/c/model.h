#ifndef PRIMITIV_C_MODEL_H_
#define PRIMITIV_C_MODEL_H_

#include <primitiv/c/define.h>
#include <primitiv/c/device.h>
#include <primitiv/c/parameter.h>

/**
 * Opaque type of Model.
 */
typedef struct primitivModel primitivModel_t;

/**
 * Creates a new Model object.
 * @param model Pointer to receive a handler.
 * @return Status code.
 */
PRIMITIV_C_API PRIMITIV_C_STATUS primitiv_Model_new(
    primitivModel_t **model);

/**
 * Deletes the Model object.
 * @param model Pointer of a handler.
 * @return Status code.
 */
PRIMITIV_C_API PRIMITIV_C_STATUS primitiv_Model_delete(
    primitivModel_t *model);

/**
 * Loads all parameters from a file.
 * @param model Pointer of a handler.
 * @param path Path of the file.
 * @param with_stats Whether or not to load all additional statistics.
 * @param device Device object to manage parameters.
 * @return Status code.
 */
PRIMITIV_C_API PRIMITIV_C_STATUS primitiv_Model_load(
    primitivModel_t *model, const char *path, PRIMITIV_C_BOOL with_stats,
    primitivDevice_t *device);

/**
 * Saves all parameters to a file.
 * @param model Pointer of a handler.
 * @param path Path of the file.
 * @param with_stats Whether or not to save all additional statistics.
 * @return Status code.
 */
PRIMITIV_C_API PRIMITIV_C_STATUS primitiv_Model_save(
    const primitivModel_t *model, const char *path, PRIMITIV_C_BOOL with_stats);

/**
 * Registers a new parameter.
 * @param model Pointer of a handler.
 * @param name Name of the parameter.
 * @param param Pointer to the parameter.
 * @return Status code.
 * @remarks `name` should not be overlapped with all registered parameters and
 *          submodels.
 */
PRIMITIV_C_API PRIMITIV_C_STATUS primitiv_Model_add_parameter(
    primitivModel_t *model, const char *name, primitivParameter_t *param);

/**
 * Registers a new submodel.
 * @param model Pointer of a handler.
 * @param name Name of the submodel.
 * @param model Pointer to the submodel.
 * @return Status code.
 * @remarks `name` should not be overlapped with all registered parameters and
 *          submodels.
 */
PRIMITIV_C_API PRIMITIV_C_STATUS primitiv_Model_add_model(
    primitivModel_t *model, const char *name, primitivModel_t *submodel);

/**
 * Recursively searches a parameter with specified name hierarchy.
 * @param model Pointer of a handler.
 * @param names Name hierarchy of the parameter.
 * @param n Number of the names.
 * @param param Pointer to receive a const-reference of the corresponding
 *              `Parameter` object.
 * @return Status code.
 */
PRIMITIV_C_API PRIMITIV_C_STATUS primitiv_Model_get_parameter(
    const primitivModel_t *model, const char **names, size_t n,
    const primitivParameter_t **param);

/**
 * Recursively searches a submodel with specified name hierarchy.
 * @param model Pointer of a handler.
 * @param names Name hierarchy of the submodel.
 * @param n Number of the names.
 * @param submodel Pointer to receive a const-reference of the corresponding
 *                 `Model` object.
 * @return Status code.
 */
PRIMITIV_C_API PRIMITIV_C_STATUS primitiv_Model_get_submodel(
    const primitivModel_t *model, const char **names, size_t n,
    const primitivModel_t **submodel);

// @TODO: Implement primitiv_Model_get_all_parameters()
// @TODO: Implement primitiv_Model_get_trainable_parameters()

#endif  // PRIMITIV_C_MODEL_H_
