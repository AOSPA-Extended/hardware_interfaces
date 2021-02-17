/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Fingerprint.h"
#include "Session.h"

namespace aidl::android::hardware::biometrics::fingerprint {
namespace {

constexpr int SENSOR_ID = 1;
constexpr common::SensorStrength SENSOR_STRENGTH = common::SensorStrength::STRONG;
constexpr int MAX_ENROLLMENTS_PER_USER = 5;
constexpr FingerprintSensorType SENSOR_TYPE = FingerprintSensorType::REAR;
constexpr bool SUPPORTS_NAVIGATION_GESTURES = true;
constexpr char HW_DEVICE_NAME[] = "fingerprintSensor";
constexpr char HW_VERSION[] = "vendor/model/revision";
constexpr char FW_VERSION[] = "1.01";
constexpr char SERIAL_NUMBER[] = "00000001";

}  // namespace

Fingerprint::Fingerprint() {}

ndk::ScopedAStatus Fingerprint::getSensorProps(std::vector<SensorProps>* out) {
    std::vector<common::HardwareInfo> hardwareInfos = {
            {HW_DEVICE_NAME, HW_VERSION, FW_VERSION, SERIAL_NUMBER}};

    common::CommonProps commonProps = {SENSOR_ID, SENSOR_STRENGTH, MAX_ENROLLMENTS_PER_USER,
                                       hardwareInfos};

    SensorLocation sensorLocation = {0 /* displayId */, 0 /* sensorLocationX */,
                                     0 /* sensorLocationY */, 0 /* sensorRadius */};

    *out = {{commonProps,
             SENSOR_TYPE,
             {sensorLocation},
             SUPPORTS_NAVIGATION_GESTURES,
             false /* supportsDetectInteraction */}};
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Fingerprint::createSession(int32_t /*sensorId*/, int32_t /*userId*/,
                                              const std::shared_ptr<ISessionCallback>& cb,
                                              std::shared_ptr<ISession>* out) {
    *out = SharedRefBase::make<Session>(cb);
    return ndk::ScopedAStatus::ok();
}
}  // namespace aidl::android::hardware::biometrics::fingerprint
