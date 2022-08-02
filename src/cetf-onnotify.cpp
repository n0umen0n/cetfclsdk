
//SAVES HOW MUCH BOXAUJ STAKED
[[eosio::on_notify("lptoken.defi::transfer")]] void saveetfstk(name from, name to, asset quantity, std::string memo)
{
     
        // deposit,1232 seda pole vaja
        if (quantity.symbol == symbol("BOXAUJ", 0) && memo == ("deposit,1232") && to == ("consortiumtt"_n))
    {
        divperiod_def divpertb(_self, _self.value);
        divperiod divperiter;
        divperiter = divpertb.get();

        //
        //check (false, "pede");

        indstkdetftb personstktbl(_self, from.value);
        personstktbl.emplace(_self, [&](auto& s) {
            s.id = personstktbl.available_primary_key();
            ;
            s.staked = quantity;
            s.staketime = current_time_point();
            s.stakeperiod = divperiter.claimperiod;
               
        });

        totstk_def totalstktbl(_self, _self.value);
        totstk newstats;

        newstats = totalstktbl.get();
        newstats.totstketf.amount += quantity.amount;
        totalstktbl.set(newstats, _self);
    }
}

[[eosio::on_notify("eosio.token::transfer")]] void captureeos(name from, name to, asset quantity, std::string memo)
{
    if (from == "swap.defi"_n && to == _self)

    {
        eoscaptura etffeestb(_self, _self.value);
        eoscapt feeitr;

        if (!etffeestb.exists()) {
            etffeestb.set(feeitr, _self);
        }
        else {
            feeitr = etffeestb.get();
        }
        feeitr.capturedeos += quantity;
        etffeestb.set(feeitr, _self);
    }
}

//ALL ON_NOTIFY ARE USED WHEN CREATING EOSETF, TO CHECK IF CORRECT AMOUNTS AND TOKENS ARE BEING SENT IN
[[eosio::on_notify("tethertether::transfer")]] void issueetfusdt(name from, name to, asset quantity, std::string memo)
{
      if (from != "swap.defi"_n) { savetokens(from, quantity, to); }
}

[[eosio::on_notify("dappservices::transfer")]] void issueetfdapp(name from, name to, asset quantity, const string memo)

{
     
  
        //dappfund1 sends dividends, hence is being ignored.
        if (from != "thedappfund1"_n && from != "swap.defi"_n)

    {
        savetokens(from, quantity, to);
    }
}

[[eosio::on_notify("effecttokens::transfer")]] void issueetfefx(name from, name to, asset quantity, std::string memo)
{
      if (from != "swap.defi"_n) { savetokens(from, quantity, to); }
}

[[eosio::on_notify("core.ogx::transfer")]] void issueetfogx(name from, name to, asset quantity, std::string memo)
{
    if (from != "swap.defi"_n) {
        savetokens(from, quantity, to);
    }
}

[[eosio::on_notify("everipediaiq::transfer")]] void issueetfiq(name from, name to, asset quantity, std::string memo)
{
    if (from != "swap.defi"_n) {
        savetokens(from, quantity, to);
    }
}

[[eosio::on_notify("vig111111111::transfer")]] void issueetfvig(name from, name to, asset quantity, std::string memo)
{
    if (from != "swap.defi"_n) {
        savetokens(from, quantity, to);
    }
}

//SOME ADJUSTMENTS HERE BECAUSE DEFIBOX HAS REWARDS FOR SWAPPING
[[eosio::on_notify("token.defi::transfer")]] void issueetfbox(name from, name to, asset quantity, std::string memo)
{
    if (from != "swap.defi"_n && from != "mine2.defi"_n) {
        savetokens(from, quantity, to);
    }

    if (from == "mine2.defi"_n) {
        double quandoub = static_cast<double>(quantity.amount) / 1000000;

        rebalontb rebaltab(get_self(), _self.value);

        const auto & rebaliter = rebaltab.get(quantity.symbol.code().raw() , "No such token in rebal table" );

        if (rebaliter.tokeninfund != 0)

        {
            auto iterkolm = rebaltab.find(quantity.symbol.code().raw() );
            rebaltab.modify(
                iterkolm, name("consortiumtt"), [&]( auto& s ) {
                                  s.tokeninfund    += quandoub;
                            
                });
        }
    }
}

[[eosio::on_notify("dadtoken1111::transfer")]] void issueetfdad(name from, name to, asset quantity, std::string memo)
{
    if (from != "swap.defi"_n) {
        savetokens(from, quantity, to);
    }
}

[[eosio::on_notify("pizzatotoken::transfer")]] void issueetfpizza(name from, name to, asset quantity, std::string memo)
{
    if (from != "swap.defi"_n) {
        savetokens(from, quantity, to);
    }
}

[[eosio::on_notify("chexchexchex::transfer")]] void issueetfchex(name from, name to, asset quantity, std::string memo)
{
    if (from != "swap.defi"_n) {
        savetokens(from, quantity, to);
    }
}

[[eosio::on_notify("token.newdex::transfer")]] void issueetfdex(name from, name to, asset quantity, std::string memo)
{
    if (from != "swap.defi"_n) {
        savetokens(from, quantity, to);
    }
}

[[eosio::on_notify("eosiotptoken::transfer")]] void issueetftpt(name from, name to, asset quantity, std::string memo)
{
    if (from != "swap.defi"_n) {
        savetokens(from, quantity, to);
    }
}

[[eosio::on_notify("emanateoneos::transfer")]] void issueetfemt(name from, name to, asset quantity, std::string memo)
{
    if (from != "swap.defi"_n) {
        savetokens(from, quantity, to);
    }
}

[[eosio::on_notify("minedfstoken::transfer")]] void issueetfdfs(name from, name to, asset quantity, std::string memo)
{
    if (from != "swap.defi"_n) {
        savetokens(from, quantity, to);
    }
}

[[eosio::on_notify("boidcomtoken::transfer")]] void issueetfbd(name from, name to, asset quantity, std::string memo)
{
    if (from != "swap.defi"_n) {
        savetokens(from, quantity, to);
    }
}

[[eosio::on_notify("swap.pcash::transfer")]] void issueetfmln(name from, name to, asset quantity, std::string memo)
{
    if (from != "swap.defi"_n) {
        savetokens(from, quantity, to);
    }
}

[[eosio::on_notify("prospectorsg::transfer")]] void issueetfdpg(name from, name to, asset quantity, std::string memo)
{
    if (from != "swap.defi"_n) {
        savetokens(from, quantity, to);
    }
}

[[eosio::on_notify("xsovxsovxsov::transfer")]] void issueetfxv(name from, name to, asset quantity, std::string memo)
{
    if (from != "swap.defi"_n) {
        savetokens(from, quantity, to);
    }
}

[[eosio::on_notify("thezeostoken::transfer")]] void issueetzs(name from, name to, asset quantity, std::string memo)
{
    if (from != "swap.defi"_n) {
        savetokens(from, quantity, to);
    }
}

[[eosio::on_notify("dop.efi::transfer")]] void issueetfdop(name from, name to, asset quantity, std::string memo)
{
    if (from != "swap.defi"_n) {
        savetokens(from, quantity, to);
    }
}

[[eosio::on_notify("btc.ptokens::transfer")]] void issueetfbtc(name from, name to, asset quantity, std::string memo)
{
    if (from != "swap.defi"_n) {
        savetokens(from, quantity, to);
    }
}

[[eosio::on_notify("eth.ptokens::transfer")]] void issueetfeth(name from, name to, asset quantity, std::string memo)
{
    if (from != "swap.defi"_n) {
        savetokens(from, quantity, to);
    }
}
