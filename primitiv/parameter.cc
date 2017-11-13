#include <config.h>

#include <fstream>
#include <primitiv/error.h>
#include <primitiv/file_format.h>
#include <primitiv/initializer.h>
#include <primitiv/operators.h>
#include <primitiv/parameter.h>

using std::string;
using std::vector;

namespace {

// Reads Shape data.
primitiv::Shape read_shape(primitiv::msgpack::Reader &reader) {
  std::vector<std::uint32_t> dims;
  std::uint32_t batch;
  reader >> dims >> batch;
  return primitiv::Shape(dims, batch);
}

// Reads Tensor data.
primitiv::Tensor read_tensor(primitiv::msgpack::Reader &reader, primitiv::Device &device) {
  primitiv::Shape shape = ::read_shape(reader);
  primitiv::msgpack::objects::Binary data;
  reader >> data;
  if (data.size() != shape.size() * sizeof(float)) {
    THROW_ERROR(
        "Shape and data length mismatched. "
        "shape.size() * sizeof(float): " << (shape.size() * sizeof(float))
        << " != data.size(): " << data.size());
  }
  return device.new_tensor_by_array(shape, reinterpret_cast<const float *>(data.data()));
}

// Writes Shape data.
void write_shape(const primitiv::Shape &src, primitiv::msgpack::Writer &writer) {
  writer << src.dims() << src.batch();
}

// Writes Tensor data.
void write_tensor(const primitiv::Tensor &src, primitiv::msgpack::Writer &writer) {
  const primitiv::Shape &shape = src.shape();
  const std::vector<float> raw_data = src.to_vector();
  primitiv::msgpack::objects::Binary data(
      shape.size() * sizeof(float), reinterpret_cast<const char *>(raw_data.data()));
  ::write_shape(shape, writer);
  writer << data;
}

void check_shape(
    const primitiv::Tensor &value,
    const primitiv::Tensor &grad) {
  const primitiv::Shape &sv = value.shape();
  const primitiv::Shape &sg = grad.shape();
  if (sv != sg) {
    THROW_ERROR(
        "Shape mismatched between value and gradient. value: "
        << sv.to_string() << ", gradient: " << sg.to_string());
  }
  if (sv.has_batch()) {
    THROW_ERROR(
        "The batch size of the parameter should be 1. shape: "
        << sv.to_string());
  }
}

}  // namespace

namespace primitiv {

Parameter::Parameter(
    const Shape &shape,
    const vector<float> & value,
    Device &device)
: shape_(shape)
, device_(&device)
, value_(operators::input<Tensor>(shape, value, device))
, grad_(operators::zeros<Tensor>(shape, device)) {
  ::check_shape(value_, grad_);
}

Parameter::Parameter(
    const Shape &shape,
    const Initializer &initializer,
    Device &device)
: shape_(shape)
, device_(&device)
, value_(operators::zeros<Tensor>(shape, device))
, grad_(operators::zeros<Tensor>(shape, device)) {
  ::check_shape(value_, grad_);
  initializer.apply(value_);
}

void Parameter::init(
    const Shape &shape,
    const std::vector<float> &value,
    Device &device) {
  Tensor value_temp = operators::input<Tensor>(shape, value, device);
  Tensor grad_temp = operators::zeros<Tensor>(shape, device);
  ::check_shape(value_temp, grad_temp);

  // Initialization succeeded. Move all objects to `this`.
  shape_ = shape;
  device_ = &device;
  value_ = std::move(value_temp);
  grad_ = std::move(grad_temp);
  stats_.clear();
}

void Parameter::init(
    const Shape &shape,
    const Initializer &initializer,
    Device &device) {
  Tensor value_temp = operators::zeros<Tensor>(shape, device);
  Tensor grad_temp = operators::zeros<Tensor>(shape, device);
  ::check_shape(value_temp, grad_temp);
  initializer.apply(value_temp);

  // Initialization succeeded. Move all objects to `this`.
  shape_ = shape;
  device_ = &device;
  value_ = std::move(value_temp);
  grad_ = std::move(grad_temp);
  stats_.clear();
}

void Parameter::load_inner(
    msgpack::Reader &reader, bool with_stats, Device &device) {
  Tensor value_temp = ::read_tensor(reader, device);

  std::uint32_t num_stats;
  reader >> num_stats;

  std::unordered_map<string, Tensor> stats;
  for (std::uint32_t i = 0; i < num_stats; ++i) {
    std::string key;
    reader >> key;
    Tensor value = ::read_tensor(reader, device);
    if (with_stats) {
      stats.emplace(std::move(key), std::move(value));
    }
  }

  const Shape &shape_temp = value_temp.shape();
  Tensor grad_temp = operators::zeros<Tensor>(shape_temp, device);
  ::check_shape(value_temp, grad_temp);

  // Loading succeeded. Move all data to `this`.
  shape_ = shape_temp;
  device_ = &device;
  value_ = std::move(value_temp);
  grad_ = std::move(grad_temp);
  stats_ = std::move(stats);
}

void Parameter::save_inner(msgpack::Writer &writer, bool with_stats) const {
  ::write_tensor(value_, writer);

  if (with_stats) {
    if (stats_.size() > 0xffffffffull) {
      THROW_ERROR(
          "Could not store more than 2^32 - 1 stats in one parameter file.");
    }
    writer << static_cast<std::uint32_t>(stats_.size());
    for (const auto &kv : stats_) {
      writer << kv.first;
      ::write_tensor(kv.second, writer);
    }
  } else {
    writer << std::uint32_t(0);
  }
}

void Parameter::load(const string &path, bool with_stats, Device &device) {
  std::ifstream ifs(path);
  if (!ifs.is_open()) {
    THROW_ERROR("Could not open file: " << path);
  }
  msgpack::Reader reader(ifs);

  std::uint32_t major, minor;
  reader >> major >> minor;
  FileFormat::check_version(major, minor);

  std::uint32_t datatype;
  reader >> datatype;
  FileFormat::check_datatype(FileFormat::DataType::PARAMETER, datatype);

  load_inner(reader, with_stats, device);
}

void Parameter::save(const string &path, bool with_stats) const  {
  if (!valid()) THROW_ERROR("Attempted to save an invalid Parameter object.");

  std::ofstream ofs(path);
  if (!ofs.is_open()) {
    THROW_ERROR("Could not open file: " << path);
  }
  msgpack::Writer writer(ofs);

  writer << FileFormat::CurrentVersion::MAJOR;
  writer << FileFormat::CurrentVersion::MINOR;
  writer << static_cast<std::uint32_t>(FileFormat::DataType::PARAMETER);

  save_inner(writer, with_stats);
}

void Parameter::reset_gradient() {
  if (!valid()) THROW_ERROR("Invalid parameter.");
  grad_.reset(0);
}

void Parameter::add_stats(const string &name, const Shape &shape) {
  if (!valid()) THROW_ERROR("Invalid parameter.");
  if (has_stats(name)) {
    THROW_ERROR("Statistics with name `" << name << "` already exists.");
  }
  stats_.emplace(
      std::make_pair(name, operators::zeros<Tensor>(shape, *device_)));
}

}  // namespace primitiv
