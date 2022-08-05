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

//POSSIBILITY TO PAUSE CREATION AND REDEMPTION IN CASE OF BUG / EMERGENCY
void pause(bool ispaused)
{
    require_auth(_self);

    pausetab pausetable(_self, _self.value);
    pausetabla soloiter;
    if (!pausetable.exists()) {
        pausetable.set(soloiter, _self);
    }
    else {
        soloiter = pausetable.get();
    }
    soloiter.ispaused = ispaused;
    pausetable.set(soloiter, _self);
}

//Private function that checks whether creation and redemption if EOESETF is currently halted.
void pauseornot()
{
    pausetab pauztab(_self, _self.value);
    pausetabla iter;

    iter = pauztab.get();
    //0 means pause.
    check(iter.ispaused, "Creation and redemption is currently halted.");
}

//SAVES NEW TOKEN BALANCE (USED IN NEXT REBALANCING)
void adjusttok(name contract, symbol token, int64_t decimals, double tokenpercnew)
{
    require_auth(_self);

    //KUI KÕIK SOLD SIIS TA FJUKOF KUNA SEE L2heb nulli ehk siis ei saa querida midagi.

    rebalontb rebaltab(get_self(), _self.value);
    auto iterkolm = rebaltab.find( token.code().raw() );

    if (tokenpercnew != 0) {
        accounts from_acnts(contract, _self.value);
        const auto& from = from_acnts.get(token.code().raw(), "Fjukof");

        //NO need for double actually, UINT would be more precise.
        double afterbuyingamt = static_cast<double>(from.balance.amount) / decimals;

        rebaltab.modify(
            iterkolm, name("consortiumtt"), [&]( auto& s ) {               s.tokeninfund    = afterbuyingamt; });
    }

    else {
        rebaltab.modify(
            iterkolm, name("consortiumtt"), [&]( auto& s ) {               s.tokeninfund    = 0; });
    }
}

void createetf(name from, asset reward)
{
    action(permission_level{get_self(), "active"_n}, "consortiumtt"_n, "issuetoken"_n, std::make_tuple(from, reward)).send();
};

void send(name from, name to, asset quantity, std::string memo, name contract)
{
    action(permission_level{get_self(), "active"_n}, contract, "transfer"_n, std::make_tuple(from, to, quantity, memo)).send();
};

void adjusttokk(name contract, symbol token, int64_t decimals, double tokenpercnew)
{
    action(permission_level{get_self(), "active"_n}, _self, "adjusttok"_n, std::make_tuple(contract, token, decimals, tokenpercnew)).send();
};

void rebalancetwoin(vector<symbol> answers)
{
    action(permission_level{get_self(), "active"_n}, _self, "rebalancetwo"_n, std::make_tuple(answers)).send();
};
}
;

EOSIO_ABIGEN(actions(cetf_contract::actions),
             table("agreement"_n, cetf_contract::Agreement),
             table("signatures"_n, cetf_contract::Signature),

             ricardian_clause("Fractal contract ricardian clause", cetf_contract::ricardian_clause))
// clang-format on
