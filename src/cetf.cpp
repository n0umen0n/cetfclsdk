#include <eosio/action.hpp>
#include <eosio/eosio.hpp>
#include <eosio/name.hpp>
#include <limits>
#include <map>
#include <numeric>
#include <string>
#include <token/token.hpp>

#include "cetf.hpp"

using namespace cetf_contract;
using namespace cetf_contract::errors;
namespace {

    // Some compile-time configuration
    const vector<name> admins{"dan"_n, "jseymour.gm"_n, "chkmacdonald"_n, "james.vr"_n};

}  // namespace

cetf_contract::cetf_contract(name receiver, name code, datastream<const char*> ds) : contract(receiver, code, ds) {}

/*
void cetf_contract::validate_symbol(const symbol& symbol)
{
    check(symbol.value == eden_symbol.value, "invalid symbol");
    check(symbol == eden_symbol, "symbol precision mismatch");
}

void cetf_contract::validate_quantity(const asset& quantity)
{
    check(quantity.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "quantity must be positive");
}

void cetf_contract::validate_memo(const string& memo)
{
    check(memo.size() <= 256, "memo has more than 256 bytes");
}

void cetf_contract::require_admin_auth()
{
    bool hasAuth = std::any_of(admins.begin(), admins.end(), [](auto& admin) { return has_auth(admin); });
    check(hasAuth, requiresAdmin.data());
}

EOSIO_ACTION_DISPATCHER(cetf_contract::actions)
*/
// clang-format off
EOSIO_ABIGEN(actions(cetf_contract::actions), 
    table("agreement"_n, cetf_contract::Agreement), 
    table("signatures"_n, cetf_contract::Signature),


    ricardian_clause("Fractal contract ricardian clause", cetf_contract::ricardian_clause)
)
// clang-format on
