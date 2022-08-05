
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

[[eosio::action]] void votersnulli(name community, uint64_t pollkey)

{
    require_auth("consortiumtt"_n);

    kysimustes pollstbl(_self, community.value);
    //portftb pollstbl(_self, community.value);
    auto pollsrow = pollstbl.find(pollkey);

    pollstbl.modify(pollsrow, _self, [&](auto& contract) {
        contract.nrofvoters = 0;
        contract.sumofallopt = 0;
    });

    auto& pede = pollstbl.get(pollkey, "k2ivittupede");

    for (size_t i = 0; i < pede.totalvote.size(); ++i) {
        pollstbl.modify(pollsrow, _self, [&](auto& contract) { contract.totalvote[i] = 0; });
    }
    approvedaccs whitetbl(_self, community.value);

   for (auto iter =whitetbl.begin(); iter !=whitetbl.end(); iter++)
{
    kasutajapolzs userstbl(_self, iter->accounts.value);
    const auto& usersrow = userstbl.find(pollkey);

    //
    if (usersrow != userstbl.end()) {
        userstbl.erase(usersrow);
    }
}
}