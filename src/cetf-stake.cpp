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

//STAKE CETF IN ORDER TO RECEIVE FEES ACCUMULATED FOR CREATING EOSETF
//EACH STAKE HAS SEPARATE ROW
void stakecetf(name user, asset quantity, uint64_t id)
{
    require_auth(user);

    auto sym = quantity.symbol.code();
    auto symcetf = symbol("CETF", 4);

    check(quantity.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "must stake positive quantity");
    check(quantity.symbol == symcetf, "Only possible to stake CETF.");

    accounts from_acnts(_self, user.value);
    const auto& from = from_acnts.get(quantity.symbol.code().raw(), "no balance object found");

    check(from.balance.amount >= quantity.amount, "Staking more CETF than you have.");

    divperiod_def divpertb(_self, _self.value);
    divperiod divperiter;
    divperiter = divpertb.get();

    perzonstkd personstktbl(_self, user.value);
    auto userrow = personstktbl.find(id);
    if (userrow == personstktbl.end() )
         
        {
            personstktbl.emplace(_self, [&](auto& s) {
                s.id = id;
                s.staked = quantity;
                s.staketime = current_time_point();
                s.stakeperiod = divperiter.claimperiod;
                   
            });
        }
    if (userrow != personstktbl.end() )
          { check(false, "This ID already in use, please try staking again."); }

for
     (auto iter = personstktbl.begin(); iter != personstktbl.end(); iter++)
    {
        perstotlskd perstottb(_self, _self.value);
        auto totrow = perstottb.find(user.value);

        if (totrow == perstottb.end() )
             
            {
                perstottb.emplace(_self, [&](auto& s) {
                    s.indtotstaked = iter->staked;
                    s.staker = user;
                });
            }
        if (totrow != perstottb.end() )
             
            {
                perstottb.modify(
                    totrow, name("consortiumtt"), [&]( auto& s ) {
                        s.indtotstaked += iter->staked;
                           
                    });
            }
    }

perstotlskd indtotstk(_self, _self.value);
const auto & pede = indtotstk.get(user.value, "Individual has not staked." );

check(from.balance.amount >= pede.indtotstaked.amount, "Trying to stake more than available CETF.");

totstk_def totalstktbl(_self, _self.value);
totstk newstats;

newstats = totalstktbl.get();
newstats.totalstaked.amount += quantity.amount;
totalstktbl.set(newstats, _self);

auto totalrow = indtotstk.find(user.value);
indtotstk.modify(
    totalrow, name("consortiumtt"), [&]( auto& s ) {
        s.indtotstaked.amount = 0;
           
    });
}

void unstakecetf(name user, vector<asset> quantity, vector<uint64_t> id, name clmspecifier)
{
    require_auth(user);

    check(clmspecifier == "eosetfeosetf"_n, "Wrong claiming specifier");
    claimtimetb claimtab(_self, clmspecifier.value);
    const auto& claimiter = claimtab.get(user.value, "Claim at least once to unstake.");

    divperiod_def divpertb(_self, _self.value);
    divperiod divperiter;
    divperiter = divpertb.get();

    check(claimiter.claimperiod != divperiter.claimperiod, "Please don't claim next period, then you will be able to unstake.");

      for (int i = 0; i < quantity.size(); i++)
    {
        auto sym = quantity[i].symbol.code();
        stats statstable(_self, sym.raw());
        const auto& st = statstable.get(sym.raw());

        check(quantity[i].is_valid(), "invalid quantity");
        check(quantity[i].amount > 0, "must ustake positive quantity");
        check(quantity[i].symbol == st.supply.symbol, "symbol precision mismatch while staking");

        accounts from_acnts(_self, user.value);
        const auto& from = from_acnts.get(quantity[i].symbol.code().raw(), "no balance object found");

        perzonstkd personstktbl(_self, user.value);

        auto userrow = personstktbl.find(id[i]);

        const auto & iterone = personstktbl.get(id[i], "No such staking ID(1)." );

        check(iterone.staked.amount >= quantity[i].amount, "Unstaking too much CETF.");

        personstktbl.modify(
            userrow, name("consortiumtt"), [&]( auto& s ) {
                s.staked.amount -= quantity[i].amount;
                   
            });

        const auto & itertwo = personstktbl.get(id[i], "No such staking ID(2)." );

        if (itertwo.staked.amount == 0) {
            personstktbl.erase(userrow);
        }

        totstk_def totalstktbl(_self, _self.value);
        totstk newstats;

        newstats = totalstktbl.get();

        newstats.totalstaked.amount -= quantity[i].amount;
        totalstktbl.set(newstats, _self);
    }
}

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
