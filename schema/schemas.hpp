#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/name.hpp>
#include <string>

namespace cetf_contract {

    // EOSIO-TOKEN TABLES
    struct account {
        eosio::asset balance;

        uint64_t primary_key() const { return balance.symbol.code().raw(); }
    };
    EOSIO_REFLECT(account, balance);

    struct currency_stats {
        eosio::asset supply;
        eosio::asset max_supply;
        eosio::name issuer;

        uint64_t primary_key() const { return supply.symbol.code().raw(); }
    };
    EOSIO_REFLECT(currency_stats, supply, max_supply, issuer);

    //EOSETF CREATION TABLES

    //Stores the amounts of tokens user sends in.
    struct usertokens {
        eosio::asset token;
        double ratio;

        uint64_t primary_key() const { return token.symbol.code().raw(); }
    };
    EOSIO_REFLECT(usertokens, token, ratio);

    //Stores the fee percentage that gets charged while creating or puchasing EOSETF.
    struct refundrate {
        float rate;
    };
    EOSIO_REFLECT(refundrate, rate);

    //Table stores the base token that is used to check the ratios when creation the EOSETF.
    struct basetok {
        eosio::symbol base;
    };
    EOSIO_REFLECT(basetok, base);

    /*Stores all the information in regards to token that can be included in the fund. In order for managers to add a 
        token to the fund first it has to be added to this table. Contains key information of each token in the fund. 
        */

    struct rebalon {
        double tokeninfund;

        double tokenwortheos;

        double tokenperold;

        double tokenpercnew;

        int64_t decimals;

        uint64_t pairid;

        std::string strpairid;

        eosio::symbol token;

        eosio::name contract;

        double ratio;

        eosio::asset minamount;

        std::string image;

        auto primary_key() const { return token.code().raw(); }
    };

    EOSIO_REFLECT(rebalon, tokeninfund, tokenwortheos, tokenperold, tokenpercnew, decimals, pairid, strpairid, token, contract, ratio, minamount, image);

    //Stores info on whether the creation and redemption of EOSETF is paused.
    struct pausetabla {
        bool ispaused;
    };
    EOSIO_REFLECT(pausetabla, ispaused);

    //STAKING TABLES and FEE CLAIMING TABLES

    //Person total staked table, shows how much CETF has user staked in total. It is always reset to zero.
    struct prstotstkd {
        eosio::name staker;

        eosio::asset indtotstaked;

        auto primary_key() const { return staker.value; }
    };
    EOSIO_REFLECT(prstotstkd, staker, indtotstaked);

    //Person total staked table, shows how much BOXAUJ has user staked in total. It is always reset to zero.
    struct prstotstkdbx {
        eosio::name staker;

        eosio::asset indtotstaked;

        auto primary_key() const { return staker.value; }
    };

    EOSIO_REFLECT(prstotstkdbx, staker, indtotstaked);

    //Person staked table, shows on each row separately how much CETF has user staked.
    struct perzonstaked {
        uint64_t id;

        eosio::asset staked;

        eosio::time_point_sec staketime;

        uint64_t stakeperiod;

        auto primary_key() const { return id; }
    };

    EOSIO_REFLECT(perzonstaked, id, staked, staketime, stakeperiod);

    //Person staked table, shows on each row separately how much BOXAUJ has user staked.
    struct indstkdetf {
        uint64_t id;

        eosio::asset staked;

        eosio::time_point_sec staketime;

        uint64_t stakeperiod;

        auto primary_key() const { return id; }
    };
    EOSIO_REFLECT(indstkdetf, id, staked, staketime, stakeperiod);

    //Shows in which period user staked last time.
    struct claimtime {
        eosio::name user;

        uint64_t claimperiod;

        auto primary_key() const { return user.value; }
    };
    EOSIO_REFLECT(claimtime, user, claimperiod);

    //Shows how much BOXAUJ and CETF is staked in total.
    struct totstk {
        eosio::asset totalstaked = {int64_t(00000), eosio::symbol("CETF", 4)};

        eosio::asset totstketf = {int64_t(0), eosio::symbol("BOXAUJ", 0)};
    };
    EOSIO_REFLECT(totstk, totalstaked, totstketf);

    //Shows current distribution period and when it started. Separate scope for CETF and EOSETF.
    struct divperiod {
        eosio::time_point_sec periodstart;

