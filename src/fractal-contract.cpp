#include <eosio/action.hpp>
#include <eosio/eosio.hpp>
#include <eosio/name.hpp>
#include <limits>
#include <map>
#include <numeric>
#include <string>
#include <token/token.hpp>

#include "fractal-contract.hpp"

using namespace eden_fractal;
using namespace eden_fractal::errors;
namespace {

    // Some compile-time configuration
    const vector<name> admins{"dan"_n, "jseymour.gm"_n, "chkmacdonald"_n, "james.vr"_n};

    constexpr int64_t max_supply = static_cast<int64_t>(1'000'000'000e4);

    const auto defaultRewardConfig = RewardConfig{.eos_reward_amt = (int64_t)100e4, .fib_offset = 5};
    constexpr auto min_groups = size_t{2};
    constexpr auto min_group_size = size_t{5};
    constexpr auto max_group_size = size_t{6};

    constexpr std::string_view edenTransferMemo = "Eden fractal respect distribution";
    constexpr std::string_view eosTransferMemo = "Eden fractal participation $EOS reward";

    // Coefficients of 6th order poly where p is phi (ratio between adjacent fibonacci numbers)
    // xp^0 + xp^1 ...
    constexpr std::array<double, max_group_size> polyCoeffs{1, 1.618, 2.617924, 4.235801032, 6.85352607, 11.08900518};

    // Other helpers
    auto fib(auto index) -> decltype(index)
    {  //
        return (index <= 1) ? index : fib(index - 1) + fib(index - 2);
    };

}  // namespace

fractal_contract::fractal_contract(name receiver, name code, datastream<const char*> ds) : contract(receiver, code, ds) {}

void fractal_contract::setagreement(const std::string& agreement)
{
    require_admin_auth();

    AgreementSingleton singleton(default_contract_account, default_contract_account.value);
    auto record = singleton.get_or_default(Agreement{});
    check(record.versionNr != std::numeric_limits<decltype(record.versionNr)>::max(), "version nr overflow");

    record.agreement = agreement;
    record.versionNr += 1;

    singleton.set(record, get_self());
}

void fractal_contract::sign(const name& signer)
{
    require_auth(signer);

    AgreementSingleton singleton(default_contract_account, default_contract_account.value);
    check(singleton.exists(), noAgreement.data());

    SignersTable table(default_contract_account, default_contract_account.value);

    if (table.find(signer.value) == table.end()) {
        table.emplace(signer, [&](auto& row) { row.signer = signer; });
    }
    else {
        check(false, alreadySigned.data());
    }
}

void fractal_contract::unsign(const name& signer)
{
    require_auth(signer);
    SignersTable table(default_contract_account, default_contract_account.value);

    table.erase(*table.require_find(signer.value, notSigned.data()));
}

/*** Token-related ***/

void fractal_contract::create()
{
    // Anyone is allows to call this action.
    // It can only be called once.
    auto new_asset = asset{max_supply, eden_symbol};

    auto sym = new_asset.symbol;
    check(new_asset.is_valid(), "invalid supply");
    check(new_asset.amount > 0, "max-supply must be positive");

    stats statstable(get_self(), sym.code().raw());
    check(std::distance(statstable.begin(), statstable.end()) == 0, tokenAlreadyCreated);

    statstable.emplace(get_self(), [&](auto& s) {
        s.supply.symbol = new_asset.symbol;
        s.max_supply = new_asset;
        s.issuer = get_self();
    });
}

void fractal_contract::issue(const name& to, const asset& quantity, const string& memo)
{
    // Since this contract is the registered eden token "issuer", only this contract
    //   is able to issue tokens. Anyone can try calling this, but it will fail.
    check(to == get_self(), "tokens can only be issued to issuer account");
    require_auth(get_self());

    validate_symbol(quantity.symbol);
    validate_quantity(quantity);
    validate_memo(memo);

    auto sym = quantity.symbol.code();
    stats statstable(get_self(), sym.raw());
    const auto& st = statstable.get(sym.raw());
    check(quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify(st, same_payer, [&](auto& s) { s.supply += quantity; });

    add_balance(st.issuer, quantity, st.issuer);
}

void fractal_contract::retire(const asset& quantity, const string& memo)
{
    require_auth(get_self());

    validate_symbol(quantity.symbol);
    validate_quantity(quantity);
    validate_memo(memo);

    auto sym = quantity.symbol.code();
    stats statstable(get_self(), sym.raw());
    const auto& st = statstable.get(sym.raw());

    statstable.modify(st, same_payer, [&](auto& s) { s.supply -= quantity; });

    sub_balance(st.issuer, quantity);
}

void fractal_contract::transfer(const name& from, const name& to, const asset& quantity, const string& memo)
{
    check(from == get_self(), untradeable.data());
    require_auth(from);

    validate_symbol(quantity.symbol);
    validate_quantity(quantity);
    validate_memo(memo);

    check(from != to, "cannot transfer to self");
    check(is_account(to), "to account does not exist");

    require_recipient(from);
    require_recipient(to);

    auto payer = has_auth(to) ? to : from;

    sub_balance(from, quantity);
    add_balance(to, quantity, payer);
}

void fractal_contract::open(const name& owner, const symbol& symbol, const name& ram_payer)
{
    require_auth(ram_payer);

    validate_symbol(symbol);

    check(is_account(owner), "owner account does not exist");
    accounts acnts(get_self(), owner.value);
    check(acnts.find(symbol.code().raw()) == acnts.end(), "specified owner already holds a balance");

    acnts.emplace(ram_payer, [&](auto& a) { a.balance = asset{0, symbol}; });
}

void fractal_contract::close(const name& owner, const symbol& symbol)
{
    require_auth(owner);

    accounts acnts(get_self(), owner.value);
    auto it = acnts.find(symbol.code().raw());
    check(it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect.");
    check(it->balance.amount == 0, "Cannot close because the balance is not zero.");
    acnts.erase(it);
}

void fractal_contract::eosrewardamt(const asset& quantity)
{
    require_admin_auth();

    RewardConfigSingleton rewardConfigTable(default_contract_account, default_contract_account.value);
    auto record = rewardConfigTable.get_or_default(defaultRewardConfig);

    validate_quantity(quantity);
    check(quantity.symbol == eos_symbol, requiresEosToken.data());

    record.eos_reward_amt = quantity.amount;
    rewardConfigTable.set(record, get_self());
}

void fractal_contract::fiboffset(uint8_t offset)
{
    require_admin_auth();

    RewardConfigSingleton rewardConfigTable(default_contract_account, default_contract_account.value);
    auto record = rewardConfigTable.get_or_default(defaultRewardConfig);

    record.fib_offset = offset;
    rewardConfigTable.set(record, get_self());
}

void fractal_contract::submitranks(const AllRankings& ranks)
{
    // This action calculates both types of rewards: EOS rewards, and the new token rewards.
    constexpr auto eosPrecisionMult = 1e4;
    require_admin_auth();

    RewardConfigSingleton rewardConfigTable(default_contract_account, default_contract_account.value);
    auto rewardConfig = rewardConfigTable.get_or_default(defaultRewardConfig);

    std::map<name, uint8_t> accounts;

    auto numGroups = ranks.allRankings.size();
    check(numGroups >= min_groups, too_few_groups.data());

    auto coeffSum = std::accumulate(std::begin(polyCoeffs), std::end(polyCoeffs), 0);
    auto multiplier = (double)rewardConfig.eos_reward_amt / (numGroups * coeffSum);

    std::vector<int64_t> eosRewards;
    std::transform(std::begin(polyCoeffs), std::end(polyCoeffs), std::back_inserter(eosRewards), [&](const auto& c) {
        auto eosQuant = multiplier * c;
        auto finalEosQuant = static_cast<int64_t>(eosQuant * eosPrecisionMult);
        check(finalEosQuant > 0, "Total configured EOS distribution is too small to distibute any reward to rank 1s");
        return finalEosQuant;
    });

    for (const auto& rank : ranks.allRankings) {
        size_t group_size = rank.ranking.size();
        check(group_size >= min_group_size, group_too_small.data());
        check(group_size <= max_group_size, group_too_large.data());

        auto rankIndex = max_group_size - group_size;
        for (const auto& acc : rank.ranking) {
            check(is_account(acc), "account " + acc.to_string() + " DNE");
            check(0 == accounts[acc]++, "account " + acc.to_string() + " listed more than once");

            auto fibAmount = static_cast<int64_t>(fib(rankIndex + rewardConfig.fib_offset));
            auto edenAmt = static_cast<int64_t>(fibAmount * std::pow(10, eden_symbol.precision()));
            auto edenQuantity = asset{edenAmt, eden_symbol};

            // TODO: To better scale this contract, any distributions should not use require_recipient.
            //       (Otherwise contracts could fail this action)
            // Therefore,
            //   Eden tokens should be added/subbed from balances directly (without calling transfer)
            //   and EOS distribution should be stored, and then accounts can claim the EOS themselves.

            // Distribute EDEN
            issue(get_self(), edenQuantity, "Mint new Eden tokens");
            transfer(get_self(), acc, edenQuantity, edenTransferMemo.data());

            // Distribute EOS
            token::actions::transfer eosTransfer{"eosio.token"_n, {get_self(), "active"_n}};
            check(eosRewards.size() > rankIndex, "Shouldn't happen.");  // Indicates that the group is too large, but we already check for that?
            auto eosQuantity = asset{eosRewards[rankIndex], eos_symbol};
            eosTransfer.send(get_self(), acc, eosQuantity, eosTransferMemo.data());

            ++rankIndex;
        }
    }
}

void fractal_contract::sub_balance(const name& owner, const asset& value)
{
    accounts from_acnts(get_self(), owner.value);

    const auto& from = from_acnts.get(value.symbol.code().raw(), "no balance object found");
    check(from.balance.amount >= value.amount, "overdrawn balance");

    from_acnts.modify(from, owner, [&](auto& a) { a.balance -= value; });
}

void fractal_contract::add_balance(const name& owner, const asset& value, const name& ram_payer)
{
    accounts to_acnts(get_self(), owner.value);
    auto to = to_acnts.find(value.symbol.code().raw());
    if (to == to_acnts.end()) {
        to_acnts.emplace(ram_payer, [&](auto& a) { a.balance = value; });
    }
    else {
        to_acnts.modify(to, same_payer, [&](auto& a) { a.balance += value; });
    }
}

void fractal_contract::validate_symbol(const symbol& symbol)
{
    check(symbol.value == eden_symbol.value, "invalid symbol");
    check(symbol == eden_symbol, "symbol precision mismatch");
}

void fractal_contract::validate_quantity(const asset& quantity)
{
    check(quantity.is_valid(), "invalid quantity");
    check(quantity.amount > 0, "quantity must be positive");
}

void fractal_contract::validate_memo(const string& memo)
{
    check(memo.size() <= 256, "memo has more than 256 bytes");
}

void fractal_contract::require_admin_auth()
{
    bool hasAuth = std::any_of(admins.begin(), admins.end(), [](auto& admin) { return has_auth(admin); });
    check(hasAuth, requiresAdmin.data());
}

EOSIO_ACTION_DISPATCHER(eden_fractal::actions)

// clang-format off
EOSIO_ABIGEN(actions(eden_fractal::actions), 
    table("agreement"_n, eden_fractal::Agreement), 
    table("signatures"_n, eden_fractal::Signature),

    table("accounts"_n, eden_fractal::account),
    table("stat"_n, eden_fractal::currency_stats),

    table("rewardconf"_n, eden_fractal::RewardConfig),

    ricardian_clause("Fractal contract ricardian clause", eden_fractal::ricardian_clause)
)
// clang-format on
