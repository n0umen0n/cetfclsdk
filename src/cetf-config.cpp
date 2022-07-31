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

//CONFIG ACTIONS TO BE USED IN CASE OF BUGS

//SET HOW MUCH TOTAL CETF AND BOXAUJ STAKED
void cetf_contract::settotstkd(asset totstketfbx, asset totstkcetf)
{
    require_auth(_self);
    //
    totstk_def totalstktbl(_self, _self.value);
    totstk newstats;

    if (!totalstktbl.exists()) {
        totalstktbl.set(newstats, _self);
    }
    else {
        newstats = totalstktbl.get();
    }
    newstats.totstketf = totstketfbx;
    newstats.totalstaked = totstkcetf;

    totalstktbl.set(newstats, _self);
}

//SET WHEN DIVIDEND DISTRIBUTION PERIOD STARTS AND WHICH PERIOD IS IT

void cetf_contract::setdivper(uint64_t claimperiod)
{
    require_auth(_self);
    //
    divperiod_def divpertb(_self, _self.value);
    divperiod divperiter;

    if (!divpertb.exists()) {
        divpertb.set(divperiter, _self);
    }
    else {
        divperiter = divpertb.get();
    }
    divperiter.periodstart = current_time_point();
    divperiter.claimperiod = claimperiod;

    divpertb.set(divperiter, _self);
}

//SET HOW FREQUENTLY CAN DIVIDENDS BE CLAIMED
void cetf_contract::setdivperfrq(int64_t claimfreq)
{
    require_auth(_self);
    //
    divperiodfrq_def divperfqtb(_self, _self.value);
    clmperfreq divperfrqit;

    if (!divperfqtb.exists()) {
        divperfqtb.set(divperfrqit, _self);
    }
    else {
        divperfrqit = divperfqtb.get();
    }
    divperfrqit.periodfreq = claimfreq;

    divperfqtb.set(divperfrqit, _self);
}

//SET HOW MUCH FEES HAVE BEEN ACCUMULATED (EOSETF)
void cetf_contract::settotfeeamt(asset quantity)
{
    require_auth(_self);
    //
    etffees_def totfeestb(_self, _self.value);
    etffees totfeeiter;

    if (!totfeestb.exists()) {
        totfeestb.set(totfeeiter, _self);
    }
    else {
        totfeeiter = totfeestb.get();
    }
    totfeeiter.totalfees = quantity;
    totfeestb.set(totfeeiter, _self);
}

//SET BY HOW MUCH SHOULD THE TOTAL FEES AMOUNT BE ADJUSTED (EOSETF) DURING THE CLAIM PERIOD adjustcrtclm
//increases if somebody creates EOSETF and decreases if somebody claims the amount. at the end of the claiming period totalfees - adjustcrtclm
void cetf_contract::seteosetfadj(asset quantity)
{
    require_auth(_self);

    feesadjust_def etffeestb(_self, _self.value);
    feesadjust feeitr;

    if (!etffeestb.exists()) {
        etffeestb.set(feeitr, _self);
    }
    else {
        feeitr = etffeestb.get();
    }
    feeitr.adjustcrtclm = quantity;
    etffeestb.set(feeitr, _self);
}

//SET FEE RATE. 0.95. RATE DETERMINES WHAT IS THE FEE TO CREATE OR BUY EOSETF
void cetf_contract::setrefundrate(float rate)
{
    require_auth(_self);

    refundratetb eostable(_self, _self.value);
    refundrate soloiter;

    if (!eostable.exists()) {
        eostable.set(soloiter, _self);
    }
    else {
        soloiter = eostable.get();
    }
    soloiter.rate = rate;
    eostable.set(soloiter, _self);
}

//SET WHAT IS THE TOTAL WORTH OF TOKENS IN THE FUND
void cetf_contract::seteosworth(double eosworth)
{
    require_auth(_self);

    totleostab eostable(_self, _self.value);
    totaleosworth soloiter;

    if (!eostable.exists()) {
        eostable.set(soloiter, _self);
    }
    else {
        soloiter = eostable.get();
    }
    soloiter.eosworth = eosworth;
    eostable.set(soloiter, _self);
}

//FOR TESTING, BUT MAYBE GOOD TO HAVE JUST IN CASE THERE'S A BUG.
void modddtokens(vector<double> tokeninfund, vector<asset> minamount, vector<symbol> token)

{
    require_auth(_self);

    for (size_t i = 0; i < tokeninfund.size(); ++i)

    {
        rebalontb rebaltab(_self, _self.value);
        auto existing = rebaltab.find(token[i].code().raw());
         

            const auto& st
            = *existing;

                    rebaltab.modify(
            st, name("consortiumtt"), [&]( auto& s ) {
                s.tokeninfund = tokeninfund[i];
                s.minamount = minamount[i];
            });
    }
}

//ERASING THE ROW OF TOKENS THE USER HAS SENT IN
void usertok(name from)

{
    require_auth(_self);

    useritokans input(get_self(), from.value);

    for (auto iter = input.begin(); iter != input.end();)

    {
        input.erase(iter++);
    }
}

// VAATA MIDA VITTU SIIN TOIMUB
void deltoken(vector<symbol> token, vector<uint64_t> totalvote, name community, int64_t pollkey, symbol sym)

{
    require_auth(_self);

    rebalontb rebaltab(_self, _self.value);
    auto existing = rebaltab.find(sym.code().raw());
      rebaltab.erase(existing);

    portftb pollstbl(_self, community.value);

    auto pollsrow = pollstbl.find(pollkey);

    pollstbl.modify(pollsrow, _self, [&](auto& contract) {
        contract.answers = token;
        contract.totalvote = totalvote;
    });
}

void deltokoncet(symbol sym)

{
    require_auth(_self);

    rebalontb rebaltab(_self, _self.value);
    auto existing = rebaltab.find(sym.code().raw());
      rebaltab.erase(existing);
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