        uint64_t claimperiod;
    };
    EOSIO_REFLECT(divperiod, periodstart, claimperiod);

    //Shows how frequently the distribution period occurs.

    struct clmperfreq {
        int64_t periodfreq;
    };
    EOSIO_REFLECT(clmperfreq, periodfreq);

    //Shows by how much should be the accumulated fees/profit adjusted. Amount is positive if
    //fees accumulated during the period were larger than the amount claimed by CETF holders.
    struct feesadjust {
        eosio::asset adjustcrtclm = {int64_t(00000), eosio::symbol("EOSETF", 4)};
        ;
    };
    EOSIO_REFLECT(feesadjust, adjustcrtclm);

    //Shows accumulated fees in EOSETF, available to be claimed.
    struct etffees {
        eosio::asset totalfees = {int64_t(00000), eosio::symbol("EOSETF", 4)};
    };
    EOSIO_REFLECT(etffees, totalfees);

    //REBALANCING TABLES

    //Table that stores what is the EOS worth of tokens that were sold during the rebalancing. Used to calculate how much of each token should be bought.
    struct eoscapt {
        eosio::asset capturedeos = {int64_t(00000), eosio::symbol("EOS", 4)};
    };
    EOSIO_REFLECT(eoscapt, capturedeos);

    //Table stores total EOS worth of tokens to be bought. That amount is required in order to calculate percentage from eos  Why not to use eoscapt table?
    struct totalbuy {
        double amountbuy;
    };
    EOSIO_REFLECT(totalbuy, amountbuy);

    //Table stores total EOS worth of each token to be bought.
    struct eosworthbuy {
        double eosworthbuy;

        eosio::symbol token;

        auto primary_key() const { return token.code().raw(); }
    };
    EOSIO_REFLECT(eosworthbuy, eosworthbuy, token);

    //Table stores number of tokens currently included in the fund.
    struct etfsize {
        int8_t size;
    };
    EOSIO_REFLECT(etfsize, size);

    //Table stores value in EOS of all the tokens in the fund.
    struct totaleosworth {
        double eosworth;
    };
    EOSIO_REFLECT(totaleosworth, eosworth);

    //Stores number of managers for the fund.
    struct mngtab {
        uint64_t nrofmanagers;

        eosio::name community;

        auto primary_key() const { return community.value; }
    };
    EOSIO_REFLECT(mngtab, nrofmanagers, community);

    //TABLES USED IN VOTING FOR PORTFOLIO ALLOCATION

    //Table for a poll
    struct portfolios {
        uint64_t pollkey;

        eosio::name community;

        eosio::name creator;

        std::vector<uint64_t> totalvote;

        std::vector<eosio::symbol> answers;

        uint8_t nrofvoters = 0;

        uint64_t sumofallopt = 0;

        auto primary_key() const { return pollkey; }

        uint64_t by_secondary() const { return community.value; }
    };

    EOSIO_REFLECT(portfolios, pollkey, community, creator, totalvote, answers, nrofvoters, sumofallopt);

    //Stores info on who is fund manager
    struct white {
        eosio::name accounts;

        auto primary_key() const { return accounts.value; }
    };
    EOSIO_REFLECT(white, accounts);

    /* DELETE IF COMPILES portftb
 typedef eosio::multi_index<"portfolios"_n, portfolios,
  eosio::indexed_by<"bycomjus"_n, eosio::const_mem_fun<portfolios, uint64_t, &portfolios::by_secondary>>> portftb;
*/

    //TABLES OF OTHER CONTRACTS

    //Defibox
    struct token {
        eosio::name contract;
        eosio::symbol symbol;
    };
    struct pair {
        uint64_t id;

        token token0;

        token token1;

        token reserve0;

        token reserve1;

        uint64_t liquidity_token;

        double price0_last;

        double price1_last;

        uint64_t price0_cumulative_last;

        uint64_t price1_cumulative_last;

        eosio::time_point_sec block_time_last;

        auto primary_key() const { return id; }
    };
    EOSIO_REFLECT(pair, id, token0, token1, reserve0, reserve1, liquidity_token, price0_last, price1_last, price0_cumulative_last, price1_cumulative_last, block_time_last);

}  // namespace cetf_contract