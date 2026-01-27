#ifndef INTERFACES_AC_H
#define INTERFACES_AC_H

#include <vector>
#include <optional>
#include "Event.h"

namespace Ac
{
    template<typename T>
    concept HasNestedTypeEnumValue = requires
    {
        { T::typeValue } -> std::convertible_to<typename T::Type>;
        typename T::Type;
        requires std::is_enum_v<typename T::Type>;
    };


    class IACCheckerBase 
    {
    public:
        virtual ~IACCheckerBase() = default;
        virtual std::optional<ACFlag> processEventBase(const ACEvent& event) = 0;
    };


    template<HasNestedTypeEnumValue EventT>
    class IACChecker : public IACCheckerBase
    {
    public:
        virtual std::optional<ACFlag> processEvent(const EventT& event) = 0;

        std::optional<ACFlag> processEventBase(const ACEvent& event) override
        {
            if (event.type != EventT::typeValue)
                return std::nullopt;

            return processEvent(static_cast<const EventT&>(event));
        }
    };

}
#endif