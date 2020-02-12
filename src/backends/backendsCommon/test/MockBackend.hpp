//
// Copyright © 2017 Arm Ltd. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#include "armnn/backends/profiling/IBackendProfiling.hpp"
#include "armnn/backends/profiling/IBackendProfilingContext.hpp"
#include "MockBackendId.hpp"

#include <LayerSupportCommon.hpp>
#include <armnn/backends/IBackendInternal.hpp>
#include <armnn/backends/OptimizationViews.hpp>
#include <backendsCommon/LayerSupportBase.hpp>
#include <armnn/backends/profiling/IBackendProfiling.hpp>
#include <backends/BackendProfiling.hpp>

namespace armnn
{

class MockBackendInitialiser
{
public:
    MockBackendInitialiser();
    ~MockBackendInitialiser();
};

class MockBackendProfilingService
{
public:
    // Getter for the singleton instance
    static MockBackendProfilingService& Instance()
    {
        static MockBackendProfilingService instance;
        return instance;
    }

    armnn::profiling::IBackendProfilingContext* GetContext()
    {
        return m_sharedContext.get();
    }

    void SetProfilingContextPtr(IBackendInternal::IBackendProfilingContextPtr& shared)
    {
        m_sharedContext = shared;
    }

private:
    IBackendInternal::IBackendProfilingContextPtr m_sharedContext;
};

class MockBackendProfilingContext : public profiling::IBackendProfilingContext
{
public:
    MockBackendProfilingContext(IBackendInternal::IBackendProfilingPtr& backendProfiling)
        : m_BackendProfiling(backendProfiling)
        , m_CapturePeriod(0)
    {}

    ~MockBackendProfilingContext() = default;

    IBackendInternal::IBackendProfilingPtr& GetBackendProfiling()
    {
        return m_BackendProfiling;
    }

    uint16_t RegisterCounters(uint16_t currentMaxGlobalCounterId)
    {
        std::unique_ptr<profiling::IRegisterBackendCounters> counterRegistrar =
            m_BackendProfiling->GetCounterRegistrationInterface(currentMaxGlobalCounterId);

            std::string categoryName("MockCounters");
            counterRegistrar->RegisterCategory(categoryName);
            uint16_t nextMaxGlobalCounterId = counterRegistrar->RegisterCounter(
                0, categoryName, 0, 0, 1.f, "Mock Counter One", "Some notional counter");

            nextMaxGlobalCounterId = counterRegistrar->RegisterCounter(
                1, categoryName, 0, 0, 1.f, "Mock Counter Two", "Another notional counter");

            std::string units("microseconds");
            nextMaxGlobalCounterId = counterRegistrar->RegisterCounter(
                2, categoryName, 0, 0, 1.f, "Mock MultiCore Counter", "A dummy four core counter", units, 4);
            return nextMaxGlobalCounterId;
    }

    Optional<std::string> ActivateCounters(uint32_t capturePeriod, const std::vector<uint16_t>& counterIds)
    {
        if ( capturePeriod == 0 || counterIds.size() == 0)
        {
            m_ActiveCounters.clear();
        }
        else if (capturePeriod == 15939u)
        {
            return armnn::Optional<std::string>("ActivateCounters example test error");
        }
        m_CapturePeriod = capturePeriod;
        m_ActiveCounters = counterIds;
        return armnn::Optional<std::string>();
    }

    std::vector<profiling::Timestamp> ReportCounterValues()
    {
        std::vector<profiling::CounterValue> counterValues;

        for(auto counterId : m_ActiveCounters)
        {
            counterValues.emplace_back(profiling::CounterValue{counterId, counterId+1u});
        }

        uint64_t timestamp = m_CapturePeriod;
        return  {profiling::Timestamp{timestamp, counterValues}};
    }

    void EnableProfiling(bool)
    {}

private:

    IBackendInternal::IBackendProfilingPtr& m_BackendProfiling;
    uint32_t m_CapturePeriod;
    std::vector<uint16_t> m_ActiveCounters;
};

class MockBackend : public IBackendInternal
{
public:
    MockBackend()  = default;
    ~MockBackend() = default;

    static const BackendId& GetIdStatic();
    const BackendId& GetId() const override
    {
        return GetIdStatic();
    }

    IBackendInternal::IMemoryManagerUniquePtr CreateMemoryManager() const override;

    IBackendInternal::IWorkloadFactoryPtr
        CreateWorkloadFactory(const IBackendInternal::IMemoryManagerSharedPtr& memoryManager = nullptr) const override;

    IBackendInternal::IBackendContextPtr CreateBackendContext(const IRuntime::CreationOptions&) const override;
    IBackendInternal::IBackendProfilingContextPtr
        CreateBackendProfilingContext(const IRuntime::CreationOptions& creationOptions,
                                      IBackendProfilingPtr& backendProfiling) override;

    IBackendInternal::Optimizations GetOptimizations() const override;
    IBackendInternal::ILayerSupportSharedPtr GetLayerSupport() const override;

    OptimizationViews OptimizeSubgraphView(const SubgraphView& subgraph) const override;
};

class MockLayerSupport : public LayerSupportBase
{
public:
    bool IsInputSupported(const TensorInfo& /*input*/,
                          Optional<std::string&> /*reasonIfUnsupported = EmptyOptional()*/) const override
    {
        return true;
    }

    bool IsOutputSupported(const TensorInfo& /*input*/,
                           Optional<std::string&> /*reasonIfUnsupported = EmptyOptional()*/) const override
    {
        return true;
    }

    bool IsAdditionSupported(const TensorInfo& /*input0*/,
                             const TensorInfo& /*input1*/,
                             const TensorInfo& /*output*/,
                             Optional<std::string&> /*reasonIfUnsupported = EmptyOptional()*/) const override
    {
        return true;
    }

    bool IsConvolution2dSupported(const TensorInfo& /*input*/,
                                  const TensorInfo& /*output*/,
                                  const Convolution2dDescriptor& /*descriptor*/,
                                  const TensorInfo& /*weights*/,
                                  const Optional<TensorInfo>& /*biases*/,
                                  Optional<std::string&> /*reasonIfUnsupported = EmptyOptional()*/) const override
    {
        return true;
    }
};

}    // namespace armnn
