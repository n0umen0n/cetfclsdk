#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/name.hpp>
#include <eosio/singleton.hpp>
#include <string>
#include <vector>

#include "errors.hpp"
#include "schemas.hpp"

using namespace eosio;
using std::string;
using std::vector;

namespace cetf_contract {

    // Ricardian contracts live in ricardian/cetf_contract-ricardian.cpp
    extern const char* ricardian_clause;

    extern const char* setagreement_ricardian;

    // The account at which this contract is deployed
    inline constexpr auto default_contract_account = "eden.fractal"_n;

    constexpr std::string_view eden_ticker{"EDEN"};
    constexpr symbol eos_symbol{"EOS", 4};
    constexpr symbol eden_symbol{eden_ticker, 4};

    class cetf_contract : public contract {
       public:
        using eosio::contract::contract;

        /* EXAMPLE
        using AgreementSingleton = eosio::singleton<"agreement"_n, Agreement>;

        // eosio.token tables
        using accounts = eosio::multi_index<"accounts"_n, account>;
        using stats = eosio::multi_index<"stat"_n, currency_stats>;

        //
*/

        //EOSIO.TOKEN TABLES

        using accounts = eosio::multi_index<"accounts"_n, account>;
        using stats = eosio::multi_index<"stat"_n, currency_stats>;

        //EOSETF CREATION TABLES

        //Stores the amounts of tokens user sends in.
        using useritokans = eosio::multi_index<"usertokens"_n, usertokens>;

        //Stores the fee percentage that gets charged while creating or puchasing EOSETF.
        using refundratetb = eosio::multi_index<"refundrate"_n, refundrate>;

        //Table stores the base token that is used to check the ratios when creation the EOSETF.
        using basetoktab = eosio::singleton<"basetok"_n, basetok>;

        /*Stores all the information in regards to token that can be included in the fund. In order for managers to add a 
        token to the fund first it has to be added to this table. Contains key information of each token in the fund. 
        */
        using rebalontb = eosio::multi_index<"rebalon"_n, rebalon>;

        //STAKING TABLES and FEE CLAIMING TABLES

        //Person total staked table, shows how much CETF has user staked in total. It is always reset to zero.
        using perstotlskd = eosio::multi_index<"prstotstkd"_n, prstotstkd>;

        //Person total staked table, shows how much BOXAUJ has user staked in total. It is always reset to zero.
        using perstotstbx = eosio::multi_index<"prstotstkdbx"_n, prstotstkdbx>;

        //Person staked table, shows on each row separately how much CETF has user staked.
        using perzonstkd = eosio::multi_index<"persznstaked"_n, perzonstaked>;

        //Person staked table, shows on each row separately how much BOXAUJ has user staked.
        using indstkdetftb = eosio::multi_index<"indstkdetf"_n, indstkdetf>;

        //Shows in which period user staked last time.
        using claimtimetb = eosio::multi_index<"claimtime"_n, claimtime>;

        //Shows how much BOXAUJ and CETF is staked in total.
        using totstk_def = eosio::singleton<"totstk"_n, totstk>;

        //Shows current distribution period and when it started. Separate scope for CETF and EOSETF.
        using divperiod_def = eosio::singleton<"divperiod"_n, divperiod>;

        //Shows how frequently the distribution period occurs.
        using divperiodfrq_def = eosio::singleton<"clmperfreq"_n, clmperfreq>;

        //Shows by how much should be the accumulated fees/profit adjusted. Amount is positive if
        //fees accumulated during the period were larger than the amount claimed by CETF holders.
        using feesadjust_def = eosio::singleton<"feesadjust"_n, feesadjust>;

        //Shows accumulated fees in EOSETF, available to be claimed.
        using etffees_def = eosio::singleton<"etffees"_n, etffees>;

        //REBALANCING TABLES

        //Table that stores what is the EOS worth of tokens that were sold during the rebalancing. Used to calculate how much of each token should be bought.
        using eoscaptura = eosio::singleton<"eoscapt"_n, eoscapt>;

        //Table stores total EOS worth of tokens to be bought. That amount is required in order to calculate percentage from eos  Why not to use eoscapt table?
        using totalbuy_tab = eosio::singleton<"totalbuy"_n, totalbuy>;

        //Table stores total EOS worth of each token to be bought.
        using eosworthbuytb = eosio::singleton<"eosworthbuy"_n, eosworthbuy>;

        //Table stores number of tokens currently included in the fund.
        using etfsizetab = eosio::singleton<"etfsize"_n, etfsize>;

        //Table stores value in EOS of all the tokens in the fund.
        using totleostab = eosio::singleton<"totleosworth"_n, totleosworth>;

        //Stores number of managers for the fund.
        using nrofmngtab = eosio::multi_index<"mngtab"_n, mngtab>;

        //PAUSE EOSETF REDEMPTION AND CREATION TABLE
        using pausetab = eosio::singleton<"pauze"_n, pausetabla>;

        cetf_contract(name receiver, name code, datastream<const char*> ds);

        // Agreement-related actions
        void setagreement(const std::string& agreement);

        /*
        // Tester/contract interface to simplify token queries
        static asset get_supply(const symbol_code& sym_code)
        {
            stats statstable(default_contract_account, sym_code.raw());
            const auto& st = statstable.get(sym_code.raw());
            return st.supply;
        }
       */

       private:
        void sub_balance(const name& owner, const asset& value);
        void add_balance(const name& owner, const asset& value, const name& ram_payer);

        void validate_quantity(const asset& quantity);
        void validate_memo(const string& memo);
        void validate_symbol(const symbol& symbol);

        void require_admin_auth();
    };

    // clang-format off
    EOSIO_ACTIONS(cetf_contract,
                  default_contract_account,
                  action(setagreement, ricardian_contract(setagreement_ricardian)),
                 
                  
    )
    // clang-format on

}  // namespace cetf_contract