#pragma once

#include <optional>
#include <tuple>
#include <vector>

namespace viennaps {

using namespace viennacore;

template <typename NumericType, typename... FeedbackType> class ValueEstimator {
public:
  using SizeType = size_t;
  using ItemType = std::vector<NumericType>;
  using VectorType = std::vector<ItemType>;
  using VectorPtr = SmartPointer<std::vector<ItemType>>;
  using ConstPtr = SmartPointer<const std::vector<ItemType>>;

protected:
  SizeType inputDim{0};
  SizeType outputDim{0};

  ConstPtr data = nullptr;

  bool dataChanged = true;

public:
  void setDataDimensions(SizeType passedInputDim, SizeType passedOutputDim) {
    inputDim = passedInputDim;
    outputDim = passedOutputDim;
  }

  void setData(ConstPtr passedData) {
    data = passedData;
    dataChanged = true;
  }

  virtual bool initialize() { return true; }

  virtual std::optional<std::tuple<ItemType, FeedbackType...>>
  estimate(const ItemType &input) = 0;
};

} // namespace viennaps
