/* Copyright 2020 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#include <string>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/java/src/main/native/op_resolver.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/model_builder.h"

namespace tflite {
bool RunWithRandomInputs(const std::string& filename) {
  std::unique_ptr<tflite::FlatBufferModel> model =
      tflite::FlatBufferModel::BuildFromFile(filename.c_str());

  // Build the interpreter
  std::unique_ptr<OpResolver> resolver = CreateOpResolver();
  std::unique_ptr<tflite::Interpreter> interpreter;
  if (tflite::InterpreterBuilder(*model, *resolver)(&interpreter) !=
      kTfLiteOk) {
    LOG(ERROR) << "Could not initialize interpreter for TFLite model.";
    return false;
  }

  // Resize input tensors, if desired.
  if (interpreter->AllocateTensors() != kTfLiteOk) {
    LOG(ERROR) << "Could not allocate tensor.";
    return false;
  }

  // Fill the random data.
  std::vector<std::vector<uint8_t>> sample;
  for (int tensor_idx : interpreter->inputs()) {
    auto tensor = interpreter->tensor(tensor_idx);
    std::vector<uint8_t> data(tensor->bytes);
    for (auto it = data.begin(); it != data.end(); ++it) {
      *it = random();
    }
    tensor->data.raw = reinterpret_cast<char*>(data.data());
    sample.push_back(data);
  }

  // Running inference.
  if (interpreter->Invoke() != kTfLiteOk) {
    LOG(ERROR) << "Failed to run the model.";
    return false;
  }
  return true;
}

TEST(SelectiveBuiltTest, AddModel) {
  std::string model = "third_party/tensorflow/lite/testdata/add.bin";
  EXPECT_THAT(RunWithRandomInputs(model), true);
}

TEST(SelectiveBuiltTest, LGTMModel) {
  std::string model = "third_party/tensorflow/lite/testdata/lstm.bin";
  EXPECT_THAT(RunWithRandomInputs(model), true);
}
}  // namespace tflite

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
