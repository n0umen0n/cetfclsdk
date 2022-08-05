

//FUNCTION TO REDEEM TOKENS
void refund_tokens_back(name from, name to, asset quantity, std::string memo)
{
    check(quantity.symbol == symbol("EOSETF", 4), "Token with wrong symbol.");

    check(quantity.amount >= 10000, "Can't redeem less than 1 EOSETF");

    check(quantity.amount <= 2000000, "Can't more than 200 EOSETF");

    pauseornot();

        rebalontb reftab(get_self(), _self.value);

    for (auto iter = reftab.begin(); iter != reftab.end(); iter++) {
        //REDEEMS ONLY IF CURRENTLY IN THE FUND
        if (iter->tokenpercnew > 0)

        {
            struct asset refundasset = {int64_t((quantity.amount * iter->minamount.amount) / 10000), iter->token};
            send(to, from, refundasset, memo, iter->contract);

            //DEDUCTING FROM REBALTAB TOKENSINFUND
            rebalontb rebaltab(get_self(), _self.value);

            const auto & rebaliter = rebaltab.get(iter->token.code().raw() , "No such token in rebal table" );

            double deductfund = static_cast<double>((quantity.amount * iter->minamount.amount) / 10000) / iter->decimals;

            auto existing = rebaltab.find( iter->token.code().raw() );
            rebaltab.modify(
                existing, name("consortiumtt"), [&]( auto& s ) {
                                s.tokeninfund    -= deductfund;
                            
                });
        }
    }
}
