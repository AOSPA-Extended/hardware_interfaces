/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "FakeVehicleHardware.h"

#include <DefaultConfig.h>
#include <VehicleHalTypes.h>
#include <VehicleUtils.h>
#include <utils/Log.h>
#include <utils/SystemClock.h>

#include <vector>

namespace android {
namespace hardware {
namespace automotive {
namespace vehicle {

using ::aidl::android::hardware::automotive::vehicle::GetValueRequest;
using ::aidl::android::hardware::automotive::vehicle::GetValueResult;
using ::aidl::android::hardware::automotive::vehicle::RawPropValues;
using ::aidl::android::hardware::automotive::vehicle::SetValueRequest;
using ::aidl::android::hardware::automotive::vehicle::SetValueResult;
using ::aidl::android::hardware::automotive::vehicle::StatusCode;
using ::aidl::android::hardware::automotive::vehicle::VehiclePropConfig;
using ::aidl::android::hardware::automotive::vehicle::VehiclePropertyStatus;
using ::aidl::android::hardware::automotive::vehicle::VehiclePropValue;

void FakeVehicleHardware::storePropInitialValue(const defaultconfig::ConfigDeclaration& config) {
    const VehiclePropConfig& vehiclePropConfig = config.config;
    int propId = vehiclePropConfig.prop;

    // A global property will have only a single area
    bool globalProp = isGlobalProp(propId);
    size_t numAreas = globalProp ? 1 : vehiclePropConfig.areaConfigs.size();

    for (size_t i = 0; i < numAreas; i++) {
        int32_t curArea = globalProp ? 0 : vehiclePropConfig.areaConfigs[i].areaId;

        // Create a separate instance for each individual zone
        VehiclePropValue prop = {
                .areaId = curArea,
                .prop = propId,
                .timestamp = elapsedRealtimeNano(),
        };

        if (config.initialAreaValues.empty()) {
            if (config.initialValue == RawPropValues{}) {
                // Skip empty initial values.
                continue;
            }
            prop.value = config.initialValue;
        } else if (auto valueForAreaIt = config.initialAreaValues.find(curArea);
                   valueForAreaIt != config.initialAreaValues.end()) {
            prop.value = valueForAreaIt->second;
        } else {
            ALOGW("failed to get default value for prop 0x%x area 0x%x", propId, curArea);
            continue;
        }

        auto result =
                mServerSidePropStore->writeValue(mValuePool->obtain(prop), /*updateStatus=*/true);
        if (!result.ok()) {
            ALOGE("failed to write default config value, error: %s",
                  result.error().message().c_str());
        }
    }
}

FakeVehicleHardware::FakeVehicleHardware() {
    mValuePool = std::make_shared<VehiclePropValuePool>();
    init(mValuePool);
}

FakeVehicleHardware::FakeVehicleHardware(std::unique_ptr<VehiclePropValuePool> valuePool)
    : mValuePool(std::move(valuePool)) {
    init(mValuePool);
}

void FakeVehicleHardware::init(std::shared_ptr<VehiclePropValuePool> valuePool) {
    mServerSidePropStore.reset(new VehiclePropertyStore(valuePool));
    for (auto& it : defaultconfig::getDefaultConfigs()) {
        VehiclePropConfig cfg = it.config;
        mServerSidePropStore->registerProperty(cfg);
        storePropInitialValue(it);
    }

    mServerSidePropStore->setOnValueChangeCallback(
            [this](const VehiclePropValue& value) { return onValueChangeCallback(value); });
}

std::vector<VehiclePropConfig> FakeVehicleHardware::getAllPropertyConfigs() const {
    return mServerSidePropStore->getAllConfigs();
}

StatusCode FakeVehicleHardware::setValues(FakeVehicleHardware::SetValuesCallback&&,
                                          const std::vector<SetValueRequest>&) {
    // TODO(b/201830716): Implement this.
    return StatusCode::OK;
}

StatusCode FakeVehicleHardware::getValues(FakeVehicleHardware::GetValuesCallback&&,
                                          const std::vector<GetValueRequest>&) const {
    // TODO(b/201830716): Implement this.
    return StatusCode::OK;
}

DumpResult FakeVehicleHardware::dump(const std::vector<std::string>&) {
    DumpResult result;
    // TODO(b/201830716): Implement this.
    return result;
}

StatusCode FakeVehicleHardware::checkHealth() {
    // TODO(b/201830716): Implement this.
    return StatusCode::OK;
}

void FakeVehicleHardware::registerOnPropertyChangeEvent(OnPropertyChangeCallback&& callback) {
    std::lock_guard<std::mutex> lockGuard(mCallbackLock);
    mOnPropertyChangeCallback = std::move(callback);
}

void FakeVehicleHardware::registerOnPropertySetErrorEvent(OnPropertySetErrorCallback&& callback) {
    std::lock_guard<std::mutex> lockGuard(mCallbackLock);
    mOnPropertySetErrorCallback = std::move(callback);
}

void FakeVehicleHardware::onValueChangeCallback(const VehiclePropValue& value) {
    std::lock_guard<std::mutex> lockGuard(mCallbackLock);
    if (mOnPropertyChangeCallback != nullptr) {
        std::vector<VehiclePropValue> updatedValues;
        updatedValues.push_back(value);
        mOnPropertyChangeCallback(std::move(updatedValues));
    }
}

}  // namespace vehicle
}  // namespace automotive
}  // namespace hardware
}  // namespace android
