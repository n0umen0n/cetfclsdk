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

//FUNCTION THAT ENABLES TO CLAIM FEES IF USER STAKED CETF.
//FEES CAN BE CLAIMED ANYTIME.
//FEES THAT ARE CLAIMED ARE FOR THE PREVIOUS PERIOD.
//IF USER DOES NOT CLAIM THE FEES HE PARTIALLY LOSES OUT, EVEN THOUGH HIS PART THAT WAS NOT CLAIMED IS CARRIED OVER TO THE NEXT PERIOD.
void getdiv(name user, name clmspecifier)

{
    require_auth(user);

    //GET WHEN CURRENT PERIOD STARTED AND OTHER STATS
    divperiod_def divpertb(_self, _self.value);
    divperiod divperiter;
    divperiter = divpertb.get();

    divperiodfrq_def divperfqtb(_self, _self.value);
    clmperfreq divperfrqit;
    divperfrqit = divperfqtb.get();

    etffees_def etffeestb(_self, _self.value);
    etffees feeitr;
    feeitr = etffeestb.get();

    feesadjust_def etffeestbadj(_self, _self.value);
    feesadjust feeitradj;
    feeitradj = etffeestbadj.get();

    perzonstkd personstktbl(_self, user.value);
    //CALCULATE HOW MUCH IN TOTAL USER HAS STAKED.
 for (auto iter = personstktbl.begin(); iter != personstktbl.end(); iter++)
{
    //if (iter->staketime + divperfrqit.periodfreq  < current_time_point() && divperiter.claimperiod != iter->stakeperiod) {

    // one period has lapsed
    if (iter->staketime + divperfrqit.periodfreq < current_time_point()) {
        perstotlskd perstottb(_self, _self.value);
        auto totrow = perstottb.find(user.value);

        if (totrow == perstottb.end() )
             
            {
                perstottb.emplace(_self, [&](auto& s) { s.indtotstaked = iter->staked; });
            }
        if (totrow != perstottb.end() )
             
            {
                perstottb.modify(
                    totrow, name("consortiumtt"), [&]( auto& s ) {
                        s.indtotstaked += iter->staked;
                           
                    });
            }
    }
}
//LOOP END CALCULATE HOW MUCH IN TOTAL USER HAS STAKED.

check(clmspecifier == "eosetfeosetf"_n, "Wrong claiming specifier");

claimtimetb claimtab(_self, clmspecifier.value);
auto claimrow = claimtab.find(user.value);

//GET THE PERC BASED ON HOW MUCH STAKED
totstk_def totalstktbl(_self, _self.value);
totstk newstats;

newstats = totalstktbl.get();

//Multiple times declared, should be put on top?
perstotlskd indtotstk(_self, _self.value);
const auto& iter = indtotstk.get(user.value, "Individual has not staked, or stake has not matured.");

check(iter.indtotstaked.amount != 0, "You have nothing staked CETF.");

double percgets = static_cast<double>(iter.indtotstaked.amount) / newstats.totalstaked.amount;

//CHECK IF PERIOD IS STILL ON OR NEW HAS TO START
if (divperiter.periodstart + divperfrqit.periodfreq > current_time_point()) {
    if (claimrow == claimtab.end() )
         
        {
            claimtab.emplace(_self, [&](auto& s) {
                s.claimperiod = divperiter.claimperiod;
                s.user = user;
            });
        }
    //NEW PERIOD NOT STARTED AND USER TRIES TO CLAIM AGAIN.
    if (claimrow != claimtab.end() )
         
        {
            const auto& claimiter = claimtab.get(user.value, "User has not staked nah");

            check(claimiter.claimperiod != divperiter.claimperiod, "New period not started yet.");

            claimtab.modify(
                claimrow, name("consortiumtt"), [&]( auto& s ) {
                    s.claimperiod = divperiter.claimperiod;
                       
                });
        }

    double divsint = (feeitr.totalfees.amount * percgets);

    struct asset divs = {int64_t(divsint), symbol("EOSETF", 4)};

    createetf(user, divs);

    //ADJUSTCRTCLM ADJUSTS TOTAL FEES WHEN NEW PERIOD STARTS.NUMBER IS POSITIVE IF MORE WAS CREATED EOSETF THAT CLAIMED.
    feeitradj.adjustcrtclm.amount -= divs.amount;
    etffeestbadj.set(feeitradj, _self);
}

//TRIGGERING START OF A NEW PERIOD
else {
    divperiter.periodstart = current_time_point();
    divperiter.claimperiod += 1;
    divpertb.set(divperiter, _self);

    if (claimrow == claimtab.end() )
         
        {
            claimtab.emplace(_self, [&](auto& s) {
                s.claimperiod = divperiter.claimperiod;
                s.user = user;
            });
        }
    if (claimrow != claimtab.end() )
         
        {
            claimtab.modify(
                claimrow, name("consortiumtt"), [&]( auto& s ) {
                    s.claimperiod = divperiter.claimperiod;
                       
                });
        }

    //HERE'S THE ADJUSTMENT
    feeitr.totalfees.amount += feeitradj.adjustcrtclm.amount;
    etffeestb.set(feeitr, _self);

    feeitr = etffeestb.get();
    double divsint = (feeitr.totalfees.amount * percgets);
    struct asset divs = {int64_t (divsint), symbol ("EOSETF", 4)};

    createetf(user, divs);

    //ADJUSTCRTCLM ADJUSTS TOTAL FEES WHEN NEW PERIOD STARTS.NUMBER IS POSITIVE IF MORE WAS CREATED EOSETF THAT CLAIMED.
    //feeitradj = etffeestbadj.get();

    /*
feeitr = etffeestb.get();
feeitr.totalfees.amount -= divsint;
etffeestb.set(feeitr, _self);

//PERIOD STARTS WITH 0
feeitradj.adjustcrtclm.amount = 0;
etffeestbadj.set(feeitradj, _self);

*/
    feeitradj.adjustcrtclm.amount = (0 - divsint);
    etffeestbadj.set(feeitradj, _self);
}

feeitr = etffeestb.get();
check(feeitr.totalfees.amount >= 0, "Total fees to be distr fell below 0.");

//THIS IS TABLE THAT TRACKS HOW MUCH INDIVIDUAL HAS STAKED.
auto totalrow = indtotstk.find(user.value);
indtotstk.modify(
    totalrow, name("consortiumtt"), [&]( auto& s ) {
        s.indtotstaked.amount = 0;
           
    });
}

