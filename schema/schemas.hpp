#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/name.hpp>
#include <string>

namespace cetf_contract {
    struct account {
        asset balance;

        uint64_t primary_key() const { return balance.symbol.code().raw(); }
    };

    struct currency_stats {
        asset supply;
        asset max_supply;
        name issuer;

        uint64_t primary_key() const { return supply.symbol.code().raw(); }
    };

    // Agreement-related
    struct Agreement {
        std::string agreement;
        uint8_t versionNr;
    };
    EOSIO_REFLECT(Agreement, agreement, versionNr);

    struct Signature {
        eosio::name signer;
        uint64_t primary_key() const { return signer.value; }
    };
    EOSIO_REFLECT(Signature, signer);

    // Token-related

}  // namespace cetf_contract