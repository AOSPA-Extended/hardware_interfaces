/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef android_hardware_automotive_vehicle_V2_0_VehicleConnector_H_
#define android_hardware_automotive_vehicle_V2_0_VehicleConnector_H_

#include <vector>

#include <android/hardware/automotive/vehicle/2.0/types.h>

namespace android {
namespace hardware {
namespace automotive {
namespace vehicle {
namespace V2_0 {

/**
 *  This file defines the interface of client/server pair for HAL-vehicle
 *  communication. Vehicle HAL may use this interface to talk to the vehicle
 *  regardless of the underlying communication channels.
 */

/**
 *  Vehicle HAL talks to the vehicle through a client, instead of accessing
 *  the car bus directly, to give us more flexibility on the implementation.
 *  Android OS do not need direct access to the vehicle, and the communication
 *  channel is also customizable.
 *
 *  Client lives on the Android (HAL) side to talk to the vehicle
 */
class IVehicleClient {
  public:
    IVehicleClient() = default;

    IVehicleClient(const IVehicleClient&) = delete;

    IVehicleClient& operator=(const IVehicleClient&) = delete;

    IVehicleClient(IVehicleClient&&) = default;

    virtual ~IVehicleClient() = default;

    // Get configuration of all properties from server
    virtual std::vector<VehiclePropConfig> getAllPropertyConfig() const = 0;

    // Send the set property request to server
    virtual StatusCode setProperty(const VehiclePropValue& value) = 0;

    // Receive a new property value from server
    virtual void onPropertyValue(const VehiclePropValue& value) = 0;
};

/**
 *  Server lives on the vehicle side to talk to Android HAL
 */
class IVehicleServer {
  public:
    IVehicleServer() = default;

    IVehicleServer(const IVehicleServer&) = delete;

    IVehicleServer& operator=(const IVehicleServer&) = delete;

    IVehicleServer(IVehicleServer&&) = default;

    virtual ~IVehicleServer() = default;

    // Receive the get property configuration request from HAL.
    // Return a list of all property config
    virtual std::vector<VehiclePropConfig> onGetAllPropertyConfig() const = 0;

    // Receive the set property request from HAL.
    // Process the setting and return the status code
    virtual StatusCode onSetProperty(const VehiclePropValue& value) = 0;

    // Receive a new property value from car (via direct connection to the car bus or the emulator)
    // and forward the value to HAL
    virtual void onPropertyValueFromCar(const VehiclePropValue& value) = 0;
};

/**
 *  If Android has direct access to the vehicle, then the client and
 *  the server may act in passthrough mode to avoid extra IPC
 *
 *  Template is used here for spliting the logic of operating Android objects (VehicleClientType),
 *  talking to cars (VehicleServerType) and the commucation between client and server (passthrough
 *  mode in this case), so that we can easily combine different parts together without duplicating
 *  codes (for example, in Google VHAL, the server talks to the fake car in the same way no matter
 *  if it is on top of passthrough connector or VSOCK or any other communication channels between
 *  client and server)
 *
 *  The alternative may be factoring the common logic of every operations for both client and
 *  server. Which is not always the case. Making sure different non-template connectors calling
 *  the same method is hard, especially when the engineer maintaining the code may not be aware
 *  of it when making changes. Template is a clean and easy way to solve this problem in this
 *  case.
 */
template <typename VehicleClientType, typename VehicleServerType>
class IPassThroughConnector : public VehicleClientType, public VehicleServerType {
    static_assert(std::is_base_of_v<IVehicleClient, VehicleClientType>);
    static_assert(std::is_base_of_v<IVehicleServer, VehicleServerType>);

  public:
    std::vector<VehiclePropConfig> getAllPropertyConfig() const override {
        return this->onGetAllPropertyConfig();
    }

    StatusCode setProperty(const VehiclePropValue& value) override {
        return this->onSetProperty(value);
    }

    void onPropertyValueFromCar(const VehiclePropValue& value) override {
        return this->onPropertyValue(value);
    }

    // To be implemented:
    // virtual std::vector<VehiclePropConfig> onGetAllPropertyConfig() = 0;
    // virtual void onPropertyValue(const VehiclePropValue& value) = 0;
    // virtual StatusCode onSetProperty(const VehiclePropValue& value) = 0;
};

}  // namespace V2_0
}  // namespace vehicle
}  // namespace automotive
}  // namespace hardware
}  // namespace android

#endif  // android_hardware_automotive_vehicle_V2_0_VehicleConnector_H_