//MONSTER FUNCTION
void rebalance(name user, uint64_t pollkey, name community)

{
    require_auth( user );

    //SET CAPTURED EOS TO ZERO, IF NEW REBALANCE STARTS IT HAS TO BE ZERO.
    eoscaptura litatb(_self, _self.value);
    eoscapt litaitr;

    litaitr = litatb.get();
    litaitr.capturedeos.amount = 0;
    litatb.set(litaitr, _self);

    //CHECK IF USER IS FUND MANAGER

    approvedaccs whitetbl("consortiumlv"_n, community.value);
    auto whiterow = whitetbl.find(user.value);
    check(whiterow != whitetbl.end(), "Account not whitelisted.");

    nrofmngtab managtbl("consortiumlv"_n, "consortiumlv"_n.value);

    const auto & itermang = managtbl.get(community.value, "No manager nr table found." );

    kysimustes pollstbl("consortiumlv"_n, community.value);
    //portftb pollstbl("consortiumlv"_n, community.value);

    const auto & iter = pollstbl.get( pollkey, "No poll found with such key" );

    //THIS SHOULD BE ADDED IF 2/3 HAVE TO VOTE IN ORDER TO REBALANCE
    /*
if (static_cast<double>(iter.nrofvoters) / itermang.nrofmanagers < 0.656)

{
check(false, "2/3 of managers have to vote in order to rebalnce.");
}
*/

    //SETTING THAT NOBODY HAS VOTED IN THE POLL.
    votersnulli(community, pollkey);

        //LOOP START THAT CALCULATES NEW PERCENTAGES
  for (int i = 0; i < iter.answers.size(); i++)
    {
        //CALCULATING THE NEW ALLOCATION OF TOKENS BASED ON THE VOTE RESULTS
           double newpercentage = static_cast<double>(iter.totalvote[i]) / iter.sumofallopt;

        check(newpercentage == 0 || newpercentage >= 0.01, "Min token allocation % is 1.");

            auto sym = iter.answers[i];
            rebalontb rebaltab(get_self(), _self.value);
            auto existing = rebaltab.find( sym.code().raw() );
              //SAVING NEW ALLOCATION PERCENTAGE
            rebaltab.modify(
            existing, name("consortiumtt"), [&]( auto& s ) {
                              s.tokenpercnew    = newpercentage;
                        
            });
                
         
    }
               //LOOP ENDED THAT CALCULATES NEW PERCENTAGES

        //SETTING TOTAL FUND WORTH TO 0, NEXT LOOP CALCULATES CURRENT VALUE
        totleostab eostable(_self, _self.value);
    totaleosworth soloiterr;

    if (eostable.exists()) {
        soloiterr = eostable.get();

        soloiterr.eosworth = 0;

        eostable.set(soloiterr, _self);
    }

    rebalontb rebaltab(get_self(), _self.value);

    //LOOP CALCULATING HOW MUCH TOKENS ARE WORTH IN EOS
           for (auto iter = rebaltab.begin(); iter != rebaltab.end(); iter++)
{
    pairs pairtab("swap.defi"_n, "swap.defi"_n.value);

    const auto & iterpair = pairtab.get(iter->pairid, "No row with such pairid" );

    //CHECK DUE TO HOW DEFIBOX TABLES ARE BUILT
    if
         (iterpair.reserve0.symbol == iter->token) 
        {
            double eosworth = iterpair.price0_last * iter->tokeninfund;

                        auto existing = rebaltab.find( iter->token.code().raw() );
            rebaltab.modify(
                existing, name("consortiumtt"), [&]( auto& s ) {
                                s.tokenwortheos    = eosworth;
                            
                });
        }

    //CHECK DUE TO HOW DEFIBOX TABLES ARE BUILT
    if
         (iterpair.reserve1.symbol == iter->token) 
        {
            double eosworth = iterpair.price1_last * iter->tokeninfund;

                        auto existing = rebaltab.find( iter->token.code().raw());
                        rebaltab.modify(
                existing, name("consortiumtt"), [&]( auto& s ) {
                                s.tokenwortheos    = eosworth;
                            
                });
        }
    //CALCULATING TOTAL EOS WORTH OF TOKENS IN FUND
    totleostab eostable(_self, _self.value);
    totaleosworth soloiter;
    soloiter = eostable.get();

    soloiter.eosworth += iter->tokenwortheos;

    eostable.set(soloiter, _self);
}
//END OF FIRST LOOP CALCULATING TOKEN WORTH IN EOS

//LOOP CALCULATING THE CURRENT PERCENTAGE OF TOKENS IN FUND 
 for (int i = 0; i < iter.answers.size(); i++)
{
     totleostab eostable(_self, _self.value);
     totaleosworth soloiter;
     soloiter = eostable.get();
      rebalontb reblatab(get_self(), _self.value);

    const auto & rebapiter = reblatab.get(iter.answers[i].code().raw(), "No pairid for such symbol" );

    double tokenperold = rebapiter.tokenwortheos / soloiter.eosworth;

    auto uus = reblatab.find( iter.answers[i].code().raw() );
    reblatab.modify(
        uus, name("consortiumtt"), [&]( auto& s ) {
                          s.tokenperold    = tokenperold;
             
        });
}

//LOOP THAT SELLS TOKENS THROUGH DEFIBOX
for (int i = 0; i < iter.answers.size(); i++) {
    rebalontb rbtab(get_self(), _self.value);

    const auto & rbaliter = rbtab.get(iter.answers[i].code().raw(), "No pairid for such symbol" );

    //SELLING TOKENS IF CURRENT PERCENTAGE IS LARGER THAN NEW
    if
         (rbaliter.tokenperold > rbaliter.tokenpercnew && rbaliter.tokenperold != 0) 
        {
            double diffpertosell = rbaliter.tokenperold - rbaliter.tokenpercnew;

            double perdiff = diffpertosell / rbaliter.tokenperold;

            double toselldoub = rbaliter.tokeninfund * perdiff;

            struct asset tosell = {int64_t (toselldoub * rbaliter.decimals), rbaliter.token};

            string memo = "swap,0," + rbaliter.strpairid;

            //ACTION THAT TRIGGERS SELLING
            send(_self, "swap.defi"_n, tosell, memo, rbaliter.contract);
              

                //SAVE AMOUNTS AFTER SELLING
                //INLINE ACTION NEEDED OTHERWISE send IS EXECUTED LAST AND THUS OLD BAlANCE IS SAVED
                adjusttokk(rbaliter.contract, rbaliter.token, rbaliter.decimals, rbaliter.tokenpercnew);
        }
}
//END LOOP THAT SELLS TOKENS THROUGH DEFIBOX

rebalancetwoin(iter.answers);

}  //END OF REBAL PART 1

