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

void create(const name& issuer, const asset& maximum_supply)
{
    require_auth(_self);

    auto sym = maximum_supply.symbol;
    check(sym.is_valid(), "invalid symbol name");
    check(maximum_supply.is_valid(), "invalid supply");
    check(maximum_supply.amount > 0, "max-supply must be positive");

    stats statstable(_self, sym.code().raw());
    auto existing = statstable.find(sym.code().raw());
    check(existing == statstable.end(), "token with symbol already exists");

    statstable.emplace(_self, [&](auto& s) {
        s.supply.symbol = maximum_supply.symbol;
        s.max_supply = maximum_supply;
        s.issuer = issuer;
    });
}

void issue(const name& to, const asset& quantity, const string& memo)
{
    auto sym = quantity.symbol;
    check(sym.is_valid(), "invalid symbol name");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    stats statstable(_self, sym.code().raw());
    auto existing = statstable.find(sym.code().raw());
    check(existing != statstable.end(), "token with symbol does not exist, create token before issue");
    const auto& st = *existing;
    check(to == st.issuer, "tokens can only be issued to issuer account");

    require_auth(st.issuer);
    check(quantity.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "must issue positive quantity");

    check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
    check(quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify(st, same_payer, [&](auto& s) { s.supply += quantity; });

    add_balance(st.issuer, quantity, st.issuer);
}

void transfer(name from, name to, asset quantity, std::string memo)
{
    check(from != to, "cannot transfer to self");
    require_auth(from);
    check(is_account(to), "to account does not exist");
    auto sym = quantity.symbol.code();
    stats statstable(_self, sym.raw());
    const auto& st = statstable.get(sym.raw());

    require_recipient(from);
    require_recipient(to);

    check(quantity.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "must transfer positive quantity");
    check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    auto payer = has_auth(to) ? to : from;

    auto separator_pos = memo.find(':');

    if (to == get_self()) {
        //check(false, "This action will be activated when CETF distribution ends or latest on 31.04.2021");

        refund_tokens_back(from, to, quantity, memo);

        sub_balance(from, quantity);
        add_balance(to, quantity, payer);

        retire(quantity, memo);
    }

    //IN CASE SOMEBODY PURCHASES FROM DEFIBOX THEY GET 95% FROM THE PURCHASED AMOUNT 5% GOES TO CETF HOLDERS.
    if (from == "swap.defi"_n && memo != "Defibox: withdraw" && quantity.symbol == symbol("EOSETF", 4)) {
        feesadjust_def etffeestb(_self, _self.value);
        feesadjust feeitr;

        feeitr = etffeestb.get();

        refundratetb eostable(_self, _self.value);
        refundrate soloiter;
        soloiter = eostable.get();

        //ADD TO THE SINGLETON THAT CALCULATES HOW MUCH FEE WAS ACCUMULATED DURING A PERIOD

        feeitr.adjustcrtclm.amount += (quantity.amount * (1 - soloiter.rate));
        etffeestb.set(feeitr, _self);

        int64_t intadjquan = static_cast<double>(quantity.amount) * soloiter.rate;

        struct asset adjquantity = {int64_t(intadjquan), symbol("EOSETF", 4)};

        sub_balance(from, quantity);

        add_balance(to, adjquantity, payer);  //*feerate
    }

    else {
        sub_balance(from, quantity);
        add_balance(to, quantity, payer);
    }
}

//FUNCTION THAT ENABLES TO CLAIM FEES IF USER STAKED CETF.
//FEES CAN BE CLAIMED ANYTIME.
//FEES THAT ARE CLAIMED ARE FOR THE PREVIOUS PERIOD.
//IF USER DOES NOT CLAIM THE FEES HE PARTIALLY LOSES OUT, EVEN THOUGH HIS PART THAT WAS NOT CLAIMED IS CARRIED OVER TO THE NEXT PERIOD.

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
