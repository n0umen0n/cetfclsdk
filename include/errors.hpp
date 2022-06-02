#pragma once

#include <string_view>

namespace eden_fractal {
    namespace errors {
        // Agreement-related
        constexpr std::string_view setAgreementAuth = "Talk to Dan Singjoy or James Mart, they currently control the agreement";
        constexpr std::string_view alreadySigned = "You already signed the agreement";
        constexpr std::string_view noAgreement = "No agreement has been added yet";
        constexpr std::string_view notSigned = "You haven't signed this agreement. Nothing to unsign";

        // Token-related
        constexpr std::string_view tokenAlreadyCreated = "Token already created";

    }  // namespace errors
}  // namespace eden_fractal