[[eosio::action]] void rebalancetwo(vector<symbol> answers)

{
    //check (false, "pede");

    require_auth(_self);

    totalbuy_tab tottbb(_self, _self.value);
    totalbuy totiter;

    //LOOP THAT CALCULATES FOR HOW MUCH EOS WILL TOKENS BE BOUGHT AND SAVES THE AMOUNTS
    for (int i = 0; i < answers.size(); i++) {
        rebalontb rebaltab(get_self(), _self.value);

        const auto & rebaliter = rebaltab.get(answers[i].code().raw(), "No pairid for such symbol" );

        if
             (rebaliter.tokenperold < rebaliter.tokenpercnew && rebaliter.tokenperold != 0) 
            {
                const auto & rebit = rebaltab.get(answers[i].code().raw(), "No pairid for such symbol" );

                pairs pairtab("swap.defi"_n, "swap.defi"_n.value);

                const auto & iterpair = pairtab.get(rebit.pairid, "No row with such pairid" );

                double diffpertobuy = rebaliter.tokenpercnew - rebaliter.tokenperold;

                double perdiff = diffpertobuy / rebaliter.tokenperold;

                double eosworthtobuy = rebaliter.tokenwortheos * perdiff;

                //totalbuy_tab tottbb(_self, _self.value);
                //totalbuy totiter;

                if (!tottbb.exists()) {
                    tottbb.set(totiter, _self);
                }
                else {
                    totiter = tottbb.get();
                }
                totiter.amountbuy += eosworthtobuy;
                tottbb.set(totiter, _self);

                eosworthbuytb singblebuytb(_self, _self.value);
                const auto& indrow = singblebuytb.find(rebaliter.token.code().raw() );

                if (indrow == singblebuytb.end()) {
                    singblebuytb.emplace(_self, [&](auto& item) {
                        item.eosworthbuy = eosworthtobuy;
                        item.token = rebaliter.token;
                    });
                }
                else {
                    singblebuytb.modify(indrow, _self, [&](auto& contract) { contract.eosworthbuy = eosworthtobuy; });
                }
            }

        if
             (rebaliter.tokenperold == 0 && rebaliter.tokenpercnew != 0) 
            {
                const auto & rebit = rebaltab.get(answers[i].code().raw(), "No pairid for such symbol" );

                pairs pairtab("swap.defi"_n, "swap.defi"_n.value);

                const auto & iterpair = pairtab.get(rebit.pairid, "No row with such pairid" );

                totleostab eostable(_self, _self.value);
                totaleosworth soloiter;

                soloiter = eostable.get();

                double eosworthtobuy = rebaliter.tokenpercnew * soloiter.eosworth;

                //totalbuy_tab tottbb(_self, _self.value);
                //totalbuy totiter;

                if (!tottbb.exists()) {
                    tottbb.set(totiter, _self);
                }
                else {
                    totiter = tottbb.get();
                }
                totiter.amountbuy += eosworthtobuy;
                tottbb.set(totiter, _self);

                eosworthbuytb singblebuytb(_self, _self.value);
                const auto& indrow = singblebuytb.find(rebaliter.token.code().raw() );

                if (indrow == singblebuytb.end()) {
                    singblebuytb.emplace(_self, [&](auto& item) {
                        item.eosworthbuy = eosworthtobuy;
                        item.token = rebaliter.token;
                    });
                }
                else {
                    singblebuytb.modify(indrow, _self, [&](auto& contract) { contract.eosworthbuy = eosworthtobuy; });
                }
            }
    }

    //LOOP THAT BUYS FROM DEFIBOX
    for (int i = 0; i < answers.size(); i++) {
        rebalontb rebaltablakas(get_self(), _self.value);

        const auto & rebaliter = rebaltablakas.get(answers[i].code().raw(), "No pairid for such symbol" );

        eoscaptura eoscapletb(_self, _self.value);
        eoscapt eoscapitr;

        eoscapitr = eoscapletb.get();

        if
             (rebaliter.tokenperold < rebaliter.tokenpercnew) 
            {
                eosworthbuytb solobuytb(_self, _self.value);
                const auto & solobuyitr = solobuytb.get(rebaliter.token.code().raw(), "No such solobuy" );

                totiter = tottbb.get();

                double perctobuy = solobuyitr.eosworthbuy / totiter.amountbuy;

                //check (false, perctobuy * 10000)

                //check(false,eoscapitr.capturedeos.amount);

                double tobuydoub = static_cast<double>(eoscapitr.capturedeos.amount) * perctobuy;

                struct asset tobuy = {int64_t(tobuydoub), symbol ("EOS", 4)};

                string memo = "swap,0," + rebaliter.strpairid;

                //ACTION THAT TRIGGERS BUYING
                send(_self, "swap.defi"_n, tobuy, memo, "eosio.token"_n);
                  

                    //SAVE AMOUNTS AFTER BUYING
                    //INLINE ACTION NEEDED OTHERWISE send IS EXECUTED LAST AND THUS OLD BAlANCE IS SAVED
                    adjusttokk(rebaliter.contract, rebaliter.token, rebaliter.decimals, rebaliter.tokenpercnew);
            }
    }  //END LOOP THAT BUYS FROM DEFIBOX

    rebalontb pedetb(get_self(), _self.value);
    //LOOP TO GET MIN AMOUNTS OF TOKENS TO CREATE EOSETF
    for (auto iter = pedetb.begin(); iter != pedetb.end(); iter++)
{
    const auto & rebaliter = pedetb.get(iter->token.code().raw(), "No pairid for such symbol" );

    pairs pairtab("swap.defi"_n, "swap.defi"_n.value);

    const auto & etfpair = pairtab.get(1232, "No row with such pairid" );

    //GET HOW MUCH EOS WORTH SHOULD THAT TOKEN HAVE
    double mineostokworth = iter->tokenpercnew * etfpair.price1_last;

    const auto & iterpair = pairtab.get(iter->pairid, "No row with such pairid" );

    if
         (iterpair.reserve0.symbol == iter->token) 
        {
            double mintokenamt = mineostokworth / iterpair.price0_last;

            struct asset minamount = {int64_t (rebaliter.decimals * mintokenamt), rebaliter.token};

            auto iterviis = pedetb.find( iter->token.code().raw() );

            pedetb.modify(
                iterviis, name("consortiumtt"), [&]( auto& s ) {               s.minamount    = minamount; });

            check(minamount.amount < 10000000000000, "Minamount too large");
        }

    if
         (iterpair.reserve1.symbol == iter->token) 
        {
            double mintokenamt = mineostokworth / iterpair.price1_last;

            struct asset minamount = {int64_t (rebaliter.decimals * mintokenamt), rebaliter.token};

            auto iterkuus = pedetb.find( iter->token.code().raw() );

            pedetb.modify(
                iterkuus, name("consortiumtt"), [&]( auto& s ) {               s.minamount    = minamount; });

            check(minamount.amount < 10000000000000, "Minamount too large");
        }
}
//LOOP TO GET MIN AMOUNTS CLOSED

//DELETING BASE ITER, BASE ITER IS USED TO DETERMINE THE CORRECT RATIOS WHEN CREATING EOSETF.
//EVERY REBALANCING GETS NEW BASE ITER, THE SMALLEST AMOUNT FROM ALL THE TOKENS CURRENTLY IN THE FUND.
basetoktab basetable(_self, _self.value);
basetok baseiter;

if (basetable.exists()) {
    baseiter = basetable.get();

    basetable.remove();
}

rebalontb vitttb(get_self(), _self.value);

//LOOP TO GET THE SMALLEST VALUE IN THE BASE ITERATOR
    for (auto iter = vitttb.begin(); iter != vitttb.end(); iter++)
{
    if (iter->minamount.amount > 0) {
        basetoktab basetable(_self, _self.value);
        basetok baseiter;

        if (!basetable.exists())

        {
            basetable.set(baseiter, _self);
            baseiter.base = iter->minamount.symbol;
            basetable.set(baseiter, _self);
        }

        else

        {
            baseiter = basetable.get();

            const auto & itrbase = vitttb.get(baseiter.base.code().raw(), "No token with such symbol." );

            if (itrbase.minamount.amount > iter->minamount.amount) {
                baseiter = basetable.get();
                baseiter.base = iter->minamount.symbol;
                basetable.set(baseiter, _self);
            }
        }
    }
}
//END LOOP TO GET THE SMALLEST VALUE IN THE BASE ITERATOR

//LOOP TO GET NEW RATIOS (VERY CRUCIAL COMPONENT, RATIOS ENSURE THAT CORRECT AMOUNTS ARE SENT WHEN CREATING EOSETF)
  for (int i = 0; i < answers.size(); i++)
{
    rebalontb geitb(get_self(), _self.value);

    const auto & rebaliter = geitb.get(answers[i].code().raw(), "No token with such symbol." );

    basetoktab basetable(_self, _self.value);
    basetok baseiter;

    baseiter = basetable.get();

    const auto & itrbase = geitb.get(baseiter.base.code().raw(), "No token with such symbol." );

    double ratio = static_cast<double>(rebaliter.minamount.amount) / itrbase.minamount.amount;

    auto iterseitse = geitb.find( answers[i].code().raw() );

    geitb.modify(
        iterseitse, name("consortiumtt"), [&]( auto& s ) {               s.ratio    = ratio; });
}
//LOOP TO GET NEW RATIOS CLOSED

//SET SIZE *NUBMBER OF TOKENS IS THE FUND TO ZERO. SIZE NUMBER OF TOKENS IN FUND NEEDED WHEN CREATING EOSETF.
//EOSETF IS ONLY ISSUED IF NUMBER OF DIFFERENT TOKENS YOU ARE SENDING IN EQUALS TO THE NUMBER OF TOKENS CURRENTLY IN THE FUND.
etfsizetab sizetable(_self, _self.value);
etfsize soloiter;

soloiter = sizetable.get();
soloiter.size = 0;
sizetable.set(soloiter, _self);

totiter = tottbb.get();
totiter.amountbuy = 0;
tottbb.set(totiter, _self);

//LOOP TO GET THE SIZE (NUBMBER OF TOKENS IS THE FUND)
rebalontb vasaktb(get_self(), _self.value);

 for (auto iter = vasaktb.begin(); iter != vasaktb.end(); iter++)
{
    if (iter->tokenpercnew > 0)

    {
        etfsizetab sizetable(_self, _self.value);
        etfsize soloiter;

        soloiter = sizetable.get();
        soloiter.size += 1;
        sizetable.set(soloiter, _self);
    }
}
//END LOOP TO GET THE SIZE *NUBMBER OF TOKENS IS THE FUND
}
//REBALANCE FUNCTION END

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
